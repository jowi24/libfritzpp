/*
 * Listener.cpp
 *
 *  Created on: Apr 8, 2013
 *      Author: vdr
 */

#include "gtest/gtest.h"
#include "BasicInitFixture.h"

#include <libfritz++/Listener.h>

namespace test {

class Listener : public BasicInitFixture {
public:

	Listener()
	:BasicInitFixture("49", "7251") {};

	void SetUp() {
		BasicInitFixture::SetUp();
	}
};

class MyEventHandler : public fritz::EventHandler {
	virtual void handleCall(bool outgoing, int connId, std::string remoteNumber, std::string remoteName, fritz::FonbookEntry::eType remoteType, std::string localParty, std::string medium, std::string mediumName)  {}
	virtual void handleConnect(int connId) {}
	virtual void handleDisconnect(int connId, std::string duration) {}

};

TEST_F(Listener, CreateAndDeleteListenerWithConnect) {
    fritz::Config::Setup("www.joachim-wilke.de", "", "", true);
	fritz::Config::SetupPorts(80, 8080, 47000);

	MyEventHandler e;
	fritz::Listener::CreateListener(&e);
	std::this_thread::sleep_for(std::chrono::seconds(1));
	fritz::Listener::DeleteListener();
}

TEST_F(Listener, CreateAndDeleteListenerWithoutConnect) {
    fritz::Config::Setup("localhost", "", "", true);
	fritz::Config::SetupPorts(64999, 8080, 47000);

	MyEventHandler e;
	fritz::Listener::CreateListener(&e);
	std::this_thread::sleep_for(std::chrono::seconds(1));
	fritz::Listener::DeleteListener();
}

}


