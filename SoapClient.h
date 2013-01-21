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

#ifndef SOAPCLIENT_H
#define SOAPCLIENT_H

#include "HttpClient.h"

namespace fritz {

class SoapClient : public HttpClient {
private:
	std::string soapAction;
public:
	explicit SoapClient(const std::string &host, int port = 80);
	virtual ~SoapClient();
	std::string Post(const std::string &request, const std::string &action, const std::string &body);

};

}

#endif /* SOAPCLIENT_H_ */
