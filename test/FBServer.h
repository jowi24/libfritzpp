/*
 * libfritz++
 *
 * Copyright (C) 2007-2011 Joachim Wilke <libfritz@joachim-wilke.de>
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

#ifndef FBSERVER_H_
#define FBSERVER_H_

#include <cc++/socket.h>
#include <cc++/thread.h>

class FBCallMonitor: public ost::Thread {
public:

	enum responseType {
		responseOutgoingCall,
		responseIncomingCall,
	};

	FBCallMonitor(responseType r, int port);
	virtual ~FBCallMonitor();
	void run();

private:
	ost::TCPSocket *socket;
	responseType response;
};

class FBWebServer: public ost::Thread {
public:
	FBWebServer(int port, std::string fw);
	virtual ~FBWebServer();
	void run();

private:
	ost::TCPSocket *socket;
	std::string fw;
};

#endif /* FBSERVER_H_ */
