#include "World.h"
#include "Toolkit.h"
#include "data.h"

#define humansNum 200
#define tempNum 50

World::World()
{
	humans.reserve(humansNum + tempNum);

	// 이름 파일 로드
	std::vector<std::string> maleNames = loadSentences("data/names_male.txt");
	std::vector<std::string> femaleNames = loadSentences("data/names_female.txt");

	// 이름 셔플
	std::random_device rd;
	std::default_random_engine rng(rd());
	std::shuffle(maleNames.begin(), maleNames.end(), rng);
	std::shuffle(femaleNames.begin(), femaleNames.end(), rng);

	int maleIdx = 0;
	int femaleIdx = 0;

	for (size_t i = 0; i < humansNum; ++i)
	{
		humans.emplace_back(std::make_unique<Human>());

		// 성별 랜덤 배정 (약 50:50)
		bool isMale = (rng() % 2 == 0);
		humans[i]->SetMale(isMale);

		// 이름 배정 (겹치지 않도록)
		if (isMale && maleIdx < static_cast<int>(maleNames.size())) {
			humans[i]->SetName(maleNames[maleIdx++]);
		}
		else if (!isMale && femaleIdx < static_cast<int>(femaleNames.size())) {
			humans[i]->SetName(femaleNames[femaleIdx++]);
		}
		else {
			// 이름이 부족하면 반대 성별에서 가져오거나 기본 이름
			if (isMale && femaleIdx < static_cast<int>(femaleNames.size())) {
				humans[i]->SetName(femaleNames[femaleIdx++]);
			}
			else if (!isMale && maleIdx < static_cast<int>(maleNames.size())) {
				humans[i]->SetName(maleNames[maleIdx++]);
			}
			else {
				humans[i]->SetName("시민" + std::to_string(i));
			}
		}
	}

	// 구역별 인구 배정
	int regionCounts[static_cast<int>(Region::COUNT)] = { 0 };
	int idx = 0;
	for (int r = 0; r < static_cast<int>(Region::COUNT); ++r) {
		Region region = static_cast<Region>(r);
		int count = GetRegionPopulationRatio(region);
		for (int j = 0; j < count && idx < humansNum; ++j) {
			humans[idx]->SetRegion(region);
			++idx;
		}
	}
	// 나머지는 거주구역1에 배정
	while (idx < humansNum) {
		humans[idx]->SetRegion(Region::ResidentialArea1);
		++idx;
	}

	city = std::make_unique<City>(humans);
	eventManager = std::make_unique<EventManager>();

	// 이벤트 파일 로드
	eventManager->LoadEventDefsFromText("data/events.txt");

	// 첫날 이벤트 스케줄링
	eventManager->ProcessDailyEvents(*city, city->GetCityMet(), humans, currentDay);
}

void World::Update(float deltaTime)
{
	// 시간 누적
	accumulatedTime += deltaTime;

	// 하루 전환 시 새로운 날 이벤트 스케줄링
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

	// 시간 경과에 따른 이벤트 발동 체크
	float dayRatio = accumulatedTime / dayDuration;
	eventManager->UpdateTime(dayRatio, *city, humans);

	// 인간/도시 업데이트
	CityMetrics cm{city->GetCityMet()};
	for (auto& h : humans) {
		h->UpdateDrive(deltaTime, cm);
		h->UpdateMentalState();
	}
	city->Update(humans);
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

// ========== 플레이어 구역 관리 ==========
Region World::GetPlayerRegion() const
{
	return playerRegion;
}

void World::SetPlayerRegion(Region r)
{
	playerRegion = r;
}

bool World::MovePlayerToRegion(Region target)
{
	if (AreRegionsConnected(playerRegion, target)) {
		playerRegion = target;
		// 해당 구역의 미확인 이벤트 표시 해제
		MarkRegionEventSeen(target);
		return true;
	}
	return false;
}

std::vector<Region> World::GetAccessibleRegions() const
{
	return GetAdjacentRegions(playerRegion);
}

// ========== 구역별 시민 관리 ==========
std::vector<Human*> World::GetHumansInRegion(Region r)
{
	std::vector<Human*> result;
	for (auto& h : humans) {
		if (h->GetRegion() == r) {
			result.push_back(h.get());
		}
	}
	return result;
}

int World::GetHumanCountInRegion(Region r) const
{
	int count = 0;
	for (const auto& h : humans) {
		if (h->GetRegion() == r) {
			++count;
		}
	}
	return count;
}

// ========== 구역 이벤트 알림 관리 ==========
void World::AddRegionEventAlert(Region r, const std::string& eventName)
{
	regionEventAlerts.push_back({ r, eventName });
	unseenEventRegions.insert(r);
}

const std::vector<std::pair<Region, std::string>>& World::GetRegionEventAlerts() const
{
	return regionEventAlerts;
}

void World::ClearRegionEventAlerts()
{
	regionEventAlerts.clear();
}

bool World::HasUnseenEventInRegion(Region r) const
{
	return unseenEventRegions.find(r) != unseenEventRegions.end();
}

void World::MarkRegionEventSeen(Region r)
{
	unseenEventRegions.erase(r);
}

const std::set<Region>& World::GetUnseenEventRegions() const
{
	return unseenEventRegions;
}
