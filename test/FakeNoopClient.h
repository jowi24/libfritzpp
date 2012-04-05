/*
 * FakeClient.h
 *
 *  Created on: Apr 2, 2012
 *      Author: jo
 */

#ifndef FAKECLIENT_H_
#define FAKECLIENT_H_

#include <FritzClient.h>

namespace test {

class FakeNoopClient : public fritz::FritzClient {
public:
	virtual std::string RequestLocationSettings() {
		return "";
	}
	virtual std::string RequestSipSettings() {
		return "";
	}

};

class FakeNoopClientFactory : public fritz::FritzClientFactory {
public:
	virtual ~FakeNoopClientFactory() {}
	virtual fritz::FritzClient *create() {
		return new FakeNoopClient;
	}
};

}

#endif /* FAKECLIENT_H_ */
