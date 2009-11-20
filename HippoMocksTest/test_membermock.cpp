#include "hippomocks.h"
#include "yaffut.h"
#include <string>

class IT {
public:
	std::string text;
	virtual std::string getText() { return text; }
};

FUNC (checkMemberWorks)
{
	MockRepository mocks;
	IT *iamock = mocks.Mock<IT>();
	mocks.Member(iamock, &IT::text);
	iamock->text = "helloworld";
	EQUAL("helloworld", iamock->text);
}

