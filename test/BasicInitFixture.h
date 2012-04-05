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

	void SetUp() {
		fritz::Config::Setup("localhost", "pwd", true);

		delete fritz::gConfig->fritzClientFactory;
		fritz::gConfig->fritzClientFactory = new FakeNoopClientFactory();

		bool locsettings;
		std::string countryCode = "49";
		std::string cityCode = "7251";
		fritz::Config::Init(&locsettings, &countryCode, &cityCode);
	}

};

}

#endif /* BASICINITFIXTURE_H_ */
