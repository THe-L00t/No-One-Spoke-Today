#include "World.h"

#define humansNum 200

World::World()
{
	city = std::make_unique<City>();
	humans.reserve(humansNum);
	for (size_t i = 0; i < humansNum; ++i)
	{
		humans[i] = std::make_unique<Human>();
	}
}
