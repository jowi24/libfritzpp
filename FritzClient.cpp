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
#include "Log.h"

#define RETRY_BEGIN                                  \
	    ost::Thread::setException(ost::Thread::throwException); \
    	unsigned int retry_delay = RETRY_DELAY / 2;  \
		bool dataRead = false;                       \
		do {  				                         \
			try {                                    \
				validPassword = Login();                             \
				retry_delay = retry_delay > 1800 ? 3600 : retry_delay * 2;

#define RETRY_END																							\
			dataRead = true;                                                                                \
		} catch (ost::SockException &se) {																\
			ERR("Exception in connection to " << gConfig->getUrl() << " - " << se.what());								\
			ERR("waiting " << retry_delay << " seconds before retrying");	\
			sleep(retry_delay); /* delay a possible retry */												\
		}																									\
	} while (!dataRead);

namespace fritz {

std::mutex* FritzClient::mutex = new std::mutex();

FritzClient::FritzClient() {
	validPassword = false;
	mutex->lock();
	// init libgcrypt
	gcry_check_version (NULL);
    // disable secure memory
    gcry_control (GCRYCTL_DISABLE_SECMEM, 0);
    gcry_control (GCRYCTL_INITIALIZATION_FINISHED, 0);
    // init HttpClient
    httpClient = new HttpClient(gConfig->getUrl(), gConfig->getUiPort());
    soapClient = new SoapClient(gConfig->getUrl(), gConfig->getUpnpPort());
}

FritzClient::~FritzClient() {
	delete httpClient;
	mutex->unlock();
}

std::string FritzClient::CalculateLoginResponse(std::string challenge) {
	std::string challengePwd = challenge + '-' + gConfig->getPassword();
	// the box needs an md5 sum of the string "challenge-password"
	// to make things worse, it needs this in UTF-16LE character set
	// last but not least, for "compatibility" reasons (*LOL*) we have to replace
	// every char > "0xFF 0x00" with "0x2e 0x00"
	CharSetConv conv(NULL, "UTF-16LE");
	char challengePwdConv[challengePwd.length()*2];
	memcpy(challengePwdConv, conv.Convert(challengePwd.c_str()), challengePwd.length()*2);
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

std::string FritzClient::UrlEncode(std::string &s_input) {
	std::string result;
	std::string s;
	std::string hex = "0123456789abcdef";
	CharSetConv *conv = new CharSetConv(CharSetConv::SystemCharacterTable(), "ISO-8859-15");
	s = conv->Convert(s_input.c_str());
	delete(conv);
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
//	TODO: With introduction of libccgnu2, this implementation could be replaced by
//	char result[4*s_input.length()];
//	ost::urlEncode(s_input.c_str(), result, sizeof(result));
	return result;
}

bool FritzClient::Login() {
	// when using SIDs, a new login is only needed if the last request was more than 5 minutes ago
	if ((gConfig->getLoginType() == Config::SID || gConfig->getLoginType() == Config::LUA) && (time(NULL) - gConfig->getLastRequestTime() < 300)) {
		return true;
	}

	// detect type of login once
	std::string sXml; // sXml is used twice!
	if (gConfig->getLoginType() == Config::UNKNOWN || gConfig->getLoginType() == Config::SID || gConfig->getLoginType() == Config::LUA) {
		// detect if this Fritz!Box uses SIDs
		DBG("requesting login_sid.lua from Fritz!Box.");
		// might return 404 with older fw-versions, our httpClient throws a SockeException for this, catched here
		try {
		  sXml = httpClient->Get(std::stringstream().flush()
			     << "/login_sid.lua?sid=" << gConfig->getSid());
		} catch (ost::SockException &se) {}
		if (sXml.find("<Rights") != std::string::npos)
			gConfig->setLoginType(Config::LUA);
		else {
			DBG("requesting login_sid.xml from Fritz!Box.");
			sXml = httpClient->Get(std::stringstream().flush()
					<< "/cgi-bin/webcm?getpage=../html/login_sid.xml");
			if (sXml.find("<iswriteaccess>") != std::string::npos)
				gConfig->setLoginType(Config::SID);
			else
				gConfig->setLoginType(Config::PASSWORD);
		}
	}

	if (gConfig->getLoginType() == Config::SID || gConfig->getLoginType() == Config::LUA) {
		std::stringstream loginPath;
//		std::stringstream postdataLogout;
		if (gConfig->getLoginType() == Config::LUA) {
			loginPath << "/login_sid.lua";
//			postdataLogout << "sid=" << gConfig->getSid() << "&logout=abc";
		} else {
			loginPath << "/cgi-bin/webcm";
//			postdataLogout << "sid=" << gConfig->getSid() << "&security:command/logout=abc";
		}
//		DBG("logging into fritz box using SIDs.");
//		if (gConfig->getSid().length() > 0) {
//			// logout, drop old SID (if FB has not already dropped this SID because of a timeout)
//			DBG("dropping old SID");
//			std::string sDummy;
//			sDummy = httpClient->Post(loginPath, postdataLogout);
//		}
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
            std::string response = CalculateLoginResponse(challenge);
            // send response to box
			std::string sMsg;

			std::stringstream postdata;
			if (gConfig->getLoginType() == Config::SID)
				postdata  << "login:command/response=" << response
				          << "&getpage=../html/de/menus/menu2.html";
			else
				postdata << "username=&response=" << response;
			DBG("Sending login request...");
			sMsg = httpClient->Post(loginPath, postdata);
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

		sMsg = httpClient->Post(std::stringstream().flush()
			<< "/cgi-bin/webcm",
			std::stringstream().flush()
			<< "login:command/password="
			<< UrlEncode(gConfig->getPassword()));

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

std::string FritzClient::GetLang() {
	if ( gConfig && gConfig->getLang().size() == 0) {
		std::vector<std::string> langs;
		langs.push_back("en");
		langs.push_back("de");
		langs.push_back("fr");
		for (unsigned int p=0; p<langs.size(); p++) {
			std::string sMsg;
			sMsg = httpClient->Get(std::stringstream().flush()
				<< "/cgi-bin/webcm?getpage=../html/"
				<< langs[p]
				<< "/menus/menu2.html"
				<< (gConfig->getSid().size() ? "&sid=" : "") << gConfig->getSid());
			if (sMsg.find("<html>") != std::string::npos) {
				gConfig->setLang(langs[p]);
				DBG("interface language is " << gConfig->getLang().c_str());
				return gConfig->getLang();
			}
		}
		DBG("error parsing interface language, assuming 'de'");
		gConfig->setLang("de");
	}
	return gConfig->getLang();
}

bool FritzClient::InitCall(std::string &number) {
	std::string msg;
	if (number.length() == 0)
		return false;
	if (!Login())
		return false;
	try {
		INF("sending call init request " << (gConfig->logPersonalInfo() ? number.c_str() : HIDDEN));
		msg = httpClient->Post(std::stringstream().flush()
		   << "/cgi-bin/webcm",
		   std::stringstream().flush()
		   << "getpage=../html/"
		   << GetLang()
		   << "/menus/menu2.html&var%3Apagename=fonbuch&var%3Amenu=home&telcfg%3Acommand/Dial="
		   << number
  	       << (gConfig->getSid().size() ? "&sid=" : "") << gConfig->getSid());
		INF("call initiated.");
	} catch (ost::SockException &se) {
		ERR("Exception - " << se.what());
		return false;
	}
	return true;
}

std::string FritzClient::RequestLocationSettings() {
	std::string msg;

	RETRY_BEGIN {
		if (gConfig->getSid().size()) {
			DBG("Looking up Phone Settings (using lua)...");
			try {
				msg = httpClient->Get(std::stringstream().flush()
					  << "/fon_num/sip_option.lua?sid=" << gConfig->getSid());
			} catch (ost::SockException &se) {}
			if (msg.find("<!-- pagename:/fon_num/sip_option.lua-->") != std::string::npos)
				return msg;
			DBG("failed.");
		}

		DBG("Looking up Phone Settings (using webcm)...");
		msg = httpClient->Get(std::stringstream().flush()
			<< "/cgi-bin/webcm?getpage=../html/"
			<<  GetLang()
			<< "/menus/menu2.html&var%3Alang="
			<<  GetLang()
			<< "&var%3Apagename=sipoptionen&var%3Amenu=fon"
			<< (gConfig->getSid().size() ? "&sid=" : "") << gConfig->getSid());
	} RETRY_END
	return msg;
}

std::string FritzClient::RequestSipSettings() {
	std::string msg;

	RETRY_BEGIN {
		if (gConfig->getSid().size()) {
			DBG("Looking up SIP Settings (using lua)...");
			try {
				msg = httpClient->Get(std::stringstream().flush()
					  << "/fon_num/fon_num_list.lua?sid=" << gConfig->getSid());
			} catch (ost::SockException &se) {}
			if (msg.find("<!-- pagename:/fon_num/fon_num_list.lua-->") != std::string::npos)
				return msg;
			DBG("failed.");
		}

		DBG("Looking up SIP Settings (using webcm)...");
		msg = httpClient->Get(std::stringstream().flush()
				<< "/cgi-bin/webcm?getpage=../html/"
				<< GetLang()
				<< "/menus/menu2.html&var%3Alang="
				<< GetLang()
				<< "&var%3Apagename=siplist&var%3Amenu=fon"
				<< (gConfig->getSid().size() ? "&sid=" : "") << gConfig->getSid());
	} RETRY_END
	return msg;
}

std::string FritzClient::RequestCallList () {
	std::string msg = "";
	std::string csv = "";
	RETRY_BEGIN {
		// now, process call list
		DBG("sending callList update request.");
		// force an update of the fritz!box csv list and wait until all data is received
		msg = httpClient->Get(std::stringstream().flush()
			<< "/cgi-bin/webcm?getpage=../html/"
			<<  GetLang()
			<< "/menus/menu2.html&var:lang="
			<<  GetLang()
			<< "&var:pagename=foncalls&var:menu=fon"
			<< (gConfig->getSid().size() ? "&sid=" : "") << gConfig->getSid());

		// new method to request call list (FW >= xx.05.50?)
		try {
			DBG("sending callList request (using lua)...");
			csv = httpClient->Get(std::stringstream().flush() << "/fon_num/foncalls_list.lua?"
			  	                  << "csv=&sid=" << gConfig->getSid());
			if (csv.find("Typ;Datum;Name;") != std::string::npos) {
				// we assume utf8 as encoding
				// the FB sends encoding in the response, however we do not parse it, yet
				CharSetConv *conv = new CharSetConv("utf8", CharSetConv::SystemCharacterTable());
				const char *csv_converted = conv->Convert(csv.c_str());
				csv = csv_converted;
				delete(conv);
				return csv;
			}
		} catch (ost::SockException e) {}

		// old method, parsing url to csv from page above

		// get the URL of the CSV-File-Export
		unsigned int urlPos   = msg.find(".csv");
		unsigned int urlStop  = msg.find('"', urlPos);
		unsigned int urlStart = msg.rfind('"', urlPos) + 1;
		std::string csvUrl    = msg.substr(urlStart, urlStop-urlStart);
		// retrieve csv list
		DBG("sending callList request (using webcm)...");
		csv = httpClient->Get(std::stringstream().flush()
				<< "/cgi-bin/webcm?getpage="
				<<  csvUrl
				<< (gConfig->getSid().size() ? "&sid=" : "") << gConfig->getSid());

		// convert answer to current SystemCodeSet (we assume, Fritz!Box sends its answer in latin15)
		CharSetConv *conv = new CharSetConv("ISO-8859-15", CharSetConv::SystemCharacterTable());
		const char *csv_converted = conv->Convert(csv.c_str());
		csv = csv_converted;
		delete(conv);
	} RETRY_END
	return csv;
}

std::string FritzClient::RequestFonbook () {
	std::string msg;
	// new method, returns an XML
	RETRY_BEGIN {
		if (gConfig->getSid().length()) {
			ost2::MIMEMultipartForm *mmpf = new ost2::MIMEMultipartForm();

			new ost2::MIMEFormData( mmpf, "sid", gConfig->getSid().c_str());
			new ost2::MIMEFormData( mmpf, "PhonebookId", "0");
			new ost2::MIMEFormData( mmpf, "PhonebookExportName", "Telefonbuch");
			new ost2::MIMEFormData( mmpf, "PhonebookExport", "");
			DBG("sending fonbook XML request.");
			try {
				msg = httpClient->PostMIME(std::stringstream().flush()
				  	  << "/cgi-bin/firmwarecfg", *mmpf);
			} catch (ost::SockException &se) {}
			if (msg.find("<phonebooks>") != std::string::npos) {
				return msg;
			}
		}

	// use old fashioned website (for old FW versions)
		DBG("sending fonbook HTML request.");
		msg = httpClient->Get(std::stringstream().flush()
			<< "/cgi-bin/webcm?getpage=../html/"
			<< GetLang()
			<< "/menus/menu2.html"
			<< "&var:lang="
			<< GetLang()
			<< "&var:pagename=fonbuch&var:menu=fon"
			<< (gConfig->getSid().size() ? "&sid=" : "") << gConfig->getSid());
	} RETRY_END

	return msg;
}

void FritzClient::WriteFonbook(std::string xmlData) {
	std::string msg;
	DBG("Saving XML Fonbook to FB...");
	RETRY_BEGIN {
		ost2::MIMEMultipartForm *mmpf = new ost2::MIMEMultipartForm();

		new ost2::MIMEFormData( mmpf, "sid", gConfig->getSid().c_str());
		new ost2::MIMEFormData( mmpf, "PhonebookId", "0");
		new ost2::MIMEFormData( mmpf, "PhonebookImportFile", "FRITZ.Box_Telefonbuch_01.01.10_0000.xml", "text/xml", xmlData.c_str());

		msg = httpClient->PostMIME(std::stringstream().flush()
				<< "/cgi-bin/firmwarecfg", *mmpf);
	} RETRY_END
}


bool FritzClient::reconnectISP() {
	std::string msg;
	DBG("Sending reconnect request to FB.");
	try {
		msg = soapClient->Post(
				std::stringstream().flush()
				<< "/upnp/control/WANIPConn1",
				std::stringstream().flush()
				<< "urn:schemas-upnp-org:service:WANIPConnection:1#ForceTermination",
				std::stringstream().flush()
				<< "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
				   "<s:Envelope s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\">"
				   "<s:Body>"
				   "<u:ForceTermination xmlns:u=\"urn:schemas-upnp-org:service:WANIPConnection:1\" />"
				   "</s:Body>"
				   "</s:Envelope>");
	} catch (ost::SockException &se) {
		ERR("Exception in connection to " << gConfig->getUrl() << " - " << se.what());
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
		msg = soapClient->Post(
				std::stringstream().flush()
				<< "/upnp/control/WANIPConn1",
				std::stringstream().flush()
				<< "urn:schemas-upnp-org:service:WANIPConnection:1#GetExternalIPAddress",
				std::stringstream().flush()
				<< "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
				   "<s:Envelope s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\">"
				   "<s:Body>"
				   "<u:GetExternalIPAddress xmlns:u=\"urn:schemas-upnp-org:service:WANIPConnection:1\" />"
				   "</s:Body>"
				   "</s:Envelope>");
	} catch (ost::SockException &se) {
		ERR("Exception in connection to " << gConfig->getUrl() << " - " << se.what());
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
