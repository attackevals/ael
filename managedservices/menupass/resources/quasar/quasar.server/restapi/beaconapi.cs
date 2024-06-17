using Quasar.Common.Logger;
using Quasar.Server.Networking;
using Quasar.Server.RestApiConstants;
using System.Collections.Generic;
using System.Net;
using System.Text;
using System.Text.Json;

namespace Quasar.Server.RestApi
{
    public static partial class RestServer
    {
        private static void HandleBeaconEndpointGet(HttpListenerRequest request, HttpListenerResponse response)
        {
            if (request.HttpMethod != "GET")
            {
                Logger.Error("Expected GET request for beacon endpoint, received " + request.HttpMethod);
                SendResponse(response, HttpStatusCode.MethodNotAllowed, "Unsupported HTTP method " + request.HttpMethod);
                return;
            }
            Logger.Debug("Received beacon information request.");
            SendSuccessResponse(response, GetSerializedClientList());
            Logger.Debug("Sent beacon information.");
        }

        /// <summary>
        /// Generate byte representation of JSON dictionary list containing relevant client information for each client in the provided list. 
        /// </summary>
        private static byte[] GetSerializedClientList()
        {
            var clientDictList = new List<Dictionary<string, string>>();
            foreach (Client c in mainForm.GetConnectedClients())
            {
                clientDictList.Add(GenerateClientDictionary(c));
            }
            string clientDictStr = JsonSerializer.Serialize(clientDictList);
            return Encoding.UTF8.GetBytes(clientDictStr);
        }

        /// <summary>
        /// Returns dictionary containing relevant client information for API requests.
        /// </summary>
        /// <returns>A dictionary representing client information.</returns>
        public static Dictionary<string, string> GenerateClientDictionary(Client client)
        {
            return new Dictionary<string, string>
            {
                [RestConstants.CLIENT_ID_FIELD] = client.ClientId, 
                [RestConstants.CLIENT_USERNAME_FIELD] = client.Value.Username,
                [RestConstants.CLIENT_HOSTNAME_FIELD] = client.Value.PcName,
                [RestConstants.CLIENT_IP_ADDR_FIELD] = client.EndPoint.Address.ToString(),
                [RestConstants.CLIENT_OS_FIELD] = client.Value.OperatingSystem,
                [RestConstants.CLIENT_ACCOUNT_TYPE_FIELD] = client.Value.AccountType,
                [RestConstants.CLIENT_TAG_FIELD] = client.Value.Tag,
                [RestConstants.CLIENT_COUNTRY_FIELD] = client.Value.CountryCode,
                [RestConstants.CLIENT_HARDWARE_ID_FIELD] = client.Value.Id
            };
        }
    }
}