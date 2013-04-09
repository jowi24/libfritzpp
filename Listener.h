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

#ifndef FRITZLISTENER_H
#define FRITZLISTENER_H

#include <string>
#include <thread>
#include <vector>
#include <libnet++/TcpClient.h>


#include "Fonbook.h"

namespace fritz{

class sCallInfo{
public:
	bool isOutgoing;
	std::string remoteNumber;
	std::string remoteName;
	std::string localNumber;
	std::string medium;
};

class EventHandler {

public:
	EventHandler() { }
	virtual ~EventHandler() { }

	virtual void handleCall(bool outgoing, int connId, std::string remoteNumber, std::string remoteName, fritz::FonbookEntry::eType remoteType, std::string localParty, std::string medium, std::string mediumName) = 0;
	virtual void handleConnect(int connId) = 0;
	virtual void handleDisconnect(int connId, std::string duration) = 0;
};

class Listener {
private:
	bool cancelRequested = false;
	static Listener *me;
	EventHandler *event;
	std::vector<int> activeConnections;
	std::thread *thread;
	network::TcpClient *tcpClientPtr = nullptr;
	Listener(EventHandler *event);
	void handleNewCall(bool outgoing, int connId, std::string remoteNumber, std::string localParty, std::string medium);
	void handleConnect(int connId);
	void handleDisconnect(int connId, std::string duration);
	void cancelThread();
public:
	/**
	 * Activate listener support.
	 * This method instantiates a Listener object, which takes care of call events from the
	 * Fritz!Box. The application has to provide an EventHandler object, which has to inherit
	 * fritz::EventHandler. The listener notifies the application about call events using this object.
	 * @param A pointer to the eventHandler. Subsequent calls to CreateListener, e.g., in case of
	 * configuration changes, can omit this parameter. Then, the existing EventHandler is used.
	 */
	static void CreateListener(EventHandler *event = nullptr);
	static void DeleteListener();
	virtual ~Listener();
	void run();
};

}

#endif /*FRITZLISTENER_H_*/
