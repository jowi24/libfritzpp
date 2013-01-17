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


#include "FonbookManager.h"

#include <string>

#include "Config.h"
#include "FritzFonbook.h"
#include "LocalFonbook.h"
#include "Nummerzoeker.h"
#include "OertlichesFonbook.h"
#include "TelLocalChFonbook.h"
#include "Log.h"

namespace fritz{

FonbookManager* FonbookManager::me = nullptr;

FonbookManager::FonbookManager(bool saveOnShutdown)
:Fonbook("Manager", "MNGR")
{
	this->saveOnShutdown = saveOnShutdown;
	// create all fonbooks
	fonbooks.push_back(new FritzFonbook());
	fonbooks.push_back(new OertlichesFonbook());
	fonbooks.push_back(new TelLocalChFonbook());
	fonbooks.push_back(new NummerzoekerFonbook());
	fonbooks.push_back(new LocalFonbook());
	// initialize the fonbooks that are used
	for (int i=gConfig->getFonbookIDs().size()-1; i>=0; i--) {
		Fonbook *fb = fonbooks[gConfig->getFonbookIDs()[i]];
		if (fb)
			fb->Initialize();
		else
			gConfig->getFonbookIDs().erase(gConfig->getFonbookIDs().begin()+i);
	}
	// track the currently active (=shown) fonbook
	activeFonbookPos = std::string::npos;
    // set activeFonbookPos to the last displayed fonbook (if this is still valid and displayable)
	size_t pos = 0;
	while (pos < gConfig->getFonbookIDs().size() &&
			gConfig->getFonbookIDs()[pos] != gConfig->getActiveFonbook() )
		pos++;
	if (pos < gConfig->getFonbookIDs().size()) {
		if (fonbooks[gConfig->getFonbookIDs()[pos]]->isDisplayable())
			activeFonbookPos = pos;
	}
	// if no valid phone book is selected, advance to the next valid one
	if (!GetActiveFonbook())
		NextFonbook();
}

FonbookManager::~FonbookManager()
{
	for (size_t i= 0; i < fonbooks.size(); i++) {
		DBG("deleting fonbook with ID: " << fonbooks[i]->GetTechId());
		// save pending changes
		if (saveOnShutdown)
			fonbooks[i]->Save();
		delete(fonbooks[i]);
	}
}

void FonbookManager::CreateFonbookManager(std::vector <std::string> vFonbookID, std::string activeFonbook, bool saveOnShutdown) {
	if (gConfig) {
		// if there is already a FonbookManager, delete it, so it can adapt to configuration changes
		DeleteFonbookManager();
		// save new list of fonbook ids
		gConfig->setFonbookIDs(vFonbookID);
		// check if activeFonbook is valid
		if (activeFonbook.size() > 0) {
			bool activeFonbookValid = false;
			for (unsigned int pos = 0; pos < vFonbookID.size(); pos++)
				if (vFonbookID[pos].compare(activeFonbook) == 0) {
					activeFonbookValid = true;
					break;
				}
			if (activeFonbookValid)
				gConfig->setActiveFonbook(activeFonbook);
			else
				ERR("Invalid call parameter. ActiveFonbook '" << activeFonbook << "' is not enabled or unknown");
		}
		// create fonbookmanger (was deleted above) so that it can initialize all fonbooks
		me = new FonbookManager(saveOnShutdown);
	} else {
		ERR("Wrong call sequence. Configuration does not exist when trying to create FonbookManager." );
	}
}

Fonbook* FonbookManager::GetFonbook() {
	return (Fonbook*) me;
}

FonbookManager* FonbookManager::GetFonbookManager() {
	return me;
}

void FonbookManager::DeleteFonbookManager() {
	if (me) {
		DBG("deleting Fonbook Manager");
		delete me;
		me = nullptr;
	}
}

void FonbookManager::NextFonbook() {
	size_t pos = activeFonbookPos + 1;
    // no phonebooks -> no switching
	if ( gConfig->getFonbookIDs().size() == 0)
		return;
	while (pos < gConfig->getFonbookIDs().size() &&
			fonbooks[gConfig->getFonbookIDs()[pos]]->isDisplayable() == false)
		pos++;
	// if no displayable fonbook found -> start from beginning
	if (pos == gConfig->getFonbookIDs().size()) {
		pos = 0;
		while (pos < gConfig->getFonbookIDs().size() &&
				fonbooks[gConfig->getFonbookIDs()[pos]]->isDisplayable() == false)
			pos++;
		// if this fails, too, just return npos
		if (pos == gConfig->getFonbookIDs().size()) {
			pos = std::string::npos;
		}
	}
	activeFonbookPos = pos;
	if (activeFonbookPos != std::string::npos) {
		// save the tech-id of the active fonbook in setup
		gConfig->setActiveFonbook( gConfig->getFonbookIDs()[pos] );
	}
}

Fonbook::sResolveResult FonbookManager::ResolveToName(std::string number) {
	sResolveResult result(number);
	for (size_t i=0; i<gConfig->getFonbookIDs().size(); i++) {
		result = fonbooks[gConfig->getFonbookIDs()[i]]->ResolveToName(number);
		DBG("ResolveToName: " << gConfig->getFonbookIDs()[i] << " " << (gConfig->logPersonalInfo() ? result.name : HIDDEN));
		if (result.successful)
			return result;
	}
	return result;
}

Fonbook *FonbookManager::GetActiveFonbook() const {
	if (activeFonbookPos == std::string::npos) {
		return nullptr;
	}
	return fonbooks[gConfig->getFonbookIDs()[activeFonbookPos]];
}

const FonbookEntry *FonbookManager::RetrieveFonbookEntry(size_t id) const {
	return GetActiveFonbook() ? GetActiveFonbook()->RetrieveFonbookEntry(id) : nullptr;
}

bool FonbookManager::ChangeFonbookEntry(size_t id, FonbookEntry &fe) {
	return GetActiveFonbook() ? GetActiveFonbook()->ChangeFonbookEntry(id, fe) : false;
}

bool FonbookManager::SetDefault(size_t id, size_t pos) {
	return GetActiveFonbook() ? GetActiveFonbook()->SetDefault(id, pos) : false;
}

void FonbookManager::AddFonbookEntry(FonbookEntry &fe, size_t position) {
	if (GetActiveFonbook())
		GetActiveFonbook()->AddFonbookEntry(fe, position);
}

bool FonbookManager::DeleteFonbookEntry(size_t id) {
	return GetActiveFonbook() ? GetActiveFonbook()->DeleteFonbookEntry(id) : false;
}

void FonbookManager::Clear() {
	if (GetActiveFonbook())
		GetActiveFonbook()->Clear();
}

void FonbookManager::Save() {
	if (GetActiveFonbook())
			GetActiveFonbook()->Save();
}

bool FonbookManager::isDisplayable() const {
	return GetActiveFonbook() ? GetActiveFonbook()->isDisplayable() : false;
}

bool FonbookManager::isInitialized() const {
	return GetActiveFonbook() ? GetActiveFonbook()->isInitialized() : false;
}

bool FonbookManager::isWriteable() const {
	return GetActiveFonbook() ? GetActiveFonbook()->isWriteable() : false;
}

bool FonbookManager::isModified() const {
	return GetActiveFonbook() ? GetActiveFonbook()->isModified() : false;
}

void FonbookManager::setInitialized(bool isInitialized) {
	if (GetActiveFonbook())
		GetActiveFonbook()->setInitialized(isInitialized);
}

void FonbookManager::Sort(FonbookEntry::eElements element, bool ascending){
	if (GetActiveFonbook())
		GetActiveFonbook()->Sort(element, ascending);
}

size_t FonbookManager::GetFonbookSize() const {
	return GetActiveFonbook() ? GetActiveFonbook()->GetFonbookSize() : 0;
}

std::string FonbookManager::GetTitle() const {
	return GetActiveFonbook() ? GetActiveFonbook()->GetTitle() : "";
}

std::string FonbookManager::GetTechId() const {
	return GetActiveFonbook() ? GetActiveFonbook()->GetTechId() : "";
}

void FonbookManager::Reload() {
	for (size_t i=0; i<gConfig->getFonbookIDs().size(); i++) {
		fonbooks[gConfig->getFonbookIDs()[i]]->Reload();
	}
}

Fonbooks *FonbookManager::GetFonbooks() {
	return &fonbooks;
}

}
