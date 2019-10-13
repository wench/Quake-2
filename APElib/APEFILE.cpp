#include "APEFILE.h"

bool APEFILE::Parse(READER& reader)
{
	auto header = reader.readTuple< uint32_t, uint32_t>();
	auto check = std::make_tuple< uint32_t, uint32_t>(317, 0xFFFFFFFF);
	if (header != check) return false;
	
	for (;;)
	{
		auto v = reader.readi32();
		if ((v == 0))
		{
			v = reader.readi32();

			if (v == 0xFFFFFFFE)
			{
				while (v = reader.readi32())
				{
					auto st = reader.readTuple< uint32_t, uint32_t>();
					check = std::make_tuple< uint32_t, uint32_t>(1, 0);
					if (st != check) return false;
					switches.push_back(new APE::Switch(reader, v));
				}

			}
			else {
				return true;
			}
		}
		else
		windows.push_back(new APE::Window(reader, v));
	}
	
}
