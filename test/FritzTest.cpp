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

#include <cc++/url.h>
#include <cc++/mime.h>
#include <iostream>
#include <QtTest/QTest>
#include <QtCore/QObject>
#include <string>

#include <Config.h>
#include <Listener.h>
#include <FritzClient.h>
#include <Tools.h>

#include "FBServer.h"

class FritzTest : public QObject, public fritz::EventHandler {
	Q_OBJECT

public:
	FritzTest();
	virtual void HandleCall(bool outgoing, int connId, std::string remoteNumber, std::string remoteName, fritz::FonbookEntry::eType remoteType, std::string localParty, std::string medium, std::string mediumName){
		HandleCallCalled = true;
	}
	virtual void HandleConnect(int connId){
		HandleConnectCalled = true;
	}
	virtual void HandleDisconnect(int connId, std::string duration){
		HandleDisconnectCalled = true;
	}

private:
	int CallMonitorPort;
	int WebServerPort;
	bool HandleCallCalled;
	bool HandleConnectCalled;
	bool HandleDisconnectCalled;

private slots:
//	void GetLocationSettings();
//	void OutgoingCall();
//	void IncomingCall();
	void urlTest();
};

FritzTest::FritzTest(){
	CallMonitorPort = 9012;
	WebServerPort   = 9080;

}


//void FritzTest::GetLocationSettings() {
//	// setup of test case infrastructure
//	const int port = 8080;
//	tcpclient::TcpSendFile sf = tcpclient::TcpSendFile("29.04.67/de-sipoptionen", port);
//
//	mConfig.lang = "de";
//
//	fritz::Tools::GetLocationSettings(false);
//
//	QVERIFY(mConfig.countryCode == "11");
//	QVERIFY(mConfig.regionCode == "12345");
//}

//void FritzTest::OutgoingCall(){
//
//	FBCallMonitor callMonitor(FBCallMonitor::responseOutgoingCall, CallMonitorPort);
//	FBWebServer   webServer(WebServerPort);
//
//	fritz::Config::Setup("localhost","DontCare");
//	fritz::Config::SetupPorts(CallMonitorPort, WebServerPort, 9049);
//	fritz::Config::Init();
//	CallMonitorPort++;
//	WebServerPort++;
//
//
//	fritz::Listener::CreateListener(this);
//	ost::Thread::sleep(3000);
//	QVERIFY(HandleCallCalled == true);
//	QVERIFY(HandleConnectCalled == true);
//	QVERIFY(HandleDisconnectCalled == true);
//}
//
//void FritzTest::IncomingCall(){
//
//	FBCallMonitor callMonitor(FBCallMonitor::responseIncomingCall, CallMonitorPort);
//	FBWebServer   webServer(WebServerPort);
//
//	fritz::Config::Setup("localhost","DontCare");
//	fritz::Config::SetupPorts(CallMonitorPort, WebServerPort, 9049);
//	fritz::Config::Init();
//	CallMonitorPort++;
//	WebServerPort++;
//
//
//	fritz::Listener::CreateListener(this);
//	ost::Thread::sleep(3000);
//	QVERIFY(HandleCallCalled == true);
//	QVERIFY(HandleConnectCalled == true);
//	QVERIFY(HandleDisconnectCalled == true);
//}

void FritzTest::urlTest() {
	FBWebServer   webServer(WebServerPort, "74.04.86");

	fritz::Config::Setup("localhost","DontCare");
	fritz::Config::SetupPorts(CallMonitorPort, WebServerPort, 9049);


//		fritz::HttpClient hc("localhost", WebServerPort);
	//fritz::HttpClient hc("fritz.box", 80);


	ost2::MIMEMultipartForm *mmpf = new ost2::MIMEMultipartForm();

	new ost2::MIMEFormData( mmpf, "sid", "1234");
	new ost2::MIMEFormData( mmpf, "PhonebookId", "0");

	ost2::URLStream urlStream;

//	ost2::URLStream::Error rc = urlStream.post("http://www.mx2.eu/test/post.php",*mmpf);
	ost2::URLStream::Error rc = urlStream.post("http://localhost/test/post.php",*mmpf);
	DBG("Return code: " << rc);


}


QTEST_MAIN(FritzTest)
#include "FritzTest.moc"
