#include "util.hpp"

// Writes buffer to specified file
DWORD WriteFileBytes(shared_func_pointers* fp, HANDLE h_file, char* buffer, DWORD buffer_len) {
    char* p_seek_buffer = buffer;
    DWORD remaining_bytes = buffer_len;
    DWORD bytes_written;
    while (remaining_bytes > 0) {
        if (!fp->fp_WriteFile(h_file, p_seek_buffer, remaining_bytes, &bytes_written, NULL)) {
            return fp->fp_GetLastError();
        }
        p_seek_buffer += bytes_written;
        remaining_bytes -= bytes_written;
    }
    return ERROR_SUCCESS;
}

// Position-independent implementation of memcpy
void pi_memcpy(void* dst, const void* src, size_t n) {
    char* c_src = (char*)src;
    char* c_dst = (char*)dst;
    for (size_t i = 0; i < n; i++) {
        c_dst[i] = c_src[i];
    }
}

// Position-independent implementation of memset
void pi_memset(void* dst, unsigned char a, size_t n) {
    unsigned char* c_dst = (unsigned char*)dst;
    for (size_t i = 0; i < n; i++) {
        c_dst[i] = a;
    }
}
