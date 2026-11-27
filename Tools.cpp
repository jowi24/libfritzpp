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

#include "Tools.h"

#include <string>
#include <cstdlib>
#include <locale.h>
#include <langinfo.h>
#include <sstream>
#include <iostream>
#include <errno.h>

#include "Config.h"
#include "FritzClient.h"
#include <liblog++/Log.h>

#include <vector>
#include <exception>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace fritz{
Tools::Tools()
{
}

Tools::~Tools()
{
}

bool Tools::MatchesMsnFilter(const std::string &number){
	// if no MSN filter is set, true is returned
	if (gConfig->getMsnFilter().size() == 0)
		return true;
	// if number does contain a MSN out of the MSN filter, true is returned
	for (size_t pos=0; pos < gConfig->getMsnFilter().size(); pos++) {
		std::string msn = gConfig->getMsnFilter()[pos];
		size_t index = number.find(msn);
		if ( index != std::string::npos && index + msn.length() == number.length()) {
			//matched
			return true;
		}
	}
	// no match
	return false;
}

std::string Tools::NormalizeNumber(std::string number) {
	// Remove Fritz!Box control codes *xyz# if used
	if (number[0] == '*') {
		size_t hashPos = number.find('#');
		if (hashPos != std::string::npos)
			number.erase(0, hashPos + 1);
	}
	// Only for Germany: Remove Call-By-Call Provider Selection Codes 010(0)xx
	if ( gConfig->getCountryCode() == "49") {
		if (number[0] == '0' && number[1] == '1' && number[2] == '0') {
			if (number[3] == '0')
				number.erase(0, 6);
			else
				number.erase(0, 5);
		}
	}
	// Modifies 'number' to the following format
	// '00' + countryCode + regionCode + phoneNumber
	if (number[0] == '+') {
		//international prefix given in form +49 -> 0049
		number.replace(0, 1, "00");
	} else if (number[0] == '0' && number[1] != '0') {
		//national prefix given 089 -> 004989
		number.replace(0, 1, gConfig->getCountryCode().c_str());
		number = "00" + number;
	} else if (number[0] != '0') {
		// number without country or region code, 1234 -> +49891234
		number = "00" + gConfig->getCountryCode() + gConfig->getRegionCode() + number;
	} // else: number starts with '00', do not change
	return number;
}

int Tools::CompareNormalized(std::string number1, std::string number2) {
	return NormalizeNumber(number1).compare(NormalizeNumber(number2));
}

bool Tools::GetLocationSettings() {
	// get location settings from Fritz!Box
	FritzClient *fc = gConfig->fritzClientFactory->create();
	std::string msg = fc->requestLocationSettings();

	bool validPassword = fc->hasValidPassword();
	delete fc;

	try {
		std::stringstream stream(msg);
		boost::property_tree::ptree tree;

		boost::property_tree::read_json(stream, tree);

		const boost::property_tree::ptree &location =
			tree.get_child("data.location");

		std::string lkz =
			location.get<std::string>("LKZ.value", "");

		std::string okz =
		location.get<std::string>("OKZ.value", "");

		if (lkz.empty()) {
			ERR("Could not find LKZ in Fritz!Box response.");
			return validPassword;
		}

		if (okz.empty()) {
			ERR("Could not find OKZ in Fritz!Box response.");
			return validPassword;
		}

		gConfig->setCountryCode(lkz);
		gConfig->setRegionCode(okz);

		DBG("Found LKZ "
			<< (gConfig->logPersonalInfo() ? lkz : HIDDEN));

		DBG("Found OKZ "
			<< (gConfig->logPersonalInfo() ? okz : HIDDEN));

		INF("Successfully parsed location settings.");

	} catch (const std::exception &e) {
		ERR("Error parsing location settings JSON: " << e.what());
		return validPassword;
	}

	return validPassword;
}

void Tools::GetSipSettings() {
	// if SIP settings are already set, exit here...
	if (gConfig->getSipNames().size() > 0)
		return;

	// ...otherwise get settings from Fritz!Box.
	FritzClient *fc = gConfig->fritzClientFactory->create();
	std::string msg = fc->requestSipSettings();
	delete fc;

	std::vector<std::string> sipNames;
	std::vector<std::string> sipMsns;

	// search for JavaScript-Arrays
	size_t arrayStart = msg.find("var gFonNums");

	if (arrayStart == std::string::npos) {
		ERR("Parser error in GetSipSettings(). Could not find gFonNums.");
		ERR("SIP provider names not set!");
		return;
	}

	arrayStart = msg.find('[', arrayStart);

	if (arrayStart == std::string::npos) {
		ERR("Parser error in GetSipSettings(). Could not find SIP array.");
		return;
	}

	size_t arrayEnd = msg.find("];", arrayStart);

	if (arrayEnd == std::string::npos) {
		ERR("Parser error in GetSipSettings(). Could not find end of SIP array.");
		return;
	}

	// JSON-Array in ein JSON-Objekt einbetten.
	std::string json =
	"{\"gFonNums\":" +
	msg.substr(arrayStart, arrayEnd - arrayStart + 1) +
	"}";

	try {
		std::stringstream stream(json);
		boost::property_tree::ptree tree;

		boost::property_tree::read_json(stream, tree);

		const boost::property_tree::ptree &numbers =
		tree.get_child("gFonNums");

		for (const auto &entry : numbers) {
			const boost::property_tree::ptree &number = entry.second;

			std::string type =
			number.get<std::string>("type", "");

			// Nur SIP-Rufnummern übernehmen.
			if (type != "sip")
				continue;

			std::string name =
			number.get<std::string>("providername", "");

			if (name.empty())
				name = number.get<std::string>("name", "");

			std::string msn =
			number.get<std::string>("msnnum", "");

			sipNames.push_back(name);
			sipMsns.push_back(msn);

			DBG("Found SIP provider name " << name
			<< " / MSN "
			<< (gConfig->logPersonalInfo() ? msn : HIDDEN));
		}

		if (sipNames.empty()) {
			ERR("No SIP numbers found in gFonNums.");
			return;
		}

		gConfig->setSipNames(sipNames);
		gConfig->setSipMsns(sipMsns);

		INF("Successfully parsed " << sipNames.size()
		<< " SIP numbers from Fritz!Box.");

	} catch (const std::exception &e) {
		ERR("Error parsing gFonNums JSON: " << e.what());
		return;
	}
}

std::string Tools::Tokenize(const std::string &buffer, const char delimiter, size_t pos) {
	size_t tokenStart = 0;
	for (size_t i=0; i<pos; i++) {
		tokenStart = buffer.find(delimiter, tokenStart+1);
		if (tokenStart == std::string::npos)
			return "";
	}
	if (tokenStart > 0)
		tokenStart++;
	size_t tokenStop = buffer.find(delimiter, tokenStart);
	if (tokenStop == std::string::npos)
		tokenStop = buffer.size();
	std::string token = buffer.substr(tokenStart, tokenStop - tokenStart);
	return token;
}

}

