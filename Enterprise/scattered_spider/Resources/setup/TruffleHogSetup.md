---
tags: [enterprise2025, trufflehog, gitlab]
---

# Overview

This document contains instructions for setting up a GitLab server on Linux Ubuntu to test with TruffleHog.

## TruffleHog Infrastructure Dependencies

* GitLab server requirements (Linux recommended):
  * Ubuntu (20.04 LTS, 22.04 LTS, 24.04 LTS), Debian (10, 11, 12), AlmaLinux (8, 9), CentOS 7, Amazon Linux 2, Amazon Linux 2023
  * 4 GiB RAM
* GitLab access token with `read_api` privileges

## Setting up TruffleHog

* Recommend to use [GitLab Premium AMI](https://aws.amazon.com/marketplace/pp/prodview-amk6tacbois2k) from AWS with GitLab pre-installed
  * If not able to launch new AMI for the GitLab instance, follow these [instructions](https://about.gitlab.com/install/) to install GitLab

* SSH to the GitLab instance
  * If using the `GitLab Premium AMI` setup by the Evals Infrastructure team and running into `Permission denied (publickey)`, you may need to have infra add
  your SSH key to the instance

* Follow the instructions below to configure the external URL for the GitLab server
  * Run the following to open the editor

    ```cmd
    sudo nano /etc/gitlab/gitlab.rb
    ```

  * Locate the field `external_url` and set this equal to `http://<ip of instance>`. For example:

    ```cmd
    external_url "http://10.100.0.64"
    ```

  * Save and exit

* Reconfigure GitLab

  ```shell
  sudo gitlab-ctl reconfigure
  ```

* Grab the password to authenticate to the GitLab server

  ```shell
  sudo cat /etc/gitlab/initial_root_password
  ```

* Open a browser and navigate to your GitLab URL. Use the password obtained above with user `root` to authenticate.

    | Username | Password |
    | -------- | -------- |
    | root | *initial_root_password* |

* Create a new project/repository, group, etc. and populate the repository with secrets (AWS key, SSH keys, etc.) for TruffleHog to scrape
  * Test keys and secrets can be found [here](https://github.com/trufflesecurity/test_keys)
  * To generate your own SSH keys for testing:

    ```cmd
    ssh-keygen -f /path/to/file -t rsa
    ```

* Generate an access token (Settings > Access Tokens > Add New Token) with `read_api` access to your test repository to begin testing
