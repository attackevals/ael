package main

import (
    "flag"
    "fmt"
    "os"
    "os/signal"
    "strings"

    "evals.mitre.org/control_server/config"
    "evals.mitre.org/control_server/handlers"
    "evals.mitre.org/control_server/handlers/handler_manager"
    "evals.mitre.org/control_server/logger"
    "evals.mitre.org/control_server/restapi"
    "evals.mitre.org/control_server/sessions"
)

var testSession1 = sessions.Session{
    GUID:          "abcdef123456",
    IPAddr:        "127.0.0.1",
    HostName:      "myHostName",
    User:          "myUserName",
    Dir:           "C:\\MyDir\\",
    PID:           "1234",
    PPID:          "4",
    SleepInterval: 60,
    Jitter:        1.5,
}

var (
    defaultRestConfigPath = "./config/restAPI_config.yml"
    defaultPayloadDir     = "../payloads"
    defaultUsingDatabase  = false
    usingDatabase         bool
    usingExistingDatabase bool
    restConfigFile        string
    handlerConfigFile     string
)

func main() {
    flag.StringVar(&restConfigFile, "rest-config", defaultRestConfigPath, "Path to the REST API config file. Default: ./config/restAPI_config.yml")
    flag.StringVar(&restConfigFile, "r", defaultRestConfigPath, "Path to the REST API config file. Default: ./config/restAPI_config.yml")
    flag.StringVar(&handlerConfigFile, "config", "", "REQUIRED. Path to the handler config file.")
    flag.StringVar(&handlerConfigFile, "c", "", "REQUIRED. Path to the handler config file.")
    flag.BoolVar(&usingDatabase, "db", defaultUsingDatabase, "Bool indicating if RestAPI will be started with the database.")
    flag.BoolVar(&usingExistingDatabase, "existing-db", defaultUsingDatabase, "Bool indicating if the server will be initialized with an existing database.")
    flag.Parse()

    if len(handlerConfigFile) == 0 {
        fmt.Fprintln(os.Stderr, "Required argument -c/--config NOT provided. Please provide a path to the handler config file using -c/--config")
        os.Exit(1)
    }

    logger.Info("Initializing REST API from config file: ", restConfigFile)
    err := config.SetRestAPIConfig(restConfigFile)
    if err != nil {
        logger.Fatal(err)
    }
    logger.Success("REST API configuration set")

    loadHandlerConfig(handlerConfigFile)
    payloadDirectories := GetPayloadDirectories()
    if payloadDirectories == nil {
        payloadDirectories = map[string]string{"": defaultPayloadDir}
    }

    logger.Info("Starting REST API server")
    restAPIaddress := config.GetRestAPIListenAddress()
    restapi.Start(restAPIaddress, payloadDirectories, usingDatabase)
    logger.Success("REST API server is listening on: ", restAPIaddress)

    logger.Info("Starting C2 handlers")
    handlers.StartHandlers()

    if usingDatabase ||  usingExistingDatabase {
        logger.Info("Initializing database")
        restapi.SetupDatabase(usingExistingDatabase)
    } else {
        logger.Info("Initializing REST API server without database")
    }

    logger.Info("Waiting for connections")

    signalChannel := make(chan os.Signal, 1)
    signal.Notify(signalChannel, os.Interrupt)
    s := <-signalChannel
    logger.Info(fmt.Sprintf("Received signal %s: stopping handlers.", s))
    handlers.StopHandlers()
    restapi.Stop()
}

// Load in the handler configuration.
func loadHandlerConfig(configFile string) {
    logger.Info("Setting C2 handler configurations from config file: ", configFile)
    err := config.HandlerConfig.SetHandlerConfig(configFile)
    if err != nil {
        logger.Fatal(err)
    }
    logger.Success("C2 Handler configuration set")
}

// Get the payload directories
func GetPayloadDirectories() map[string]string {
    logger.Info("Retrieving payload directories")
    payloadDirectories := map[string]string{}
    handlerConfigMap := config.HandlerConfig.GetHandlerConfigMapLowercase()
    if len(handlerConfigMap) == 0 {
        return nil
    }
    if len(handler_manager.AvailableHandlers) == 0 {
        logger.Fatal("No handlers available to retreive payload directories.")
        return nil
    }
    for handlerName := range handler_manager.AvailableHandlers {
        configEntry, ok := handlerConfigMap[strings.ToLower(handlerName)]
        if !ok || strings.ToLower(handlerName) == "simplefileserver" {
            continue
        }
        enabled, err := config.ConfigEntryEnabled(configEntry)
        if err != nil {
            logger.Error(fmt.Sprintf("Failed to check if handler %s is enabled: %s", handlerName, err.Error()))
            continue
        }
        if enabled {
            payloadDir, err := config.GetHandlerPayloadDir(configEntry)
            if err != nil {
                logger.Info(fmt.Sprintf("Using default payload directory for handler %s: %s", handlerName, err.Error()))
                payloadDirectories[handlerName] = defaultPayloadDir
            } else {
                logger.Info(fmt.Sprintf("Handler %s payload directory: %s", handlerName, payloadDir))
                payloadDirectories[handlerName] = payloadDir
            }
        }
    }
    return payloadDirectories
}
