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

#include <algorithm>
#include "Fonbook.h"
#include "Tools.h"
#include "Config.h"

namespace fritz{

FonbookEntry::FonbookEntry(std::string name, bool important) {
	this->name      = name;
	this->important = important;
	for (int type=0; type < TYPES_COUNT; type++)
		numbers[type].priority = 0;
}

void FonbookEntry::AddNumber(std::string number, eType type, std::string quickdial, std::string vanity, int priority) {
	numbers[type].number    = number;
	numbers[type].quickdial = quickdial;
	numbers[type].vanity    = vanity;
	numbers[type].priority  = priority;
}

FonbookEntry::eType FonbookEntry::GetDefaultType() const {
	eType t = (eType) 0;
	while (t < TYPES_COUNT) {
		if (GetPriority(t) == 1)
			return t;
		t = (eType) (t+1);
	}
	return TYPE_NONE;
}

void FonbookEntry::SetDefaultType(eType type) {
	eType oldType = GetDefaultType();
	if (type != oldType) {
		SetPrioriy(0, oldType);
		SetPrioriy(1, type);
		SetQuickdial(GetQuickdial(oldType), type);
		SetVanity(GetVanity(oldType), type);
		SetQuickdial("", oldType);
		SetVanity("", oldType);
	}
}

std::string FonbookEntry::GetQuickdialFormatted(eType type) const {
	switch (GetQuickdial(type).length()) {
	case 1:
		return "**70" + GetQuickdial(type);
	case 2:
		return "**7"  + GetQuickdial(type);
	default:
		return "";
	}
}

std::string FonbookEntry::GetQuickdial(eType type) const {
	// if no special type is given, the default "TYPES_COUNT" indicates,
	// that the correct type has to be determined first, i.e., priority == 1

	return numbers[type == TYPES_COUNT ? GetDefaultType() : type].quickdial;
}

void FonbookEntry::SetQuickdial(std::string quickdial, eType type) { //TODO: sanity check
	numbers[type == TYPES_COUNT ? GetDefaultType() : type].quickdial = quickdial;
}

std::string FonbookEntry::GetVanity(eType type) const {
	return numbers[type == TYPES_COUNT ? GetDefaultType() : type].vanity;
}

std::string FonbookEntry::GetVanityFormatted(eType type) const {
	return GetVanity(type).length() ? "**8"+GetVanity(type) : "";
}

void FonbookEntry::SetVanity(std::string vanity, eType type) { //TODO: sanity check
	numbers[type == TYPES_COUNT ? GetDefaultType() : type].vanity = vanity;
}

bool FonbookEntry::operator<(const FonbookEntry &fe) const {
	int cresult = this->name.compare(fe.name);
	if (cresult == 0)
		return false;
	return (cresult < 0);
}

size_t FonbookEntry::GetSize() {
	size_t size = 0;
	// ignore TYPE_NONE
	for (int type = 1; type < TYPES_COUNT; type++)
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

Fonbook::Fonbook()
{
	title       = "Phonebook";
	techId      = "BASE";
	displayable = false;
	initialized = false;
	writeable   = false;
	dirty       = false;
}

void Fonbook::SetDirty() {
	if (initialized)
		dirty = true;
}

Fonbook::sResolveResult Fonbook::ResolveToName(std::string number) {
	sResolveResult result;
	result.name = number;
	result.type = FonbookEntry::TYPE_NONE;
	if (number.length() > 0)
		for (unsigned int pos=0; pos < fonbookList.size(); pos++)
			for (int type=0; type < FonbookEntry::TYPES_COUNT; type++) {
				std::string fonbookNumber = fonbookList[pos].GetNumber((FonbookEntry::eType)type);
				if (fonbookNumber.length() > 0 && Tools::CompareNormalized(number, fonbookNumber) == 0) {
					result.name = fonbookList[pos].GetName();
					result.type = (FonbookEntry::eType) type;
					return result;
				}
			}
	return result;
}

const FonbookEntry *Fonbook::RetrieveFonbookEntry(size_t id) {
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

bool Fonbook::SetDefaultType(size_t id, fritz::FonbookEntry::eType type) {
	if (id < GetFonbookSize()) {
		fonbookList[id].SetDefaultType(type);
		SetDirty();
		return true;
	} else {
		return false;
	}
}

void Fonbook::AddFonbookEntry(FonbookEntry &fe) {
	fonbookList.push_back(fe);
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

size_t Fonbook::GetFonbookSize() {
	return fonbookList.size();
}

void Fonbook::Sort(FonbookEntry::eElements element, bool ascending) {
	FonbookEntrySort fes(element, ascending);
	std::sort(fonbookList.begin(), fonbookList.end(), fes);
}

}
