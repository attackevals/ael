# Email Generation

Send emails with attachments from an arbritrary address using the send_email.py Python script. This
script depends on an already configured SMTP server.

# Usage

```
usage: send_email.py [-h] -t RECIPIENTS_STR [-f SENDER] [-fn SENDER_NAME] [-s SUBJECT] [-l LOGO] [-a ATTACHMENT_PATH] [-an ATTACHMENT_NAME] smtp_server html_path

Send an email with optional attachments to a target email address

positional arguments:
  smtp_server           IP or hostname of the SMTP server, port is hardcoded to 2525
  html_path             Path to an HTML file containing the email body

options:
  -h, --help            show this help message and exit
  -t RECIPIENTS_STR, --to RECIPIENTS_STR
                        Comma-separated email addresses to send the email to
  -f SENDER, --from SENDER
                        Email address to send the email from
  -fn SENDER_NAME, --sendername SENDER_NAME
                        Name of the sender
  -s SUBJECT, --subject SUBJECT
                        Subject of the email
  -l LOGO, --logo LOGO  Path to a logo image to be used in email signature
  -a ATTACHMENT_PATH, --attachment ATTACHMENT_PATH
                        Path to a file to be attached to the email
  -an ATTACHMENT_NAME, --attachment-name ATTACHMENT_NAME
                        Name of the attachment file
```

Example usage:

```
python3 send_email.py mail.evilcorp.com evals_test.html -t victim@acme.org -f dev@redteamlocal -fn 'Red Development' -s 'Red Team Dev Test Email' -a evals_png.zip -an evals.zip -l evals.png
```

# Requirements

```
pip install argparse smtplib email
```
