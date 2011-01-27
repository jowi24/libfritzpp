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


#ifndef HTTPCLIENT_H_
#define HTTPCLIENT_H_

#include "cc++/url.h"

namespace fritz {

class HttpClient {
private:
	std::string host;
	int port;
	ost2::URLStream urlStream;
	ost2::URLStream::Error returnCode;
	std::string Result();
public:
	HttpClient(std::string host, int port);
	virtual ~HttpClient();
	std::string Get(const std::ostream& os);
	std::string Post(const std::ostream &url, const std::ostream &postdata);
};

}

#endif /* HTTPCLIENT_H_ */
