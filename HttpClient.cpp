/*
 * libfritz++
 *
 * Copyright (C) 2007-2010 Joachim Wilke <libfritz@joachim-wilke.de>
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
#include "Config.h"

namespace fritz {

HttpClient::HttpClient(std::string host, int port) {
	this->host = host;
	this->port = port;

}

HttpClient::~HttpClient() {
	// TODO Auto-generated destructor stub
}

std::string HttpClient::Result() {
    std::string response;
	while (!urlStream.eof())  {
	  char buffer[1024];
	  urlStream.read(buffer, sizeof(buffer)-1);
	  buffer[urlStream.gcount()] = 0;
	  response += buffer;
	}
	urlStream.close();
	DBG("Result size: " << response.length());
	return response;
}

std::string HttpClient::Get(const std::ostream& url) {
	std::stringstream request;
	request << "http://" <<  host << ":" << port << url.rdbuf(); //todo: url must start with '/'

	returnCode = urlStream.get(request.str().c_str());

	return Result();
}

std::string HttpClient::Post(const std::ostream &url, const std::ostream &postdata) {
	std::stringstream request;
	request << "http://" <<  host << ":" << port << url.rdbuf();
	DBG("Post request: " << request.str() );

	const std::stringstream &payload = static_cast<const std::stringstream&>(postdata);
	DBG("Post data: " << payload.str() );
	char param0[payload.str().size()+1];
	strcpy(param0, payload.str().c_str());
	const char * params[2];
	params[0] = param0;
	params[1] = 0;

	returnCode = urlStream.post(request.str().c_str(), params);
	DBG("UrlStream return code: " << returnCode );

	return Result();
}

}
