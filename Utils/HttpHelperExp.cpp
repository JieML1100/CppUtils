#include <iostream>
#include <string>
#include <regex>
#include <sstream>
#include <iomanip>
#include <ws2tcpip.h>
#include "HttpHelper.h"
#pragma comment(lib, "Ws2_32.lib")
std::string HttpHelper::HttpRequest(const std::string& host, const std::string& port,
	const std::string& path, const std::string& method,
	const std::string& data) {
	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo* result = nullptr, hints;
	int iResult;
	std::string response;

	// 初始化Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		std::cerr << "WSAStartup失败: " << iResult << std::endl;
		return "";
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;        // 支持IPv4或IPv6
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;    // 使用TCP协议

	// 解析服务器地址和端口
	iResult = getaddrinfo(host.c_str(), port.c_str(), &hints, &result);
	if (iResult != 0) {
		std::cerr << "getaddrinfo失败: " << iResult << std::endl;
		WSACleanup();
		return "";
	}

	// 尝试连接到第一个解析出的地址
	struct addrinfo* ptr = result;
	ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
	if (ConnectSocket == INVALID_SOCKET) {
		std::cerr << "socket创建失败: " << WSAGetLastError() << std::endl;
		freeaddrinfo(result);
		WSACleanup();
		return "";
	}

	// 连接到服务器
	iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		closesocket(ConnectSocket);
		ConnectSocket = INVALID_SOCKET;
		std::cerr << "无法连接到服务器!" << std::endl;
		freeaddrinfo(result);
		WSACleanup();
		return "";
	}

	freeaddrinfo(result);

	// 准备HTTP请求
	std::string request;
	if (method == "GET") {
		request = method + " " + path + " HTTP/1.1\r\n";
		request += "Host: " + host + "\r\n";
		request += "Connection: close\r\n\r\n";
	}
	else if (method == "POST") {
		request = method + " " + path + " HTTP/1.1\r\n";
		request += "Host: " + host + "\r\n";
		request += "Content-Type: application/x-www-form-urlencoded\r\n";
		request += "Content-Length: " + std::to_string(data.length()) + "\r\n";
		request += "Connection: close\r\n\r\n";
		request += data;
	}
	else {
		std::cerr << "无效的HTTP方法: " << method << std::endl;
		closesocket(ConnectSocket);
		WSACleanup();
		return "";
	}

	// 发送HTTP请求
	iResult = send(ConnectSocket, request.c_str(), (int)request.length(), 0);
	if (iResult == SOCKET_ERROR) {
		std::cerr << "发送失败: " << WSAGetLastError() << std::endl;
		closesocket(ConnectSocket);
		WSACleanup();
		return "";
	}

	// 接收HTTP响应
	const int bufSize = 512;
	char recvbuf[bufSize];
	do {
		iResult = recv(ConnectSocket, recvbuf, bufSize, 0);
		if (iResult > 0) {
			response.append(recvbuf, iResult);
		}
		else if (iResult == 0) {
			// 连接关闭
			break;
		}
		else {
			std::cerr << "接收失败: " << WSAGetLastError() << std::endl;
			break;
		}
	} while (iResult > 0);

	// 清理资源
	closesocket(ConnectSocket);
	WSACleanup();

	return response;
}
static void ParseURL(const std::string& url, std::string& host, std::string& port, std::string& path) {
	// 使用正则表达式解析URL
	std::regex url_regex(R"(^(http://|https://)?([^:/\s]+)(:([0-9]+))?([^?\s]*)?.*$)",
		std::regex::icase);
	std::smatch url_match_result;

	if (std::regex_match(url, url_match_result, url_regex)) {
		std::string protocol = url_match_result[1].str();
		host = url_match_result[2].str();
		port = url_match_result[4].str();
		path = url_match_result[5].str();

		if (path.empty())
			path = "/";

		if (port.empty()) {
			if (!protocol.empty() && protocol == "https://")
				port = "443";  // HTTPS默认端口
			else
				port = "80";   // HTTP默认端口
		}

		// 检查是否为HTTPS协议
		if (!protocol.empty() && protocol == "https://") {
			// 注意：此示例不支持HTTPS协议
			std::cerr << "当前示例不支持HTTPS协议。" << std::endl;
		}
	}
	else {
		std::cerr << "URL解析失败。" << std::endl;
	}
}
static void ParseHttpResponse(const std::string& response, std::string& header, std::string& body) {
	// 查找header和body之间的分隔符（\r\n\r\n）
	size_t pos = response.find("\r\n\r\n");
	if (pos != std::string::npos) {
		// 提取header
		header = response.substr(0, pos);
		// 提取body（包括可能的chunked编码）
		std::string raw_body = response.substr(pos + 4);

		// 检查是否为chunked编码
		std::regex transfer_encoding_regex(R"(Transfer-Encoding:\s*chunked)", std::regex::icase);
		if (std::regex_search(header, transfer_encoding_regex)) {
			// 解析chunked编码的body
			body = "";
			std::istringstream stream(raw_body);
			std::string line;
			while (std::getline(stream, line)) {
				// 移除可能的\r字符
				if (!line.empty() && line.back() == '\r')
					line.pop_back();

				// 如果行为空，则跳过
				if (line.empty())
					continue;

				// 解析chunk大小
				std::istringstream size_stream(line);
				size_t chunk_size;
				size_stream >> std::hex >> chunk_size;

				if (chunk_size == 0) {
					// 读取到最后一个chunk，结束
					break;
				}

				// 读取chunk数据
				std::string chunk_data(chunk_size, '\0');
				stream.read(&chunk_data[0], chunk_size);
				body += chunk_data;

				// 读取\r\n
				stream.get();
				if (stream.peek() == '\n')
					stream.get();
			}
		}
		else {
			// 非chunked编码，直接将raw_body赋值给body
			body = raw_body;
		}
	}
	else {
		// 未找到分隔符，视为没有header
		header = "";
		body = response;
	}
}
static int GetStatusCode(const std::string& header) {
	std::regex status_line_regex(R"(HTTP/\d\.\d\s+(\d{3}))");
	std::smatch match;
	if (std::regex_search(header, match, status_line_regex)) {
		return std::stoi(match[1]);
	}
	return -1;
}
static std::string GetLocation(const std::string& header) {
	std::regex location_regex(R"(Location:\s*(.+))", std::regex::icase);
	std::smatch match;
	if (std::regex_search(header, match, location_regex)) {
		// 移除可能的\r字符
		std::string location = match[1];
		location.erase(std::remove(location.begin(), location.end(), '\r'), location.end());
		return location;
	}
	return "";
}
#include "../Utils/Utils.h"
std::string HttpHelper::HttpRequestGet(const std::string& url) {
	const int redirect_limit = 10;
	std::string current_url = url;
	int redirects = 0;

	while (redirects <= redirect_limit) {
		std::string host, port, path;
		ParseURL(current_url, host, port, path);
		auto response = HttpRequest(host, port, path, "GET", "");
		std::string header, body;
		ParseHttpResponse(response, header, body);

		// 获取HTTP状态码
		int status_code = GetStatusCode(header);

		// 检查是否需要重定向
		if (status_code >= 300 && status_code < 400) {
			std::string location = GetLocation(header);
			if (location.empty()) {
				std::cerr << "重定向时未找到Location头部，停止。" << std::endl;
				return "";
			}

			// 处理相对URL
			if (location.find("http://") != 0 && location.find("https://") != 0) {
				// 如果Location是相对路径，构建完整的URL
				if (location[0] != '/') {
					location = "/" + location;
				}
				location = "http://" + host + location;
			}

			// 更新current_url，准备下一次请求
			current_url = location;
			redirects++;
			continue; // 继续下一个循环
		}
		else {
			// 非重定向状态码，处理响应内容

			// 解析头部，检查Content-Encoding
			auto headers = StringHelper::Split(header, { '\r', '\n' });
			for (const auto& h : headers) {
				if (h.find("Content-Encoding: gzip") != std::string::npos) {
					// 调用GDecompress解压缩
					auto dec = GDecompress((uint8_t*)body.data(), body.size());
					body = std::string((char*)dec.data(), dec.size());
					break;
				}
			}

			return body;
		}
	}

	std::cerr << "重定向次数超过限制（" << redirect_limit << "次），停止。" << std::endl;
	return "";
}
std::string HttpHelper::HttpRequestPost(const std::string& url, const std::string& data) {
	std::string host, port, path;
	ParseURL(url, host, port, path);
	return HttpRequest(host, port, path, "POST", data);
}