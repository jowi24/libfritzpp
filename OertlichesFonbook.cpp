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

#include "OertlichesFonbook.h"

#include <unistd.h>
#include <sstream>
#include <iomanip>

#include "Config.h"
#include <libnet++/HttpClient.h>
#include "Tools.h"
#include <liblog++/Log.h>
#include <libconv++/EntityConverter.h>
#include <cctype>

namespace fritz{

namespace {
std::string urlEncode(const std::string &value) {
    std::ostringstream encoded;
    encoded << std::uppercase << std::hex;
    for (unsigned char c : value) {
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.' || c == '~') {
            encoded << c;
        } else {
            encoded << '%' << std::setw(2) << std::setfill('0') << static_cast<int>(c);
        }
    }
    return encoded.str();
}
}


OertlichesFonbook::OertlichesFonbook()
:LookupFonbook(I18N_NOOP("das-oertliche.de"), "OERT")
{}

Fonbook::sResolveResult OertlichesFonbook::lookup(std::string number) const {
	Fonbook::sResolveResult result(number);

	// resolve only (german) phone numbers
	if (number.length() == 0 || Tools::NormalizeNumber(number).find("0049") != 0)
		return result;

	std::string msg;
	std::string name;
	try {
		DBG("sending reverse lookup request for " << (gConfig->logPersonalInfo()? Tools::NormalizeNumber(number) : HIDDEN) << " to www.dasoertliche.de");
		const std::string normalized = Tools::NormalizeNumber(number);
		const std::string url =
			"https://www.dasoertliche.de/rueckwaertssuche/?ph=" + urlEncode(normalized);
		msg = network::HttpClient::getURL(url);
	} catch (std::runtime_error &re) {
		ERR("Exception - " << re.what());
		return result;
	}
	// Check for multiple results
	size_t second_result = msg.find("id=\"entry_1\"");
	if (second_result != std::string::npos) {
		INF("multiple entries found, not returning any.");
		return result;
	}

	// Find address block
	size_t addressStart = msg.find("class=\"addressblock\"");
	if (addressStart == std::string::npos) {
		INF("no address block found.");
		return result;
	}

	// Find name in h1 element
	size_t nameStart = msg.find("<h1>", addressStart);
	if (nameStart == std::string::npos) {
		INF("no name found.");
		return result;
	}

	nameStart += 4;

	size_t nameStop = msg.find("</h1>", nameStart);
	if (nameStop == std::string::npos) {
		INF("invalid name element.");
		return result;
	}

	name = msg.substr(nameStart, nameStop - nameStart);

	// Decode HTML entities
	name = convert::EntityConverter::DecodeEntities(name);

	// Trim whitespace
	size_t first = name.find_first_not_of(" \t\r\n");
	size_t last = name.find_last_not_of(" \t\r\n");

	if (first == std::string::npos) {
		INF("empty name found.");
		return result;
	}

	name = name.substr(first, last - first + 1);

	INF("resolves to " << (gConfig->logPersonalInfo() ? name.c_str() : HIDDEN));

	result.name = name;
	result.successful = true;
	return result;
}

}
