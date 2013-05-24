/*
 * TelLocalChFonbook.cpp
 *
 *  Created on: Apr 25, 2012
 *      Author: jo
 */



#include "gtest/gtest.h"
#include "BasicInitFixture.h"

#include <Config.h>
#include <FonbookManager.h>
#include <Fonbook.h>
#include <FritzClient.h>

namespace test {

class TelLocalChFonbook : public BasicInitFixture {
public:
	TelLocalChFonbook()
	:BasicInitFixture("41", "7251") {}
protected:
	fritz::FonbookManager *fbm;

	void SetUp() {
		BasicInitFixture::SetUp();

		std::vector <std::string> vFonbookID;
		vFonbookID.push_back("LOCCH");
		fritz::FonbookManager::CreateFonbookManager(vFonbookID, "", false);
		fbm = fritz::FonbookManager::GetFonbookManager();
	}

};

TEST_F(TelLocalChFonbook, Resolve) {
	fritz::Fonbook::sResolveResult result = fbm->resolveToName("0713700426");
	ASSERT_STREQ("TÜV AUSTRIA SCHWEIZ GMBH", result.name.c_str());
}

TEST_F(TelLocalChFonbook, NoResolve) {
	fritz::Fonbook::sResolveResult result = fbm->resolveToName("998877");
	ASSERT_STREQ("998877", result.name.c_str());
}

}


