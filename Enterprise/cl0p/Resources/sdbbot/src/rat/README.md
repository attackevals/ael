# Rat

## Packet Structure

### Tasking Packet

```json
{
"header":"0xC0DE0000",   // string
"command":(Command Value),    // int
"Other/requirements":"(file paths, target executables, etc.)"    // string
}
```

### Task Create/Download file FROM C2

```json
{
"header":"0xC0DE0000",   // string
"command":15, // int
"filePath":"path/to/file/destination", // string
"fileContents":"Base64 encoded file content",  // string
}
```

### Task Upload File TO C2

```json
{
"header":"0xC0DE0000",   // string
"command":24, // int
"filePath":"path/to/file/destination", // string
}
```

### Task Delete File From Disk

```json
{
"header":"0xC0DE0000",   // string
"command":26, // int
"filePath":"path/to/file/destination", // string
}
```

### Task Execute a Command

```json
{
"header":"0xC0DE0000",   // string
"command":2,  // int
"execute":"command to execute",   // string

}
```

### Implant Response Packet

```json
{
"header":"0xC0DE0000",   // string
"command":"(Command Value)",    // string
"response":"(command output, file data, etc.)" // string
}
```

### Implant Recon Packet

```json
{
"header":"0xC0DE0000",   // string
"command":"42"  //string
"ver":"2.0",  // string
"domain":"Domain name",  // string
"pc":"Computer name",    // string
"geo":"two letter country code",  // string
"os":"OS Build info", // string
"rights":"admin"/"user", // string
"proxyenabled":0,  // boolean
}
```

### Packet Encryption

Encryption can be toggled on/off using the CmakePresets setting `RAT_ENCRYPT_COMMS`. The key used
for encryption is located at `Settings::key` in `settings.cpp`.
Encryption consists of:

* XORing the payload data (string)
* Then Base64 encoding that string

## Logging/Troubleshooting

SDBBot creates a log file in the image execution directory. The log file name is `mswinsdr64.log`.

### Decoding the log file

The log file is base64 encoded and XOR encrypted. The log file decryptor can be found here: [log_decryptor](../../../log_decryptor/)

The XOR key used to decrypt the log file is `0x0F, 0x00, 0x00, 0x0D`:

```bash
python3 aes_base64_log_decryptor.py -i mswinsdr64.log -o dec_mswinsdr64.log -k 0F00000D --xor
```

View the contents of the decrypted log file:

```bash
cat dec_mswinsdr64.log
```

## Integration Test

Placeholder; TCP integration test located at `%PROJECT_ROOT%/tests/scripts/integration_test.ps1`
