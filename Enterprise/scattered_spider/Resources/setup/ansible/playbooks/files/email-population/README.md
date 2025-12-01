# Email prestaging directions

This directory contains the files needed to populate emails into an inbox.
The email generation/population code is located in [email_generation_script/send_email.py](email_generation_script/send_email.py)

## Usage
>
> `NOTE:` more detailed usage can be found at [email_generation_script/](email_generation_script/)

```
python send_email.py [MAIL_SERVER] [EMAIL_HTML_FILE] -t [EMAIL_DEST] -f [EMAIL_ORIGIN] -s [EMAIL_SUBJECT]
```

## Email Population Default Parameters
>
> `NOTE:` replace python3 with whatever version of python you have.

### Airbyte Emails

---

```
python3 ./email_generation_script/send_email.py mail.hulln.net ./html-templates/airbyte/htmls/health_check_alert.html -t tlannister@kingslanding.net -f no-reply@airbyte.com  -s "Connection Warning"
python3 ./email_generation_script/send_email.py mail.hulln.net ./html-templates/airbyte/htmls/new_user.html -t tlannister@kingslanding.net  -f no-reply@airbyte.com -s Welcome_to_Airbyte
python3 ./email_generation_script/send_email.py mail.hulln.net ./html-templates/airbyte/htmls/sync_job.html -t tlannister@kingslanding.net  -f no-reply@airbyte.com -s “Sync_Job_Failed”
```

### AWS Emails

---

```
python3 ./email_generation_script/send_email.py mail.hulln.net ./html-templates/aws/htmls/Amazon_Web_Services_Billing_Statement_Available.html -t tlannister@kingslanding.net -s "Amazon Web Services Billing Statement Available [Account 980921758675]" -f no-reply-aws@amazon.com
python3 ./email_generation_script/send_email.py mail.hulln.net ./html-templates/aws/htmls/AWS_Free_limit_alert.html -t tlannister@kingslanding.net -s "AWS Free limit alert" -f no-reply-aws@amazon.com
python3 ./email_generation_script/send_email.py mail.hulln.net ./html-templates/aws/htmls/Invitation_to_join_AWS_IAM.html -t tlannister@kingslanding.net -s "[EXT] Invitation to join AWS IAM" -f no-reply-aws@amazon.com

# Can't send yet...
# python3 send_email.py mail.hulln.net Password_updated.html -t tlannister@kingslanding.net -s "[EXT] Password updated" -f no-reply-aws@amazon.com
```
