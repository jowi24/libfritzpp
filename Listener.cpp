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

#include "Listener.h"

#include <cc++/socket.h>
#include <string>
#include <cstdlib>
#include <sstream>
#include <vector>

#include "CallList.h"
#include "Config.h"
#include "FonbookManager.h"
#include "Tools.h"

namespace fritz{

Listener *Listener::me = NULL;

Listener::Listener(EventHandler *event)
:Thread()
{
	setName("Listener");
	setCancel(cancelDeferred);
	this->event = event;
	start();
}

Listener::~Listener()
{
	terminate();
}

void Listener::CreateListener(EventHandler *event) {
	EventHandler *oldEvent = me ? me->event : NULL;
	DeleteListener();
	if (event || oldEvent)
		me = new Listener(event ? event : oldEvent);
	else
		ERR("Invalid call parameter. First call to CreateListener needs event handler object.");
}

void Listener::DeleteListener() {
	if (me) {
		DBG("deleting listener");
		delete me;
		me = NULL;
	}
}

void Listener::HandleNewCall(bool outgoing, int connId, std::string remoteNumber, std::string localParty, std::string medium) {
	if ( Tools::MatchesMsnFilter(localParty) ) {
		// do reverse lookup
		Fonbook::sResolveResult result = FonbookManager::GetFonbook()->ResolveToName(remoteNumber);
		// resolve SIP names
		std::string mediumName;
		if (medium.find("SIP")           != std::string::npos &&
				gConfig->getSipNames().size() > (size_t)atoi(&medium[3]))
			mediumName = gConfig->getSipNames()[atoi(&medium[3])];
		else
			mediumName = medium;
		// notify application
		if (event) event->HandleCall(outgoing, connId, remoteNumber, result.name, result.type, localParty, medium, mediumName);
		activeConnections.push_back(connId);
	}
}

void Listener::HandleConnect(int connId) {
	// only notify application if this connection is part of activeConnections
	bool notify = false;
	for (std::vector<int>::iterator it = activeConnections.begin(); it < activeConnections.end(); ++it) {
		if (*it == connId) {
			notify = true;
			break;
		}
	}
	if (notify)
		if (event) event->HandleConnect(connId);
}

void Listener::HandleDisconnect(int connId, std::string duration) {
	// only notify application if this connection is part of activeConnections
	bool notify = false;
	for (std::vector<int>::iterator it = activeConnections.begin(); it < activeConnections.end(); ++it) {
		if (*it == connId) {
			activeConnections.erase(it);
			notify = true;
			break;
		}
	}
	if (notify) {
		if (event) event->HandleDisconnect(connId, duration);
		// force reload of callList
		CallList *callList = CallList::getCallList(false);
		if (callList)
			callList->start();
	}
}

void Listener::run() {

	Thread::setException(Thread::throwException);
	unsigned int retry_delay = RETRY_DELAY / 2;
	while (true) {
		try {
			retry_delay = retry_delay > 1800 ? 3600 : retry_delay * 2;
			ost::TCPStream tcpStream(ost::IPV4Host(gConfig->getUrl().c_str()), gConfig->getListenerPort());
			while (true) {
				DBG("Waiting for a message.");
				char data[1024];
				tcpStream.getline(data, sizeof(data));
				if (gConfig->logPersonalInfo())
					DBG("Got message " << data);

				// split line into tokens
				std::string date  = Tools::Tokenize(data, ';', 0);
				std::string type  = Tools::Tokenize(data, ';', 1);
				int connId        = atoi(Tools::Tokenize(data, ';', 2).c_str());
				std::string partA = Tools::Tokenize(data, ';', 3);
				std::string partB = Tools::Tokenize(data, ';', 4);
				std::string partC = Tools::Tokenize(data, ';', 5);
				std::string partD = Tools::Tokenize(data, ';', 6);

#if 0 // some strings sent from the FB, made available to xgettext
					I18N_NOOP("POTS");
					I18N_NOOP("ISDN");
#endif

				if (type.compare("CALL") == 0) {
					// partA => box port
					// partB => caller Id (local)
					// partC => called Id (remote)
					// partD => medium (POTS, SIP[1-9], ISDN, ...)
					DBG("CALL " << ", " << partA
							    << ", " << (gConfig->logPersonalInfo() ? partB : HIDDEN)
		                        << ", " << (gConfig->logPersonalInfo() ? partC : HIDDEN)
		                        << ", " << partD);

					// an '#' can be appended to outgoing calls by the phone, so delete it
					if (partC[partC.length()-1] == '#')
						partC = partC.substr(0, partC.length()-1);

					HandleNewCall(true, connId, partC, partB, partD);

				} else if (type.compare("RING") == 0) {
					// partA => caller Id (remote)
					// partB => called Id (local)
					// partC => medium (POTS, SIP[1-9], ISDN, ...)
					DBG("RING " << ", " << (gConfig->logPersonalInfo() ? partA : HIDDEN)
							    << ", " << (gConfig->logPersonalInfo() ? partB : HIDDEN)
		                        << ", " << partC);

					HandleNewCall(false, connId, partA, partB, partC);

				} else if (type.compare("CONNECT") == 0) {
					// partA => box port
					// partB => local/remote Id
					DBG("CONNECT " << ", " << partA
							       << ", " << (gConfig->logPersonalInfo() ? partB : HIDDEN));

					HandleConnect(connId);

				} else if (type.compare("DISCONNECT") == 0) {
					// partA => call duration
					DBG("DISCONNECT " << ", " << partA );

					HandleDisconnect(connId, partA);

				} else {
					DBG("Got unknown message " << data);
					throw this;
				}
				retry_delay = RETRY_DELAY;
			}
		} catch(ost::SockException& se) {
			ERR("Exception - " << se.what());
			if (se.getSocketError() == ost::Socket::errConnectRefused)
				ERR("Make sure to enable the Fritz!Box call monitor by dialing #96*5* once.");
		} catch (Listener *listener) {
			ERR("Exception unknown data received.");
		}

		ERR("waiting " << retry_delay << " seconds before retrying");
		ost::Thread::sleep(retry_delay*1000); // delay the retry
	}
}

}
