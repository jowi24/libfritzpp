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

#include "Config.h"
#include <libnet++/HttpClient.h>
#include "Tools.h"
#include <liblog++/Log.h>

namespace fritz{

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
		network::HttpClient::param_t params = {
				{ "form_name", "search_inv"},
				{ "ph", Tools::NormalizeNumber(number)},
		};
		network::HttpClient tc("www.dasoertliche.de");
		msg = tc.get("/Controller", params);
	} catch (std::runtime_error &re) {
		ERR("Exception - " << re.what());
		return result;
	}
	// check that at most one result is returned
	size_t second_result = msg.find("id=\"entry_1\"");
	if (second_result != std::string::npos) {
		INF("multiple entries found, not returning any.");
		return result;
	}
	// parse answer
	size_t start = msg.find("getItemData(");
	if (start == std::string::npos) {
		INF("no entry found.");
		return result;
	}
	// add the length of the last search pattern
	start += 12;

	size_t stop  = msg.find(");", start);
	std::string dataset = msg.substr(start, stop - start);
	name = Tools::Tokenize(dataset, ',', 5);
	name = name.substr(2, name.length()-3);
	INF("resolves to " << (gConfig->logPersonalInfo() ? name.c_str() : HIDDEN));
	result.name = name;
	result.successful = true;
	return result;
}

}
