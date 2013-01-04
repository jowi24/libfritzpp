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

namespace fritz {

const char *Entities[97][2] = {
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
		CharSetConv *conv = new CharSetConv("UTF-8", CharSetConv::SystemCharacterTable());

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
		for (int i=0; i<97; i++) {
			std::string::size_type pos = s.find(Entities[i][0]);
			if (pos != std::string::npos) {
				s.replace(pos, strlen(Entities[i][0]), conv->Convert(Entities[i][1]));
				i--; //search for the same entity again
			}
		}
		delete (conv);
	}
	return s;
}

FonbookEntry::FonbookEntry(std::string name, bool important) {
	this->name      = name;
	this->important = important;
	for (size_t pos=0; pos < MAX_NUMBERS; pos++) {
		numbers[pos].priority = 0;
		numbers[pos].type     = TYPE_NONE;
	}
}

void FonbookEntry::AddNumber(size_t pos, std::string number, eType type, std::string quickdial, std::string vanity, int priority) {
	numbers[pos].number    = number;
	numbers[pos].type      = type;
	numbers[pos].quickdial = quickdial;
	numbers[pos].vanity    = vanity;
	numbers[pos].priority  = priority;
}

size_t FonbookEntry::GetDefault() const {
	size_t t = 0;
	while (t < MAX_NUMBERS) {
		if (GetPriority(t) == 1)
			return t;
		t++;
	}
	return 0;
}

void FonbookEntry::SetDefault(size_t pos) {
	size_t oldPos = GetDefault();
	if (pos != oldPos) {
		SetPrioriy(0, oldPos);
		SetPrioriy(1, pos);
		SetQuickdial(GetQuickdial(oldPos), pos);
		SetVanity(GetVanity(oldPos), pos);
		SetQuickdial("", oldPos);
		SetVanity("", oldPos);
	}
}

std::string FonbookEntry::GetQuickdialFormatted(size_t pos) const {
	switch (GetQuickdial(pos).length()) {
	case 1:
		return "**70" + GetQuickdial(pos);
	case 2:
		return "**7"  + GetQuickdial(pos);
	default:
		return "";
	}
}

std::string FonbookEntry::GetQuickdial(size_t pos) const {
	// if no special type is given, the default "TYPES_COUNT" indicates,
	// that the correct type has to be determined first, i.e., priority == 1

	return numbers[pos == std::string::npos ? GetDefault() : pos].quickdial;
}

void FonbookEntry::SetQuickdial(std::string quickdial, size_t pos) { //TODO: sanity check
	numbers[pos == std::string::npos ? GetDefault() : pos].quickdial = quickdial;
}

std::string FonbookEntry::GetVanity(size_t pos) const {
	return numbers[pos == std::string::npos ? GetDefault() : pos].vanity;
}

std::string FonbookEntry::GetVanityFormatted(size_t pos) const {
	return GetVanity(pos).length() ? "**8"+GetVanity(pos) : "";
}

void FonbookEntry::SetVanity(std::string vanity, size_t pos) { //TODO: sanity check
	numbers[pos == std::string::npos ? GetDefault() : pos].vanity = vanity;
}

bool FonbookEntry::operator<(const FonbookEntry &fe) const {
	int cresult = this->name.compare(fe.name);
	if (cresult == 0)
		return false;
	return (cresult < 0);
}

size_t FonbookEntry::GetSize() const {
	size_t size = 0;
	// ignore TYPE_NONE
	for (size_t type = 1; type < MAX_NUMBERS; type++)
		if (numbers[type].number.size())
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
			return (ascending ? (fe1.GetName() < fe2.GetName()) : (fe1.GetName() > fe2.GetName()));
			break;
//		case FonbookEntry::ELEM_TYPE:
//			return (ascending ? (fe1.getType() < fe2.getType()) : (fe1.getType() > fe2.getType()));
//			break;
//		case FonbookEntry::ELEM_NUMBER:
//			return (ascending ? (fe1.getNumber() < fe2.getNumber()) : (fe1.getNumber() > fe2.getNumber()));
//			break;
		case FonbookEntry::ELEM_IMPORTANT:
			return (ascending ? (fe1.IsImportant() < fe2.IsImportant()) : (fe1.IsImportant() > fe2.IsImportant()));
			break;
		case FonbookEntry::ELEM_QUICKDIAL: {
			int qd1 = atoi(fe1.GetQuickdial().c_str());
			int qd2 = atoi(fe2.GetQuickdial().c_str());
			return (ascending ? (qd1 < qd2) : (qd1 > qd2));
		}
			break;
		case FonbookEntry::ELEM_VANITY: {
			int vt1 = atoi(fe1.GetVanity().c_str());
			int vt2 = atoi(fe2.GetVanity().c_str());
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

Fonbook::sResolveResult Fonbook::ResolveToName(std::string number) {
	sResolveResult result(number);
	if (number.length() > 0)
		for (unsigned int pos=0; pos < fonbookList.size(); pos++)
			for (size_t num=0; num < FonbookEntry::MAX_NUMBERS; num++) {
				std::string fonbookNumber = fonbookList[pos].GetNumber(num);
				if (fonbookNumber.length() > 0 && Tools::CompareNormalized(number, fonbookNumber) == 0) {
					result.name = fonbookList[pos].GetName();
					result.type = fonbookList[pos].GetType(num);
					result.successful = true;
					return result;
				}
			}
	return result;
}

const FonbookEntry *Fonbook::RetrieveFonbookEntry(size_t id) const {
	if (id >= GetFonbookSize())
		return NULL;
	return &fonbookList[id];
}

bool Fonbook::ChangeFonbookEntry(size_t id, FonbookEntry &fe) {
	if (id < GetFonbookSize()) {
		fonbookList[id] = fe;
		SetDirty();
		return true;
	} else {
		return false;
	}
}

bool Fonbook::SetDefault(size_t id, size_t pos) {
	if (id < GetFonbookSize()) {
		fonbookList[id].SetDefault(pos);
		SetDirty();
		return true;
	} else {
		return false;
	}
}

void Fonbook::AddFonbookEntry(FonbookEntry &fe, size_t position) {
	if (position == std::string::npos || position > fonbookList.size())
		fonbookList.push_back(fe);
	else
		fonbookList.insert(fonbookList.begin() + position, fe);
	SetDirty();
}

bool Fonbook::DeleteFonbookEntry(size_t id) {
	if (id < GetFonbookSize()) {
		fonbookList.erase(fonbookList.begin() + id);
		SetDirty();
		return true;
	} else {
		return false;
	}
}

void Fonbook::Save() {
	if (dirty && writeable) {
		Write();
		dirty = false;
	}
}

void Fonbook::setInitialized(bool isInitialized) {
	initialized = isInitialized;
	if (displayable && isInitialized)
		INF(title << " initialized (" << GetFonbookSize() << " entries).");
}

size_t Fonbook::GetFonbookSize() const {
	if (initialized)
		return fonbookList.size();
	else
		return 0;
}

void Fonbook::Sort(FonbookEntry::eElements element, bool ascending) {
	FonbookEntrySort fes(element, ascending);
	std::sort(fonbookList.begin(), fonbookList.end(), fes);
}

}
