﻿using Quasar.Common.Cryptography;
using Quasar.Common.Logger;
using Quasar.Common.Messages;
using System;
using System.Linq;
using System.Security.Cryptography;
using System.Security.Cryptography.X509Certificates;
using System.Text;

namespace Quasar.Server.Networking
{
    public class QuasarServer : Server
    {
        /// <summary>
        /// Gets the clients currently connected and identified to the server.
        /// </summary>
        public Client[] ConnectedClients
        {
            get { return Clients.Where(c => c != null && c.Identified).ToArray(); }
        }

        /// <summary>
        /// Occurs when a client connected.
        /// </summary>
        public event ClientConnectedEventHandler ClientConnected;

        /// <summary>
        /// Represents the method that will handle the connected client.
        /// </summary>
        /// <param name="client">The connected client.</param>
        public delegate void ClientConnectedEventHandler(Client client);

        /// <summary>
        /// Fires an event that informs subscribers that the client is connected.
        /// </summary>
        /// <param name="client">The connected client.</param>
        private void OnClientConnected(Client client)
        {
            if (ProcessingDisconnect || !Listening) return;
            var handler = ClientConnected;
            handler?.Invoke(client);
        }

        /// <summary>
        /// Occurs when a client disconnected.
        /// </summary>
        public event ClientDisconnectedEventHandler ClientDisconnected;

        /// <summary>
        /// Represents the method that will handle the disconnected client.
        /// </summary>
        /// <param name="client">The disconnected client.</param>
        public delegate void ClientDisconnectedEventHandler(Client client);

        /// <summary>
        /// Fires an event that informs subscribers that the client is disconnected.
        /// </summary>
        /// <param name="client">The disconnected client.</param>
        private void OnClientDisconnected(Client client)
        {
            if (ProcessingDisconnect || !Listening) return;
            var handler = ClientDisconnected;
            handler?.Invoke(client);
        }

        /// <summary>
        /// Constructor, initializes required objects and subscribes to events of the server.
        /// </summary>
        /// <param name="serverCertificate">The server certificate.</param>
        public QuasarServer(X509Certificate2 serverCertificate) : base(serverCertificate)
        {
            base.ClientState += OnClientState;
            base.ClientRead += OnClientRead;
        }

        /// <summary>
        /// Decides if the client connected or disconnected.
        /// </summary>
        /// <param name="server">The server the client is connected to.</param>
        /// <param name="client">The client which changed its state.</param>
        /// <param name="connected">True if the client connected, false if disconnected.</param>
        private void OnClientState(Server server, Client client, bool connected)
        {
            if (!connected)
            {
                if (client.Identified)
                {
                    OnClientDisconnected(client);
                }
            }
        }

        /// <summary>
        /// Forwards received messages from the client to the MessageHandler.
        /// </summary>
        /// <param name="server">The server the client is connected to.</param>
        /// <param name="client">The client which has received the message.</param>
        /// <param name="message">The received message.</param>
        private void OnClientRead(Server server, Client client, IMessage message)
        {
            if (!client.Identified)
            {
                if (message.GetType() == typeof (ClientIdentification))
                {
                    client.Identified = IdentifyClient(client, (ClientIdentification) message);
                    if (client.Identified)
                    {
                        Logger.Info("Identified and verified client with ID " + client.ClientId);
                        _clientMap[client.ClientId] = client; // register client in mapping
                        client.Send(new ClientIdentificationResult {Result = true}); // finish handshake
                        OnClientConnected(client);
                    }
                    else
                    {
                        // identification failed
                        Logger.Error("Failed to identify or verify client with ID " + client.ClientId);
                        client.Disconnect();
                    }
                }
                else
                {
                    client.Disconnect();
                }
                return;
            }

            MessageHandler.Process(client, message);
        }

        /// <summary>
        /// Generate Client ID string, using MD5 hash of hostname + username + useraccounttype
        /// </summary>
        public static string GenerateClientID(Client client)
        {
            string input = client.Value.PcName + client.Value.Username + client.Value.AccountType;
            using (MD5 md5 = MD5.Create())
            {
                byte[] hash = md5.ComputeHash(Encoding.ASCII.GetBytes(input));
                return BitConverter.ToString(hash).Replace("-","");
            }
        }

        private bool IdentifyClient(Client client, ClientIdentification packet)
        {
            if (packet.Id.Length != 64)
                return false;

            client.Value.Version = packet.Version;
            client.Value.OperatingSystem = packet.OperatingSystem;
            client.Value.AccountType = packet.AccountType;
            client.Value.Country = packet.Country;
            client.Value.CountryCode = packet.CountryCode;
            client.Value.Id = packet.Id;
            client.Value.Username = packet.Username;
            client.Value.PcName = packet.PcName;
            client.Value.Tag = packet.Tag;
            client.Value.ImageIndex = packet.ImageIndex;
            client.Value.EncryptionKey = packet.EncryptionKey;
            client.ClientId = GenerateClientID(client);

            // TODO: Refactor tooltip
            //if (Settings.ShowToolTip)
            //    client.Send(new GetSystemInfo());

#if !DEBUG
            try
            {
                var csp = (RSACryptoServiceProvider)ServerCertificate.PublicKey.Key;
                return csp.VerifyHash(Sha256.ComputeHash(Encoding.UTF8.GetBytes(packet.EncryptionKey)),
                    CryptoConfig.MapNameToOID("SHA256"), packet.Signature);
            }
            catch (Exception ex)
            {
                Logger.Error("Exception when identifying client: " + ex.ToString());
                return false;
            }
#else
            return true;
#endif
        }
    }
}
