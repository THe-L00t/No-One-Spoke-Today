#include "World.h"

#define humansNum 200

World::World()
{
	city = std::make_unique<City>();
	humans.reserve(humansNum);
	for (size_t i = 0; i < humansNum; ++i)
	{
		humans.emplace_back(std::make_unique<Human>());
	}
}

void World::Debug()
{
	if (city not_eq nullptr)
		std::cout << "city 문제 없음" << std::endl;
	std::cout << humans.size() << std::endl;
}
