/*
 * FritzClient.cpp
 *
 *  Created on: 20.01.2013
 *      Author: jo
 */

#include "gtest/gtest.h"
#include "BasicInitFixture.h"

#include <FritzClient.h>

namespace test {

class FritzClient : public BasicInitFixture {
public:

	FritzClient()
	: BasicInitFixture("49", "721", "fritz.box", "pwd")
	{};

	void SetUp() {
		BasicInitFixture::SetUp();
	}
};

TEST_F(FritzClient, RequestIP) {
	fritz::FritzClient fc;
	std::string ip = fc.getCurrentIP();
	ASSERT_TRUE((ip.length() >= 7) && (ip.length() <= 15));
}

}




