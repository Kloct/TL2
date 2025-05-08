# TL2

TL2 is a re-implementation of the generic TL.exe that absrtacts the client->launcher communications to a pipe for the MMORPG TERA.
### Usage

`TL2.exe <exePath> <LANGUAGEEXT> <pipeID> <dllPath(optional)>`

### Parameters

- `exePath`: The path to the executable that will be launched.
- `LANGUAGEEXT`: The language extension: (ie: EUR, USA, RUS, etc.)
- `pipeID`: The ID of the pipe. TL2 will connect to `\\.pipe\tera_launcher<pipeID>`.
- `dllPath(optional)`: Indicates the path to a DLL that will be injected into the process.

### Usage
TL2 will send client events over the pipe and wait for a response from the launcher that it is connected to.

Basic events:
- `1` - Account Request
- `3` - Token Request
- `5` - Server List Request
- `1000` - Game Start
- `1020` - Game Exit
- `1021` - Game Error

For account request, token request, and server list request you will need to respond with the appropiate information. Account request and token request will be sent when connecting to the server in the client message `C_LOGIN_ARBITER`. The response for server list request will need to be serialized with protobuf. The structure is defined `serverlist.proto` and can be use to generate serialization code for whatever language your launcher is written in.

For more information on the launcher protocol check out [this excelent documentation](https://docs.vezel.dev/novadrop/game/launcher-client-protocol) by [vezel-dev](https://github.com/vezel-dev)

### Dependencies
protobuf (can be installed with vcpkg)
