#pragma once
#include "READER.h"
#include "APESTRUCTS.h"

class APEFILE
{
public:

	bool Parse(READER& reader);
	
	std::list<APE::Window*> windows;
	std::list<APE::Switch*> switches;
};

