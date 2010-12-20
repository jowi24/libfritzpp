/*
 * FBServer.h
 *
 *  Created on: Nov 28, 2010
 *      Author: jo
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
