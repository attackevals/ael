set securityCommand to "dump-keychain"
set securityFlag to "-d"
set securityKeychain to "/Library/Keychains/System.keychain"

do shell script "security " & securityCommand & " " & securityFlag & " " & securityKeychain & " > /tmp/kc.txt" with administrator privileges