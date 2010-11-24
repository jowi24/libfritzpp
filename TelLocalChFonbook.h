/*
 * libfritz++
 *
 * Copyright (C) 2007-2010 Joachim Wilke <libfritz@joachim-wilke.de>
 * TelLocalChFonbook created by Christian Richter <cr@crichter.net>
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


#ifndef TelLocalChFonbook_H_
#define TelLocalChFonbook_H_

#include <string>
#include "Fonbook.h"

namespace fritz{

class TelLocalChFonbook : public Fonbook
{
	friend class cFactory;
	friend class FonbookManager;
private:
	TelLocalChFonbook();
public:
	virtual ~TelLocalChFonbook();
	/**
	 * Take action to fill phonebook with content.
	 * Initialize() may be called more than once per session.
	 * @return if initialization was successful
	 */
	virtual bool Initialize();
	/**
	 * Resolves the number given to the corresponding name.
	 * @param number to resolve
	 * @return resolved name and type or the number, if unsuccessful
	 */
	virtual sResolveResult ResolveToName(std::string number);
	/**
	 *  Returns the number of entries in the telephonebook.
	 * @return the number of entries
	 */
	virtual size_t GetFonbookSize() { return 0; }
	/**
	 * Returns a specific telephonebook entry.
	 * @param id unique identifier of the requested entry
	 * @return the entry with key id or NULL, if unsuccesful
	 */
	virtual FonbookEntry *RetrieveFonbookEntry(size_t id __attribute__((unused))) { return NULL; }

};

}

#endif /*TelLocalChFonbook_H_*/