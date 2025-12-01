---
tags: [enterprise2025, jecretz, wekan]
---

# Jecretz Setup

- [Installing Components](#installing-components)
  - [Wekan v7.61](#wekan-v761)
  - [Node.js 14.21.4](#nodejs-14214)
  - [MongoDB 6.0.18](#mongodb-6018)
- [Configure the server](#configure-the-server)
- [Running the server](#running-the-server)
- [Adding data](#adding-data)
  - [Users](#users)
  - [Tickets](#tickets)

## Installing Components

### Wekan v7.61

- Link to the installation doc for [Wekan](https://wekan.fi/install/) (version 7.61 was used for testing).
  - [Wekan's Windows On-Premise](https://github.com/wekan/wekan/blob/main/docs/Platforms/Propietary/Windows/Offline.md) guide contains instructions to download and install Wekan and its dependencies. The node version in this documentation may need adjustments.
- Download the [Wekan installation zip](https://github.com/wekan/wekan/releases/download/v7.61/wekan-7.61-amd64-windows.zip).

### Node.js 14.21.4

Wekan offline setup doc contains [this link to node.exe](https://nodejs.org/dist/latest-v14.x/win-x64/node.exe).

- [v14.21.3](https://nodejs.org/dist/v14.21.3/node-v14.21.3-x64.msi) was used for testing purposes.
- If another version is needed, use [this release](https://github.com/wekan/node-v14-esm/releases/tag/v14.21.4) as a fallback.

### MongoDB 6.0.18

Install MongoDB directly from [their website](https://www.mongodb.com/try/download/community).

- For reference, Wekan provides [this link to MongoDB 6.0.18](https://fastdl.mongodb.org/windows/mongodb-windows-x86_64-6.0.18-signed.msi)

## Configure the server

Download Wekan's [`start-wekan.bat`](https://github.com/wekan/wekan/blob/main/start-wekan.bat) script.
Wekan suggests extracting the Wekan install zip, and then placing `start-wekan.bat` and `node.exe` within the `bundle` directory alongside `main.js`:

```text
bundle (directory)
  |_ start-wekan.bat (downloaded file)
  |_ node.exe (downloaded file)
  |_ main.js (extracted file)
```

Afterwards, modify `start-wekan.bat` and update the `ROOT_URL` variable, as well as the `PORT` and `MONGO_URL` variables if needed.

## Running the server

Run `start-wekan.bat` to start the server.

## Adding data

Wekan provides a [python script](https://github.com/wekan/wekan/blob/db6ebe04701f0a1ec3e416eee0084705d7477e2a/api.py) for interacting with its [REST API](https://github.com/wekan/wekan/blob/main/docs/API/New-card-with-Python3-and-REST-API.md).

Prior to using the script, update the `username`, `password` and `wekanurl` variables in lines 79-83.

### Users

After the admin user is set up and the API script is configured, add new users using the `api.py` script:

```bash
python3 api.py newuser USERNAME EMAIL PASSWORD
```

### Tickets

After the admin user is set up and the API script is configured, create boards and swimlanes.

To create lists using the API script:

```bash
python3 api.py createlist BOARDID LISTTITLE
```

To create cards using the API script:

```bash
python3 api.py addcard AUTHORID BOARDID SWIMLANEID LISTID CARDTITLE CARDDESCRIPTION
```
