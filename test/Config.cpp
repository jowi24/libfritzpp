/*
 * Config.cpp
 *
 *  Created on: Apr 2, 2012
 *      Author: jo
 */

#include "gtest/gtest.h"
#include "FakeBoxClient.h"

#include <Config.h>

namespace test {

class Config : public ::testing::Test {
protected:

	void SetUp() {
        fritz::Config::Setup("localhost", "", "pwd", true);

		delete fritz::gConfig->fritzClientFactory;
		fritz::gConfig->fritzClientFactory = new FakeBoxClientFactory();

		bool locsettings;
		std::string countryCode = "";
		std::string cityCode = "";
		fritz::Config::Init(&locsettings, &countryCode, &cityCode);
	}

};

TEST_F(Config, Settings) {
	EXPECT_EQ("49", fritz::gConfig->getCountryCode());
	EXPECT_EQ("721", fritz::gConfig->getRegionCode());
	EXPECT_EQ(0, fritz::gConfig->getMsnFilter().size());
	EXPECT_EQ(3, fritz::gConfig->getSipMsns().size());
	EXPECT_EQ(3, fritz::gConfig->getSipNames().size());
}

}
