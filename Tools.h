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

#ifndef FRITZTOOLS_H
#define FRITZTOOLS_H

#include <stdexcept>
#include <string>

#define I18N_NOOP(x) x

namespace fritz{

class Tools
{
public:
	Tools();
	virtual ~Tools();
	static bool MatchesMsnFilter(const std::string &number);
	static std::string NormalizeNumber(std::string number);
	static int CompareNormalized(std::string number1, std::string number2);
	static bool GetLocationSettings();
	static void GetSipSettings();
	static std::string Tokenize(const std::string &buffer, const char delimiter, size_t pos);
};

}

#endif /*FRITZTOOLS_H_*/
