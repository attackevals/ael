# Email Generation

Send emails with attachments from an arbitrary address using the send_email.py Python script. This
script depends on an already configured SMTP server.

# Usage

```
usage: send_email.py [-h] -t RECIPIENTS_STR [-f SENDER] [-s SUBJECT] [-l LOGO] [-a ATTACHMENT_PATH] [-an ATTACHMENT_NAME] smtp_server html_path

Send an email with optional attachments to a target email address

positional arguments:
  smtp_server           IP or hostname of the SMTP server
  html_path             Path to an HTML file containing the email body

options:
  -h, --help            show this help message and exit
  -t RECIPIENTS_STR, --to RECIPIENTS_STR
                        Comma-separated email addresses to send the email to
  -f SENDER, --from SENDER
                        Email address to send the email from, default: dev@redteam.local
  -s SUBJECT, --subject SUBJECT
                        Subject of the email, default: Red Team Dev Test Email
  -l LOGO, --logo LOGO  Path to a logo image to be used in email signature, default: evals.png
  -a ATTACHMENT_PATH, --attachment ATTACHMENT_PATH
                        Path to a file to be attached to the email
  -an ATTACHMENT_NAME, --attachment-name ATTACHMENT_NAME
                        Name of the attachment file, default: Attachment_Test
```

Example usage:

```
python3 send_email.py mail.evilcorp.com evals_test.html -t mchan@reddev.local -a evals_png.zip -an evals.zip
```

* Use SMTP server `mail.evilcorp.com`
* Use email body `evals_test.html`

  ```
  <html>
    <body>
        <p style="margin:0">Good morning,<br><br>
            Please open the attached ZIP archive and enter password: P@ssw0rd<br><br>

            Then click <a href="https://attackevals.mitre-engenuity.org">here</a> to browse to our website.

            Please let us know if you have any questions or concerns!<br><br>

            V/R,<br><br>

            The ATT&CK Evaluations Red Development Team
            <p style="color:#6241c5; margin:0; font-size:12px"><b>MITRE Engenuity ATT&CK Evaluations</b><br>
            <img style="width:300px; height:32px" src="cid:{logo_cid}" alt="Evaluations Logo"></span>
        </p>
    </body>
  </html>
  ```

* Send email to `mchan@reddev.local`
* Add attachment `evals_png.zip` (this contains evals.png that's been zipped and encrypted with the password `P@ssw0rd`)
* Name the attachment `evals.zip`
* Use default `evals.png` logo
* Use default email subject `Red Team Dev Test Email`

Result:
![Example](example/example.png)

# Requirements

```
pip install argparse smtplib email
```
