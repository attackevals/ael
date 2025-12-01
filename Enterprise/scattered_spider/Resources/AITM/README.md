# Adversary-In-The-Middle MFA Bypass

This AITM phishing framework imitates an [Authentik](https://goauthentik.io/) SSO login page to harvest session cookies when users
provide their credentials and MFA token.

The framework's backend takes the provided credentials and MFA token and attempts to authenticate to the real Authentik server
as the victim user to obtain a valid logon session cookie for the user.
If the user provides valid credentials and MFA token, then they are provided with a fake success message that notifies them that
their login has been successfully validated and that they should close the browser for security purposes.
At this point, the adversary can import the session cookie into a web browser and bypass the real Authentik SSO login page.

The framework will also notify the victim user appropriately of any invalid credentials or other login errors.

This phishing framework emulates the AITM behavior performed by Octo Tempest<sup>[1](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a)</sup>.

## Main Features

- Golang-based web server that performs adversary-in-the-middle MFA bypass by serving a fake Authentik login page to phish user
  credentials and MFA tokens<sup>[1](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a)</sup>
  - `/if/flow/default-authentication-flow/` URI takes the user to the fake login portal
  - `/api/v3/flows/executor/default-authentication-flow` endpoint processes login attempts
  - `/` redirects to `/if/flow/default-authentication-flow/`
- Front-end components that imitate the Authentik login page and associated login forms
  - Custom javascript that sends the provided user credentials and MFA token to the `/api/v3/flows/executor/default-authentication-flow` endpoint via POST requests
- Back-end logic that takes provided user credentials and sends authentication requests to the real Authentik server
  to steal the user's session token.
  - `/api/v3/flows/executor/default-authentication-flow` endpoint receives the user creds and MFA token, passes them to the target Authentik server, and receives the victim user's session cookie and CSRF token cookie.

The below screenshots showcase the fake login portal as the user provides credentials:

<img width="1200" alt="authentikusername" src="screenshots/aitm_username.png">
<img width="1200" alt="authentikpassword" src="screenshots/aitm_password.png">
<img width="1200" alt="authentikmfa" src="screenshots/aitm_mfa.png">
<img width="1200" alt="authentiksuccess" src="screenshots/aitm_success.png">

## Usage

```bash
sudo ./aitm [-ip <IPv4 bind address>] [-port <TCP bind port>] [-authentikUrl <real Authentik server base URL>] [-certFile <path to HTTPS certificate file>] [-keyFile <path to HTTPS cert private key>] [-resourceDir <static dir path>]
```

- `-ip` specifies which IPv4 address the web server should listen on (e.g. `1.2.3.4`). Default is `0.0.0.0` for all interfaces
- `-port` specifies which TCP port the web server should listen on. Default is `8081`
- `-authentikUrl` specifies the base URL of the real Authentik server to forward requests to. Default is `http://localhost:8888`
- `-certFile` specifies the path to the HTTPS certificate file to use if using HTTPS for the phishing framework. Default is empty string
- `-keyFile` specifies the path to the HTTPS certificate private key to use if using HTTPS for the phishing framework. Default is empty string
- `-resourceDir` specifies the path to the static web server resource directory that contains the HTML, JS, and CSS files. Default is `./static`

Example:

```bash
# Default values
sudo ./aitm

# Non-default values
# listens on https://10.100.0.70:8888 using the provided cert/key and
# uses the target Authentik server listening on https://10.100.0.61:9999
sudo ./aitm -ip 10.100.0.70 -port 8888 -authentikUrl https://10.100.0.61:9999 -certFile cert.pem -keyFile private.key -resourceDir ./custom/static/path
```

Once the web server is up and running, simply wait for the victim user to enter their credentials and MFA token, and the web server logs will
display a message that looks like the following:

```text
   [INFO] 2024/10/02 11:39:20 Authentik server successfully accepted username tlannister. Advancing to password stage.
...
   [INFO] 2024/10/02 11:39:28 Authentik server successfully accepted password for user tlannister. Advancing to MFA stage
...
[SUCCESS] 2024/10/02 11:39:49 Successful login for user tlannister
   [INFO] 2025/07/14 22:30:32 Obtained CSRF token for user tlannister: h8yWiJWEfi7bhhmrWPCWK69rMWUjjKck
   [INFO] 2025/07/14 22:30:32 Obtained final session cookie for user tlannister: eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJzaWQiOiJqMW84bWVvMDIxeHd3cXNqdWk4dTRrNnNkdmcydWVibSIsImlzcyI6ImF1dGhlbnRpayIsInN1YiI6IjEwZjM4ODMzZDY5Y2UwODE1ZTg0YTdhMDNjZWMwMjUwMzRkMzUxMGVkMzUwNjRmZTIwYmVkZmUwOWUzZDBmNzQiLCJhdXRoZW50aWNhdGVkIjp0cnVlLCJhY3IiOiJnb2F1dGhlbnRpay5pby9jb3JlL2RlZmF1bHQifQ.8hh2EcY6TtHeSornXNWXzJHEbjmU4ZIiEClmvkxvcpc
[SUCCESS] 2025/07/14 22:30:32 Use the following cookie names and values to bypass login in a browser window:
        authentik_session: eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJzaWQiOiJqMW84bWVvMDIxeHd3cXNqdWk4dTRrNnNkdmcydWVibSIsImlzcyI6ImF1dGhlbnRpayIsInN1YiI6IjEwZjM4ODMzZDY5Y2UwODE1ZTg0YTdhMDNjZWMwMjUwMzRkMzUxMGVkMzUwNjRmZTIwYmVkZmUwOWUzZDBmNzQiLCJhdXRoZW50aWNhdGVkIjp0cnVlLCJhY3IiOiJnb2F1dGhlbnRpay5pby9jb3JlL2RlZmF1bHQifQ.8hh2EcY6TtHeSornXNWXzJHEbjmU4ZIiEClmvkxvcpc
        authentik_csrf: h8yWiJWEfi7bhhmrWPCWK69rMWUjjKck
```

To bypass the Authentik SSO login portal and authenticate as the victim user, follow these steps:

- Visit the legitimate Authentik login portal in a browser
- Open up the browser's developer tools, typically by right-clicking somewhere on the page and selecting "Inspect"
- Go to the storage or application tab within the developer tools
  - For FireFox, select `Storage` for FireFox
  - For Chrome, select `Application`
  - For Edge, select `Application`. You may need to click the `+` plus sign first to see the `Application` tab as an option.
- Go to the cookie options for the Authentik website.
  - You should already see pre-populated values for the `authentik_session` cookie
- Set the `authentik_session` cookie value to the value provided by the AITM server logs. Ensure the corresponding path is set to `/`
- Create a new cookie called `authentik_csrf` and set the value to the one provided by the AITM server logs. Ensure the corresponding path is set to `/`
- Revisit the Authentik login portal site, but first make sure to edit the URL in the browser to trim off everything after the first `/` (e.g. `https://auth.romeo.com:9999/if/flow/default-authentication-flow/?next=%2F` becomes `https://auth.romeo.com:9999/`).
  - This allows the proper redirect to occur and for the cookies to apply correctly

The below screenshots show how to access the developer tools and perform the cookie adjustments in FireFox:

## Configuration

The phishing framework can run in either HTTP or HTTPS mode (though not at the same time). To enable HTTPS mode, simply provide both the HTTPS certificate and associated private key using the `-certFile` and `-keyFile` command-line arguments.

<img width="750" alt="firefox_devtools1" src="screenshots/firefox_devtools1.png" />

- Access developer tools (part 1)

<img width="750" alt="firefox_devtools2" src="screenshots/firefox_devtools2.png" />

- Access developer tools (part 2)

<img width="750" alt="firefox_cookies" src="screenshots/firefox_cookies.png" />

- Set cookies

For testing purposes, you can use the following commands in a Linux machine to
generate a dummy cert:

```bash
openssl genrsa -out private.key 2048
openssl req -new -x509 -sha256 -key private.key -out server.crt -days 365
```

The phishing server can then be started as follows:

```bash
sudo ./aitm -authentikUrl https://auth.romeo.com:9999 -certFile server.crt -key private.key
```

## Alternative Binary

This framework also comes with an alternative binary defined in `cmd/alternative/alternative.go`, which will perform the same HTTP GET and POST
requests that the original phishing server will send to the Authentik server to obtain a valid session cookie. The alternative binary does not
start a web server or attempt to obtain user credentials - it only uses credentials provided via command-line arguments.

Usage:

```bash
./aitm_alternative -targetUrl <real Authentik server URL> -user <username> -password <password> -token <MFA token>
```

- `-targetUrl` specifies the base URL of the Authentik server to get a cookie from (e.g. `https://auth.romeo.com:9999`)
- `-user` specifies the username for authentication
- `-password` specifies password for authentication
- `-token` specifies the MFA token for authentication

## Build

The Golang web server can be built using the following command:

```bash
go build -o aitm cmd/main/main.go
```

The alternative step binary can be built using the following command:

```bash
go build -o aitm_alternative cmd/alternative/alternative.go
```

### Dependencies

- Linux host and elevated user to run the web server
- Golang version `1.23` or later if building on the host
- Target network requires a publicly accessible Authentik server with MFA configured for the target user

## Test 🧪

### Unit Tests

Golang unit tests can be run using the following command:

```bash
go test ./...
```

### Logs

The web server will output logs to the terminal and to the log file `logs.txt` in the current working directory.

## Developer Notes

The following table describes the project files and their purposes:
<details>
  <summary>:link: Click to expand project structure table</summary>

  | File/Directory | Description |
  | -------- | ------- |
  | `cmd/alternative/` | Contains main source file and associated unit tests for alternative binary |
  | `cmd/main/` | Contains main source file for the phishing framework web server |
  | `pkg/` | Contains source files used across the main and alternative binaries |
  | `pkg/authentik_util/` | Contains utilities related to interacting with the Authentik server |
  | `pkg/http_util/` | contains HTTP utilities |
  | `pkg/logger/` | logging utility and associated unit tests |
  | `pkg/server/` | contains main web server logic and associated unit tests |
  | `pkg/test_util/` | contains unit test utilities |
  | `static/` | contains front-end web resources (HTML, JS, CSS) |
  | `static/icons/` | contains icon resources |
  | `static/images/` | contains images resources |
  | `static/index-2024.8.2.js` | JavaScript that grabs user credentials and sends them to server back-end |
  | `static/auth.html` | fake login page |
  | `static/authentik.css` | style sheet for login UI |

</details>

## CTI

1. [Microsoft. (2023, Oct 25). Octo Tempest crosses boundaries to facilitate extortion, encryption, and destruction](https://www.microsoft.com/en-us/security/blog/2023/10/25/octo-tempest-crosses-boundaries-to-facilitate-extortion-encryption-and-destruction/)
