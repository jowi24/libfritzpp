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

#include "LookupFonbook.h"

#include "Config.h"

namespace fritz {

LookupFonbook::LookupFonbook(std::string title, std::string techId, bool writeable)
:Fonbook(title, techId, writeable) {
	displayable = false;
}

LookupFonbook::~LookupFonbook() {}

bool LookupFonbook::Initialize() {
	setInitialized(true);
	return true;
}

Fonbook::sResolveResult LookupFonbook::ResolveToName(std::string number) {
	// First, try to get a cached result
	sResolveResult resolve = Fonbook::ResolveToName(number);
	// Second, to lookup (e.g., via HTTP)
	if (! resolve.successful) {
		resolve = Lookup(number);
		// cache result despite it was successful
		FonbookEntry fe(resolve.name, false);
		fe.AddNumber(0, number, resolve.type, "", "", 0);
		AddFonbookEntry(fe);
	}
	return resolve;
}

Fonbook::sResolveResult LookupFonbook::Lookup(std::string number) const {
	sResolveResult result(number);
	return result;
}

} /* namespace fritz */
