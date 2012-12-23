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

#include "cc++/url.h"
#include "TcpClient.h"

namespace fritz {

class HttpClient : public TcpClient {
private:
	ost2::URLStream *urlStream;
protected:
	HttpClient(std::string &host, int port, ost2::URLStream *stream);
	std::string Result();
    std::string BuildUrl(const std::ostream & url);
public:
	explicit HttpClient(std::string &host, int port = 80);
	virtual ~HttpClient();
	std::string Get(const std::ostream& os);
	std::string Post(const std::ostream &url, const std::ostream &postdata);
	std::string PostMIME(const std::ostream &url, ost2::MIMEMultipartForm &form);
};

}

#endif /* HTTPCLIENT_H_ */
