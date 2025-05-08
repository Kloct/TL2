# TL2

TL2 is a re-implementation of the generic TL.exe that absrtacts the client->launcher communications to a pipe for the MMORPG TERA.
### Usage

`TL2.exe <exePath> <LANGUAGEEXT> <pipeID> <dllPath(optional)>`

### Parameters

- `exePath`: The path to the executable that will be launched.
- `LANGUAGEEXT`: The language extension: (ie: EUR, USA, RUS, etc.)
- `pipeID`: The ID of the pipe. TL2 will connect to `\\.pipe\tera_launcher<pipeID>`.
- `dllPath(optional)`: Indicates the path to a DLL that will be injected into the process.