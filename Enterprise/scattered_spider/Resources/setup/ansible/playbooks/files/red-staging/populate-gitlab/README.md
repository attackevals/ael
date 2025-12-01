# GitLab Repository Creation and Population

Script for creating new repository (if one does not already exist) and populating it with necessary files for ER7 infrastructure setup.

## Requirements

- python
- `python-gitlab` python library
- GitLab repository URL, Group ID, new repository name, and private access token

## Installation

Clone the repository:

```bash
git clone <link to repository>
```

Ensure the SSH key file path in the script is correct:

```
ssh_key_path = ...
```

## Quickstart

Argument breakdown:

| Arg Name | Required | Default Value | Description |
| -------- | -------- | ------------- | ----------- |
| url | True | N/A | The URL to the target GitLab Organization |
| user | True | N/A | The GitLab username associated with the private access token |
| repoName | False | airbyte-config-storage | The name of the repository to be created and/or populated |
| repoGroup | False | N/A | The group ID of the group that the project is in. If none provided, repository will be created without a group (personal user project). |
| token | True | N/A | The GitLab private access token of any user with access to the GitLab. |

From a terminal window navigate to the `er7-infra/utils/populate-gitlab` directory and then run the script:

```bash
cd ./er7-infra/utils/populate-gitlab
python populate-gitlab.py --url '<url>' --user '<user>' --repoName '<repository name>' --repoGroup 1 --token '<private access token>'
```
