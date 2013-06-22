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

#include "LookupFonbook.h"

#include "Config.h"

namespace fritz {

LookupFonbook::LookupFonbook(std::string title, std::string techId, bool writeable)
:Fonbook(title, techId, writeable) {
	displayable = false;
}

LookupFonbook::~LookupFonbook() {}

bool LookupFonbook::initialize() {
	setInitialized(true);
	return true;
}

Fonbook::sResolveResult LookupFonbook::resolveToName(std::string number) {
	// First, try to get a cached result
	sResolveResult resolve = Fonbook::resolveToName(number);
	// Second, to lookup (e.g., via HTTP)
	if (! resolve.successful) {
		resolve = lookup(number);
		// cache result despite it was successful
		FonbookEntry fe(resolve.name, false);
		fe.addNumber(number, resolve.type, "", "", 0);
		addFonbookEntry(fe);
	}
	return resolve;
}

Fonbook::sResolveResult LookupFonbook::lookup(std::string number) const {
	sResolveResult result(number);
	return result;
}

} /* namespace fritz */
