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
#include "Log.h"

namespace fritz {

XmlFonbook::XmlFonbook(std::string title, std::string techId, bool writeable)
: Fonbook(title, techId, writeable) {
	charset = CharSetConv::SystemCharacterTable() ? CharSetConv::SystemCharacterTable() : "UTF-8";
}

XmlFonbook::~XmlFonbook() {
}


std::string XmlFonbook::ExtractXmlAttributeValue(std::string element, std::string attribute, std::string xml) {
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

std::string XmlFonbook::ExtractXmlElementValue(std::string element, std::string xml) {
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

void XmlFonbook::ParseXmlFonbook(std::string *msg) {
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

	CharSetConv *conv = new CharSetConv(charset.c_str(), CharSetConv::SystemCharacterTable());
	const char *s_converted = conv->Convert(msg->c_str());
	std::string msgConv = s_converted;
	delete (conv);

	pos = msgConv.find("<contact>");
	while (pos != std::string::npos) {
		std::string msgPart = msgConv.substr(pos, msgConv.find("</contact>", pos) - pos + 10);
		std::string category = ExtractXmlElementValue("category", msgPart);
		std::string name     = convertEntities(ExtractXmlElementValue("realName", msgPart));
		FonbookEntry fe(name, category == "1");
		size_t posNumber = msgPart.find("<number");
		size_t numberCount = 0;
		while (posNumber != std::string::npos) {
			std::string msgPartofPart = msgPart.substr(posNumber, msgPart.find("</number>", posNumber) - posNumber + 9);
			std::string number    = ExtractXmlElementValue  ("number",              msgPartofPart);
			std::string typeStr   = ExtractXmlAttributeValue("number", "type",      msgPartofPart);
			std::string quickdial = ExtractXmlAttributeValue("number", "quickdial", msgPartofPart);
			std::string vanity    = ExtractXmlAttributeValue("number", "vanity",    msgPartofPart);
			std::string prio      = ExtractXmlAttributeValue("number", "prio",      msgPartofPart);

			if (number.size()) { // the xml may contain entries without a number!
				FonbookEntry::eType type = FonbookEntry::TYPE_NONE;
				if (typeStr == "home")
					type = FonbookEntry::TYPE_HOME;
				if (typeStr == "mobile")
					type = FonbookEntry::TYPE_MOBILE;
				if (typeStr == "work")
					type = FonbookEntry::TYPE_WORK;

				fe.AddNumber(numberCount++, number, type, quickdial, vanity, atoi(prio.c_str()));
			}
			posNumber = msgPart.find("<number", posNumber+1);
		}
		AddFonbookEntry(fe);
		pos = msgConv.find("<contact>", pos+1);
	}
}

std::string XmlFonbook::SerializeToXml() {

	std::stringstream result;
	result << "<?xml version=\"1.0\" encoding=\"" << charset << "\"?>"
			  "<phonebooks>"
			  "<phonebook>";
	for (auto fe : getFonbookList()) {
		result << "<contact>"
			   << "<category>" << (fe.IsImportant() ? "1" : "0") << "</category>"
			   << "<person>"
		       << "<realName>" << fe.GetName() << "</realName>"
		       << "</person>"
		       << "<telephony>";
		for (size_t numberPos = 0; numberPos < fe.GetSize(); numberPos++)
			if (fe.GetNumber(numberPos).length() > 0) {  //just iterate over all numbers
				std::string typeName = "";
				switch (fe.GetType(numberPos)) {
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
						          "quickdial=\"" << fe.GetQuickdial(numberPos) << "\" "
						          "vanity=\""    << fe.GetVanity(numberPos)    << "\" "
						          "prio=\""      << fe.GetPriority(numberPos)  << "\">"
				       << fe.GetNumber(numberPos)
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

	CharSetConv *conv = new CharSetConv(CharSetConv::SystemCharacterTable(), charset.c_str());
	const char *result_converted = conv->Convert(result.str().c_str());
	std::string xmlData = result_converted;
	delete (conv);

	// replace '&' with '&amp;'
	std::string::size_type pos = 0;
	while ((pos = xmlData.find('&', pos)) != std::string::npos) {
		xmlData.replace(pos, 1, "&amp;");
		pos += 5;
	}

	return xmlData;
}

}
