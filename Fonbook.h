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
		eType       type;
		std::string quickdial;
		std::string vanity;
		int         priority;
	};
private:
	std::string name;
	bool important;
	std::vector<sNumber> numbers;
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
	void addNumber(std::string number, eType type = TYPE_NONE, std::string quickdial = "", std::string vanity = "", int priority = 0);
	std::string getName() const { return name; }
	void setName(std::string name) { this->name = name; }

    #define CHECK(x) if (numbers.size() <= pos) return x;

    std::string getNumber(size_t pos) const { CHECK(""); return numbers[pos].number; }
	const std::vector<sNumber> &getNumbers() const { return numbers; }
    void setNumber(std::string number,size_t pos) { CHECK(); numbers[pos].number = number; }
    eType getType(size_t pos) const { CHECK(FonbookEntry::TYPE_NONE); return numbers[pos].type; }
	void setType(eType type, size_t pos) { numbers[pos].type = type; }
	bool isImportant() const { return important; }
	void setImportant(bool important) { this->important = important; }
	size_t getDefault() const;
	void setDefault(size_t pos);
	std::string getQuickdialFormatted( size_t pos = std::string::npos) const;
	std::string getQuickdial(size_t pos = std::string::npos) const;
	void setQuickdial(std::string quickdial, size_t pos = std::string::npos);
	std::string getVanity(size_t pos = std::string::npos) const;
	std::string getVanityFormatted(size_t pos = std::string::npos) const;
	void setVanity(std::string vanity, size_t pos = std::string::npos);
    int getPriority(size_t pos) const { CHECK(0); return numbers[pos].priority; }
	void setPrioriy(int priority, size_t pos) { numbers[pos].priority = priority; }
	bool operator<(const FonbookEntry & fe) const;
	/*
	 * Get number of typed numbers (TYPE_NONE is ignored)
	 * @return count of different numbers available
	 */
	size_t getSize() const;
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
	 * The descriptive title of this phonebook.
	 */
	std::string title;
	/**
	 * The technical id of this phonebook (should be a short letter code).
	 */
	std::string techId;
	/**
	 * True, if this phonebook is writeable
	 */
	bool writeable;
    /**
     * Data structure for storing the phonebook.
     */
	std::vector<FonbookEntry> fonbookList;
protected:
	/**
	 * The constructor may only be used by cFonbookManager.
	 * Subclasses must make their constructor private, too.
	 */
	Fonbook(std::string title, std::string techId, bool writeable = false);
	/**
	 * Method to persist contents of the phone book (if writeable)
	 */
	virtual void write() { }
	/**
	 * True, if this phonebook has displayable entries.
	 */
	bool displayable;
	/**
	 *
	 */
	const std::vector<FonbookEntry> &getFonbookList() const { return fonbookList; }
public:
	struct sResolveResult {
		sResolveResult(std::string name, FonbookEntry::eType type = FonbookEntry::TYPE_NONE, bool successful = false)
		: name(name), type(type), successful(successful) {}
		std::string name;
		FonbookEntry::eType type;
		bool successful;
	};
	virtual ~Fonbook() { }
	/**
	 * Take action to fill phonebook with content.
	 * Initialize() may be called more than once per session.
	 * @return if initialization was successful
	 */
	virtual bool initialize(void) { return true; }
	/**
	 * Resolves the number given to the corresponding name.
	 * @param number to resolve
	 * @return resolved name and type or the number, if unsuccessful
	 */
	virtual sResolveResult resolveToName(std::string number);
	/**
	 * Returns a specific telephonebook entry.
	 * @param id unique identifier of the requested entry
	 * @return the entry with key id or nullptr, if unsuccessful
	 */
	virtual const FonbookEntry *retrieveFonbookEntry(size_t id) const;
	/**
	 * Changes the Fonbook entry with the given id
	 * @param id unique identifier to the entry to be changed
	 * @param fe FonbookEntry with the new content
	 * @return true, if successful
	 */
	virtual bool changeFonbookEntry(size_t id, FonbookEntry &fe);
	/**
	 * Sets the default number for a Fonbook entry with the given id
	 * @param id unique identifier to the entry to be changed
	 * @param pos the new default number
	 * @return true, if successful
	 */
	virtual bool setDefault(size_t id, size_t pos);
	/**
	 * Adds a new entry to the phonebook.
	 * @param fe a new phonebook entry
	 * @param position position at which fe is added (at the end of the list per default)
	 */
	virtual void addFonbookEntry(FonbookEntry &fe, size_t position = std::string::npos);
	/**
	 * Adds a new entry to the phonebook.
	 * @param id unique id to the entry to be deleted
	 * @return true, if deletion was successful
	 */
	virtual bool deleteFonbookEntry(size_t id);
	/**
	 * Clears all entries from phonebook.
	 */
	virtual void clear() { SetDirty(); fonbookList.clear(); }
	/**
	 * Save pending changes.
	 * Can be called periodically to assert pending changes in a phone book are written.
	 */
	virtual void save();
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
	 * Returns if this phonebook has changes that are not yet written.
	 * @return true, if changes are pending
	 */
	virtual bool isModified() const { return dirty; }
	/**
	 * Sets the initialized-status.
	 * @param isInititalized the value initialized is set to
	 */
	virtual void setInitialized(bool isInitialized);
	/**
	 * Sets writeable to true
	 */
	virtual void setWriteable() { writeable = true; }
	/**
	 *  Returns the number of entries in the telephonebook.
	 * @return the number of entries
	 */
	virtual size_t getFonbookSize() const;
	/**
	 *  Reloads the telephonebook's content
	 */
	virtual void reload() { }
	/**
	 *  Returns a string that should be displayed as title in the menu when the telephonebook is displayed.
	 * @return the long title of this phonebook
	 */
	virtual std::string getTitle() const { return title; }
	/**
	 * Returns the technical id of this phonebook. This id has to be unique among all phonebooks and is used when storing
	 * the plugin's setup.
	 * @return the technical id
	 */
	virtual std::string getTechId() const { return techId; }
	/**
	 * Sorts the phonebook's entries by the given element and in given order.
	 * @param the element used for sorting
	 * @param true if sort order is ascending, false otherwise
	 */
	virtual void sort(FonbookEntry::eElements element = FonbookEntry::ELEM_NAME, bool ascending = true);
};

}

#endif /*FONBOOK_H_*/
