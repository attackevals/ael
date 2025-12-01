# Wekan Info

Note: Do not add single quotes '' to variables. Having spaces still works without quotes where required.

<br>

## CREATING USERS AND LOGGING IN TO WEKAN
>
> <https://github.com/wekan/wekan/wiki/Adding-users>

<br>

## FORGOT PASSWORD
>
> <https://github.com/wekan/wekan/wiki/Forgot-Password>

<br>

## Upgrading Wekan to new version

_**NOTE**: MongoDB has changed from 3.x to 4.x, in that case you need backup/restore with_ `--noIndexRestore`<br>
see <https://github.com/wekan/wekan/wiki/Backup> <br>

1) Stop Wekan:

    > docker compose stop

2) Remove old Wekan app (wekan-app only, not that wekan-db container that has all your data)

    > docker rm wekan-app

3) Get newest docker-compose.yml from <https://github.com/wekan/wekan> to have correct image,

    > for example: "image: quay.io/wekan/wekan" or version tag "image: quay.io/wekan/wekan:v4.52"

4) Start Wekan:

    > docker compose up -d

<br>

## OPTIONAL: DEDICATED DOCKER USER

1) Optionally create a dedicated user for Wekan, for example:

    > `sudo useradd -d /home/wekan -m -s /bin/bash wekan`

2) Add this user to the docker group, then logout+login or reboot:

    > `sudo usermod -aG docker wekan`

3) Then login as user wekan.
4) Create this file /home/wekan/docker-compose.yml with your modifications.

<br>

## RUN DOCKER AS SERVICE

1) Running Docker as service, on Systemd like Debian 9, Ubuntu 16.04, CentOS 7:

    > `sudo systemctl enable docker`
    > `sudo systemctl start docker`

2) Running Docker as service, on init.d like Debian 8, Ubuntu 14.04, CentOS 6:

    > `sudo update-rc.d docker defaults`
    > `sudo service docker start`

<br>

## USAGE OF THIS `docker-compose.yml`

1) For seeing does Wekan work, try this and check with your web browser:

    > `docker compose up`

2) Stop Wekan and start Wekan in background:

    > `docker compose stop`
    > `docker compose up -d`

3) See running Docker containers:

    > `docker ps`

4) Stop Docker containers:

    > `docker compose stop`

<br>

## INSIDE DOCKER CONTAINERS, AND BACKUP/RESTORE
>
> For more information on backups see: <https://github.com/wekan/wekan/wiki/Backup>
>
> If really necessary, repair MongoDB: <https://github.com/wekan/wekan-mongodb/issues/6#issuecomment-424004116>

### Creating Database Dump

1) Going inside container:

    > The Wekan app, does not contain data. MongoDB, contains all data.
    >> `docker exec -it wekan-db bash`

2) Copy database to outside of container:

    > `cd /data`
    >>
    > `mongodump`
    >>
    > `exit`
    >>
    > `docker cp wekan-db:/data/dump .`

<br>

### Restoring database

1) Stop wekan

    > `docker stop wekan-app`

2) Go inside database container

    > `docker exec -it wekan-db bash`

3) Go inside the data directory

    > `cd /data`

4) Remove previous dump

    > `rm -rf dump`

5) Exit db container

    > `exit`

6) Copy dump to inside docker container

    > `docker cp dump wekan-db:/data/`

7) Go inside database container

    > `docker exec -it wekan-db bash`

8) Go inside data directory

    > `cd /data`

9) Restore

    > `mongorestore --drop`

10) Exit db container

    > `exit`

11) Start wekan

    > `docker start wekan-app`
