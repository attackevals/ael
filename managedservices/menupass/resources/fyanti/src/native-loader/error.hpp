#pragma once
#include <Windows.h>
#include <system_error>
#include <string>

namespace common {

	class windows_error : public std::system_error {
		using std::system_error::system_error;
	public:
		std::wstring w_what() const;
	};

	class windows_error_code : public std::error_code {
		using std::error_code::error_code;
	public:
		std::wstring w_message() const;
	};

	// upgrade this to a template which can take wstring or string
	windows_error get_last_error(std::string prefix = "");

	// upgrade this to a template which can take wstring or string
	windows_error get_last_error(
		const std::error_category& cat,
		std::string prefix = ""
	);

	// upgrade this to a template which can take wstring or string
	windows_error ntstatus_to_error(
		NTSTATUS status,
		std::string prefix = ""
	);

	class ntdll_category : public std::error_category {
		virtual const char* name() const noexcept override;
		virtual std::string message(int status) const override;
	};

	std::wstring get_error_message(DWORD code);
	std::wstring get_error_message(DWORD code, HMODULE dll);
	std::string  get_error_message(HRESULT result);

} // namespace common
