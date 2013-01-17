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

#include "TcpClient.h"

namespace fritz {

TcpClient::TcpClient(std::string &host, int port)
: host{host}, port{port}, stream{host, static_cast<std::stringstream&>(std::stringstream().flush() << port).str()} {
	if (!stream)
		throw std::runtime_error("Could not connect to host.");
}


TcpClient::~TcpClient() {
}

std::string TcpClient::ReadLine() {
	std::string line;
	std::getline(stream, line);
	line.erase(line.end()-1, line.end());
	return line;
}

void TcpClient::Write(const std::string &data) {
	stream << data;
}

}
