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

#include "XmlFonbook.h"

#include <string>
#include <cstdlib>
#include <sstream>

#include "Config.h"
#include "Tools.h"
#include <liblog++/Log.h>
#include <libconv++/CharsetConverter.h>
#include <libconv++/EntityConverter.h>

namespace fritz {

XmlFonbook::XmlFonbook(std::string title, std::string techId, bool writeable)
: Fonbook{title, techId, writeable} {
}

XmlFonbook::~XmlFonbook() {
}


std::string XmlFonbook::extractXmlAttributeValue(std::string element, std::string attribute, std::string xml) {
	size_t posStart = xml.find('<'+element);
	if (posStart != std::string::npos) {
		posStart = xml.find(attribute+"=\"", posStart);
		if (posStart != std::string::npos) {
			size_t posEnd = xml.find("\"", posStart + attribute.length() + 2);
			if (posEnd != std::string::npos)
				return xml.substr(posStart + attribute.length() + 2, posEnd - posStart - attribute.length() - 2);
		}
	}
	return "";
}

std::string XmlFonbook::extractXmlElementValue(std::string element, std::string xml) {
	size_t posStart = xml.find('<'+element);
	if (posStart != std::string::npos) {
		posStart = xml.find(">", posStart);
		if (xml[posStart-1] == '/')
			return "";
		size_t posEnd   = xml.find("</"+element+'>');
		if (posEnd != std::string::npos)
			return xml.substr(posStart + 1, posEnd - posStart - 1);
	}
	return "";
}

void XmlFonbook::parseXmlFonbook(std::string *msg) {
	DBG("Parsing fonbook using xml parser.")
	// determine charset
	size_t pos, posStart, posEnd;
	posStart = msg->find("encoding=\"");
	if (posStart != std::string::npos) {
		posEnd = msg->find("\"", posStart + 10);
		if (posEnd != std::string::npos)
			charset = msg->substr(posStart + 10, posEnd - posStart - 10);
	}
	DBG("using charset " << charset);

	std::string msgConv = convert::CharsetConverter::ConvertToLocalEncoding(*msg, charset);

	pos = msgConv.find("<contact>");
	while (pos != std::string::npos) {
		std::string msgPart = msgConv.substr(pos, msgConv.find("</contact>", pos) - pos + 10);
		std::string category = extractXmlElementValue("category", msgPart);
		std::string name     = convert::EntityConverter::DecodeEntities(extractXmlElementValue("realName", msgPart));
		FonbookEntry fe(name, category == "1");
		size_t posNumber = msgPart.find("<number");
		while (posNumber != std::string::npos) {
			std::string msgPartofPart = msgPart.substr(posNumber, msgPart.find("</number>", posNumber) - posNumber + 9);
			std::string number    = extractXmlElementValue  ("number",              msgPartofPart);
			std::string typeStr   = extractXmlAttributeValue("number", "type",      msgPartofPart);
			std::string quickdial = extractXmlAttributeValue("number", "quickdial", msgPartofPart);
			std::string vanity    = extractXmlAttributeValue("number", "vanity",    msgPartofPart);
			std::string prio      = extractXmlAttributeValue("number", "prio",      msgPartofPart);

			if (number.size()) { // the xml may contain entries without a number!
				FonbookEntry::eType type = FonbookEntry::TYPE_NONE;
				if (typeStr == "home")
					type = FonbookEntry::TYPE_HOME;
				if (typeStr == "mobile")
					type = FonbookEntry::TYPE_MOBILE;
				if (typeStr == "work")
					type = FonbookEntry::TYPE_WORK;

				fe.addNumber(number, type, quickdial, vanity, atoi(prio.c_str()));
			}
			posNumber = msgPart.find("<number", posNumber+1);
		}
		addFonbookEntry(fe);
		pos = msgConv.find("<contact>", pos+1);
	}
}

std::string XmlFonbook::serializeToXml() {

	std::stringstream result;
	result << "<?xml version=\"1.0\" encoding=\"" << charset << "\"?>"
			  "<phonebooks>"
			  "<phonebook>";
	for (auto fe : getFonbookList()) {
		result << "<contact>"
			   << "<category>" << (fe.isImportant() ? "1" : "0") << "</category>"
			   << "<person>"
		       << "<realName>" << fe.getName() << "</realName>"
		       << "</person>"
		       << "<telephony>";
		for (size_t numberPos = 0; numberPos < fe.getSize(); numberPos++)
			if (fe.getNumber(numberPos).length() > 0) {  //just iterate over all numbers
				std::string typeName = "";
				switch (fe.getType(numberPos)) {
				case FonbookEntry::TYPE_NONE:
				case FonbookEntry::TYPE_HOME:
					typeName="home";
					break;
				case FonbookEntry::TYPE_MOBILE:
					typeName="mobile";
					break;
				case FonbookEntry::TYPE_WORK:
					typeName="work";
					break;
				default:
					// should not happen
					break;
				}
				result << "<number type=\"" << typeName << "\" "
						          "quickdial=\"" << fe.getQuickdial(numberPos) << "\" "
						          "vanity=\""    << fe.getVanity(numberPos)    << "\" "
						          "prio=\""      << fe.getPriority(numberPos)  << "\">"
				       << fe.getNumber(numberPos)
				       << "</number>";
			}
        //TODO: add <mod_time>1306951031</mod_time>
		result << "</telephony>"
			   << "<services/>"
               << "<setup/>"
               << "</contact>";
	}
	result << "</phonebook>"
			  "</phonebooks>";

	convert::CharsetConverter conv("", charset);
	std::string xmlData = conv.convert(result.str());

	// replace '&' with '&amp;'
	std::string::size_type pos = 0;
	while ((pos = xmlData.find('&', pos)) != std::string::npos) {
		xmlData.replace(pos, 1, "&amp;");
		pos += 5;
	}

	return xmlData;
}

}
