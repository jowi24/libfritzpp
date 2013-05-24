/*
 * libfritz++
 *
 * Copyright (C) 2007-2012 Joachim Wilke <libfritz@joachim-wilke.de>
 * TelLocalChFonbook created by Christian Richter <cr@crichter.net>
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

#include "TelLocalChFonbook.h"

#include <unistd.h>

#include "Config.h"
#include <libnet++/HttpClient.h>
#include "Tools.h"
#include <liblog++/Log.h>
#include <libconv++/EntityConverter.h>
#include <boost/regex.hpp>

namespace fritz{

TelLocalChFonbook::TelLocalChFonbook()
: LookupFonbook(I18N_NOOP("tel.local.ch"), "LOCCH")
{}

TelLocalChFonbook::sResolveResult TelLocalChFonbook::lookup(std::string number) const {
	TelLocalChFonbook::sResolveResult result(number);

	// resolve only (swiss) phone numbers
	if (number.length() == 0 || Tools::NormalizeNumber(number).find("0041") != 0)
		return result;
	
	std::string msg;
	std::string name;
	try {
		DBG("sending reverse lookup request for " << Tools::NormalizeNumber(number) << " to tel.local.ch");
		network::HttpClient tc("mobile.tel.local.ch");
		std::stringstream ss;
		ss << "/de/q/" <<  Tools::NormalizeNumber(number) << ".html";
		msg = tc.get(ss.str());
	} catch (std::runtime_error &se) {
		ERR("Exception - " << se.what());
		return result;
	}
	// parse answer
	boost::regex expression("<h2 class[^>]+><a [^>]+>(.+)</a></h2>");
	boost::smatch what;
	if (boost::regex_search(msg, what, expression)) {
		name = what[1];
		name = convert::EntityConverter::DecodeEntities(name);

		INF("resolves to " << name.c_str());
		result.name = name;
		result.successful = true;
	} else
		INF("no entry found.");
	
	return result;
}

}
