/*
 * Fonbook.cpp
 *
 *  Created on: Apr 25, 2012
 *      Author: jo
 */



#include "gtest/gtest.h"
#include "BasicInitFixture.h"

#include <Fonbook.h>

namespace test {

class TestFonbook : fritz::Fonbook {
public:
	TestFonbook()
			:Fonbook("Test", "TEST") {};
	std::string publicConvertEntities(std::string s) {
		return convertEntities(s);
	}
};

TEST(Fonbook, ConvertEntitiesUTF) {
	TestFonbook f;
	ASSERT_STREQ("TestÄTestä", f.publicConvertEntities("Test&#xC4;Test&#xE4;").c_str());
}

TEST(Fonbook, ConvertEntities) {
	TestFonbook f;
	ASSERT_STREQ("TestÄTestä", f.publicConvertEntities("Test&Auml;Test&auml;").c_str());
}

}

