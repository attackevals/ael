#!/usr/bin/python3
# ---------------------------------------------------------------------------
# send_email.py - Generate an email using an HTML body with optional attachments.

# Copyright 2025 MITRE. Approved for public release. Document number 25-2821.
# Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at

# http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.

# This project makes use of ATT&CK®
# ATT&CK Terms of Use - https://attack.mitre.org/resources/terms-of-use/

# Usage: ./send_email.py [-h] -t RECIPIENTS_STR [-f SENDER] [-s SUBJECT] [-l LOGO] [-a ATTACHMENT_PATH] [-an ATTACHMENT_NAME] smtp_server html_path


# Revision History:

# ---------------------------------------------------------------------------

import argparse
import smtplib
from smtplib import SMTPException
from email.message import EmailMessage
from email.utils import make_msgid
from email.utils import formataddr
import mimetypes

def build_email_message(sender, sender_name, recipients, subject, body, logo, attachment):
    message = EmailMessage()
    message['Subject'] = subject
    message['From'] = formataddr((sender_name, sender))
    message['To'] = recipients

    if logo:
        logo_cid = make_msgid(domain='evilcorp.com')
        message.add_alternative(body.format(logo_cid=logo_cid[1:-1]), subtype='html')
        with open(logo, 'rb') as img:
            maintype, subtype = mimetypes.guess_type(img.name)[0].split('/')
            message.get_payload()[0].add_related(img.read(), maintype=maintype, subtype=subtype, cid=logo_cid)
    else:
        message.add_alternative(body, subtype='html')

    if attachment['attachment_path']:
        with open(attachment['attachment_path'], 'rb') as to_attach:
            attachment_data = to_attach.read()
            maintype, subtype = mimetypes.guess_type(to_attach.name)[0].split('/')
            message.add_attachment(attachment_data, maintype=maintype, subtype=subtype, filename=attachment['attachment_name'])

    return message.as_string()


def main():
    parser = argparse.ArgumentParser(
        prog = 'send_email.py',
        description = 'Send an email with optional attachments to a target email address',
    )
    parser.add_argument('smtp_server', help='IP or hostname of the SMTP server')
    parser.add_argument('html_path', help='Path to an HTML file containing the email body')
    parser.add_argument('-t', '--to', required=True, dest='recipients_str', help='Comma-separated email addresses to send the email to')
    parser.add_argument('-f', '--from', dest='sender', help='Email address to send the email from')
    parser.add_argument('-fn', '--sendername', dest='sender_name', help='Name of the sender')
    parser.add_argument('-s', '--subject', help='Subject of the email')
    parser.add_argument('-l', '--logo', help='Path to a logo image to be used in email signature')
    parser.add_argument('-a', '--attachment', dest='attachment_path', help='Path to a file to be attached to the email')
    parser.add_argument('-an', '--attachment-name', dest='attachment_name', help='Name of the attachment file')

    args = parser.parse_args()

    # read email body file
    with open(args.html_path, 'r') as f:
        html = f.read()

    # parse recipients as a list
    recipients = args.recipients_str.split(',')

    # build attachment dict
    attachment = dict(attachment_path=args.attachment_path, attachment_name=args.attachment_name)

    # build MIME email message
    mime_message = build_email_message(args.sender, args.sender_name, args.recipients_str, args.subject, html, args.logo, attachment)

    try:
        # using port 2525 since AWS silently blocks all traffic to port 25 to any non-RFC1918 addresses
        smtpObj = smtplib.SMTP(args.smtp_server, 2525)
        smtpObj.sendmail(args.sender, recipients, mime_message)
        print("Successfully sent email")
    except SMTPException as e:
        print("Error: unable to send email: " + e)


if __name__ == "__main__":
    main()
