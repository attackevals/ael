#import <Foundation/Foundation.h>
#include <CoreFoundation/CoreFoundation.h>
#include <Security/Security.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

#define FAIL_DEFAULT_KEYCHAIN_COPY 2
#define FAIL_KEYCHAIN_LOCKED 3

@interface Keychain : NSObject

// [Keychain printSecErr]
//      About:
//          Keychain class: calls SecCopyErrorMessageString to print the meaning
//          of a security result code.
//      Result:
//          string explaining the security result code is printed to the console
//      References:
//          https://developer.apple.com/documentation/security/1542001-security_framework_result_codes?language=objc
//          https://www.osstatus.com/search/results?platform=all&framework=Security&search=SecBase.h
+ (void) printSecErr:(OSStatus)status;

// [Keychain setRootDefaultKeychain]
//      About:
//          Keychain class: sets default keychain to root's keychain using 
//          SecKeychainOpen and SecKeychainSetDefault APIs with path
//          "/private/var/root/Library/Keychains"
//      Result:
//          Default keychain should point to "/private/var/root/Library/Keychains"
//      Returns:
//          [bool] true if default keychain was set successfully, false otherwise
//      References:
//          https://stackoverflow.com/questions/59290980/swift-keychain-services-seckeychainsetdefault-fails-as-root-will-not-set-def
+ (BOOL) setRootDefaultKeychain;

// [Keychain checkAccessKeychain]
//      About:
//          Keychain class: uses SecKeychainCopyDefault to create a copy of the
//          default keychain then uses SecKeychainGetStatus to check if the
//          keychain status is locked (status code 2)
//      Returns:
//          [int] representing keychain access
//              0 - default keychain accessed successfully and is unlocked
//              2 - unable to access default keychain
//              3 - default keychain is locked
//      References:
//          https://github.com/its-a-feature/LockSmith/
+ (int) checkAccessKeychain;

// [Keychain createKeychainSearchQuery]
//      About:
//          Keychain class: creates a dictionary object for use with
//          SecItemCopyMatching as the keychain search query. The query built has
//          the following parameters:
//          - Search for generic password items (genp)
//          - Return all matching items
//          - Return item attributes
//          - Don't return item data (no password prompt popups)
//          - Return reference to items (for enumerating item content later)
//          - Don't treat the macOS keychain items like iOS keychain items
//      Result:
//          Search query created with the above parameters
//      Returns:
//          [CFMutableDictionaryRef] reference to the query
//      References:
//          https://github.com/its-a-feature/LockSmith/
+ (CFMutableDictionaryRef) createKeychainSearchQuery;

// [Keychain getKeychainItemPassword]
//      About:
//          Keychain class: attempt to use SecKeychainItemCopyContent to get the
//          password content for the given keychain item
//      Result:
//          Plaintext password printed on success 
//      Returns:
//          [NULL]
//      MITRE ATT&CK Techniques:
//          T1555.001 Credentials from Password Stores: Keychain
//      References:
//          https://github.com/its-a-feature/LockSmith/
+ (void) getKeychainItemPassword:(NSDictionary *) item;

@end