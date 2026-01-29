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

	// 이벤트 파일 로드
	eventManager->LoadEventDefsFromText("data/events.txt");
}

void World::Update(float deltaTime)
{
	CityMetrics cm{city->GetCityMet()};
	for (auto& h : humans) {
		h->UpdateDrive(deltaTime, cm);
		h->UpdateMentalState();
	}
	city->Update(humans);

	// 시간 경과에 따른 이벤트 발동 체크
	float dayRatio = accumulatedTime / dayDuration;
	eventManager->UpdateTime(dayRatio, *city, humans);

	// 하루 전환 시 이벤트 처리
	accumulatedTime += deltaTime;
	if (accumulatedTime >= dayDuration) {
		accumulatedTime -= dayDuration;
		currentDay++;
		day++;
		// 월 전환 처리 (간단히 30일 기준)
		if (day > 30) {
			day = 1;
			month++;
		}
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

Human* World::GetHumans(int idx)
{
	return humans[idx].get();
}

int World::GetHumansSize() const
{
	return humans.size();
}

std::vector<std::unique_ptr<Human>>& World::GetHumansVector()
{
	return humans;
}


void World::Debug()
{
	if (city not_eq nullptr)
		std::cout << "city 문제 없음" << std::endl;
	std::cout << humans.size() << std::endl;
	city->Debug();
}

int World::GetCurrentDay() const
{
	return currentDay;
}

int World::GetMonth() const
{
	return month;
}

int World::GetDay() const
{
	return day;
}

float World::GetAccumulatedTime() const
{
	return accumulatedTime;
}

void World::SetCurrentDay(int d)
{
	currentDay = d;
}

void World::SetMonth(int m)
{
	month = m;
}

void World::SetDay(int d)
{
	day = d;
}

void World::SetAccumulatedTime(float t)
{
	accumulatedTime = t;
}

void World::ClearHumans()
{
	humans.clear();
}

void World::AddHuman(std::unique_ptr<Human> h)
{
	humans.push_back(std::move(h));
}
