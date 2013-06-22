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

#include "FritzFonbook.h"

#include <algorithm>
#include <sstream>

#include "Config.h"
#include "FritzClient.h"
#include "Tools.h"
#include <liblog++/Log.h>
#include <libconv++/CharsetConverter.h>
#include <libconv++/EntityConverter.h>

namespace fritz {

FritzFonbook::FritzFonbook()
:XmlFonbook(I18N_NOOP("Fritz!Box phone book"), "FRITZ", true), thread{nullptr}
{
	setInitialized(false);
}

FritzFonbook::~FritzFonbook() {
	if (thread) {
		thread->join();
		delete thread;
	}
}

bool FritzFonbook::initialize() {
	reload();
	return true;
}

void FritzFonbook::run() {
	DBG("FritzFonbook thread started");
	setInitialized(false);
	clear();

	FritzClient *fc = gConfig->fritzClientFactory->create();
	std::string msg = fc->requestFonbook();
	delete fc;

	if (msg.find("<?xml") == std::string::npos)
		parseHtmlFonbook(&msg);
	else {
		parseXmlFonbook(&msg);
		setWriteable(); // we can write xml back to the FB
	}

	setInitialized(true);

	sort(FonbookEntry::ELEM_NAME, true);
	DBG("FritzFonbook thread ended");
}

void FritzFonbook::parseHtmlFonbook(std::string *msg) {
	DBG("Parsing fonbook using html parser.")
	// determine charset (default for old firmware versions is iso-8859-15)
	size_t pos;
	std::string charset = "ISO-8859-15";
	pos = msg->find("<meta http-equiv=content-type");
	if (pos != std::string::npos) {
		pos = msg->find("charset=", pos);
		if (pos != std::string::npos)
			charset = msg->substr(pos+8, msg->find('"', pos)-pos-8);
	}
	DBG("using charset " << charset);

	std::string msgConv = convert::CharsetConverter::ConvertToLocalEncoding(*msg, charset);

	// parse answer
	pos = 0;
	int count = 0;
	// parser for old format
	const std::string tag("(TrFon(");
	while ((pos = msgConv.find(tag, pos)) != std::string::npos) {
		pos += 7; // points to the first "
		int nameStart     = msgConv.find(',', pos)          +3;
		int nameStop      = msgConv.find('"', nameStart)   -1;
		int numberStart   = msgConv.find(',', nameStop)    +3;
		int numberStop    = msgConv.find('"', numberStart) -1;
		if (msgConv[nameStart] == '!') // skip '!' char, older firmware versions use to mark important
			nameStart++;
		std::string namePart = msgConv.substr(nameStart, nameStop - nameStart+1);
		std::string namePart2 = convert::EntityConverter::DecodeEntities(namePart);
		std::string numberPart = msgConv.substr(numberStart, numberStop - numberStart+1);
		if (namePart2.length() && numberPart.length()) {
			FonbookEntry fe(namePart2, false); // TODO: important is not parsed here
			fe.addNumber(numberPart, FonbookEntry::TYPE_NONE);
			addFonbookEntry(fe);
			//DBG("(%s / %s)", fe.number.c_str(), fe.name.c_str());
		}
		pos += 10;
		count++;
	}
	// parser for new format
	pos = 0;

	const std::string tagName("TrFonName(");
	const std::string tagNumber("TrFonNr("  );
	// iterate over all tagNames
	while ((pos = msgConv.find(tagName, ++pos)) != std::string::npos) {
		int nameStart     = msgConv.find(',', pos+7)          +3;
		int nameStop      = msgConv.find('"', nameStart)   -1;
		std::string namePart   = msgConv.substr(nameStart, nameStop - nameStart+1);
		std::string namePartConv  = convert::EntityConverter::DecodeEntities(namePart);
		FonbookEntry fe(namePartConv, false); // TODO: important is not parsed here

		size_t posInner = pos;
		// iterate over all tagNumbers between two tagNames
		while ((posInner = msgConv.find(tagNumber, ++posInner)) != std::string::npos && posInner < msgConv.find(tagName, pos+1)) {
			int typeStart     = posInner + 9;
			int numberStart   = msgConv.find(',', posInner)    +3;
			int typeStop      = numberStart - 5;
			int numberStop    = msgConv.find('"', numberStart) -1;
			std::string numberPart = msgConv.substr(numberStart, numberStop - numberStart+1);
			std::string typePart   = msgConv.substr(typeStart, typeStop - typeStart+1);
			FonbookEntry::eType type = FonbookEntry::TYPE_NONE;
			if      (typePart.compare("home")   == 0)
				type = FonbookEntry::TYPE_HOME;
			else if (typePart.compare("mobile") == 0)
				type = FonbookEntry::TYPE_MOBILE;
			else if (typePart.compare("work")   == 0)
				type = FonbookEntry::TYPE_WORK;

			if (namePartConv.length() && numberPart.length()) {
				fe.addNumber(numberPart, type); // TODO: quickdial, vanity and priority not parsed here
				//DBG("(%s / %s / %i)", fe.number.c_str(), fe.name.c_str(), fe.type);
			}
			count++;
		}
		addFonbookEntry(fe);
	}
}

void FritzFonbook::reload() {
	if (thread) {
		thread->join();
		delete thread;
	}
	thread = new std::thread(&FritzFonbook::run, this);
}

void FritzFonbook::write() {
	if (isWriteable()) {
		INF("Uploading phonebook to Fritz!Box.");
		FritzClient *fc = gConfig->fritzClientFactory->create();
		fc->writeFonbook(serializeToXml());
		delete fc;
	}
}
}
