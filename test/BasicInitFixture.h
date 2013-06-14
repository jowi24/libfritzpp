/*
 * BasicInitFixture.h
 *
 *  Created on: Apr 2, 2012
 *      Author: jo
 */

#ifndef BASICINITFIXTURE_H_
#define BASICINITFIXTURE_H_

#include "gtest/gtest.h"
#include "FakeNoopClient.h"

#include <Config.h>

namespace test {

class BasicInitFixture : public ::testing::Test {
protected:
	std::string countryCode;
	std::string cityCode;
	std::string host;
	std::string passwd;

	BasicInitFixture(std::string countryCode = "49", std::string cityCode = "721", std::string host = "localhost", std::string passwd = "pwd")
	:countryCode(countryCode), cityCode(cityCode), host(host), passwd(passwd) {}

	void SetUp() {
        fritz::Config::Setup(host, "", passwd, true);

		delete fritz::gConfig->fritzClientFactory;
		fritz::gConfig->fritzClientFactory = new FakeNoopClientFactory();

		bool locsettings;
		fritz::Config::Init(&locsettings, &countryCode, &cityCode);
	}

};

}

#endif /* BASICINITFIXTURE_H_ */
