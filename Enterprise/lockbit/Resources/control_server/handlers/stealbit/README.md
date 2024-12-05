# StealBit HTTP Handler

The StealBit HTTP Handler functions as the server-side counterpart to the StealBit exfiltration tool.

The handler is configured to do the following:

- respond to heartbeat requests<sup>[3](https://yoroi.company/en/research/hunting-the-lockbit-gangs-exfiltration-infrastructures/)</sup> (HTTP `GET` requests to `/bs/portal`) with a hardcoded response of `8bef61f7-2776-4e55-9ee4-409b6061d6f1`
- handle exfiltrated file chunks (HTTP `PUT` requests to `/{file hash}`)<sup>[1](https://web.archive.org/web/20220924195948/https://acn-marketing-blog.accenture.com/wp-content/uploads/2022/05/CIFR-Exfil-Tool-Comparison-Final.pdf),[2](https://www.cybereason.com/blog/research/threat-analysis-report-inside-the-lockbit-arsenal-the-stealbit-exfiltration-tool),[3](https://yoroi.company/en/research/hunting-the-lockbit-gangs-exfiltration-infrastructures/)</sup>.
  - The `PUT` request data blob is expected to contain the following information<sup>[3](https://yoroi.company/en/research/hunting-the-lockbit-gangs-exfiltration-infrastructures/)</sup>:
    - the StealBit implant configuration ID
    - the target domain
    - the target hostname
    - the absolute path of the exfiltrated file
    - file chunk contents
  - The handler will append the file chunk contents to the destination file `files/{config ID}-{domain}-{hostname}/{adjusted file path}` within the `files` exfiltration directory.
    The `{adjusted file path}` value is the absolute path of the exfiltrated file with the `:`, `\`, and `/` characters replaced with `_` underscores.
    So if a StealBit implant with config ID `dummyID` is running on the target host `mydomain\myhost` and exfiltrates the file `C:\\Users\Public\test.txt`,
    the exfiltrated file will be saved on the C2 handler machine as `files/dummyID-mydomain-myhost/C___Users_Public_test.txt`.
  - Upon successfully handling the exfiltration chunk submission, the C2 handler will respond with `Ok`.

## Components

The handler consists of an HTTP server listener that listens on a specified address/port.

## Usage

### Building

To build the control server binary, run the following command from the `evalsC2server` directory:

```
go build -o controlServer main.go
```

### Configuration

To enable and configure the StealBit handler within the control server, provide a `stealbit` entry in your handler configuration YAML file (see example below). Adjust the host and port values as necessary.

Example:

```
stealbit:
  host: 10.0.2.11
  port: 8080
  enabled: true
```

Run the `controlServer` binary as `sudo` and monitor the output to see StealBit handler updates. Ex:

```
sudo ./controlServer -c ./config/your_handler_config.yml
```

### Testing

Unit tests for the StealBit handler are available in the `stealbit_test.go` file. To run these tests, run the following command from the `evalsC2server` directory:

```
sudo go test ./...
```

To run only a single test, navigate to the `evalsC2server\handlers\StealBit` directory and run:

```
sudo go test -run NameOfTestFunction
```

### Tasking

### Troubleshooting

## CTI References

1. [Accenture. (2022). Steal(Bit) or exfil, what does it (Ex)Matter? Comparative Analysis of Custom Exfiltration Tools)](https://web.archive.org/web/20220924195948/https://acn-marketing-blog.accenture.com/wp-content/uploads/2022/05/CIFR-Exfil-Tool-Comparison-Final.pdf)
2. [Cybereason. THREAT ANALYSIS REPORT: Inside the LockBit Arsenal - The StealBit Exfiltration Tool](https://www.cybereason.com/blog/research/threat-analysis-report-inside-the-lockbit-arsenal-the-stealbit-exfiltration-tool)
3. [Yoroi. (2021, Sep 24). Hunting the LockBit Gang's Exfiltration Infrastructures](https://yoroi.company/en/research/hunting-the-lockbit-gangs-exfiltration-infrastructures/)
4. [Cyware. (2021, Oct 5). Analyzing LockBit’s Data Exfiltration Model](https://cyware.com/news/analyzing-lockbits-data-exfiltration-model-f5950c8a)
