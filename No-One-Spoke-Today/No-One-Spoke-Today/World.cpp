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
	eventManager = std::make_unique<EventManager>();
}

void World::Update(float deltaTime)
{
	CityMetrics cm{city->GetCityMet()};
	for (auto& h : humans) {
		h->UpdateDrive(deltaTime, cm);
		h->UpdateMentalState();
	}
	city->Update(humans);

	// 하루 전환 시 이벤트 처리
	accumulatedTime += deltaTime;
	if (accumulatedTime >= dayDuration) {
		accumulatedTime -= dayDuration;
		currentDay++;
		eventManager->ProcessDailyEvents(*city, city->GetCityMet(), humans, currentDay);
	}
}

EventManager* World::GetEventManager()
{
	return eventManager.get();
}

City* World::GetCity()
{
	return city.get();
}

void World::Debug()
{
	if (city not_eq nullptr)
		std::cout << "city 문제 없음" << std::endl;
	std::cout << humans.size() << std::endl;
	city->Debug();
}
