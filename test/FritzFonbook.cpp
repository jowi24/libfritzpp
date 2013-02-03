/*
 * FritzFonbook.cpp
 *
 *  Created on: Jan 04, 2013
 *      Author: jo
 */



#include "gtest/gtest.h"
#include "BasicInitFixture.h"
#include "FakeSimpleClient.h"

#include <FritzFonbook.h>
#include <FonbookManager.h>

namespace test {

class FritzFonbook : public BasicInitFixture {
public:

	FritzFonbook()
	:BasicInitFixture("49", "7251") {};

	void SetUp() {
		BasicInitFixture::SetUp();
		delete fritz::gConfig->fritzClientFactory;
		fritz::gConfig->fritzClientFactory = new FakeSimpleClientFactory();
	}
};

TEST_F(FritzFonbook, ParseSimpleXMLFonbook) {
	std::vector <std::string> vFonbookID;
	vFonbookID.push_back("FRITZ");
	fritz::FonbookManager::CreateFonbookManager(vFonbookID, "FRITZ", false);
	fritz::FonbookManager *fbm = fritz::FonbookManager::GetFonbookManager();
	fritz::Fonbook *fb = fbm->GetFonbook();
	//fritz::FritzFonbook *ffb = static_cast<fritz::FritzFonbook*>(fb);

	for (size_t i=0; i<100; i++) {
		if (fb->isInitialized())
			break;
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}
	ASSERT_TRUE(fb->isInitialized());

	const fritz::FonbookEntry* fbe = fb->retrieveFonbookEntry(0);

	ASSERT_TRUE(fb->isInitialized());
	ASSERT_STREQ("FRITZ", fb->getTechId().c_str());
	ASSERT_EQ(1, (int) fb->getFonbookSize());

	ASSERT_EQ(4, (int) fbe->getSize());
	ASSERT_STREQ("00493062810000", fbe->getNumber(0).c_str());
	ASSERT_STREQ("00493062820000", fbe->getNumber(1).c_str());
	ASSERT_STREQ("004917186000000", fbe->getNumber(2).c_str());
	ASSERT_STREQ("004930254600000", fbe->getNumber(3).c_str());
}


}

