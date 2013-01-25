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

#include <liblog++/Log.h>

namespace fritz {

HttpClient::HttpClient(const std::string &host, int port)
: TcpClient{host, port} {
}

HttpClient::~HttpClient() {
}

HttpClient::response_t HttpClient::ParseResponse() {
	header_t header;
	body_t body;

    std::string http_version;
    *stream >> http_version;
    unsigned int status_code;
    *stream >> status_code;
    std::string status_message;
    std::getline(*stream, status_message);
    if (!(*stream) || http_version.substr(0, 5) != "HTTP/")
      throw std::runtime_error("Invalid response");
    DBG("HTTP status code " << status_code);

    // Process the response headers, which are terminated by a blank line
    std::string headerline;
    while (std::getline(*stream, headerline) && headerline != "\r") {
    	size_t separator = headerline.find(':');
    	if (separator == std::string::npos)
    		throw std::runtime_error("Invalid header format detected in HTTP response.");
    	std::string key = headerline.substr(0, separator);
    	std::string value = headerline.substr(separator+2);
    	header.insert(std::pair<std::string, std::string>(key, value));
    	DBG("Found header: " << key << ": " << value);
    }

    // The remaining data is the body
    std::stringstream bodystream;
    bodystream << stream->rdbuf();
    body = bodystream.str();
	DBG("Body size " << body.length() << " Bytes.");
	return response_t(header, body);
}

std::string HttpClient::SendRequest(const std::string &request, const std::ostream &postdata, const header_t &header) {
	if (!connected)
		Connect();
	std::stringstream post;
	post << postdata.rdbuf();
	int postContentLength = post.str().length();
	std::string method = postContentLength ? "POST" : "GET";

	DBG("Requesting HTTP " << method << " on " << request);
	*stream << method << " " << request << " HTTP/1.0\r\n";
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

std::string HttpClient::Get(const std::string& url, const param_t &params, const header_t &header) {
	std::stringstream ss;
	if (url.find('?') == std::string::npos)
		ss << url << "?";
	else
		ss << url << "&";
	for (auto parameter: params)
		ss << parameter.first << "=" << parameter.second << "&";
	return SendRequest(ss.str(), std::ostringstream(), header);
}

std::string HttpClient::Post(const std::string &request, const param_t &postdata, const header_t &header) {
	header_t fullheader = {
			{ "Content-Type", "application/x-www-form-urlencoded" }
	};
	fullheader.insert(begin(header), end(header));

	std::stringstream ss;
	for (auto parameter : postdata)
			ss << parameter.first << "=" << parameter.second << "&";

	return SendRequest(request, ss, fullheader);
}

std::string HttpClient::GetURL(const std::string &url, const header_t &header) {
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
	return client.Get(request, param_t(), header);
}

std::string HttpClient::PostMIME(const std::string &request, const param_t &postdata, const header_t &header) {

	const std::string boundary = "----FormBoundaryZMsGfL5JxTz5LuAW";
	header_t fullheader = {
			{ "Content-Type", "multipart/form-data; boundary=" + boundary }
	};
	fullheader.insert(begin(header), end(header));

	std::stringstream ss;
	for (auto parameter : postdata) {
		ss << "--" << boundary << "\r\n"
		   << "Content-Disposition: form-data; name=\""+parameter.first+"\"\r\n"
		   << "\r\n"
		   << parameter.second
		   << "\r\n";
	}
	ss << "--" << boundary << "--\r\n";
	return SendRequest(request, ss, fullheader);
}

}
