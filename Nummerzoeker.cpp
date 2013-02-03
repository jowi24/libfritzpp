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
#include <libnet++/HttpClient.h>
#include "Tools.h"
#include <liblog++/Log.h>

namespace fritz{

NummerzoekerFonbook::NummerzoekerFonbook()
: LookupFonbook(I18N_NOOP("nummerzoeker.com"), "ZOEK")
{}

Fonbook::sResolveResult NummerzoekerFonbook::lookup(std::string number) const {
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
		network::HttpClient tc("www.nummerzoeker.com");
		network::HttpClient::param_t params = {
				{ "search", "Zoeken" },
				{ "phonenumber", normNumber },
				{ "export", "csv" },
		};
		msg = tc.get("/index.php", params);
	} catch (std::runtime_error &re) {
		ERR("Exception - " << re.what());
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
	INF("resolves to " << (gConfig->logPersonalInfo() ? name.c_str() : HIDDEN));
	result.name = name;
	result.successful = true;
	return result;
}

}
