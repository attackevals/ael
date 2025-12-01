
# Jecretz

Jira secrets hunter by sahadnk72<sup>[1](https://github.com/sahadnk72/jecretz)</sup>, rewritten for Wekan<sup>[4](https://github.com/wekan/wekan)</sup>.

## Main Features

- Connects to Wekan API with provided credentials to obtain legitimate API key.<sup>[1](https://github.com/sahadnk72/jecretz)</sup>
- Collects description and custom fields of all cards available to given user.<sup>[1](https://github.com/sahadnk72/jecretz)</sup>
- Uses regex to attempt to locate secrets within retrieved data.<sup>[1](https://github.com/sahadnk72/jecretz)</sup>
  - Custom regex rules are based on [jecretz'](https://github.com/sahadnk72/jecretz/rules.py)
  - Regex matching code is based on [jecretz'](https://github.com/sahadnk72/jecretz/blob/386e37e24d07e24df5c05730275ba3b0a638ad17/jecretz.py#L90)

## Usage

- Command line usage and options

```text
python jecretz.py [Wekan URL] [Username] [Password]
python jecretz.py -v [Wekan URL] [Username] [Password]
```

- `-v`: Enable verbose output
- `[Wekan URL]`: The URL of the Wekan site, including the trailing slash. Ex: `http://localhost/`
- `[Username]`: The username to authenticate to Wekan with.
- `[Password]`: The password of the given user.

## Dependencies

Dependencies are included in [`requirements.txt`](./requirements.txt) and can be installed by running:

```text
pip install -r requirements.txt
```

- certifi==2024.8.30
- charset-normalizer==3.3.2
- idna==3.8
- requests==2.32.3
- truffleHogRegexes==0.0.7
- urllib3==2.2.2

## CTI

1. [sahadnk72. (2020, Jun 11). jecretz](https://github.com/sahadnk72/jecretz)
2. [Microsoft. (2023, Oct 25). Octo Tempest crosses boundaries to facilitate extortion, encryption, and destruction](https://www.microsoft.com/en-us/security/blog/2023/10/25/octo-tempest-crosses-boundaries-to-facilitate-extortion-encryption-and-destruction/)
3. [Mandiant. (2023, Sept 14). Why Are You Texting Me? UNC3944 Leverages SMS Phishing Campaigns for SIM Swapping, Ransomware, Extortion, and Notoriety](https://cloud.google.com/blog/topics/threat-intelligence/unc3944-sms-phishing-sim-swapping-ransomware/)

## Additional References

4. [WeKan ® Open-Source Kanban](https://github.com/wekan/wekan)
