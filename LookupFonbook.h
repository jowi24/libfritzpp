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

#ifndef LOOKUPFONBOOK_H_
#define LOOKUPFONBOOK_H_

#include "Fonbook.h"

namespace fritz {

class LookupFonbook: public Fonbook {
public:
	LookupFonbook(std::string title, std::string techId, bool writeable = false);
	virtual ~LookupFonbook();
	/**
	 * Take action to fill phonebook with content.
	 * Initialize() may be called more than once per session.
	 * @return if initialization was successful
	 */
	bool initialize() override;
	/**
	 * Resolves the number given to the corresponding name.
	 * @param number to resolve
	 * @return resolved name and type or the number, if unsuccessful
	 */
	sResolveResult resolveToName(std::string number) override;
	/**
	 * Resolves number doing a (costly) lookup
	 * @param number to resolve
	 * @return resolved name and type or number, if not successful
	 */
	virtual sResolveResult lookup(std::string number) const;
	/**
	 *  Returns the number of entries in the telephonebook.
	 * @return the number of entries
	 */
	size_t getFonbookSize() const override { return 0; }
	/**
	 * Returns a specific telephonebook entry.
	 * @param id unique identifier of the requested entry
	 * @return the entry with key id or nullptr, if unsuccessful
	 */
	const FonbookEntry *retrieveFonbookEntry(size_t id __attribute__((unused))) const override { return nullptr; }
};

} /* namespace fritz */
#endif /* LOOKUPFONBOOK_H_ */
