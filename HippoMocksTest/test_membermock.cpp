#include "hippomocks.h"
#include "gtest/gtest.h"
#include <string>

class IT {
public:
	std::string text;
	virtual std::string getText() { return text; }
};

TEST (TestMemberMock, checkMemberWorks)
{
	MockRepository mocks;
	IT *iamock = mocks.Mock<IT>();
	mocks.Member(iamock, &IT::text);
	iamock->text = "helloworld";
	EXPECT_EQ("helloworld", iamock->text);
}

