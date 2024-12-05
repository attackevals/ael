#include <CoreFoundation/CoreFoundation.h>
#import <Foundation/Foundation.h>
#include <Security/Security.h>
#include "keychain.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

#define FAIL_SET_SYSTEM_KEYCHAIN 1
#define FAIL_ACCOUNT_NOT_FOUND 4
#define FAIL_SEARCH_ERROR 5

int main() {
    @autoreleasepool {

        // don't allow popups
        SecKeychainSetUserInteractionAllowed(false);

        // get the current default keychain so we can reset it later
        SecKeychainRef defaultKeychain;
        SecKeychainCopyDefault(&defaultKeychain);

        if (getuid() == 0) {
            if (![Keychain setRootDefaultKeychain]) {
                return FAIL_SET_SYSTEM_KEYCHAIN;
            }
        }

        int checkKeychainStatus = [Keychain checkAccessKeychain];
        if (checkKeychainStatus != 0) {
            return checkKeychainStatus;
        }

        // query keychain
        CFMutableDictionaryRef query = [Keychain createKeychainSearchQuery];

        CFTypeRef items;
        OSStatus keychainSearchStatus = SecItemCopyMatching(query, &items);

        if (keychainSearchStatus == errSecItemNotFound) {
            return FAIL_ACCOUNT_NOT_FOUND;
        }
        else if (keychainSearchStatus != ERR_SUCCESS) {
            [Keychain printSecErr:keychainSearchStatus];
            return FAIL_SEARCH_ERROR;
        }

        NSArray *itemArray = (__bridge NSArray*)items;

        for(int i = 0; i < [itemArray count]; i++){
            [Keychain getKeychainItemPassword:itemArray[i]];
        }
        CFRelease(items);

        // set default back
        SecKeychainSetDefault(defaultKeychain);
        CFRelease(defaultKeychain);
    }
    return 0;
}