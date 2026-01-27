#include "World.h"
#include "data.h"

#define humansNum 200
#define tempNum 50

World::World()
{
	humans.reserve(humansNum + tempNum);
	for (size_t i = 0; i < humansNum; ++i)
	{
		humans.emplace_back(std::make_unique<Human>());
	}
	city = std::make_unique<City>(humans);
}

void World::Update(float deltaTime)
{
	CityMetrics cm{city->GetCityMet()};

}

void World::Debug()
{
	if (city not_eq nullptr)
		std::cout << "city 문제 없음" << std::endl;
	std::cout << humans.size() << std::endl;
	city->Debug();
}
