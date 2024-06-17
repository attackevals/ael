--- IF db exists, nothing to do
IF NOT EXISTS(SELECT *
              FROM sys.databases
              WHERE name = 'NetbnmBackup')
USE [master]
GO

--- Create logins for local users
CREATE LOGIN netbnmadmin WITH PASSWORD =N'Password*', DEFAULT_DATABASE = [master],
    CHECK_EXPIRATION = OFF, CHECK_POLICY = OFF
CREATE LOGIN zorimoto WITH PASSWORD =N'tzTVgs44isT4YxWU!', DEFAULT_DATABASE = [master],
    CHECK_EXPIRATION = OFF, CHECK_POLICY = OFF
GO

--- Create db
CREATE DATABASE [NetbnmBackup];
GO

--- Switch to new db
USE [NetbnmBackup];
GO

CREATE USER netbnmadmin FOR LOGIN netbnmadmin
GO

ALTER ROLE db_owner ADD MEMBER netbnmadmin
GO

-- Add zorimoto domain and sql accounts to sysadmin role
ALTER SERVER ROLE sysadmin ADD MEMBER zorimoto;
ALTER SERVER ROLE sysadmin ADD MEMBER [digirevenge\zorimoto];
GO

--- create table
CREATE TABLE Credentials
(
    username    varchar(100),
    password    varchar(4000),
    description varchar(1000)
);
GO
