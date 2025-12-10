#!/usr/bin/env bash

# raw dump of commands, no error checking, does not include setup

# proget free license key: 8C3NN8QU-2404-1353NS-S08U22-MHCPPKCC
# post script setup instructions for configuring feed: https://docs.inedo.com/docs/proget/feeds/chocolatey/proget-howto-private-chocolatey-repository

docker network create inedo

docker run --name inedo-sql \
	-e 'ACCEPT_EULA=Y' -e 'MSSQL_SA_PASSWORD=Dragging-Uncaring-Omnivore' \
	-e 'MSSQL_PID=Express' --net=inedo --restart=unless-stopped \
	-v sqlvolume:/var/opt/mssql \
	-d mcr.microsoft.com/mssql/server:2019-latest

# https://learn.microsoft.com/en-us/sql/tools/sqlcmd/sqlcmd-utility?view=sql-server-ver16&tabs=go,windows&pivots=cs1-bash
# C option trusts server certificate, required
docker exec -it inedo-sql '/opt/mssql-tools18/bin/sqlcmd' \
	-C -S localhost -U SA -P 'Dragging-Uncaring-Omnivore' \
	-Q 'CREATE DATABASE [ProGet] COLLATE SQL_Latin1_General_CP1_CI_AS'

docker run -d --name=proget --restart=unless-stopped \
	-v proget-packages:/var/proget/packages -p 80:80 --net=inedo \
	-e PROGET_SQL_CONNECTION_STRING='Data Source=inedo-sql; Initial Catalog=ProGet; User ID=sa; Password=Dragging-Uncaring-Omnivore' \
	proget.inedo.com/productimages/inedo/proget:latest
