/*
 * FakeSimpleClient.h
 *
 *  Created on: Jan 3, 2013
 *      Author: jo
 */

#ifndef FAKESIMPLECLIENT_H_
#define FAKESIMPLECLIENT_H_

#include <FritzClient.h>
#include <Config.h>
#include <liblog++/Log.h>

namespace test {

class FakeSimpleClient : public fritz::FritzClient {
public:
	virtual std::string requestFonbook() {
		DBG("Returning simple fonbook.");
		return "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\
                <phonebooks><phonebook name=\"Telefonbuch\"><contact><category>0</category><person><realName>A. Muster</realName></person>\
		        <telephony>\
                  <number type=\"home\" vanity=\"\" prio=\"1\">00493062810000</number>\
                  <number type=\"home\" vanity=\"\" prio=\"0\">00493062820000</number>\
                  <number type=\"mobile\" vanity=\"\" prio=\"0\" quickdial=\"1\">004917186000000</number>\
                  <number type=\"work\" vanity=\"\" prio=\"0\">004930254600000</number>\
                </telephony>\
				</contact></phonebooks>";
	}

};

class FakeSimpleClientFactory : public fritz::FritzClientFactory {
public:
	virtual ~FakeSimpleClientFactory() {}
	virtual fritz::FritzClient *create() {
		return new FakeSimpleClient;
	}
};

}

#endif /* FAKESIMPLECLIENT_H_ */
