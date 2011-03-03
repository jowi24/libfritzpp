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

#ifndef FONBOOK_H
#define FONBOOK_H

#include <string>
#include <vector>

namespace fritz {

/**
 * General telephonebook entry.
 * This defines the class, to be used by every phone book implementation.
 */


class FonbookEntry {
public:
	enum eType {
		TYPE_NONE,
		TYPE_HOME,
		TYPE_MOBILE,
		TYPE_WORK,
		TYPES_COUNT
	};

	enum eElements {
		ELEM_NAME   = 0,
		ELEM_TYPE   = 1,
		ELEM_NUMBER = 2,
		ELEM_IMPORTANT,
		ELEM_QUICKDIAL,
		ELEM_VANITY,
		ELEM_PRIORITY,
		ELEMS_COUNT
	};
	struct sNumber {
		std::string number;
		std::string quickdial;
		std::string vanity;
		int priority;
	};
private:
	std::string name;
	bool important;
	sNumber numbers[TYPES_COUNT];
public:
	/*
	 * Constructs a new FonbookEntry object
	 * @param name Full name of contact
	 * @param important Whether contact is flagged as important
	 */
	explicit FonbookEntry(std::string name, bool important = false);
	/*
	 * Copy constructor
	 * @param the fonbook entry to be copied
	 */
	FonbookEntry(const FonbookEntry *fe) { *this = *fe; }
	/**
	 * Adds new number to this contact
	 * @param number The number to be added
	 * @param type The number type
	 * @param quickdial The quickdial extension
	 * @param vanity The vanity extension
	 * @param prority '1' marks the default number of this contact, otherwise 0
	 */
	void AddNumber(std::string number, eType type = TYPE_NONE, std::string quickdial = "", std::string vanity = "", int priority = 0);
	std::string GetName() const { return name; }
	void SetName(std::string name) { this->name = name; }
	std::string GetNumber(eType type) const { return numbers[type].number; }
	void SetNumber(std::string number, eType type) { numbers[type].number = number; }
	bool IsImportant() const { return important; }
	void SetImportant(bool important) { this->important = important; }
	eType GetDefaultType() const;
	void SetDefaultType(eType type);
	std::string GetQuickdialFormatted(eType type = TYPES_COUNT) const;
	std::string GetQuickdial(eType type = TYPES_COUNT) const;
	void SetQuickdial(std::string quickdial, eType type = TYPES_COUNT);
	std::string GetVanity(eType type = TYPES_COUNT) const;
	std::string GetVanityFormatted(eType type = TYPES_COUNT) const;
	void SetVanity(std::string vanity, eType type = TYPES_COUNT);
	int GetPriority(eType type) const { return numbers[type].priority; }
	void SetPrioriy(int priority, eType type) { numbers[type].priority = priority; }
	bool operator<(const FonbookEntry & fe) const;
	/*
	 * Get number of typed numbers (TYPE_NONE is ignored)
	 * @return count of different numbers available
	 */
	size_t GetSize();
};

inline FonbookEntry::eType& operator++(FonbookEntry::eType& t) {
	return t = static_cast<FonbookEntry::eType>(static_cast<int>(t) + 1);
}
inline FonbookEntry::eType operator++(FonbookEntry::eType& t, int) {
	FonbookEntry::eType tmp(t);
	++t;
	return tmp;
}


/**
 * General telephonebook base class.
 * All specific telephonebooks have to inherit from this class.
 */

class Fonbook
{
private:
	/**
	 * True, if this phonebook is ready to use.
	 */
	bool initialized;
	/**
	 * True, if changes are pending that are not yet saved
	 */
	bool dirty;
	/**
	 * Sets dirty member if applicable
	 */
	void SetDirty();
    /**
     * Data structure for storing the phonebook.
     */
	std::vector<FonbookEntry> fonbookList;
protected:
	/**
	 * The constructor may only be used by cFonbookManager.
	 * Subclasses must make their constructor private, too.
	 */
	Fonbook();
	/**
	 * Method to persist contents of the phone book (if writeable)
	 */
	virtual void Write() { }
	/**
	 * The descriptive title of this phonebook.
	 */
	std::string title;
	/**
	 * The technical id of this phonebook (should be a short letter code).
	 */
	std::string techId;
	/**
	 * True, if this phonebook has displayable entries.
	 */
	bool displayable;
	/**
	 * True, if this phonebook is writeable
	 */
	bool writeable;

public:
	struct sResolveResult {
		std::string name;
		FonbookEntry::eType type;
	};
	virtual ~Fonbook() { }
	/**
	 * Take action to fill phonebook with content.
	 * Initialize() may be called more than once per session.
	 * @return if initialization was successful
	 */
	virtual bool Initialize(void) { return true; }
	/**
	 * Resolves the number given to the corresponding name.
	 * @param number to resolve
	 * @return resolved name and type or the number, if unsuccessful
	 */
	virtual sResolveResult ResolveToName(std::string number) const;
	/**
	 * Returns a specific telephonebook entry.
	 * @param id unique identifier of the requested entry
	 * @return the entry with key id or NULL, if unsuccessful
	 */
	virtual const FonbookEntry *RetrieveFonbookEntry(size_t id) const;
	/**
	 * Changes the Fonbook entry with the given id
	 * @param id unique identifier to the entry to be changed
	 * @param fe FonbookEntry with the new content
	 * @return true, if successful
	 */
	virtual bool ChangeFonbookEntry(size_t id, FonbookEntry &fe);
	/**
	 * Sets the default number for a Fonbook entry with the given id
	 * @param id unique identifier to the entry to be changed
	 * @param type the new default
	 * @return true, if successful
	 */
	virtual bool SetDefaultType(size_t id, fritz::FonbookEntry::eType type);
	/**
	 * Adds a new entry to the phonebook.
	 * @param fe a new phonebook entry
	 */
	virtual void AddFonbookEntry(FonbookEntry &fe, size_t position = std::string::npos);
	/**
	 * Adds a new entry to the phonebook.
	 * @param id unique id to the entry to be deleted
	 * @return true, if deletion was successful
	 */
	virtual bool DeleteFonbookEntry(size_t id);
	/**
	 * Clears all entries from phonebook.
	 */
	virtual void Clear() { SetDirty(); fonbookList.clear(); }
	/**
	 * Save pending changes.
	 * Can be called periodically to assert pending changes in a phone book are written.
	 */
	void Save();
	/**
	 * Returns if it is possible to display the entries of this phonebook.
	 * @return true, if this phonebook has displayable entries. "Reverse lookup only" phonebooks must return false here.
	 */
	virtual bool isDisplayable() const { return displayable; }
	/**
	 * Returns if this phonebook is ready to use.
	 * @return true, if this phonebook is ready to use
	 */
	virtual bool isInitialized() const { return initialized; }
	/**
	 * Returns if this phonebook is writeable, e.g. entries can be added or modified.
	 * @return true, if this phonebook is writeable
	 */
	virtual bool isWriteable() const { return writeable; }
	/**
	 * Sets the initialized-status.
	 * @param isInititalized the value initialized is set to
	 */
	virtual void setInitialized(bool isInitialized);
	/**
	 *  Returns the number of entries in the telephonebook.
	 * @return the number of entries
	 */
	virtual size_t GetFonbookSize() const;
	/**
	 *  Reloads the telephonebook's content
	 */
	virtual void Reload() { }
	/**
	 *  Returns a string that should be displayed as title in the menu when the telephonebook is displayed.
	 * @return the long title of this phonebook
	 */
	virtual std::string GetTitle() { return title; }
	/**
	 * Returns the technical id of this phonebook. This id has to be unique among all phonebooks and is used when storing
	 * the plugin's setup.
	 * @return the technical id
	 */
	virtual std::string GetTechId() { return techId; }
	/**
	 * Sorts the phonebook's entries by the given element and in given order.
	 * @param the element used for sorting
	 * @param true if sort order is ascending, false otherwise
	 */
	virtual void Sort(FonbookEntry::eElements element = FonbookEntry::ELEM_NAME, bool ascending = true);
};

}

#endif /*FONBOOK_H_*/
