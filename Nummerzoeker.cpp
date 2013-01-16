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

#include "Nummerzoeker.h"

#include <unistd.h>

#include "Config.h"
#include "HttpClient.h"
#include "Tools.h"
#include "Log.h"

namespace fritz{

NummerzoekerFonbook::NummerzoekerFonbook()
: LookupFonbook(I18N_NOOP("nummerzoeker.com"), "ZOEK")
{}

Fonbook::sResolveResult NummerzoekerFonbook::Lookup(std::string number) const {
	Fonbook::sResolveResult result(number);

	// resolve only NL phone numbers
	std::string normNumber = Tools::NormalizeNumber(number);
	if (number.length() == 0 || normNumber.find("0031") != 0)
		return result;

	// __FILE__om works only with national number: remove 0031 prefix, add 0
	normNumber = '0' + normNumber.substr(4);

	std::string msg;
	try {
		DBG("sending reverse lookup request for " << (gConfig->logPersonalInfo() ? normNumber : HIDDEN) << " to www.nummerzoeker.com");
		std::string host = "www.nummerzoeker.com";
		HttpClient tc(host);
		msg = tc.Get(std::stringstream().flush()
		   << "/index.php?search=Zoeken&phonenumber="
		   << normNumber
		   << "&export=csv");
	} catch (ost::SockException &se) {
		ERR("Exception - " << se.what());
		return result;
	}

	if (msg.find("Content-Type: text/html") != std::string::npos) {
		INF("no entry found.");
		return result;
	}

	// parse answer, format is "number",name,surname,street,zip,city
	size_t lineStart = 0;
	std::string name, surname;
	while ((lineStart = msg.find("\n", lineStart)) != std::string::npos) {
	  lineStart++;
	  if (msg[lineStart] == '"') {
			size_t nameStart    = msg.find(",", lineStart);
			size_t surnameStart = msg.find(",", nameStart+1);
			size_t streetStart  = msg.find(",", surnameStart+1);
			name                = msg.substr(nameStart, surnameStart-nameStart-1);
			surname             = msg.substr(surnameStart, streetStart-surnameStart-1);
			name = surname + ' ' + name;
			break;
	  }
	}
	// convert the string from latin1 to current system character table
	// Q: is this really ISO-8859-1, the webservers' response is unclear (html pages are UTF8)
	CharSetConv *conv = new CharSetConv("ISO-8859-1", CharSetConv::SystemCharacterTable());
	const char *s_converted = conv->Convert(name.c_str());
	name = s_converted;
	delete (conv);
	INF("resolves to " << (gConfig->logPersonalInfo() ? name.c_str() : HIDDEN));
	result.name = name;
	result.successful = true;
	return result;
}

}
