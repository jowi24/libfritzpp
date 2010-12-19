/*
 * FBServer.cpp
 *
 *  Created on: Nov 28, 2010
 *      Author: jo
 */

#include "FBServer.h"
#include <cc++/thread.h>
#include <cc++/socket.h>

FBCallMonitor::FBCallMonitor(responseType r, int port) {
	ost::Thread::setException(ost::Thread::throwException);
	ost::InetAddress addr = "127.0.0.1";
	socket = new ost::TCPSocket(addr, port);
	response = r;
	start();

}

FBCallMonitor::~FBCallMonitor() {
	terminate();
}


void FBCallMonitor::run() {

	while (1){
		if (socket->isPendingConnection()){
			ost::TCPStream tcp(*socket);
			switch(response){
			case responseOutgoingCall:
				tcp << "28.11.10 18:35:34;CALL;0;0;0001234;05678;SIP2;\r\n";
				ost::Thread::sleep(500);
				tcp << "28.11.10 18:35:35;CONNECT;0;0;05678;\r\n";
				ost::Thread::sleep(500);
				tcp << "28.11.10 18:35:41;DISCONNECT;0;6;\r\n";
				return;
			case responseIncomingCall:
				tcp << "28.11.10 20:28:03;RING;0;12345;678;SIP0;\r\n"; //TODO: Make translation of SIP-Names more robust
				ost::Thread::sleep(500);
				tcp << "28.11.10 18:35:35;CONNECT;0;0;678;\r\n";
				ost::Thread::sleep(500);
				tcp << "28.11.10 18:35:41;DISCONNECT;0;6;\r\n";
				return;
			}
		}
	}

}

FBWebServer::FBWebServer(int port) {
	ost::InetAddress addr = "127.0.0.1";
	socket = new ost::TCPSocket(addr, port);
	start();

}

FBWebServer::~FBWebServer() {
	terminate();
}


void FBWebServer::run() {

	ost::Thread::setException(ost::Thread::throwNothing);
	while (1){
		if (socket->isPendingConnection()){
			std::cout << "FBWebServer connected " << std::endl;
			ost::TCPStream tcp(*socket, true, 2000);
			ost::Thread::sleep(1000);
			char request[1024];
			tcp.getline(request, sizeof(request));
			std::cout << "FBWebServer got request: " << request << std::endl;
			if (std::string(request).find("POST") != std::string::npos) {
				while (std::string(request).length() > 0) {
					tcp.getline(request, sizeof(request));
					std::cout << "FBWebServer got request: " << request << std::endl;
				}
			}
			std::string content;
			for (int i=0; i<26*1000; i++) {
				content += (char)(i%26+0x41);
			}
			tcp << "HTTP/0.9 200 OK\r\n"
				<< "Date: Tue, 30 Nov 2010 07:27:15 GMT\r\n"
				<< "Content-Length: 13\r\n\r\n"
				<< "<html> "
				<< content
				<< " </html>";
			std::cout << "FBWebServer responded." << std::endl;
			tcp.disconnect();
			std::cout << "FBWebServer disconnected " << std::endl;
		}
	}

}
