# Configuration options for stealbit

set(CONFIG_FILE_NAME $ENV{CONFIG_FILE_NAME} CACHE STRING "Name of the encrypted Stealbit config file")
set(CONFIG_FILE_PATH $ENV{CONFIG_FILE_PATH} CACHE STRING "File path for Stealbit configuration")
set(CONFIG_ENCRYPTION_KEY_BASE64 $ENV{CONFIG_ENCRYPTION_KEY_BASE64} CACHE STRING "Encryption key for config file")
set(DJB2_HASH_SEED $ENV{DJB2_HASH_SEED} CACHE STRING "Seed hash for djb2 hash alogrithm")
set(C2_URI $ENV{C2_URI} CACHE STRING "C2 Server URI")
set(C2_MAGIC_RESPONSE $ENV{C2_MAGIC_RESPONSE} CACHE STRING "C2 Server Magic Response")