/*
 * OertlichesFonbook.cpp
 *
 *  Created on: Apr 1, 2012
 *      Author: jo
 */

#include "gtest/gtest.h"
#include "BasicInitFixture.h"

#include <Config.h>
#include <FonbookManager.h>
#include <Fonbook.h>
#include <FritzClient.h>

namespace test {

class OertlichesFonbook : public BasicInitFixture {
protected:
	fritz::FonbookManager *fbm;

	OertlichesFonbook()
	:BasicInitFixture("49", "7251") {};

	void SetUp() {
		BasicInitFixture::SetUp();

		std::vector <std::string> vFonbookID;
		vFonbookID.push_back("OERT");
		fritz::FonbookManager::CreateFonbookManager(vFonbookID, "", false);
		fbm = fritz::FonbookManager::GetFonbookManager();
	}

};

TEST_F(OertlichesFonbook, Resolve) {
	fritz::Fonbook::sResolveResult result = fbm->resolveToName("740");
	ASSERT_EQ("Finanzamt Bruchsal", result.name);
}

TEST_F(OertlichesFonbook, ResolveWithUmlaut) {
	fritz::Fonbook::sResolveResult result = fbm->resolveToName("03091203821");
	ASSERT_EQ("Chamäleon Marketing und Organsisation GmbH", result.name);
}

TEST_F(OertlichesFonbook, NoResolve) {
	fritz::Fonbook::sResolveResult result = fbm->resolveToName("998877");
	ASSERT_EQ("998877", result.name);
}

}
