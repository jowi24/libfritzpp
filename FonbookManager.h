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

#ifndef FONBOOKMANAGER_H
#define FONBOOKMANAGER_H

#include "Fonbooks.h"

namespace fritz{

class FonbookManager : public Fonbook
{
private:
	static FonbookManager* me;
	Fonbooks fonbooks;
	FonbookManager(bool saveOnShutdown);
	Fonbook *getActiveFonbook() const;
	size_t activeFonbookPos;
	bool saveOnShutdown;
public:
	virtual ~FonbookManager();
	/**
	 * Creates the central FonbookManager and activates certain fonbooks.
	 * This method instantiates the fonbookmanager. Following calls to
	 * getFonbookManager() return a reference to this object.
	 * CreateFonbookManager should be called before any call to getFonbookManager() to allow
	 * the configured fonbooks to initialize and fetch data which may be done in separate threads.
	 * If some of the fonbooks provided by libfritz++ shall be used, they need to be
	 * activated by this method. These fonbooks are used for reverse lookup on call events.
	 * The order of the fonbooks determines the priority regarding these lookups.
	 * Regarding queries to the fonbooks, a pointer is maintained which points to the currently
	 * "active" fonbook. This pointer can be moved, using FonbookManager::NextFonbook().
	 * @param the list of enabled fonbooks
	 * @param the currently "active" fonbook
	 * @param wether changes to fonbooks are saved on FonbookManager deletion
	 */
	static void CreateFonbookManager( std::vector <std::string> vFonbookID, std::string activeFonbook, bool saveOnShutdown = true);
	/**
	 * Returns the instance object of the FonbookManager casted to Fonbook.
	 */
	static Fonbook *GetFonbook();
	/**
	 * Returns the instance object of the FonbookManager
	 */
	static FonbookManager *GetFonbookManager();
	/*
	 * Deletes the FonbookManager instance.
	 */
	static void DeleteFonbookManager();
	/**
	 * Switch to next displayable phonebook.
	 * @return void
	 */
	void nextFonbook();
	/**
	 * Resolves the number given to the corresponding name.
	 * @param number to resolve
	 * @return resolved name and type or the number, if unsuccessful
	 */
	sResolveResult resolveToName(std::string number) override;
	/**
	 * Returns a specific telephonebook entry.
	 * @param id unique identifier of the requested entry
	 * @return the entry with key id or nullptr, if unsuccessful
	 */
	const FonbookEntry *retrieveFonbookEntry(size_t id) const override;
	/**
	 * Changes the Fonbook entry with the given id
	 * @param id unique identifier to the entry to be changed
	 * @param fe FonbookEntry with the new content
	 * @return true, if successful
	 */
	bool changeFonbookEntry(size_t id, FonbookEntry &fe) override;
	/**
	 * Sets the default number for a Fonbook entry with the given id
	 * @param id unique identifier to the entry to be changed
	 * @param type the new default
	 * @return true, if successful
	 */
	virtual bool setDefault(size_t id, size_t pos);
	/**
	 * Adds a new entry to the phonebook.
	 * @param fe a new phonebook entry
	 * @return true, if add was successful
	 */
	void addFonbookEntry(FonbookEntry &fe, size_t position = std::string::npos) override;
	/**
	 * Adds a new entry to the phonebook.
	 * @param id unique id to the entry to be deleted
	 * @return true, if deletion was successful
	 */
	bool deleteFonbookEntry(size_t id) override;
	/**
	 * Clears all entries from phonebook.
	 */
	void clear() override;
	/**
	 * Save pending changes.
	 * Can be called periodically to assert pending changes in a phone book are written.
	 */
	void save() override;
	/**
	 * Returns if it is possible to display the entries of this phonebook.
	 * @return true, if this phonebook has displayable entries. "Reverse lookup only" phonebooks must return false here.
	 */
	bool isDisplayable() const override;
	/**
	 * Returns if this phonebook is ready to use.
	 * @return true, if this phonebook is ready to use
	 */
	bool isInitialized() const override;
	/**
	 * Returns if this phonebook is writeable, e.g. entries can be added or modified.
	 * @return true, if this phonebook is writeable
	 */
	bool isWriteable() const override;
	/**
	 * Returns if this phonebook has changes that are not yet written.
	 * @return true, if changes are pending
	 */
	bool isModified() const override;
	/**
	 * Sets the initialized-status.
	 * @param isInititalized the value initialized is set to
	 */
	void setInitialized(bool isInitialized) override;
	/**
	 * Sorts the phonebook's entries by the given element and in given order.
	 * @param the element used for sorting
	 * @param true if sort order is ascending, false otherwise
	 */
	void sort(FonbookEntry::eElements element = FonbookEntry::ELEM_NAME, bool ascending = true) override;
	/**
	 *  Returns the number of entries in the telephonebook.
	 * @return the number of entries or cFonbook::npos, if requesting specific telephonebook entries is not possible for this telephonebook
	 */
	size_t getFonbookSize() const override;
	/**
	 *  Reloads the telephonebook's content
	 */
	void reload() override;
	/**
	 *  Returns a string that should be displayed as title in the menu when the telephonebook is displayed.
	 */
	std::string getTitle() const override;
	/**
	 * Returns the technical id of this phonebook. This id has to be unique among all phonebooks and is used when storing
	 * the plugin's setup.
	 * @return the technical id
	 */
	virtual std::string getTechId() const override;
	/**
	 *
	 */
	Fonbooks *getFonbooks();
};

}

#endif /*FONBOOKMANAGER_H_*/
