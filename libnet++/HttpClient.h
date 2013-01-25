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

#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include <string>

#include "TcpClient.h"

namespace fritz {

class HttpClient : public TcpClient {
public:
	typedef std::vector<std::pair<std::string, std::string>> param_t;
	typedef std::map<std::string, std::string> header_t;
	typedef std::string body_t;
	typedef std::pair<header_t, body_t> response_t;
private:
	const header_t defaultHeader =
	  {
	    {"User-Agent", "Lynx/2.8.6" },
	    {"Connection", "Close" },
	    {"Host", host },
	  };
protected:
	std::string SendRequest(const std::string &request, const std::ostream &postdata = std::ostringstream(), const header_t &header = header_t());
	response_t ParseResponse();
public:
	HttpClient(const std::string &host, int port = 80);
	virtual ~HttpClient();
	static std::string GetURL(const std::string &url, const header_t &header = header_t());
	std::string Get     (const std::string &request, const param_t &params = param_t(), const header_t &header = header_t());
	std::string Post    (const std::string &request, const param_t &postdata,           const header_t &header = header_t());
	std::string PostMIME(const std::string &request, const param_t &postdata,           const header_t &header = header_t());
};

}

#endif /* HTTPCLIENT_H_ */
