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

#ifndef FRITZCLIENT_H
#define FRITZCLIENT_H

#include <cstdlib>
#include <mutex>

#include <libnet++/SoapClient.h>
#include <libnet++/HttpClient.h>

namespace fritz {

class FritzClient {
private:
	static std::mutex *mutex;
    std::string CalculateLoginResponse(std::string challenge);
	std::string UrlEncode(std::string &s);
	bool Login();
	std::string GetLang();
	bool validPassword;
	HttpClient httpClient;
	SoapClient *soapClient;
public:
	FritzClient ();
	virtual ~FritzClient();
	virtual bool InitCall(std::string &number);
	virtual std::string RequestLocationSettings();
	virtual std::string RequestSipSettings();
	virtual std::string RequestCallList();
	virtual std::string RequestFonbook();
	virtual void WriteFonbook(std::string xmlData);
	virtual bool hasValidPassword() { return validPassword; }
	virtual bool reconnectISP();
	virtual std::string getCurrentIP();
};

class FritzClientFactory {
public:
	virtual ~FritzClientFactory() {}

	virtual FritzClient *create() {
		return new FritzClient;
	}
};

}

#endif /* FRITZCLIENT_H_ */
