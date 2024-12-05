--- IF db exists, nothing to do
IF NOT EXISTS(SELECT *
              FROM sys.databases
              WHERE name = 'DiscoverBackup')
USE [master]
GO

--- Create logins for local users
CREATE LOGIN discoveradmin WITH PASSWORD =N'[5SzmP2%£2zIM<D', DEFAULT_DATABASE = [master],
    CHECK_EXPIRATION = OFF, CHECK_POLICY = OFF
CREATE LOGIN faker WITH PASSWORD =N'j8oTTse9T6gL', DEFAULT_DATABASE = [master],
    CHECK_EXPIRATION = OFF, CHECK_POLICY = OFF
GO

--- Create db
CREATE DATABASE [DiscoverBackup];
GO

--- Switch to new db
USE [DiscoverBackup];
GO

CREATE USER discoveradmin FOR LOGIN discoveradmin
GO

ALTER ROLE db_owner ADD MEMBER discoveradmin
GO

-- Add faker domain and sql accounts to sysadmin role
ALTER SERVER ROLE sysadmin ADD MEMBER faker;
-- ALTER SERVER ROLE sysadmin ADD MEMBER [your_cooked\faker];
GO

--- create table
CREATE TABLE Credentials
(
    username    varchar(100),
    password    varchar(4000),
    description varchar(1000)
);
GO
