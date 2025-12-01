# AWS Consoler

AWS Consoler is a tool used to generate a GUI portal link by using a given users API keys.
After providing the required API keys, AWS Consoler will output a URL that can be pasted
into a browser for an interactive AWS session.

Repository adopted at [commit cbd99010c248aef0f24afcbdf9219e85d36f5213](https://github.com/NetSPI/aws_consoler/tree/cbd99010c248aef0f24afcbdf9219e85d36f5213).

* [Original Repository](https://github.com/NetSPI/aws_consoler)
* [Original License](https://github.com/NetSPI/aws_consoler/blob/master/LICENSE)
* [Original README](https://github.com/NetSPI/aws_consoler/blob/master/README.rst)

## Features

* Load credentials from the command line or from boto3 sources (envvars, profiles, IMDS)
* Coordinate communication to AWS Federation endpoint
* Select appropriate endpoint based on partition
* Load resultant URL in user's browser of choice

## Usage

Use the -h argument for the help menu:

```cmd
aws_consoler -h
```

Providing the `ACCESS KEY`, `TOKEN`, `SECRET` and `REGION` will return the URL:

```cmd
aws_consoler -a "ACCESS KEY" -t TOKEN -s SECRET -vv -r us-east-1
```

Help menu:

```cmd
usage: cli.py [-h] [-p PROFILE] [-a ACCESS_KEY_ID] [-s SECRET_ACCESS_KEY] [-t SESSION_TOKEN] [-r ROLE_ARN] [-R REGION]
              [-o] [-v] [-eS STS_ENDPOINT] [-eF FEDERATION_ENDPOINT] [-eC CONSOLE_ENDPOINT]

A tool to generate an AWS console sign-in link from API credentials using the federation endpoint.

options:
  -h, --help            show this help message and exit

Profile authentication:
  -p PROFILE, --profile PROFILE
                        The profile to use for generating the link. Uses named profiles from the AWS CLI, as well as
                        other Boto3 applications. Instructions available here: https://amzn.to/34ik2v7

Credential authentication:
  -a ACCESS_KEY_ID, --access-key-id ACCESS_KEY_ID
                        The AWS access key ID to use for authentication. Should start with 'AKIA' or 'ASIA', depending
                        on the credential type in use (permanent/temporary).
  -s SECRET_ACCESS_KEY, --secret-access-key SECRET_ACCESS_KEY
                        The AWS secret access key to use for authentication.
  -t SESSION_TOKEN, --session-token SESSION_TOKEN
                        The AWS session token to use for authentication. Generally required when using temporary
                        credentials.

General arguments:
  -r ROLE_ARN, --role-arn ROLE_ARN
                        The role to assume for console access, if needed.
  -R REGION, --region REGION
                        The AWS region you'd like the console link to refer to. If using -p, overrides the default
                        region of the profile.
  -o, --open            Open the generated link in your system's default browser.
  -v, --verbose         Verbosity, repeat for more verbose output (up to 3)

Advanced arguments:
  -eS STS_ENDPOINT, --sts-endpoint STS_ENDPOINT
                        [advanced] The endpoint for connecting to STS, if connecting from behind a corporate proxy or
                        an unknown partition. Expects a URL with a trailing slash. Overrides the URL based on -R.
  -eF FEDERATION_ENDPOINT, --federation-endpoint FEDERATION_ENDPOINT
                        [advanced] The endpoint for console federation, if connecting from behind a corporate proxy or
                        an unknown partition. Expects a URL to send federation requests to.
  -eC CONSOLE_ENDPOINT, --console-endpoint CONSOLE_ENDPOINT
                        [advanced] The URL for console access, if connectingfrom behind a corporate proxy or an
                        unknown partition. Expects a URL to forward the user to after obtaining their federation
                        token.
```

## Build Instructions

Install python requirements:

```cmd
pip install -r requirements.txt
```

## CTI Reporting

[Not a SIMulation: CrowdStrike Investigations Reveal Intrusion Campaign Targeting Telco and BPO Companies](https://www.crowdstrike.com/en-us/blog/analysis-of-intrusion-campaign-targeting-telecom-and-bpo-companies/)
