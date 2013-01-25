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

#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <boost/asio.hpp>

namespace fritz {

class TcpClient {
protected:
	std::string host;
	int port;
	bool connected;
	boost::asio::ip::tcp::iostream *stream;
	void Connect();
	void Disconnect();
public:
	TcpClient(const std::string &host, int port);
	virtual ~TcpClient();
	std::string ReadLine(bool removeNewline = true);
	void Write(const std::string &data);
};

}

#endif /* TCPCLIENT_H_ */
