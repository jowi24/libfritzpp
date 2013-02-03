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

#include "Fonbook.h"

#include <algorithm>

#include "Config.h"
#include "Tools.h"
#include <liblog++/Log.h>
#include <libconv++/CharsetConverter.h>

namespace fritz {

const std::map<std::string, std::string> Entities = {
	{"&nbsp;",  " "},
	{"&iexcl;", "¡"},
	{"&cent;",  "¢"},
	{"&pound;", "£"},
	{"&curren;","€"}, //krazy:exclude=spelling
	{"&yen;",   "¥"},
	{"&brvbar;","Š"},
	{"&sect;",  "§"},
	{"&uml;",   "š"},
	{"&copy;",  "©"},
	{"&ordf;",  "ª"},
	{"&laquo;", "«"},
	{"&not;",   "¬"},
	{"&shy;",   "­"},
	{"&reg;",   "®"},
	{"&macr;",  "¯"},
	{"&deg;",   "°"},
	{"&plusmn;","±"},
	{"&sup2;",  "²"},
	{"&sup3;",  "³"},
	{"&acute;", "Ž"},
	{"&micro;", "µ"},
	{"&para;",  "¶"},
	{"&middot;","·"},
	{"&cedil;", "ž"},
	{"&sup1;",  "¹"},
	{"&ordm;",  "º"},
	{"&raquo;", "»"},
	{"&frac14;","Œ"},
	{"&frac12;","œ"},
	{"&frac34;","Ÿ"},
	{"&iquest;","¿"},
	{"&Agrave;","À"},
	{"&Aacute;","Á"},
	{"&Acirc;", "Â"},
	{"&Atilde;","Ã"},
	{"&Auml;",  "Ä"},
	{"&Aring;", "Å"},
	{"&AElig;", "Æ"},
	{"&Ccedil;","Ç"},
	{"&Egrave;","È"},
	{"&Eacute;","É"},
	{"&Ecirc;", "Ê"},
	{"&Euml;",  "Ë"},
	{"&Igrave;","Ì"},
	{"&Iacute;","Í"},
	{"&Icirc;", "Î"},
	{"&Iuml;",  "Ï"},
	{"&ETH;",   "Ð"},
	{"&Ntilde;","Ñ"},
	{"&Ograve;","Ò"},
	{"&Oacute;","Ó"},
	{"&Ocirc;", "Ô"},
	{"&Otilde;","Õ"},
	{"&Ouml;",  "Ö"},
	{"&times;", "×"},
	{"&Oslash;","Ø"},
	{"&Ugrave;","Ù"},
	{"&Uacute;","Ú"},
	{"&Ucirc;", "Û"},
	{"&Uuml;",  "Ü"},
	{"&Yacute;","Ý"},
	{"&THORN;", "Þ"},
	{"&szlig;", "ß"},
	{"&agrave;","à"},
	{"&aacute;","á"},
	{"&acirc;", "â"},
	{"&atilde;","ã"},
	{"&auml;",  "ä"},
	{"&aring;", "å"},
	{"&aelig;", "æ"},
	{"&ccedil;","ç"},
	{"&egrave;","è"},
	{"&eacute;","é"},
	{"&ecirc;", "ê"},
	{"&euml;",  "ë"},
	{"&igrave;","ì"},
	{"&iacute;","í"},
	{"&icirc;", "î"},
	{"&iuml;",  "ï"},
	{"&eth;",   "ð"},
	{"&ntilde;","ñ"},
	{"&ograve;","ò"},
	{"&oacute;","ó"},
	{"&ocirc;", "ô"},
	{"&otilde;","õ"},
	{"&ouml;",  "ö"},
	{"&divide;","÷"},
	{"&oslash;","ø"},
	{"&ugrave;","ù"},
	{"&uacute;","ú"},
	{"&ucirc;", "û"},
	{"&uuml;",  "ü"},
	{"&yacute;","ý"},
	{"&thorn;", "þ"},
	{"&yuml;",  "ÿ"},
	{"&amp;",   "&"},
};

std::string Fonbook::convertEntities(std::string s) const {
	if (s.find("&") != std::string::npos) {
		// convert the entities from UTF-8 to current system character table
		convert::CharsetConverter conv("UTF-8");

		// convert entities of format &#xFF; (unicode)
		while (s.find("&#x") != std::string::npos) {
			size_t pos = s.find("&#x");
			size_t end = s.find(";", pos);
			// get hex code
			std::string unicode = s.substr(pos+3, end - pos - 3);
			// convert to int
			std::stringstream ss;
			ss << std::hex << unicode;
			int codepoint;
			ss >> codepoint;
			// get corresponding char
			char out_buffer[8];
			memset(out_buffer, 0, 8);
			char *out = &(out_buffer[0]);
			wchar_t in_buffer = codepoint;
			char *in = (char *)&(in_buffer);
			size_t inlen = sizeof(in_buffer), outlen = sizeof(out_buffer);
			iconv_t cd;
			cd = iconv_open("utf-8", "ucs-2");
			iconv(cd, &in, &inlen, &out, &outlen);
			iconv_close(cd);
			// replace it
			s.replace(pos, end-pos+1, std::string(out_buffer));
		}

		// convert other entities with table
		for (auto entity : Entities) {
			std::string::size_type pos = s.find(entity.first);
			while (pos != std::string::npos) {
				s.replace(pos, entity.first.length(), conv.convert(entity.second));
				pos = s.find(entity.first, pos-1);
			}
		}
	}
	return s;
}

FonbookEntry::FonbookEntry(std::string name, bool important) {
	this->name      = name;
	this->important = important;
}

void FonbookEntry::addNumber(size_t pos, std::string number, eType type, std::string quickdial, std::string vanity, int priority) {
	sNumber sn;
	sn.number = number;
	sn.type = type;
	sn.quickdial = quickdial;
	sn.vanity = vanity;
	sn.priority = priority;
	numbers.push_back(sn);
}

size_t FonbookEntry::getDefault() const {
	size_t t = 0;
	while (t < numbers.size()) {
		if (getPriority(t) == 1)
			return t;
		t++;
	}
	return 0;
}

void FonbookEntry::setDefault(size_t pos) {
	size_t oldPos = getDefault();
	if (pos != oldPos) {
		setPrioriy(0, oldPos);
		setPrioriy(1, pos);
		setQuickdial(getQuickdial(oldPos), pos);
		setVanity(getVanity(oldPos), pos);
		setQuickdial("", oldPos);
		setVanity("", oldPos);
	}
}

std::string FonbookEntry::getQuickdialFormatted(size_t pos) const {
	switch (getQuickdial(pos).length()) {
	case 1:
		return "**70" + getQuickdial(pos);
	case 2:
		return "**7"  + getQuickdial(pos);
	default:
		return "";
	}
}

std::string FonbookEntry::getQuickdial(size_t pos) const {
	// if no special type is given, the default "TYPES_COUNT" indicates,
	// that the correct type has to be determined first, i.e., priority == 1

	return numbers[pos == std::string::npos ? getDefault() : pos].quickdial;
}

void FonbookEntry::setQuickdial(std::string quickdial, size_t pos) { //TODO: sanity check
	numbers[pos == std::string::npos ? getDefault() : pos].quickdial = quickdial;
}

std::string FonbookEntry::getVanity(size_t pos) const {
	return numbers[pos == std::string::npos ? getDefault() : pos].vanity;
}

std::string FonbookEntry::getVanityFormatted(size_t pos) const {
	return getVanity(pos).length() ? "**8"+getVanity(pos) : "";
}

void FonbookEntry::setVanity(std::string vanity, size_t pos) { //TODO: sanity check
	numbers[pos == std::string::npos ? getDefault() : pos].vanity = vanity;
}

bool FonbookEntry::operator<(const FonbookEntry &fe) const {
	int cresult = this->name.compare(fe.name);
	if (cresult == 0)
		return false;
	return (cresult < 0);
}

size_t FonbookEntry::getSize() const {
	size_t size = 0;
	// ignore TYPE_NONE
	for (sNumber n : numbers)
		if (n.number.length())
			size++;
	return size;
}

class FonbookEntrySort {
private:
	bool ascending;
	FonbookEntry::eElements element;
public:
	FonbookEntrySort(FonbookEntry::eElements element = FonbookEntry::ELEM_NAME, bool ascending = true) {
		this->element   = element;
		this->ascending = ascending;
	}
	bool operator() (FonbookEntry fe1, FonbookEntry fe2){
		switch(element) {
		case FonbookEntry::ELEM_NAME:
			return (ascending ? (fe1.getName() < fe2.getName()) : (fe1.getName() > fe2.getName()));
			break;
//		case FonbookEntry::ELEM_TYPE:
//			return (ascending ? (fe1.getType() < fe2.getType()) : (fe1.getType() > fe2.getType()));
//			break;
//		case FonbookEntry::ELEM_NUMBER:
//			return (ascending ? (fe1.getNumber() < fe2.getNumber()) : (fe1.getNumber() > fe2.getNumber()));
//			break;
		case FonbookEntry::ELEM_IMPORTANT:
			return (ascending ? (fe1.isImportant() < fe2.isImportant()) : (fe1.isImportant() > fe2.isImportant()));
			break;
		case FonbookEntry::ELEM_QUICKDIAL: {
			int qd1 = atoi(fe1.getQuickdial().c_str());
			int qd2 = atoi(fe2.getQuickdial().c_str());
			return (ascending ? (qd1 < qd2) : (qd1 > qd2));
		}
			break;
		case FonbookEntry::ELEM_VANITY: {
			int vt1 = atoi(fe1.getVanity().c_str());
			int vt2 = atoi(fe2.getVanity().c_str());
			return (ascending ? (vt1 < vt2) : (vt1 > vt2));
		}
//			break;
//		case FonbookEntry::ELEM_PRIORITY:
//			return (ascending ? (fe1.getPriority() < fe2.getPriority()) : (fe1.getPriority() > fe2.getPriority()));
//			break;
		default:
			ERR("invalid element given for sorting.");
			return false;
		}
	}
};

Fonbook::Fonbook(std::string title, std::string techId, bool writeable)
: title(title), techId(techId), writeable(writeable)
{
	displayable = true;
	initialized = false;
	dirty       = false;
}

void Fonbook::SetDirty() {
	if (initialized)
		dirty = true;
}

Fonbook::sResolveResult Fonbook::resolveToName(std::string number) {
	sResolveResult result(number);
	if (number.length() > 0)
		for (auto fbe : fonbookList)
			for (auto fonbookNumber : fbe.getNumbers()) {
				if (fonbookNumber.number.length() > 0 && Tools::CompareNormalized(number, fonbookNumber.number) == 0) {
					result.name = fbe.getName();
					result.type = fonbookNumber.type;
					result.successful = true;
					return result;
				}
			}
	return result;
}

const FonbookEntry *Fonbook::retrieveFonbookEntry(size_t id) const {
	if (id >= getFonbookSize())
		return nullptr;
	return &fonbookList[id];
}

bool Fonbook::changeFonbookEntry(size_t id, FonbookEntry &fe) {
	if (id < getFonbookSize()) {
		fonbookList[id] = fe;
		SetDirty();
		return true;
	} else {
		return false;
	}
}

bool Fonbook::setDefault(size_t id, size_t pos) {
	if (id < getFonbookSize()) {
		fonbookList[id].setDefault(pos);
		SetDirty();
		return true;
	} else {
		return false;
	}
}

void Fonbook::addFonbookEntry(FonbookEntry &fe, size_t position) {
	if (position == std::string::npos || position > fonbookList.size())
		fonbookList.push_back(fe);
	else
		fonbookList.insert(fonbookList.begin() + position, fe);
	SetDirty();
}

bool Fonbook::deleteFonbookEntry(size_t id) {
	if (id < getFonbookSize()) {
		fonbookList.erase(fonbookList.begin() + id);
		SetDirty();
		return true;
	} else {
		return false;
	}
}

void Fonbook::save() {
	if (dirty && writeable) {
		write();
		dirty = false;
	}
}

void Fonbook::setInitialized(bool isInitialized) {
	initialized = isInitialized;
	if (displayable && isInitialized)
		INF(title << " initialized (" << getFonbookSize() << " entries).");
}

size_t Fonbook::getFonbookSize() const {
	if (initialized)
		return fonbookList.size();
	else
		return 0;
}

void Fonbook::sort(FonbookEntry::eElements element, bool ascending) {
	FonbookEntrySort fes(element, ascending);
	std::sort(fonbookList.begin(), fonbookList.end(), fes);
}

}
