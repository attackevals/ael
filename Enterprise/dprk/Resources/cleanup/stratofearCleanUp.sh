#!/bin/bash
#################################
# Usage:
# Execute the following command from the /STRATOFEAR folder
# sudo ./cleanUpScript.sh
#
# Dependencies:
# build/cmakeVariables.sh files
# 
# This script performs the following actions:
#   Properly unload the STRATOFEAR Service
#   Ensure all Stratofear processes are killed
#   Delete all files associated with STRATOFEAR (Network Configuration file,
#   STRATOFEAR mach-o, dylib files, & LaunchDaemon plist file)
#################################
# Pull in the cmake variables
variables_script="./build/cmakeVariables.sh"
if [ -f "$variables_script" ]; then
    source "$variables_script"
    echo "Sourced $variables_script"
else
    echo "File $variables_script does not exist, this file is needed to continue."
    exit 1
fi

echo -e "\n"
# ANSI escape code for purple color
PURPLE='\033[0;35m'
RESET='\033[0m'  # Reset to default color

# Print text in purple
echo -e "${PURPLE}  .d8888b.  888                     888              .d888                           ${RESET}"
echo -e "${PURPLE}d88P  Y88b 888                     888             d88P\"                            ${RESET}"
echo -e "${PURPLE}Y88b.      888                     888             888                              ${RESET}"
echo -e "${PURPLE} \"Y888b.   888888 888d888  8888b.  888888  .d88b.  888888  .d88b.   8888b.  888d888 ${RESET}"
echo -e "${PURPLE}    \"Y88b. 888    888P\"       \"88b 888    d88\"\"88b 888    d8P  Y8b     \"88b 888P\"   ${RESET}"
echo -e "${PURPLE}      \"888 888    888     .d888888 888    888  888 888    88888888 .d888888 888     ${RESET}"
echo -e "${PURPLE}Y88b  d88P Y88b.  888     888  888 Y88b.  Y88..88P 888    Y8b.     888  888 888     ${RESET}"
echo -e "${PURPLE} \"Y8888P\"   \"Y888 888     \"Y888888  \"Y888  \"Y88P\"  888     \"Y8888  \"Y888888 888     ${RESET}"

echo "     .  . '    ."
echo "      '   .            . '            .                +"
echo "              \`                          '    . '"
echo "        .                         ,'\\\`.                         ."
echo "   .                  ..\"    _.-;'    \\\`.              ."
echo "              _.-\"\\\`.##%\"_.--\" ,'        \\\`.                     ___,,od000HHHHHHMMMMMMM"
echo "           ,'\"-_ _.-.--\"\\\"   ,'            \`-_            ,,/////00000HHMMMMMMMMMMMMMMMM"
echo "         ,'     |_.'     )\`/-     __..--\"\"\`-_\`\._    /////00000HHHHMMMMMMMMMMMMMMMMMMMMM"
echo " . +   ,'   _.-\"        / /   _-\"\"           \`-.\`-_\\___\\\\\\////0000HHHHHHHHMMMMMMMMMMMMMM"
echo "     .'_.-\"\"      '    :_/_.-'                 _,\`-/__V__\\\\\\\\0000HHHHHHMMMMMMMMMMMMMMMMM"
echo " . _-\"\"                         .        '   _,////\\\\  |  /000HHHHHHHMMMMMMMMMMMMMMMMMMM"
echo "_-\"   .       '  +  .              .        ,//////0\\ \| /00HHHHHHHMMMMMMMMMMMMMMMMMMMMM"
echo "       \`                                   ,//////000\\|/00HHHHHHHMMMMMMMMMMMMMMMMMMMMMMM"
echo ".             '       .  ' .   .       '  ,//////00000\\|00HHHHHHHHMMMMMMMMMMMMMMMMMMMMMM"
echo "     .             .    .    '           ,//////000000\\|00HHHHHHHMMMMMMMMMMMMMMMMMMMMMMM"
echo "                  .  '      .       .   ,///////000000\\|0HHHHHHHHMMMMMMMMMMMMMMMMMMMMMMM"
echo "  '             '        .    '         ///////000000000HHHHHHHHMMMMMMMMMMMMMMMMMMMMMMMM"
echo "                    +  .  . '    .     ,///////000000000HHHHHHHMMMMMMMMMMMMMMMMMMMMMMMMM"
echo "     '      .              '   .       ///////000000000HHHHHHHHMMMMMMMMMMMMMMMMMMMMMMMMM"
echo "   '                  . '              ///////000000000HHHHHHHHMMMMMMMMMMMMMMMMMMMMMMMMM"
echo "                           .   '      ,///////000000000HHHHHHHHMMMMMMMMMMMMMMMMMMMMMMMMM"
echo "     '            .       .  ' .   .  /////////00000000HHHHHHHHHHMMMMMMMMMMMMMMMMMMMMMMM"


echo -e "\nASCII Art: https://patorjk.com &\JL https://www.asciiart.eu/space/spaceships"

# Ensure we are running as sudo
if [ "$EUID" -ne 0 ]; then
    echo -e "Please run this script with sudo:\n\nsudo ./cleanUpScript.sh\n\n"
    exit 1
fi

# Check if the service is loaded or processes are running
service_name="system/us.zoom.ZoomHelperTool"
keyword="ZoomHelperTool"
echo -e "\nUnloading $service_name and killing processes associated with $keyword..."
result=$(pgrep -f $keyword)
if [ $? -ne 0 ]; then
    echo "No processes with $keyword running"
fi
# Unload, find, & kill all STRATOFEAR processes
if [ -n "$result" ]; then
    echo "Process found with PID(s): $result"
    output=$(launchctl unload "$service_name" 2>&1)
    if [ $? -eq 0 ]; then
        echo "Service $service_name unloaded successfully"
    else
        echo "Error: Failed to unload service $service_name - $output"
    fi
    # Ensure processes are killed
    for pid in $result; do
        echo "Terminating process with PID $pid"
        kill -9 $pid
    done
else
    echo "Service $service_name is not loaded"
fi
# Delete all files associated with STRATOFEAR (Network Configuration file,
# STRATOFEAR mach-o, dylib files, & LaunchDaemon plist file)
echo -e "\nRemoving all files associated with STRATOFEAR..."

STRATOFEAR_FILES=(
    "/Library/LaunchDaemons/us.zoom.ZoomHelperTool.plist"
    "/Library/PrivilegedHelperTools/us.zoom.ZoomHelperTool"
    # Alternate Step Artifacts
    "/Library/PrivilegedHelperTools/us.zoom.ZoomHelperTool-discovery"
    "/Library/PrivilegedHelperTools/us.zoom.ZoomHelperTool-Runner"
)

for FILE in "${STRATOFEAR_FILES[@]}"; do
    if [ -e "$FILE" ]; then
        rm -f "$FILE"
        echo "Removed file: $FILE"
    else
        echo "File does not exist: $FILE"
    fi
done

CONFIG_FILES=(
    "$CONFIG_PATH_ENV/$CONFIG_FILENAME_ENV"
    "$CONFIG_PATH_ENV/$CONFIG_FILENAME_ENV.lock"
)

for FILE in "${CONFIG_FILES[@]}"; do
    if [ -e "$FILE" ]; then
        rm -f "$FILE"
        echo "Removed file: $FILE"
    else
        echo "File does not exist: $FILE"
    fi
done


# Remove all files uploaded or downloaded using a C2 server
C2_FILES=$(ls /tmp/*.tmp 2>&1)
if [ $? -ne 0 ]; then
    echo "No files found in /tmp: $C2_FILES"
else
    for FILE in ${C2_FILES}; do
        if [ -e "${FILE}" ]; then
            rm -f "${FILE}"
            echo "Removed file: ${FILE}"
        else
            echo "File does not exist: ${FILE}"
        fi
    done
fi



echo -e "\n################ Clean up script Complete ###############"
