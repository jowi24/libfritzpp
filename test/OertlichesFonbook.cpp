/*
 * OertlichesFonbook.cpp
 *
 *  Created on: Apr 1, 2012
 *      Author: jo
 */

#include "gtest/gtest.h"

#include <Config.h>
#include <FonbookManager.h>
#include <Fonbook.h>
#include <FritzClient.h>

class FakeClient : public fritz::FritzClient {
public:
	virtual std::string RequestLocationSettings() {
		return "";
	}
	virtual std::string RequestSipSettings() {
		return "";
	}

};

class FakeClientFactory : public fritz::FritzClientFactory {
	virtual ~FakeClientFactory() {}

	virtual fritz::FritzClient *create() {
		return new FakeClient;
	}
};


class OertlichesFonbook : public ::testing::Test {
protected:
	fritz::FonbookManager *fbm;

	void SetUp() {
		fritz::Config::Setup("localhost", "pwd", true);

		delete fritz::gConfig->fritzClientFactory;
		fritz::gConfig->fritzClientFactory = new FakeClientFactory();

		bool locsettings;
		std::string countryCode = "49";
		std::string cityCode = "7251";
		fritz::Config::Init(&locsettings, &countryCode, &cityCode);

		std::vector <std::string> vFonbookID;
		vFonbookID.push_back("OERT");
		fritz::FonbookManager::CreateFonbookManager(vFonbookID, "", false);
		fbm = fritz::FonbookManager::GetFonbookManager();
	}

};

TEST_F(OertlichesFonbook, Resolve) {
	fritz::Fonbook::sResolveResult result = fbm->ResolveToName("740");
	ASSERT_EQ("Finanzamt Bruchsal", result.name);
}
