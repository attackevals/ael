package main

import (
    "flag"
    "fmt"
    "os"
    "os/signal"

    "aitm/pkg/logger"
    "aitm/pkg/server"
)

var (
    defaultBindIP = "0.0.0.0"
    defaultBindPort = 8081
    defaultResourceDir = "./static"
    defaultAuthentikUrl = "http://localhost:8888"
)

func main() {
    bindIP := flag.String("ip", defaultBindIP, "IPv4 address to listen on.")
    bindPort := flag.Int("port", defaultBindPort, "TCP port to listen on.")
    authentikUrl := flag.String("authentikUrl", defaultAuthentikUrl, "Base URL for target authentik server to perform authentication against and fetch the auth token. E.g. https://auth.romeo.com:9999")
    resourceDir := flag.String("resourceDir", defaultResourceDir, "Path to directory containing static web resources.")
    certFile := flag.String("certFile", "", "Path to HTTPS certificate file if listening on HTTPS")
    keyFile := flag.String("keyFile", "", "Path to HTTPS certificate private key if listening on HTTPS")
    flag.Parse()

    httpServer := server.AITMServerFactory(fmt.Sprintf("%s:%d", *bindIP, *bindPort), *resourceDir, *authentikUrl, *certFile, *keyFile)
    err := httpServer.StartServer()
    if err != nil {
        logger.Error(fmt.Sprintf("Failed to start AITM server: %s", err.Error()))
    }

    // Wait for interrupt
    signalChannel := make(chan os.Signal, 1)
    signal.Notify(signalChannel, os.Interrupt)
    s := <-signalChannel
    logger.Info(fmt.Sprintf("Received signal %s: stopping HTTP server.", s))
    err = httpServer.StopServer()
    if err != nil {
        logger.Error(fmt.Sprintf("Failed to stop AITM server: %s", err.Error()))
    } else {
        logger.Info("Stopped AITM server. Goodbye.")
    }
}
