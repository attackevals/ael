# Tactical RMM Agent

RMMAgent is an agent written in golang for use with TacticalRMM.

Repository adopted at [commit 687e890a10d3f034f55497bbc34a626562ab9d2a](https://github.com/amidaware/rmmagent/tree/687e890a10d3f034f55497bbc34a626562ab9d2a).

* [Original Repository](https://github.com/amidaware/rmmagent)
* [Original License](https://github.com/amidaware/rmmagent/blob/develop/LICENSE.md)
* [Original README](https://github.com/amidaware/rmmagent/blob/develop/README.md)

## Features

* Remote Access
* Remote updates
* Remote command execution

## Usage

Debug the agent by executing the following in an administrator Command Prompt:

```cmd
"C:\Program Files\TacticalAgent\tacticalrmm.exe" -m rpc -log debug -logto stdout
```

## Build Instructions

Build the agent using the following command:

```cmd
GOOS=windows GOARCH=amd64 go build -ldflags "-s -w"
```

On the host where the agent is being installed create the following folder: `C:\Program Files\TacticalAgent\`

Move the agent executable to the newly created directory with the name: `tacticalrmm.exe`

Use Command Prompt to install the agent:

```cmd
tacticalrmm.exe -m install --api https://api.kilo.com  --client-id 1 --site-id 1 --agent-type server --auth c433975a448d53abedeffeefc39a233a6aaa71875148c19901249638c03dc9ac --insecure
```

###### Note

* `--clientid`: This is the client inside of the tacticalrmm dashboard (organizational)
* `--site-id`: This would be the physical location of the agent inside the dashboard (per clientid)
* `--agent-type`: This can be server or workstation and it changes the filter for showing only servers/workstations in the dashboard
* `--api`: his is the API endpoint the agent uses to interact with the server.
* `--auth`: This is a token required for the agent to authenticate to the server.
* `--insecure`: This is required unless the server is installed using a valid signed certificate. (essentially to ignore certificate errors)

## Troubleshooting

Debug the agent by executing the following in an administrator Command Prompt:

```
"C:\Program Files\TacticalAgent\tacticalrmm.exe" -m rpc -log debug -logto stdout
```

Standard log output will look similar to the following:

```cmd
time="2024-10-28T16:24:02Z" level=debug msg="{Hostname:hotel Arch: AgentID:CMggWmcARIOppYTxJXvfiMpkBoNCpqJvRpoBdAIH BaseURL:https://api.kilo.com ApiURL:api.kilo.com Token:6ca0d65ebd51c1bcbe5d29ea9ef7ac96dd0f7057 AgentPK:3 Cert: ProgramDir:C:\\Program Files\\TacticalAgent EXE:C:\\Program Files\\TacticalAgent\\tacticalrmm.exe SystemDrive:C: WinTmpDir:C:\\ProgramData\\TacticalRMM WinRunAsUserTmpDir:C:\\ProgramData\\TacticalRMM MeshInstaller:meshagent.exe MeshSystemEXE:C:\\Program Files\\Mesh Agent\\MeshAgent.exe MeshSVC:mesh agent PyBin:C:\\Program Files\\TacticalAgent\\python\\py3.11.9_amd64\\python.exe PyVer:3.11.9 PyBaseDir:C:\\Program Files\\TacticalAgent\\python PyDir:py3.11.9_amd64 NuBin:C:\\Program Files\\TacticalAgent\\bin\\nu.exe DenoBin:C:\\Program Files\\TacticalAgent\\bin\\deno.exe AgentHeader:trmm/2.8.0/windows/amd64 Headers:map[Authorization:Token 6ca0d65ebd51c1bcbe5d29ea9ef7ac96dd0f7057 Content-Type:application/json] Logger:0xc0000d4c00 Version:2.8.0 Debug:true rClient:0xc000236000 Proxy: LogTo: LogFile:<nil> Platform:windows GoArch:amd64 ServiceConfig:0xc000242000 NatsServer:wss://api.kilo.com:443 NatsProxyPath:natsws NatsProxyPort:443 NatsPingInterval:42 NatsWSCompression:true Insecure:true}\n"
time="2024-10-28T16:24:02Z" level=info msg="Agent service started"
time="2024-10-28T16:24:02Z" level=debug msg="&{Statistics:{InMsgs:0 OutMsgs:0 InBytes:0 OutBytes:0 Reconnects:0} mu:{w:{state:0 sema:0} writerSem:0 readerSem:0 readerCount:{_:{} v:0} readerWait:{_:{} v:0}} Opts:{Url: InProcessServer:<nil> Servers:[wss://api.kilo.com:443] NoRandomize:false NoEcho:false Name:CMggWmcARIOppYTxJXvfiMpkBoNCpqJvRpoBdAIH Verbose:false Pedantic:false Secure:true TLSConfig:0xc000240180 TLSCertCB:<nil> TLSHandshakeFirst:false RootCAsCB:<nil> AllowReconnect:true MaxReconnect:-1 ReconnectWait:3s CustomReconnectDelayCB:<nil> ReconnectJitter:500ms ReconnectJitterTLS:4s Timeout:2s DrainTimeout:30s FlusherTimeout:1m0s PingInterval:42s MaxPingsOut:2 ClosedCB:<nil> DisconnectedCB:<nil> DisconnectedErrCB:0x9a90a0 ConnectedCB:<nil> ReconnectedCB:0x9a8f40 DiscoveredServersCB:<nil> AsyncErrorCB:0x9a8dc0 ReconnectBufSize:-1 SubChanLen:65536 UserJWT:<nil> Nkey: SignatureCB:<nil> User:CMggWmcARIOppYTxJXvfiMpkBoNCpqJvRpoBdAIH Password:6ca0d65ebd51c1bcbe5d29ea9ef7ac96dd0f7057 Token: TokenHandler:<nil> Dialer:0xc0002321c0 CustomDialer:<nil> UseOldRequestStyle:false NoCallbacksAfterClientClose:false LameDuckModeHandler:<nil> RetryOnFailedConnect:true Compression:true ProxyPath:natsws InboxPrefix: IgnoreAuthErrorAbort:true SkipHostLookup:false} wg:{noCopy:{} state:{_:{} _:{} v:8589934592} sema:0} srvPool:[0xc0002560c0] current:0xc0002560c0 urls:map[api.kilo.com:443:{}] conn:0xc000282000 bw:0xc000256180 br:0xc000256140 fch:0xc000224420 info:{ID:NANJ2TTYWT7TZI3EPH237QHKSHCBUADLCC5UYY7EZEA5SFBDTRKECWXU Name:NANJ2TTYWT7TZI3EPH237QHKSHCBUADLCC5UYY7EZEA5SFBDTRKECWXU Proto:1 Version:2.10.17 Host:127.0.0.1 Port:9235 Headers:true AuthRequired:true TLSRequired:false TLSAvailable:false MaxPayload:67108864 CID:206 ClientIP:127.0.0.1 Nonce: Cluster: ConnectURLs:[] LameDuckMode:false} ssid:0 subsMu:{w:{state:0 sema:0} writerSem:0 readerSem:0 readerCount:{_:{} v:0} readerWait:{_:{} v:0}} subs:map[] ach:0xc000244100 pongs:[] scratch:[72 80 85 66 32 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0] status:1 statListeners:map[] initc:false err:<nil> ps:0xc00016b300 ptmr:0xc000238230 pout:0 ar:false rqch:0xc000224480 ws:true respSub: respSubPrefix: respSubLen:0 respMux:<nil> respMap:map[] respRand:<nil> filters:map[]}\n"
time="2024-10-28T16:24:02Z" level=debug msg="{Url: InProcessServer:<nil> Servers:[wss://api.kilo.com:443] NoRandomize:false NoEcho:false Name:CMggWmcARIOppYTxJXvfiMpkBoNCpqJvRpoBdAIH Verbose:false Pedantic:false Secure:true TLSConfig:0xc000240180 TLSCertCB:<nil> TLSHandshakeFirst:false RootCAsCB:<nil> AllowReconnect:true MaxReconnect:-1 ReconnectWait:3s CustomReconnectDelayCB:<nil> ReconnectJitter:500ms ReconnectJitterTLS:4s Timeout:2s DrainTimeout:30s FlusherTimeout:1m0s PingInterval:42s MaxPingsOut:2 ClosedCB:<nil> DisconnectedCB:<nil> DisconnectedErrCB:0x9a90a0 ConnectedCB:<nil> ReconnectedCB:0x9a8f40 DiscoveredServersCB:<nil> AsyncErrorCB:0x9a8dc0 ReconnectBufSize:-1 SubChanLen:65536 UserJWT:<nil> Nkey: SignatureCB:<nil> User:CMggWmcARIOppYTxJXvfiMpkBoNCpqJvRpoBdAIH Password:6ca0d65ebd51c1bcbe5d29ea9ef7ac96dd0f7057 Token: TokenHandler:<nil> Dialer:0xc0002321c0 CustomDialer:<nil> UseOldRequestStyle:false NoCallbacksAfterClientClose:false LameDuckModeHandler:<nil> RetryOnFailedConnect:true Compression:true ProxyPath:natsws InboxPrefix: IgnoreAuthErrorAbort:true SkipHostLookup:false}\n"
time="2024-10-28T16:24:02Z" level=debug msg="CheckRunner() init sleeping for 20 seconds"
time="2024-10-28T16:24:02Z" level=debug msg="AgentSvc() sleeping for 13 seconds"
```

When the agent fails to connect to the server you may receive timeouts similar to the following:

```cmd
time="2024-11-18T16:13:10Z" level=debug msg="Get \"https://api.kilo.com/api/v3/MxongqdGarnYKFtIlCzAXaKJSTwogxVynTNzqvvR/checkinterval/\": context deadline exceeded (Client.Timeout exceeded while awaiting headers)"
time="2024-11-18T16:13:10Z" level=debug msg="Checkrunner sleeping for 120"
```

When the agent cannot resolve the server the error output may look similar to the following:

```cmd
time="2024-11-18T16:50:10Z" level=debug msg="Post \"https://asdasfsafdsafsdfasdfapi.kiloasdfasdfasdfasdfsafsadfasdf.com/api/v3/software/\": dial tcp: lookup asdasfsafdsafsdfasdfapi.kiloasdfasdfasdfasdfsafsadfasdf.com: no such host"
time="2024-11-18T16:50:11Z" level=debug msg="Get \"https://asdasfsafdsafsdfasdfapi.kiloasdfasdfasdfasdfsafsadfasdf.com/api/v3/MxongqdGarnYKFtIlCzAXaKJSTwogxVynTNzqvvR/checkinterval/\": dial tcp: lookup asdasfsafdsafsdfasdfapi.kiloasdfasdfasdfasdfsafsadfasdf.com: no such host"
time="2024-11-18T16:50:11Z" level=debug msg="Checkrunner sleeping for 120"
```

## Agent Persistence Mechanism

* The agent will create several registry keys containing the server and authentication at `HKLM:Software/TacticalRMM`
* The agent will install MeshAgent alongside the TacticalRMM agent.
* MeshAgent service is created
* TacticalRMM Agent Service is created

## CTI Reporting

n/a
