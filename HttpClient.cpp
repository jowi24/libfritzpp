/*
 * libfritz++
 *
 * Copyright (C) 2007-2012 Joachim Wilke <libfritz@joachim-wilke.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "HttpClient.h"

#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>

#include "Config.h"
#include "Log.h"

namespace fritz {

HttpClient::HttpClient(std::string &host, int port)
: TcpClient{host, port} {
}

HttpClient::~HttpClient() {
}

HttpClient::response_t HttpClient::ParseResponse() {
	bool isHeader = true;
	header_t header;
	body_t body;

	std::stringstream bodystream;
	std::string status = ReadLine(true);
	DBG("Response: " << status);

	for (;;) {
		std::string line = ReadLine(false);
		if (line.length() == 0) {
			break;
		} else if (line.length() <= 2) {
			isHeader = false;
			continue;
		}
		if (isHeader) {
			size_t separator = line.find(':');
			if (separator == std::string::npos)
				throw std::runtime_error("Invalid header format detected in HTTP response.");
			std::string key = line.substr(0, separator);
			std::string value = line.substr(separator+2);
			header.insert(std::pair<std::string, std::string>(key, value));
			DBG("Found header: " << key << ": " << value);
		} else {
			bodystream << line;
		}
	}
	body = bodystream.str();
	DBG("Body size " << body.length() << " Bytes.");
	return response_t(header, body);
}

std::string HttpClient::SendRequest(const std::ostream &request, const std::ostream &postdata, const param_t &header) {
	if (!connected)
		Connect();
	std::stringstream post;
	post << postdata.rdbuf();
	int postContentLength = post.str().length();
	std::string method = postContentLength ? "POST" : "GET";

	std::stringstream req;
	req << request.rdbuf();

	DBG("Requesting HTTP " << method << " on " << req.str());
	*stream << method << " " << req.str() << " HTTP/1.0\r\n";
	for (auto entry : defaultHeader) {
		*stream << entry.first << ": " << entry.second << "\r\n";
	}
	for (auto entry : header) {
		*stream << entry.first << ": " << entry.second << "\r\n";
	}
	if (postContentLength)
		*stream << "Content-Length: " << postContentLength << "\r\n"
		       << "\r\n" << post.str() << "\r\n" << std::flush;
	else
		*stream << "\r\n" << std::flush;

	response_t response = ParseResponse();
	Disconnect();

	// check for redirection
	header_t responseHeader = response.first;
	if (responseHeader["Location"].length() > 0) {
		DBG("Redirect requested to " << responseHeader["Location"]);
		return GetURL(responseHeader["Location"]);
	}
	return response.second;
}

std::string HttpClient::Get(const std::string& url, const param_t &header) {
	return SendRequest(std::stringstream(url), std::ostringstream(), header);
}

std::string HttpClient::Get(const std::ostream& url, const param_t &header) {
	std::stringstream ss;
	ss << url.rdbuf();
	return Get(ss.str(), header);
}

std::string HttpClient::Post(const std::string &request, param_t &postdata, const param_t &header) {
	std::stringstream ss;
	for (auto parameter : postdata) {
		ss << parameter.first << "=" << parameter.second << "&";
	}
	return Post(std::stringstream(request), ss, header);
}

std::string HttpClient::Post(const std::ostream &request, const std::ostream &postdata, const param_t &header) {
	return SendRequest(request, postdata, header);
}

std::string HttpClient::GetURL(const std::string &url, const param_t &header) {
	//TODO support other port
	//TODO support HTTPS

	size_t protoMarker = url.find("://");
	size_t hostMarker  = url.find("/", protoMarker+4);
	if (protoMarker == std::string::npos || hostMarker == std::string::npos)
		throw std::runtime_error("Invalid url.");

	std::string proto = url.substr(0, protoMarker);
	std::string host  = url.substr(protoMarker+3, hostMarker-protoMarker-3);
	std::string request = url.substr(hostMarker);
	if (proto.compare("http") != 0)
		throw std::runtime_error("Invalid protocol in url.");

	HttpClient client(host);
	return client.Get(request, header);
}

std::string HttpClient::PostMIME(const std::string &request, const param_t &postdata, const param_t &header) {
	return PostMIME(std::stringstream(request), postdata, header);
}

std::string HttpClient::PostMIME(const std::ostream &request, const param_t &postdata, const param_t &header) {
	const std::string boundary = "---FormBoundaryZMsGfL5JxTz5LuAW";
	header_t fullheader = {
			{ "Content-Type", "multipart/form-data; boundary=" + boundary }
	};
	fullheader.insert(begin(header), end(header));

	std::stringstream ss;
	for (auto parameter : postdata) {
		ss << boundary << "\r\n"
		   << "Content-Disposition: form-data; name=\""+parameter.first+"\"\r\n"
		   << "\r\n"
		   << parameter.second
		   << "\r\n";
	}
	ss << boundary << "\r\n";
	return Post(request, ss, fullheader);
}


}
