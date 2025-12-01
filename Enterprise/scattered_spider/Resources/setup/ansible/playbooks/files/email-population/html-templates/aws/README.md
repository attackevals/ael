# eml-to-html

[![build status](https://github.com/dunnkers/eml-to-html/actions/workflows/python-app.yml/badge.svg)](https://github.com/dunnkers/eml-to-html/actions/workflows/python-app.yml) [![pypi badge](https://img.shields.io/pypi/v/eml-to-html.svg?maxAge=3600)](https://pypi.org/project/eml-to-html/) [![Black](https://img.shields.io/badge/code%20style-black-000000.svg)](https://github.com/psf/black) [![Downloads](https://pepy.tech/badge/eml-to-html/month)](https://pepy.tech/project/eml-to-html) ![PyPI - Python Version](https://img.shields.io/pypi/pyversions/eml-to-html) ![PyPI - License](https://img.shields.io/pypi/l/hydra-core)

Tiny CLI tool that converts `.eml` email files to `.html` files.

## Installation

```
pip install eml-to-html
```

## Usage

```
eml-to-html [EML FILE]...
```

Feel free to pass a _glob_. For example:

```
eml-to-html some_email_file_1.eml some_email_file_2.eml
```

and

```
eml-to-html *.eml
```

are both valid calls to the command. Cheers!

✨

## Example

Running `eml-to-html` on the [`test_emails`](https://github.com/dunnkers/eml-to-html/tree/master/test_emails) folder:

```
$ eml-to-html test_emails/*.eml
🟢 Written `test_email_1.html`
🟢 Written `test_email_2.html`
```

File tree is now:

```
$ tree test_emails
test_emails
├── test_email_1.eml
├── test_email_1.html
├── test_email_2.eml
└── test_email_2.html

0 directories, 4 files
```

## About

This micro module was written by [Jeroen Overschie](https://jeroenoverschie.nl/) in 2022.
