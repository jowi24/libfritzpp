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
#include <liblog++/Log.h>

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
			fb->initialize();
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
	if (!getActiveFonbook())
		nextFonbook();
}

FonbookManager::~FonbookManager()
{
	for (auto fonbook : fonbooks) {
		DBG("deleting fonbook with ID: " << fonbook->getTechId());
		// save pending changes
		if (saveOnShutdown)
			fonbook->save();
		delete(fonbook);
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
			for (auto id : vFonbookID)
				if (id.compare(activeFonbook) == 0) {
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

void FonbookManager::nextFonbook() {
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

Fonbook::sResolveResult FonbookManager::resolveToName(std::string number) {
	sResolveResult result(number);
	for (auto id  : gConfig->getFonbookIDs()) {
		result = fonbooks[id]->resolveToName(number);
		DBG("ResolveToName: " << id << " " << (gConfig->logPersonalInfo() ? result.name : HIDDEN));
		if (result.successful)
			return result;
	}
	return result;
}

Fonbook *FonbookManager::getActiveFonbook() const {
	if (activeFonbookPos == std::string::npos) {
		return nullptr;
	}
	return fonbooks[gConfig->getFonbookIDs()[activeFonbookPos]];
}

const FonbookEntry *FonbookManager::retrieveFonbookEntry(size_t id) const {
	return getActiveFonbook() ? getActiveFonbook()->retrieveFonbookEntry(id) : nullptr;
}

bool FonbookManager::changeFonbookEntry(size_t id, FonbookEntry &fe) {
	return getActiveFonbook() ? getActiveFonbook()->changeFonbookEntry(id, fe) : false;
}

bool FonbookManager::setDefault(size_t id, size_t pos) {
	return getActiveFonbook() ? getActiveFonbook()->setDefault(id, pos) : false;
}

void FonbookManager::addFonbookEntry(FonbookEntry &fe, size_t position) {
	if (getActiveFonbook())
		getActiveFonbook()->addFonbookEntry(fe, position);
}

bool FonbookManager::deleteFonbookEntry(size_t id) {
	return getActiveFonbook() ? getActiveFonbook()->deleteFonbookEntry(id) : false;
}

void FonbookManager::clear() {
	if (getActiveFonbook())
		getActiveFonbook()->clear();
}

void FonbookManager::save() {
	if (getActiveFonbook())
			getActiveFonbook()->save();
}

bool FonbookManager::isDisplayable() const {
	return getActiveFonbook() ? getActiveFonbook()->isDisplayable() : false;
}

bool FonbookManager::isInitialized() const {
	return getActiveFonbook() ? getActiveFonbook()->isInitialized() : false;
}

bool FonbookManager::isWriteable() const {
	return getActiveFonbook() ? getActiveFonbook()->isWriteable() : false;
}

bool FonbookManager::isModified() const {
	return getActiveFonbook() ? getActiveFonbook()->isModified() : false;
}

void FonbookManager::setInitialized(bool isInitialized) {
	if (getActiveFonbook())
		getActiveFonbook()->setInitialized(isInitialized);
}

void FonbookManager::sort(FonbookEntry::eElements element, bool ascending){
	if (getActiveFonbook())
		getActiveFonbook()->sort(element, ascending);
}

size_t FonbookManager::getFonbookSize() const {
	return getActiveFonbook() ? getActiveFonbook()->getFonbookSize() : 0;
}

std::string FonbookManager::getTitle() const {
	return getActiveFonbook() ? getActiveFonbook()->getTitle() : "";
}

std::string FonbookManager::getTechId() const {
	return getActiveFonbook() ? getActiveFonbook()->getTechId() : "";
}

void FonbookManager::reload() {
	for (auto id : gConfig->getFonbookIDs()) {
		fonbooks[id]->reload();
	}
}

Fonbooks *FonbookManager::getFonbooks() {
	return &fonbooks;
}

}
