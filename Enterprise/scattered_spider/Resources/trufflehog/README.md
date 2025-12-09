# TruffleHog

TruffleHog is an open source secret scanning tool used to scrape passwords, sensitive keys, and credentials from code repositories.

* [Original Repository](https://github.com/trufflesecurity/trufflehog)
* [Original License](https://github.com/trufflesecurity/trufflehog/blob/main/LICENSE)
* [Original README](https://github.com/trufflesecurity/trufflehog/blob/main/README.md)

An overview of the TruffleHog Open-Source project can be found [here](https://trufflesecurity.com/trufflehog?gclid=CjwKCAjwouexBhAuEiwAtW_Zx5IW87JNj97Ci7heFnA5ar6-DuNzT2Y5nIl9DuZ-FOUqx0Qg3vb9nxoClcEQAvD_BwE) and see further documentation [here](https://docs.trufflesecurity.com/). It has been checked out at [commit effee2a912e2a3a5b860c7595a49e5fa43158aa4](https://github.com/trufflesecurity/trufflehog/tree/effee2a912e2a3a5b860c7595a49e5fa43158aa4) and modified in the following way:

* Trimmed to include only needed sources and detectors
* Unit tests trimmed or modified to run successfully
* Trimmed TruffleHog Analyzer feature

## Features
>
> For the purpose of this evaluation, only TruffleHog's features in relation to GitLab will be listed.

* Scan archive files and repos
* Scan base64 encoded data
* Scan binaries
* Scan Gitlab Actions
* Include/Exclude filter for scanning results

## Usage

* Follow the instructions [here](../setup/TruffleHogSetup.md) to setup a GitLab server and repository for testing

* To access the help menu for TruffleHog:

  ```cmd
  trufflehog gitlab --help
  ```

  Output:

  ```text
  usage: TruffleHog gitlab --token=TOKEN [<flags>]

  Find credentials in GitLab repositories.


  Flags:
    -h, --[no-]help                Show context-sensitive help (also try
                                   --help-long and --help-man).
        --[no-]debug               Run in debug mode.
        --[no-]trace               Run in trace mode.
        --[no-]profile             Enables profiling and sets a pprof and fgprof
                                   server on :18066.
    -j, --[no-]json                Output in JSON format.
        --[no-]json-legacy         Use the pre-v3.0 JSON format. Only works with
                                   git, gitlab, and github sources.
        --[no-]github-actions      Output in GitHub Actions format.
        --concurrency=4            Number of concurrent workers.
        --[no-]no-verification     Don't verify the results.
        --[no-]only-verified       Only output verified results.
        --[no-]allow-verification-overlap
                                   Allow verification of similar credentials
                                   across detectors
        --[no-]filter-unverified   Only output first unverified result per
                                   chunk per detector if there are more than one
                                   results.
        --filter-entropy=FILTER-ENTROPY
                                   Filter unverified results with Shannon entropy.
                                   Start with 3.0.
        --config=CONFIG            Path to configuration file.
        --[no-]print-avg-detector-time
                                   Print the average time spent on each detector.
        --[no-]no-update           Don't check for updates.
        --[no-]fail                Exit with code 183 if results are found.
        --verifier=VERIFIER ...    Set custom verification endpoints.
        --[no-]custom-verifiers-only
                                   Only use custom verification endpoints.
        --archive-max-size=ARCHIVE-MAX-SIZE
                                   Maximum size of archive to scan. (Byte units
                                   eg. 512B, 2KB, 4MB)
        --archive-max-depth=ARCHIVE-MAX-DEPTH
                                   Maximum depth of archive to scan.
        --archive-timeout=ARCHIVE-TIMEOUT
                                   Maximum time to spend extracting an archive.
        --include-detectors="all"  Comma separated list of detector types to
                                   include. Protobuf name or IDs may be used,
                                   as well as ranges.
        --exclude-detectors=EXCLUDE-DETECTORS
                                   Comma separated list of detector types to
                                   exclude. Protobuf name or IDs may be used,
                                   as well as ranges. IDs defined here take
                                   precedence over the include list.
        --[no-]version             Show application version.
        --endpoint="https://gitlab.com"
                                   GitLab endpoint.
        --repo=REPO ...            GitLab repo url. You can repeat this flag.
                                   Leave empty to scan all repos accessible
                                   with provided credential. Example:
                                   https://gitlab.com/org/repo.git
        --token=TOKEN              GitLab token. Can be provided with environment
                                   variable GITLAB_TOKEN. ($GITLAB_TOKEN)
    -i, --include-paths=INCLUDE-PATHS
                                   Path to file with newline separated regexes for
                                   files to include in scan.
    -x, --exclude-paths=EXCLUDE-PATHS
                                   Path to file with newline separated regexes for
                                   files to exclude in scan.
  ```

* To run TruffleHog against a GitLab repository:

  ```cmd
  trufflehog gitlab --repo=<repository url> --token=<GitLab access token>
  ```

  Example with output:

  ```cmd
  PS C:\Users\drstrange> .\truffleHog.exe gitlab --repo=http://10.100.0.64/eviltruffle/truffletest --token=GITLAB_TOKEN_HERE 

  🐷🔑🐷  TruffleHog. Unearth your secrets. 🐷🔑🐷

  2024-10-04T14:10:22Z    info-0  trufflehog      running source  {"source_manager_worker_id": "gA7Ri", "with_units": true}
  2024-10-04T14:10:23Z    info-0  trufflehog      scanning repo   {"source_manager_worker_id": "gA7Ri", "unit": "http://10.100.0.64/eviltruffle/truffletest.git", "unit_kind": "repo", "repo": "http://10.100.0.64/eviltruffle/truffletest.git"}
  ✅ Found verified result 🐷🔑
  Detector Type: AWS
  Decoder Type: PLAIN
  Raw result: AKIAQYLPMN5HHHFPZAM2
  Message: This is an AWS canary token generated at canarytokens.org, and was not set off; learn more here: https://trufflesecurity.com/canaries
  Arn: arn:aws:iam::052310077262:user/canarytokens.com@@c20nnjzlioibnaxvt392i9ope
  Resource_type: Access key
  Account: 052310077262
  Is_canary: true
  Commit: 0914f1cbae390ca268431187ac4d7ceb470af312
  Email: drstrange
  File: creds/new_key.txt
  Line: 2
  Link: http://10.100.0.64/eviltruffle/truffletest/blob/0914f1cbae390ca268431187ac4d7ceb470af312/creds/new_key.txt#L2
  Repository: http://10.100.0.64/eviltruffle/truffletest.git
  Timestamp: 2024-09-30 14:48:35 +0000

  ✅ Found verified result 🐷🔑
  Detector Type: URI
  Decoder Type: PLAIN
  Raw result: https://admin:admin@the-internet.herokuapp.com
  Commit: 0914f1cbae390ca268431187ac4d7ceb470af312
  Email: drstrange
  File: creds/keys.txt
  Line: 3
  Link: http://10.100.0.64/eviltruffle/truffletest/blob/0914f1cbae390ca268431187ac4d7ceb470af312/creds/keys.txt#L3
  Repository: http://10.100.0.64/eviltruffle/truffletest.git
  Timestamp: 2024-09-30 14:48:35 +0000

  Found unverified result 🐷🔑❓
  Detector Type: PrivateKey
  Decoder Type: PLAIN
  Raw result: -----BEGIN OPENSSH PRIVATE KEY-----
  b3BlbnNzaC1rZXktdjEAAAAACmFlczI1Ni1jdHIAAAAGYmNyeXB0AAAAGAAAABAjNIZuun
  xgLkM8KuzfmQuRAAAAEAAAAAEAAAGXAAAAB3NzaC1yc2EAAAADAQABAAABgQDe3Al0EMPz
  utVNk5DixaYrGMK56RqUoqGBinke6SWVWmqom1lBcJWzor6HlnMRPPr7YCEsJKL4IpuVwu
  inRa5kdtNTyM7yyQTSR2xXCS0fUItNuq8pUktsH8VUggpMeew8hJv7rFA7tnIg3UXCl6iF
  OLZKbDA5aa24idpcD8b1I9/RzTOB1fu0of5xd9vgODzGw5JvHQSJ0FaA42aNBMGwrDhDB3
  sgnRNdWf6NNIh8KpXXMKJADf3klsyn6He8L2bPMp8a4wwys2YB35p5zQ0JURovsdewlOxH
  NT7eP19eVf4dCreibxUmRUaob5DEoHEk8WrxjKWIYUuLeD6AfcW6oXyRU2Yy8Vrt6SqFl5
  WAi47VMFTkDZYS/eCvG53q9UBHpCj7Qvb0vSkCZXBvBIhlw193F3PX4WvO1IXsMwvQ1D1X
  lmomsItbqM0cJyKw6LU18QWiBHvE7BqcphaoL5E08W2ATTSRIMCp6rt4rptM7KyGK8rc6W
  UYrCnWt6KlCA8AAAWQXk+lVx6bH5itIKKYmQr6cR/5xtZ2GHAxnYtvlW3xnGhU0MHv+lJ2
  uoWlT2RXE5pdMUQj7rNWAMqkwifSKZs9wBfYeo1TaFDmC3nW7yHSN3XTuO78mPIW5JyvmE
  Rj5qjsUn7fNmzECoAxnVERhwnF3KqUBEPzIAc6/7v/na9NTiiGaJPco9lvCoPWbVLN08WG
  SuyU+0x5zc3ebzuPcYqu5/c5nmiGxhALrIhjIS0OV1mtAAFhvdMjMIHOijOzSKVCC7rRk5
  kG9EMLNvOn/DUVSRHamw5gs2V3V+Zq2g5nYWfgq8aDSTB8XlIzOj1cz3HwfN6pfSNQ/3Qe
  wOQfWfTWdO+JSL8aoBN5Wg8tDbgmvmbFrINsJfFfSm0wZgcHhC7Ul4U3v4c8PoNdK9HXwi
  TKKzJ9nxLYb+vDh50cnkseu2gt0KwVpjIorxEqeK755mKPao3JmOMr6uFTQsb+g+ZNgPwl
  nRHA4Igx+zADFj3twldnKIiRpBQ5J4acur3uQ+saanBTXgul1TiFiUGT2cnz+IiCsdPovg
  TAMt868W5LmzpfH4Cy54JtaRC4/UuMnkTGbWgutVDnWj2stOAzsQ1YmhH5igUmc94mUL+W
  8vQDCKpeI8n+quDS9zxTvy4L4H5Iz7OZlh0h6N13BDvCYXKcNF/ugkfxZbu8mZsZQQzXNR
  wOrEtKoHc4AnXYNzsuHEoEyLyJxGfFRDSTLbyN9wFOS/c0k9Gjte+kQRZjBVGORE5sN6X3
  akUnTF76RhbEc+LamrwM1h5340bwosRbR8I+UrsQdFfJBEj1ZSyMRJlMkFUNi6blt7bhyx
  ea+Pm2A614nlYUBjw2KKzzn8N/0H2NpJjIptvDsbrx3BS/rKwOeJwavRrGnIlEzuAag4vx
  Zb2TPVta45uz7fQP5IBl83b0BJKI5Zv/fniUeLI78W/UsZqb64YQbfRyBzFtI1T/SsCi0B
  e0EyKMzbxtSceT1Mb8eJiVIq04Xpwez9fIUt5rSedZD8KPq8P6s0cGsR7Qmw6eXZ/dBR/a
  s5vPhfIUmQawmnwAVuWNRdQQ79jUBSn5M+ZRVVTgEG+vFyvxr/bZqOo1JCoq5BmQhLWGRJ
  Dk9TolbeFIVFrkuXkcu99a079ux7XSkON64oPzHrcsEzjPA1GPqs9CGBSO16wq/nI3zg+E
  kcOCaurc9yHJJPwduem0+8WLX3WoGNfQRKurtQze2ppy8KarEtDhDd96sKkhYaqOg3GOX8
  Yx827L4vuWSJSIqKuO2kH6kOCMUNO16piv0z/8u3CJxOGh9+4FZIop81fiFTKLhV3/gwLm
  fzFY++KIZrLfZcUjzd80NNEja69F452Eb9HrI5BurN/PznDEi9bzM598Y7beyl4/kd4R2e
  S7SW9/LOrGw5UgxtiU+kV8nPz1PdgxO4sRlnntSBEwkQBzMkLOpq2h2BuJ2TlMP/TWuwLQ
  sDkv1Yk1pD0roGmtMzbujnURGxqRJ8gUmuIot4hpfyRSssvnRQQZ3lQCQCwHiE+HJxXWf5
  c58zOMjW7o21tI8e13uUnbRoQVJM9XYqk1usPXIkYPYL9uOw3AW/Zn+cnDrsXvTK9ZxgGD
  /90b1BNwVqMlUK+QggHNwl5qD8eoXK5cDvav66te+E+V7FYFQ06w3tytRVz8SjoaiChN02
  muIjvl6G7Hoj1hObM2t/ZheN1EShS11z868hhS6Mx7GvIdtkXuvdiBYMiBLOshJQxB8Mzx
  iug9W+Di3upLf0UMC1TqADGphsIHRU7RbmHQ8Rwp7dogswmDfpRSapPt9p0D+6Ad5VBzi3
  f3BPXj76UBLMEJCrZR1P28vnAA7AyNHaLvMPlWDMG5v3V/UV+ugyFcoBAOyjiQgYST8F3e
  Hx7UPVlTK8dyvk1Z+Yw0nrfNClI=
  -----END OPENSSH PRIVATE KEY-----
  Commit: 0914f1cbae390ca268431187ac4d7ceb470af312
  Email: drstrange
  File: creds/keys.txt
  Line: 6
  Link: http://10.100.0.64/eviltruffle/truffletest/blob/0914f1cbae390ca268431187ac4d7ceb470af312/creds/keys.txt#L6
  Repository: http://10.100.0.64/eviltruffle/truffletest.git
  Timestamp: 2024-09-30 14:48:35 +0000

  2024-10-04T14:10:24Z    info-0  trufflehog      finished scanning       {"chunks": 13, "bytes": 10601, "verified_secrets": 2, "unverified_secrets": 2, "scan_duration": "2.4103732s", "trufflehog_version": "3.82.6"}
  ```

## Build Instructions

* From the TruffleHog directory, install the dependencies:

  ```cmd
  go install
  ```

* Build the binary
  * Unix:

    ```cmd
    go build -o trufflehog .
    ```

  * Windows:

    ```cmd
    go build -o trufflehog.exe .
    ```

## CTI Reporting

1. <https://www.microsoft.com/en-us/security/blog/2023/10/25/octo-tempest-crosses-boundaries-to-facilitate-extortion-encryption-and-destruction/>
1. <https://cloud.google.com/blog/topics/threat-intelligence/unc3944-sms-phishing-sim-swapping-ransomware/>
