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
};

}

