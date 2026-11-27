/*
 * Fonbook.cpp
 *
 *  Created on: Apr 25, 2012
 *      Author: jo
 */



#include "gtest/gtest.h"
#include "BasicInitFixture.h"

#include <Fonbook.h>
#include <XmlFonbook.h>

namespace test {

class TestFonbook : fritz::Fonbook {
public:
	TestFonbook()
			:Fonbook("Test", "TEST") {};
};

class TestXmlFonbook : public fritz::XmlFonbook {
public:
	TestXmlFonbook()
			:XmlFonbook("Test", "TEST", true) {}

	std::string serializeForTest() {
		return serializeToXml();
	}
};

TEST(TestXmlFonbook, EscapesSpecialCharacters) {
	TestXmlFonbook fonbook;
	fritz::FonbookEntry entry("A <B> & \"C\"");
	entry.addNumber("123&<456>", fritz::FonbookEntry::TYPE_HOME, "&\"", "<>");
	fonbook.addFonbookEntry(entry);

	std::string xml = fonbook.serializeForTest();

	EXPECT_NE(std::string::npos, xml.find("A &lt;B&gt; &amp; &quot;C&quot;"));
	EXPECT_NE(std::string::npos, xml.find("123&amp;&lt;456&gt;"));
	EXPECT_NE(std::string::npos, xml.find("quickdial=\"&amp;&quot;\""));
	EXPECT_NE(std::string::npos, xml.find("vanity=\"&lt;&gt;\""));
}

}

