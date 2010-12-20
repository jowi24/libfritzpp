/*
 * FBServer.cpp
 *
 *  Created on: Nov 28, 2010
 *      Author: jo
 */

#include <iostream>
#include <fstream>
#include <cc++/thread.h>
#include <cc++/socket.h>

#include "FBServer.h"

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

FBWebServer::FBWebServer(int port, std::string fw) {
	ost::InetAddress addr = "127.0.0.1";
	this->fw = fw;
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
//			ost::Thread::sleep(1000);
			char requestLine[1024];
			std::string request;
			tcp.getline(requestLine, sizeof(requestLine));
			std::cout << "FBWebServer got request: " << requestLine << std::endl;
			request += requestLine;
			if (std::string(requestLine).find("POST") != std::string::npos) {
				while (std::string(requestLine).length() > 0) {
					tcp.getline(requestLine, sizeof(requestLine));
					request += requestLine;
					std::cout << "FBWebServer got request: " << requestLine << std::endl;
				}
			}
			std::string file = fw + "/";
			if (request.find("login_sid.xml") != std::string::npos) {
				file += "login_sid";
			} else if (request.find("POST") != std::string::npos) {
				file += "fonbuch"; //workaround for login
			} else if (request.find("html/en/menus") != std::string::npos) {
				file += "error_404";
			} else if (request.find("pagename=sipoptionen") != std::string::npos) {
				file += "sipoptionen";
			} else if (request.find("pagename=siplist") != std::string::npos) {
				file += "siplist";
			} else if (request.find("pagename=foncalls") != std::string::npos) {
				file += "foncalls";
			} else if (request.find("csv") != std::string::npos) {
				file += "foncalls_csv";
			} else if (request.find("pagename=fonbuch") != std::string::npos) {
				file += "fonbuch";
			} else if (request.find("PhonebookExportName") != std::string::npos) {
				file += "fonbuch_xml";
			} else {
				file += "empty_response";
			}

			std::string line;
			std::ifstream responseFile(file.c_str());
			if (responseFile.is_open())
			{
				while ( responseFile.good() )
				{
					std::getline (responseFile,line);
					tcp << line << "\r\n";
				}
				responseFile.close();
			}
			std::cout << "FBWebServer responded with file " << file << "." << std::endl;
			tcp.disconnect();
			std::cout << "FBWebServer disconnected " << std::endl;
		}
	}

}
