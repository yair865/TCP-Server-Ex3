#pragma once
#include <string>
#include "RequestType.h"

class HttpParser {
public:
	HttpParser();
	~HttpParser();

	std::string extractResource(const std::string& request);
	std::string extractQueryParam(const std::string& request, const std::string& param);
	std::string extractHeader(const std::string& request, const std::string& header);
	std::string extractBody(const std::string& request);
	RequestType extractMethodType(const std::string& request);
	int extractLen(const std::string& request);
};