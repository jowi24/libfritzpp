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

#include "FritzClient.h"

#include <cstring>
#include <iomanip>
#include <gcrypt.h>

#include "Config.h"
#include "Tools.h"
#include <liblog++/Log.h>
#include <libconv++/CharsetConverter.h>

#define RETRY_BEGIN                                  \
    	unsigned int retry_delay = RETRY_DELAY / 2;  \
		bool dataRead = false;                       \
		do {  				                         \
			try {                                    \
				validPassword = login();                             \
				retry_delay = retry_delay > 1800 ? 3600 : retry_delay * 2;

#define RETRY_END																							\
			dataRead = true;                                                                                \
		} catch (std::runtime_error &re) {																\
			ERR("Exception in connection to " << gConfig->getUrl() << " - " << re.what());								\
			ERR("waiting " << retry_delay << " seconds before retrying");	\
			sleep(retry_delay); /* delay a possible retry */												\
		}																									\
	} while (!dataRead);

namespace fritz {

std::mutex* FritzClient::mutex = new std::mutex();

FritzClient::FritzClient()
: httpClient{gConfig->getUrl(), gConfig->getUiPort()} {
	validPassword = false;
	mutex->lock();
	// init libgcrypt
	gcry_check_version (nullptr);
    // disable secure memory
    gcry_control (GCRYCTL_DISABLE_SECMEM, 0);
    gcry_control (GCRYCTL_INITIALIZATION_FINISHED, 0);
    // init HttpClient
    soapClient = new network::SoapClient(gConfig->getUrl(), gConfig->getUpnpPort());
}

FritzClient::~FritzClient() {
	mutex->unlock();
}

std::string FritzClient::calculateLoginResponse(std::string challenge) {
	std::string challengePwd = challenge + '-' + gConfig->getPassword();
	// the box needs an md5 sum of the string "challenge-password"
	// to make things worse, it needs this in UTF-16LE character set
	// last but not least, for "compatibility" reasons (*LOL*) we have to replace
	// every char > "0xFF 0x00" with "0x2e 0x00"
	convert::CharsetConverter conv("", "UTF-16LE");
	char challengePwdConv[challengePwd.length()*2];
	memcpy(challengePwdConv, conv.convert(challengePwd).c_str(), challengePwd.length()*2);
	for (size_t pos=1; pos < challengePwd.length()*2; pos+= 2)
		if (challengePwdConv[pos] != 0x00) {
			challengePwdConv[pos] = 0x00;
			challengePwdConv[pos-1] = 0x2e;
		}
	unsigned char hash[16];
	gcry_md_hash_buffer(GCRY_MD_MD5, hash, (const char*)challengePwdConv, challengePwd.length()*2);
	std::stringstream response;
	response << challenge << '-';
	for (size_t pos=0; pos < 16; pos++)
		response << std::hex << std::setfill('0') << std::setw(2) << (unsigned int)hash[pos];
	return response.str();
}

std::string FritzClient::urlEncode(const std::string &s_input) {
	std::string result;
	std::string s;
	std::string hex = "0123456789abcdef";
	convert::CharsetConverter conv("", "ISO-8859-15");
	s = conv.convert(s_input);
	for (unsigned int i=0; i<s.length(); i++) {
		if( ('a' <= s[i] && s[i] <= 'z')
				|| ('A' <= s[i] && s[i] <= 'Z')
				|| ('0' <= s[i] && s[i] <= '9') ) {
			result += s[i];
		} else {
			result += '%';
			result += hex[(unsigned char) s[i] >> 4];
			result += hex[(unsigned char) s[i] & 0x0f];
		}
	}
	return result;
}

bool FritzClient::login() {
	// when using SIDs, a new login is only needed if the last request was more than 5 minutes ago
	if ((gConfig->getLoginType() == Config::SID || gConfig->getLoginType() == Config::LUA) && (time(nullptr) - gConfig->getLastRequestTime() < 300)) {
		return true;
	}

	// detect type of login once
	std::string sXml; // sXml is used twice!
	if (gConfig->getLoginType() == Config::UNKNOWN || gConfig->getLoginType() == Config::SID || gConfig->getLoginType() == Config::LUA) {
		// detect if this Fritz!Box uses SIDs
		DBG("requesting login_sid.lua from Fritz!Box.");
		// might return 404 with older fw-versions, our httpClient throws a SockeException for this, catched here
		try {
		  sXml = httpClient.get("/login_sid.lua", {{"sid", gConfig->getSid()}});
		} catch (std::runtime_error &re) {}
		if (sXml.find("<Rights") != std::string::npos)
			gConfig->setLoginType(Config::LUA);
		else {
			DBG("requesting login_sid.xml from Fritz!Box.");
			sXml = httpClient.get("/cgi-bin/webcm", {{"getpage", "../html/login_sid.xml"}});
			if (sXml.find("<iswriteaccess>") != std::string::npos)
				gConfig->setLoginType(Config::SID);
			else
				gConfig->setLoginType(Config::PASSWORD);
		}
	}

	if (gConfig->getLoginType() == Config::SID || gConfig->getLoginType() == Config::LUA) {
		std::stringstream loginPath;
		if (gConfig->getLoginType() == Config::LUA) {
			loginPath << "/login_sid.lua";
		} else {
			loginPath << "/cgi-bin/webcm";
		}
		// check if no password is needed (SID is directly available)
		size_t sidStart = sXml.find("<SID>");
		if (sidStart == std::string::npos) {
			ERR("Error - Expected field <SID> not found in login_sid.xml or login_sid.lua.");
			return false;
		}
		sidStart += 5;
		std::string sid = sXml.substr(sidStart, 16);
		if (sid.compare("0000000000000000") != 0) {
			// save SID
			DBG("SID is still valid - all ok.");
			gConfig->setSid(sid);
			gConfig->updateLastRequestTime();
			return true;
		} else {
			DBG("We need to log in.");
			// generate response out of challenge and password
			size_t challengeStart = sXml.find("<Challenge>");
			if (challengeStart == std::string::npos) {
				ERR("Error - Expected <Challenge> not found in login_sid.xml or login_sid.lua.");
				return false;
			}
			challengeStart += 11;
			size_t challengeStop = sXml.find("<", challengeStart);
            std::string challenge = sXml.substr(challengeStart, challengeStop - challengeStart);
            std::string response = calculateLoginResponse(challenge);
            // send response to box
			std::string sMsg;

			network::HttpClient::param_t postdata;
			if (gConfig->getLoginType() == Config::SID)
				postdata = {{"login:command/response", response},
				            {"getpage", "../html/de/menus/menu2.html"}};
			else
                postdata = {{"username", gConfig->getUsername()}, {"response", response }};

            DBG("Sending login request "
             << ( gConfig->getUsername().size() ? "for user " : "" )
             << gConfig->getUsername() << "...");

            sMsg = httpClient.post(loginPath.str(), postdata);
			size_t sidStart, sidStop;
			if (gConfig->getLoginType() == Config::SID) {
				sidStart = sMsg.find("name=\"sid\"");
				if (sidStart == std::string::npos) {
					ERR("Error - Expected sid field not found.");
					return false;
				}
				sidStart = sMsg.find("value=\"", sidStart + 10) + 7;
				sidStop = sMsg.find("\"", sidStart);
			} else {
				sidStart = sMsg.find("<SID>");
				if (sidStart == std::string::npos) {
					ERR("Error - Expected sid field not found.");
					return false;
				}
				sidStart += 5;
				sidStop = sMsg.find("</SID>");
			}
			// save SID
			gConfig->setSid(sMsg.substr(sidStart, sidStop-sidStart));
			if (gConfig->getSid().compare("0000000000000000") != 0) {
				DBG("login successful.");
				gConfig->updateLastRequestTime();
				return true;
			} else {
				ERR("login failed, check your password settings!.");
				return false;
			}
		}
	}
	if (gConfig->getLoginType() == Config::PASSWORD) {
		DBG("logging into fritz box using old scheme without SIDs.");
		// no password, no login
		if ( gConfig->getPassword().length() == 0)
			return true; //TODO: check if box really doesn't need a password

		std::string sMsg;

		sMsg = httpClient.post("/cgi-bin/webcm",
				               {{"login:command/password", urlEncode(gConfig->getPassword())}});

		// determine if login was successful
		if (sMsg.find("class=\"errorMessage\"") != std::string::npos) {
			ERR("login failed, check your password settings.");
			return false;
		}
		DBG("login successful.");
		return true;
	}
	return false;
}

std::string FritzClient::getLang() {
	if ( gConfig && gConfig->getLang().size() == 0) {
		std::vector<std::string> langs;
		langs.push_back("en");
		langs.push_back("de");
		langs.push_back("fr");
		for (auto lang : langs) {
			std::string sMsg;
			sMsg = httpClient.get("/cgi-bin/webcm",
					{
							{"getpage", "../html/" + lang + "/menus/menu2.html"},
							{"sid", gConfig->getSid() }
					});
			if (sMsg.find("<html>") != std::string::npos) {
				gConfig->setLang(lang);
				DBG("interface language is " << gConfig->getLang().c_str());
				return gConfig->getLang();
			}
		}
		DBG("error parsing interface language, assuming 'de'");
		gConfig->setLang("de");
	}
	return gConfig->getLang();
}

bool FritzClient::initCall(std::string &number) {
	std::string msg;
	if (number.length() == 0)
		return false;
	if (!login())
		return false;
	try {
		INF("sending call init request " << (gConfig->logPersonalInfo() ? number.c_str() : HIDDEN));
		network::HttpClient::param_t params =
		{
	      { "getpage", "../html/" + getLang() + "/menus/menu2.html" },
		  { "var%3Apagename", "fonbuch" },
		  { "var%3Amenu", "home" },
		  { "telcfg%3Acommand/Dial", number },
	      { "sid", gConfig->getSid() }
		};
		msg = httpClient.post("/cgi-bin/webcm", params);
		INF("call initiated.");
	} catch (std::runtime_error &re) {
		ERR("Exception - " << re.what());
		return false;
	}
	return true;
}

std::string FritzClient::requestLocationSettings() {
	std::string msg;

	RETRY_BEGIN {
		if (gConfig->getSid().size()) {
			DBG("Looking up Phone Settings (using lua)...");
			try {
				msg = httpClient.get("/fon_num/sip_option.lua", {{"sid", gConfig->getSid()}});
			} catch (std::runtime_error &re) {}
			if (msg.find("<!-- pagename:/fon_num/sip_option.lua-->") != std::string::npos)
				return msg;
			DBG("failed.");
		}

		DBG("Looking up Phone Settings (using webcm)...");
		msg = httpClient.get("/cgi-bin/webcm",
				{
						{ "getpage", "../html/" + getLang() + "/menus/menu2.html" },
						{ "var%3Alang", getLang() },
						{ "var%3Apagename", "sipoptionen" },
						{ "var%3Amenu", "fon" },
						{ "sid", gConfig->getSid() },
				});
	} RETRY_END
	return msg;
}

std::string FritzClient::requestSipSettings() {
	std::string msg;

	RETRY_BEGIN {
		if (gConfig->getSid().size()) {
			DBG("Looking up SIP Settings (using lua)...");
			try {
				msg = httpClient.get("/fon_num/fon_num_list.lua", {{"sid", gConfig->getSid()}});
			} catch (std::runtime_error &re) {}
			if (msg.find("<!-- pagename:/fon_num/fon_num_list.lua-->") != std::string::npos)
				return msg;
			DBG("failed.");
		}

		DBG("Looking up SIP Settings (using webcm)...");
		msg = httpClient.get("/cgi-bin/webcm",
				{
						{ "getpage", "../html/" + getLang() + "/menus/menu2.html" },
						{ "var%3Alang", getLang() },
						{ "var%3Apagename", "siplist" },
						{ "var%3Amenu", "fon" },
						{ "sid", gConfig->getSid() },
				});
	} RETRY_END
	return msg;
}

std::string FritzClient::requestCallList () {
	std::string msg = "";
	std::string csv = "";
	RETRY_BEGIN {
		// now, process call list
		DBG("sending callList update request.");
		// force an update of the fritz!box csv list and wait until all data is received
		msg = httpClient.get("/cgi-bin/webcm",
				{
						{ "getpage", "../html/" + getLang() + "/menus/menu2.html" },
						{ "var%3Alang", getLang() },
						{ "var%3Apagename", "foncall" },
						{ "var%3Amenu", "fon" },
						{ "sid", gConfig->getSid() },
				});
		// new method to request call list (FW >= xx.05.50?)
		try {
			DBG("sending callList request (using lua)...");
			csv = httpClient.get("/fon_num/foncalls_list.lua",
					{
							{ "csv", "" },
							{ "sid", gConfig->getSid() },
					});
			if (csv.find("Typ;Datum;Name;") != std::string::npos) {
				return csv;
			}
		} catch (std::runtime_error &re) {}

		// old method, parsing url to csv from page above

		// get the URL of the CSV-File-Export
		unsigned int urlPos   = msg.find(".csv");
		unsigned int urlStop  = msg.find('"', urlPos);
		unsigned int urlStart = msg.rfind('"', urlPos) + 1;
		std::string csvUrl    = msg.substr(urlStart, urlStop-urlStart);
		// retrieve csv list
		DBG("sending callList request (using webcm)...");
		csv = httpClient.get("/cgi-bin/webcm",
				{
						{ "getpage", csvUrl },
						{ "sid", gConfig->getSid() },
				});

		// convert answer to current SystemCodeSet (we assume, Fritz!Box sends its answer in latin15)
		convert::CharsetConverter conv("ISO-8859-15");
		csv = conv.convert(csv);
	} RETRY_END
	return csv;
}

std::string FritzClient::requestFonbook () {
	std::string msg;
	// new method, returns an XML
	RETRY_BEGIN {
		if (gConfig->getSid().length()) {
			network::HttpClient::param_t postdata =
			{
					{ "sid", gConfig->getSid() },
					{ "PhonebookId", "0" },
					{ "PhonebookExportName", "Telefonbuch" },
					{ "PhonebookExport", "" }
			};
			DBG("sending fonbook XML request.");
			try {
				msg = httpClient.postMIME("/cgi-bin/firmwarecfg", postdata);
			} catch (std::runtime_error &re) {}
			if (msg.find("<phonebooks>") != std::string::npos) {
				return msg;
			}
		}

	// use old fashioned website (for old FW versions)
		DBG("sending fonbook HTML request.");
		msg = httpClient.get("/cgi-bin/webcm",
				{
						{ "getpage", "../html/" + getLang() + "/menus/menu2.html" },
						{ "var%3Alang", getLang() },
						{ "var%3Apagename", "fonbuch" },
						{ "var%3Amenu", "fon" },
						{ "sid", gConfig->getSid() },
				});
	} RETRY_END

	return msg;
}

void FritzClient::writeFonbook(std::string xmlData) {
	std::string msg;
	DBG("Saving XML Fonbook to FB...");
	RETRY_BEGIN {
		network::HttpClient::param_t postdata =
		{
				{ "sid", gConfig->getSid() },
				{ "PhonebookId", "0" },
				{ "PhonebookImportFile\"; filename=\"FRITZ.Box_Telefonbuch.xml", xmlData }
		};
		msg = httpClient.postMIME("/cgi-bin/firmwarecfg", postdata);
	} RETRY_END
}


bool FritzClient::reconnectISP() {
	std::string msg;
	DBG("Sending reconnect request to FB.");
	try {
		msg = soapClient->post(
				"/upnp/control/WANIPConn1",
				"urn:schemas-upnp-org:service:WANIPConnection:1#ForceTermination",
				"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
				"<s:Envelope s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\">"
				"<s:Body>"
				"<u:ForceTermination xmlns:u=\"urn:schemas-upnp-org:service:WANIPConnection:1\" />"
				"</s:Body>"
				"</s:Envelope>");
	} catch (std::runtime_error &re) {
		ERR("Exception in connection to " << gConfig->getUrl() << " - " << re.what());
		return false;
	}
	if (msg.find("ForceTerminationResponse") == std::string::npos)
		return false;
	else
		return true;
}

std::string FritzClient::getCurrentIP() {
	std::string msg;
	DBG("Sending reconnect request to FB.");
	try {
		msg = soapClient->post(
				"/upnp/control/WANIPConn1",
				"urn:schemas-upnp-org:service:WANIPConnection:1#GetExternalIPAddress",
				"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
				"<s:Envelope s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\">"
				"<s:Body>"
				"<u:GetExternalIPAddress xmlns:u=\"urn:schemas-upnp-org:service:WANIPConnection:1\" />"
				"</s:Body>"
				"</s:Envelope>");
	} catch (std::runtime_error &re) {
		ERR("Exception in connection to " << gConfig->getUrl() << " - " << re.what());
		return "";
	}
	DBG("Parsing reply...");
	std::string::size_type start = msg.find("<NewExternalIPAddress>");
	std::string::size_type stop = msg.find("</NewExternalIPAddress>");
	if (start != std::string::npos && stop != std::string::npos) {
		std::string ip = msg.substr(start + 22, stop - start - 22);
		DBG("Current ip is: " << ip);
		return ip;
	} else {
		ERR("Error parsing response in getCurrentIP().");
	}
	return "";
}

//TODO: update lastRequestTime with any request
}
