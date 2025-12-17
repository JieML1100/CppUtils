#include "HttpHelper.h"
#include "Utils.h"
#include <Winhttp.h>

#pragma comment(lib, "winhttp.lib")
std::string HttpHelper::UrlEncode(const std::string str) {
	auto input = Convert::AnsiToUtf8(str);
	std::ostringstream encoded;
	for (uint8_t c : input) {
		if (
			(c >= ' ' && c <= '~')
			) {
			encoded << c;
		}
		else {
			encoded << '%' << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(c);
		}
	}
	return encoded.str();
}
std::string HttpHelper::CheckUrl(std::string input) {
	auto replace = [](std::string str, const char* old_str, const char* new_str) {
		size_t len_old = strlen(old_str);
		size_t len_new = strlen(new_str);
		size_t pos = str.find(old_str);
		while (pos != std::string::npos) {
			str.replace(pos, len_old, new_str);
			pos = str.find(old_str, pos + len_new);
		}
		};
	replace(input, " ", "%20");
	replace(input, "\"", "%22");
	replace(input, "#", "%23");
	replace(input, "%", "%25");
	replace(input, "&", "%26");
	replace(input, "(", "%28");
	replace(input, ")", "%29");
	replace(input, "+", "%2B");
	replace(input, ",", "%2C");
	replace(input, "/", "%2F");
	replace(input, ":", "%3A");
	replace(input, ";", "%3B");
	replace(input, "<", "%3C");
	replace(input, "=", "%3D");
	replace(input, ">", "%3E");
	replace(input, "?", "%3F");
	replace(input, "@", "%40");
	replace(input, "\\", "%5C");
	replace(input, "|", "%7C");
	return UrlEncode(input);
}
std::string HttpHelper::GetHostNameFromURL(std::string url) {
	if (url._Starts_with("https://")) {
		url;
	}
	else if (!url._Starts_with("http://")) {
		url = "http://" + url;
	}
	std::string hostName;
	size_t pos = url.find("//");
	if (pos != std::string::npos) {
		pos += 2;
		size_t endPos = url.find('/', pos);
		if (endPos != std::string::npos) {
			hostName = url.substr(pos, endPos - pos);
		}
		else {
			hostName = url.substr(pos);
		}
	}
	return hostName;
}
std::string HttpHelper::Get(std::string url, std::string headers, std::string cookies) {
	HINTERNET hSession = nullptr, hConnect = nullptr, hRequest = nullptr;
	std::string response;
	url = UrlEncode(url);

	// Parse URL components
	URL_COMPONENTS urlComp;
	ZeroMemory(&urlComp, sizeof(urlComp));
	urlComp.dwStructSize = sizeof(urlComp);

	wchar_t hostName[256];
	wchar_t urlPath[256];
	urlComp.lpszHostName = hostName;
	urlComp.dwHostNameLength = _countof(hostName);
	urlComp.lpszUrlPath = urlPath;
	urlComp.dwUrlPathLength = _countof(urlPath);
	std::wstring wUrl(url.begin(), url.end());
	if (!WinHttpCrackUrl(wUrl.c_str(), static_cast<DWORD>(wUrl.length()), 0, &urlComp)) {
		std::cerr << "Error: Unable to crack URL." << std::endl;
		return "";
	}

	// Initialize WinHTTP session
	hSession = WinHttpOpen(L"WinHTTP Example/1.0",
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		WINHTTP_NO_PROXY_NAME,
		WINHTTP_NO_PROXY_BYPASS, 0);
	if (!hSession) {
		std::cerr << "Error: WinHttpOpen failed." << std::endl;
		return "";
	}

	// Connect to server
	hConnect = WinHttpConnect(hSession, urlComp.lpszHostName, urlComp.nPort, 0);
	if (!hConnect) {
		std::cerr << "Error: WinHttpConnect failed." << std::endl;
		WinHttpCloseHandle(hSession);
		return "";
	}

	// Prepare the GET request
	hRequest = WinHttpOpenRequest(hConnect, L"GET", urlComp.lpszUrlPath,
		nullptr, WINHTTP_NO_REFERER,
		WINHTTP_DEFAULT_ACCEPT_TYPES,
		(urlComp.nScheme == INTERNET_SCHEME_HTTPS) ? WINHTTP_FLAG_SECURE : 0);
	if (!hRequest) {
		std::cerr << "Error: WinHttpOpenRequest failed." << std::endl;
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		return "";
	}

	// Set headers
	std::wstring wHeaders(headers.begin(), headers.end());
	if (!headers.empty()) {
		WinHttpAddRequestHeaders(hRequest, wHeaders.c_str(), static_cast<DWORD>(-1L), WINHTTP_ADDREQ_FLAG_ADD);
	}

	// Set cookies
	if (!cookies.empty()) {
		std::wstring wCookies(cookies.begin(), cookies.end());
		WinHttpAddRequestHeaders(hRequest, wCookies.c_str(), static_cast<DWORD>(-1L), WINHTTP_ADDREQ_FLAG_ADD);
	}

	// Send the request
	BOOL bResult = WinHttpSendRequest(hRequest,
		WINHTTP_NO_ADDITIONAL_HEADERS, 0,
		WINHTTP_NO_REQUEST_DATA, 0,
		0, 0);
	if (!bResult) {
		std::cerr << "Error: WinHttpSendRequest failed." << std::endl;
		WinHttpCloseHandle(hRequest);
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		return "";
	}

	// Receive the response
	bResult = WinHttpReceiveResponse(hRequest, nullptr);
	if (!bResult) {
		std::cerr << "Error: WinHttpReceiveResponse failed." << std::endl;
		WinHttpCloseHandle(hRequest);
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		return "";
	}

	// Read the response data
	DWORD dwSize = 0;
	DWORD dwDownloaded = 0;
	LPSTR pszOutBuffer;
	do {
		// Check for available data
		dwSize = 0;
		if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) {
			std::cerr << "Error: WinHttpQueryDataAvailable failed." << std::endl;
			break;
		}

		// Allocate space for the buffer
		pszOutBuffer = new char[dwSize + 1];
		if (!pszOutBuffer) {
			std::cerr << "Error: Out of memory." << std::endl;
			dwSize = 0;
			break;
		}

		// Read the data
		ZeroMemory(pszOutBuffer, dwSize + 1);
		if (!WinHttpReadData(hRequest, (LPVOID)pszOutBuffer, dwSize, &dwDownloaded)) {
			std::cerr << "Error: WinHttpReadData failed." << std::endl;
		}
		else {
			response.append(pszOutBuffer, dwDownloaded);
		}

		// Free the memory
		delete[] pszOutBuffer;

	} while (dwSize > 0);

	// Cleanup
	WinHttpCloseHandle(hRequest);
	WinHttpCloseHandle(hConnect);
	WinHttpCloseHandle(hSession);

	return response;
}
std::string HttpHelper::GetStream(std::string url, std::string headers, std::string cookies, HTTP_STREAM_CALLBACK callback) {
	HINTERNET hSession = nullptr, hConnect = nullptr, hRequest = nullptr;
	std::string response;
	url = UrlEncode(url);

	// Parse URL components
	URL_COMPONENTS urlComp;
	ZeroMemory(&urlComp, sizeof(urlComp));
	urlComp.dwStructSize = sizeof(urlComp);

	wchar_t hostName[256];
	wchar_t urlPath[256];
	urlComp.lpszHostName = hostName;
	urlComp.dwHostNameLength = _countof(hostName);
	urlComp.lpszUrlPath = urlPath;
	urlComp.dwUrlPathLength = _countof(urlPath);
	std::wstring wUrl(url.begin(), url.end());
	if (!WinHttpCrackUrl(wUrl.c_str(), static_cast<DWORD>(wUrl.length()), 0, &urlComp)) {
		std::cerr << "Error: Unable to crack URL." << std::endl;
		return "";
	}

	// Initialize WinHTTP session
	hSession = WinHttpOpen(L"WinHTTP Example/1.0",
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		WINHTTP_NO_PROXY_NAME,
		WINHTTP_NO_PROXY_BYPASS, 0);
	if (!hSession) {
		std::cerr << "Error: WinHttpOpen failed." << std::endl;
		return "";
	}

	// Connect to server
	hConnect = WinHttpConnect(hSession, urlComp.lpszHostName, urlComp.nPort, 0);
	if (!hConnect) {
		std::cerr << "Error: WinHttpConnect failed." << std::endl;
		WinHttpCloseHandle(hSession);
		return "";
	}

	// Prepare the GET request
	hRequest = WinHttpOpenRequest(hConnect, L"GET", urlComp.lpszUrlPath,
		nullptr, WINHTTP_NO_REFERER,
		WINHTTP_DEFAULT_ACCEPT_TYPES,
		(urlComp.nScheme == INTERNET_SCHEME_HTTPS) ? WINHTTP_FLAG_SECURE : 0);
	if (!hRequest) {
		std::cerr << "Error: WinHttpOpenRequest failed." << std::endl;
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		return "";
	}

	// Set headers
	std::wstring wHeaders(headers.begin(), headers.end());
	if (!headers.empty()) {
		WinHttpAddRequestHeaders(hRequest, wHeaders.c_str(), static_cast<DWORD>(-1L), WINHTTP_ADDREQ_FLAG_ADD);
	}

	// Set cookies
	if (!cookies.empty()) {
		std::wstring wCookies(cookies.begin(), cookies.end());
		WinHttpAddRequestHeaders(hRequest, wCookies.c_str(), static_cast<DWORD>(-1L), WINHTTP_ADDREQ_FLAG_ADD);
	}

	// Send the request
	BOOL bResult = WinHttpSendRequest(hRequest,
		WINHTTP_NO_ADDITIONAL_HEADERS, 0,
		WINHTTP_NO_REQUEST_DATA, 0,
		0, 0);
	if (!bResult) {
		std::cerr << "Error: WinHttpSendRequest failed." << std::endl;
		WinHttpCloseHandle(hRequest);
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		return "";
	}

	// Receive the response
	bResult = WinHttpReceiveResponse(hRequest, nullptr);
	if (!bResult) {
		std::cerr << "Error: WinHttpReceiveResponse failed." << std::endl;
		WinHttpCloseHandle(hRequest);
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		return "";
	}

	// Read the response data
	DWORD dwSize = 0;
	DWORD dwDownloaded = 0;
	LPSTR pszOutBuffer;
	do {
		// Check for available data
		dwSize = 0;
		if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) {
			std::cerr << "Error: WinHttpQueryDataAvailable failed." << std::endl;
			break;
		}

		// Allocate space for the buffer
		pszOutBuffer = new char[dwSize + 1];
		if (!pszOutBuffer) {
			std::cerr << "Error: Out of memory." << std::endl;
			dwSize = 0;
			break;
		}

		// Read the data
		ZeroMemory(pszOutBuffer, dwSize + 1);
		if (!WinHttpReadData(hRequest, (LPVOID)pszOutBuffer, dwSize, &dwDownloaded)) {
			std::cerr << "Error: WinHttpReadData failed." << std::endl;
		}
		else {
			response.append(pszOutBuffer, dwDownloaded);
			if (callback)
				callback(pszOutBuffer);
		}

		// Free the memory
		delete[] pszOutBuffer;

	} while (dwSize > 0);

	// Cleanup
	WinHttpCloseHandle(hRequest);
	WinHttpCloseHandle(hConnect);
	WinHttpCloseHandle(hSession);

	return response;
}
std::vector<uint8_t> HttpHelper::HttpGetBytes(std::string url) {
	auto bytes = Get(url);
	return std::vector<uint8_t>((uint8_t*)bytes.data(), (uint8_t*)bytes.data() + bytes.size());
}
std::string HttpHelper::Post(std::string url, std::string body, std::string headers, std::string cookies) {
	HINTERNET hSession = nullptr, hConnect = nullptr, hRequest = nullptr;
	std::string response;
	url = UrlEncode(url);

	// Parse URL components
	URL_COMPONENTS urlComp;
	ZeroMemory(&urlComp, sizeof(urlComp));
	urlComp.dwStructSize = sizeof(urlComp);

	wchar_t hostName[256];
	wchar_t urlPath[256];
	urlComp.lpszHostName = hostName;
	urlComp.dwHostNameLength = _countof(hostName);
	urlComp.lpszUrlPath = urlPath;
	urlComp.dwUrlPathLength = _countof(urlPath);
	std::wstring wUrl(url.begin(), url.end());
	if (!WinHttpCrackUrl(wUrl.c_str(), static_cast<DWORD>(wUrl.length()), 0, &urlComp)) {
		std::cerr << "Error: Unable to crack URL." << std::endl;
		return "";
	}

	// Initialize WinHTTP session
	hSession = WinHttpOpen(L"WinHTTP Example/1.0",
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		WINHTTP_NO_PROXY_NAME,
		WINHTTP_NO_PROXY_BYPASS, 0);
	if (!hSession) {
		std::cerr << "Error: WinHttpOpen failed." << std::endl;
		return "";
	}

	// Connect to server
	hConnect = WinHttpConnect(hSession, urlComp.lpszHostName, urlComp.nPort, 0);
	if (!hConnect) {
		std::cerr << "Error: WinHttpConnect failed." << std::endl;
		WinHttpCloseHandle(hSession);
		return "";
	}

	// Prepare the POST request
	hRequest = WinHttpOpenRequest(hConnect, L"POST", urlComp.lpszUrlPath,
		nullptr, WINHTTP_NO_REFERER,
		WINHTTP_DEFAULT_ACCEPT_TYPES,
		(urlComp.nScheme == INTERNET_SCHEME_HTTPS) ? WINHTTP_FLAG_SECURE : 0);
	if (!hRequest) {
		std::cerr << "Error: WinHttpOpenRequest failed." << std::endl;
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		return "";
	}

	// Set headers
	std::wstring wHeaders(headers.begin(), headers.end());
	if (!headers.empty()) {
		WinHttpAddRequestHeaders(hRequest, wHeaders.c_str(), static_cast<DWORD>(-1L), WINHTTP_ADDREQ_FLAG_ADD);
	}

	// Set cookies
	if (!cookies.empty()) {
		std::wstring wCookies(cookies.begin(), cookies.end());
		WinHttpAddRequestHeaders(hRequest, wCookies.c_str(), static_cast<DWORD>(-1L), WINHTTP_ADDREQ_FLAG_ADD);
	}

	// Send the request
	BOOL bResult = WinHttpSendRequest(hRequest,
		WINHTTP_NO_ADDITIONAL_HEADERS, 0,
		(LPVOID)body.c_str(), static_cast<DWORD>(body.length()),
		static_cast<DWORD>(body.length()), 0);
	if (!bResult) {
		std::cerr << "Error: WinHttpSendRequest failed." << std::endl;
		WinHttpCloseHandle(hRequest);
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		return "";
	}

	// Receive the response
	bResult = WinHttpReceiveResponse(hRequest, nullptr);
	if (!bResult) {
		std::cerr << "Error: WinHttpReceiveResponse failed." << std::endl;
		WinHttpCloseHandle(hRequest);
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		return "";
	}

	// Read the response data
	DWORD dwSize = 0;
	DWORD dwDownloaded = 0;
	LPSTR pszOutBuffer;
	do {
		// Check for available data
		dwSize = 0;
		if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) {
			std::cerr << "Error: WinHttpQueryDataAvailable failed." << std::endl;
			break;
		}

		// Allocate space for the buffer
		pszOutBuffer = new char[dwSize + 1];
		if (!pszOutBuffer) {
			std::cerr << "Error: Out of memory." << std::endl;
			dwSize = 0;
			break;
		}

		// Read the data
		ZeroMemory(pszOutBuffer, dwSize + 1);
		if (!WinHttpReadData(hRequest, (LPVOID)pszOutBuffer, dwSize, &dwDownloaded)) {
			std::cerr << "Error: WinHttpReadData failed." << std::endl;
		}
		else {
			response.append(pszOutBuffer, dwDownloaded);
		}

		// Free the memory
		delete[] pszOutBuffer;

	} while (dwSize > 0);

	// Cleanup
	WinHttpCloseHandle(hRequest);
	WinHttpCloseHandle(hConnect);
	WinHttpCloseHandle(hSession);

	return response;
}
std::string HttpHelper::PostStream(std::string url, std::string body, std::string headers, std::string cookies, HTTP_STREAM_CALLBACK callback) {
	HINTERNET hSession = nullptr, hConnect = nullptr, hRequest = nullptr;
	std::string response;
	url = UrlEncode(url);

	// Parse URL components
	URL_COMPONENTS urlComp;
	ZeroMemory(&urlComp, sizeof(urlComp));
	urlComp.dwStructSize = sizeof(urlComp);

	wchar_t hostName[256];
	wchar_t urlPath[256];
	urlComp.lpszHostName = hostName;
	urlComp.dwHostNameLength = _countof(hostName);
	urlComp.lpszUrlPath = urlPath;
	urlComp.dwUrlPathLength = _countof(urlPath);
	std::wstring wUrl(url.begin(), url.end());
	if (!WinHttpCrackUrl(wUrl.c_str(), static_cast<DWORD>(wUrl.length()), 0, &urlComp)) {
		std::cerr << "Error: Unable to crack URL." << std::endl;
		return "";
	}

	// Initialize WinHTTP session
	hSession = WinHttpOpen(L"WinHTTP Example/1.0",
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		WINHTTP_NO_PROXY_NAME,
		WINHTTP_NO_PROXY_BYPASS, 0);
	if (!hSession) {
		std::cerr << "Error: WinHttpOpen failed." << std::endl;
		return "";
	}

	// Connect to server
	hConnect = WinHttpConnect(hSession, urlComp.lpszHostName, urlComp.nPort, 0);
	if (!hConnect) {
		std::cerr << "Error: WinHttpConnect failed." << std::endl;
		WinHttpCloseHandle(hSession);
		return "";
	}

	// Prepare the POST request
	hRequest = WinHttpOpenRequest(hConnect, L"POST", urlComp.lpszUrlPath,
		nullptr, WINHTTP_NO_REFERER,
		WINHTTP_DEFAULT_ACCEPT_TYPES,
		(urlComp.nScheme == INTERNET_SCHEME_HTTPS) ? WINHTTP_FLAG_SECURE : 0);
	if (!hRequest) {
		std::cerr << "Error: WinHttpOpenRequest failed." << std::endl;
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		return "";
	}

	// Set headers
	std::wstring wHeaders(headers.begin(), headers.end());
	if (!headers.empty()) {
		WinHttpAddRequestHeaders(hRequest, wHeaders.c_str(), static_cast<DWORD>(-1L), WINHTTP_ADDREQ_FLAG_ADD);
	}

	// Set cookies
	if (!cookies.empty()) {
		std::wstring wCookies(cookies.begin(), cookies.end());
		WinHttpAddRequestHeaders(hRequest, wCookies.c_str(), static_cast<DWORD>(-1L), WINHTTP_ADDREQ_FLAG_ADD);
	}

	// Send the request
	BOOL bResult = WinHttpSendRequest(hRequest,
		WINHTTP_NO_ADDITIONAL_HEADERS, 0,
		(LPVOID)body.c_str(), static_cast<DWORD>(body.length()),
		static_cast<DWORD>(body.length()), 0);
	if (!bResult) {
		std::cerr << "Error: WinHttpSendRequest failed." << std::endl;
		WinHttpCloseHandle(hRequest);
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		return "";
	}

	// Receive the response
	bResult = WinHttpReceiveResponse(hRequest, nullptr);
	if (!bResult) {
		std::cerr << "Error: WinHttpReceiveResponse failed." << std::endl;
		WinHttpCloseHandle(hRequest);
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		return "";
	}

	// Read the response data
	DWORD dwSize = 0;
	DWORD dwDownloaded = 0;
	LPSTR pszOutBuffer;
	do {
		// Check for available data
		dwSize = 0;
		if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) {
			std::cerr << "Error: WinHttpQueryDataAvailable failed." << std::endl;
			break;
		}

		// Allocate space for the buffer
		pszOutBuffer = new char[dwSize + 1];
		if (!pszOutBuffer) {
			std::cerr << "Error: Out of memory." << std::endl;
			dwSize = 0;
			break;
		}

		// Read the data
		ZeroMemory(pszOutBuffer, dwSize + 1);
		if (!WinHttpReadData(hRequest, (LPVOID)pszOutBuffer, dwSize, &dwDownloaded)) {
			std::cerr << "Error: WinHttpReadData failed." << std::endl;
		}
		else {
			callback(pszOutBuffer);
			response.append(pszOutBuffer, dwDownloaded);
		}

		// Free the memory
		delete[] pszOutBuffer;

	} while (dwSize > 0);

	// Cleanup
	WinHttpCloseHandle(hRequest);
	WinHttpCloseHandle(hConnect);
	WinHttpCloseHandle(hSession);

	return response;
}
