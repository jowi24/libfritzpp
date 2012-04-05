/*
 * Tools.cpp
 *
 *  Created on: Apr 2, 2012
 *      Author: jo
 */

#include "gtest/gtest.h"
#include "BasicInitFixture.h"

#include <Tools.h>

namespace test {

class Tools : public BasicInitFixture {
protected:
	void SetUp() {
		BasicInitFixture::SetUp();

		std::vector<std::string> vFilter;
		vFilter.push_back("0815");
		vFilter.push_back("072514444");
		fritz::gConfig->SetupMsnFilter(vFilter);
	}
};

TEST_F(Tools, MatchesMSNFilterExactMatchFirst) {
	ASSERT_TRUE(fritz::Tools::MatchesMsnFilter("0815"));
}

TEST_F(Tools, MatchesMSNFilterExactMatchSecond) {
	ASSERT_TRUE(fritz::Tools::MatchesMsnFilter("072514444"));
}

TEST_F(Tools, MatchesMSNFilterMatchWithPrefix) {
	ASSERT_TRUE(fritz::Tools::MatchesMsnFilter("072510815"));
}

TEST_F(Tools, MatchesMSNFilterNoMatchPostfix) {
	ASSERT_FALSE(fritz::Tools::MatchesMsnFilter("08151"));
}

TEST_F(Tools, MatchesMSNFilterNoMatchPartial) {
	ASSERT_FALSE(fritz::Tools::MatchesMsnFilter("4444"));
}

TEST_F(Tools, MatchesMSNFilterNoMatchPrefix) {
	ASSERT_FALSE(fritz::Tools::MatchesMsnFilter("07251"));
}

TEST_F(Tools, MatchesMSNFilterNoMatch) {
	ASSERT_FALSE(fritz::Tools::MatchesMsnFilter("99"));
}

TEST_F(Tools, NormalizeNumberShort) {
	ASSERT_EQ("004972514711", fritz::Tools::NormalizeNumber("4711"));
}

TEST_F(Tools, NormalizeNumberNormal) {
	ASSERT_EQ("004972514711", fritz::Tools::NormalizeNumber("072514711"));
}

TEST_F(Tools, NormalizeNumberFull) {
	ASSERT_EQ("004972514711", fritz::Tools::NormalizeNumber("004972514711"));
}

TEST_F(Tools, CompareNormalizedShortWithNormal) {
	ASSERT_TRUE(fritz::Tools::CompareNormalized("69695", "0725169695") == 0);
}

TEST_F(Tools, CompareNormalizedShortWithLong) {
	ASSERT_TRUE(fritz::Tools::CompareNormalized("69695", "0049725169695") == 0);
}

TEST_F(Tools, CompareNormalizedShortWithShort) {
	ASSERT_TRUE(fritz::Tools::CompareNormalized("69695", "69695") == 0);
}

TEST_F(Tools, CompareNormalizedShortWithNormalNoMatchExtension) {
	ASSERT_FALSE(fritz::Tools::CompareNormalized("69695", "072516969") == 0);
}

TEST_F(Tools, CompareNormalizedShortWithLongNoMatch) {
	ASSERT_FALSE(fritz::Tools::CompareNormalized("69695", "49725169695") == 0);
}

TEST_F(Tools, CompareNormalizedShortWithNormalNoMatchAreaCode) {
	ASSERT_FALSE(fritz::Tools::CompareNormalized("69695", "072569695") == 0);
}

TEST_F(Tools, Tokenize) {
	std::string input = "(Bla, Blubb, Dings, Bumms)";
	ASSERT_EQ("(Bla", fritz::Tools::Tokenize(input, ',', 0));
	ASSERT_EQ(" Blubb", fritz::Tools::Tokenize(input, ',', 1));
	ASSERT_EQ(" Dings", fritz::Tools::Tokenize(input, ',', 2));
	ASSERT_EQ(" Bumms)", fritz::Tools::Tokenize(input, ',', 3));
}

}

