using Quasar.Common.Messages;
using Quasar.Server.Messages;
using Quasar.Server.Networking;
using Quasar.Server.RestApiConstants;
using System.Collections.Generic;
using System;
using System.Text.Json;

namespace Quasar.Server.RestApi
{
    public static partial class RestServer
    {
        private static readonly Dictionary<string, PortScanHandler> clientPortScanManagerHandlers = new Dictionary<string, PortScanHandler>();

        /// <summary>
        /// Creates a new PortScanHandler for the client or gets the current one, if one exists already.
        /// </summary>
        /// <param name="clientId">The ID for the client used for the PortScanHandler.</param>
        /// <returns>
        /// Returns a new PortScanHandler for the client if there is none currently available, otherwise creates a new one.
        /// </returns>
        public static PortScanHandler CreateNewOrGetExistingPortScanManagerHandler(string clientId)
        {
            if (clientPortScanManagerHandlers.ContainsKey(clientId))
            {
                return clientPortScanManagerHandlers[clientId];
            }
            Client client = mainForm.LookupClientById(clientId);
            if (client == null)
            {
                throw new Exception("Could not find connected client by ID " + clientId);
            }
            PortScanHandler handler = new PortScanHandler(client, viaRestApi: true);
            MessageHandler.Register(handler);
            clientPortScanManagerHandlers[clientId] = handler;
            return handler;
        }

        public static PortScanApiRequest GetPortScanApiRequestFromPostDict(Dictionary<string, JsonElement> dataDict)
        {
            PortScanApiRequest ret = new PortScanApiRequest();

            // Client ID
            if (!dataDict.ContainsKey(PortScanApiRequest.CLIENT_ID_FIELD_NAME))
            {
                throw new JsonException(String.Format("Port scan task POST request did not contain required key \"{0}\"", PortScanApiRequest.CLIENT_ID_FIELD_NAME));
            }
            else
            {
                ret.ClientId = dataDict[PortScanApiRequest.CLIENT_ID_FIELD_NAME].ToString();
            }

            // Target range
            if (!dataDict.ContainsKey(PortScanApiRequest.TARGET_RANGE_FIELD_NAME))
            {
                throw new JsonException(String.Format("Port scan task POST request did not contain required key \"{0}\"", PortScanApiRequest.TARGET_RANGE_FIELD_NAME));
            }
            else
            {
                ret.TargetRange = dataDict[PortScanApiRequest.TARGET_RANGE_FIELD_NAME].ToString();
            }

            // Target ports
            if (!dataDict.ContainsKey(PortScanApiRequest.TARGET_PORTS_FIELD_NAME))
            {
                throw new JsonException(String.Format("Port scan task POST request did not contain required key \"{0}\"", PortScanApiRequest.TARGET_PORTS_FIELD_NAME));
            }
            else
            {
                int numPorts = dataDict[PortScanApiRequest.TARGET_PORTS_FIELD_NAME].GetArrayLength();
                if (numPorts == 0)
                {
                    ret.TargetPorts = new int[0];
                }
                else
                {
                    var portEnumerator = dataDict[PortScanApiRequest.TARGET_PORTS_FIELD_NAME].EnumerateArray();
                    var portList = new List<int>();
                    foreach (JsonElement portObj in portEnumerator)
                    {
                        portList.Add(portObj.GetInt32());
                    }
                    ret.TargetPorts = portList.ToArray();
                }
            }

            return ret;
        }
    }
}
