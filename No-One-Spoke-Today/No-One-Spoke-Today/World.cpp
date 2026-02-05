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
	navigation = std::make_unique<Navigation>();

	// 네비게이션 초기화 (지역 데이터 로드 및 랜덤 배치)
	navigation->Initialize();

	// 이벤트 발생 시 힌트 획득 콜백 등록
	Navigation* navPtr = navigation.get();
	eventManager->SetOnEventTriggeredCallback([navPtr]() {
		if (navPtr) {
			navPtr->TryDiscoverHintOnEvent();
		}
	});

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

		// 하루 시작 시 플레이어를 조타실로 이동
		playerRegion = Region::Cockpit;

		// 하부구동부 지시 횟수 리셋
		angleOrderCountToday = 0;
		lastAngleOrderDay = currentDay;

		eventManager->ProcessDailyEvents(*city, city->GetCityMet(), humans, currentDay);

		// 네비게이션 업데이트 (이동 진행, 정비 체크)
		if (navigation) {
			if (navigation->IsInMaintenance()) {
				navigation->UpdateMaintenance();
			}
			else {
				// 도시 활력과 평균 피로 계산
				int cityActivity = city->GetCityMet().activity;
				int avgFatigue = 0;
				if (!humans.empty()) {
					int fatigueSum = 0;
					for (const auto& h : humans) {
						fatigueSum += h->GetFatigue();
					}
					avgFatigue = fatigueSum / static_cast<int>(humans.size());
				}

				// 도시 상태 기반 이동 (활력 높고 피로 낮으면 빠름)
				navigation->UpdateTravel(cityActivity, avgFatigue);

				// 지역 근접 체크
				int nearbyRegion = navigation->CheckNearbyRegion(30);
				if (nearbyRegion >= 0) {
					navigation->OnRegionArrival(nearbyRegion);
				}

				// 최종 목적지 도착 체크
				if (navigation->CheckSanctuaryArrival(30)) {
					// TODO: 엔딩 트리거
				}

				// 이동 중 힌트 발견 시도
				navigation->TryDiscoverHintDuringTravel();
			}
		}
	}

	// 시간 경과에 따른 이벤트 발동 체크
	float dayRatio = accumulatedTime / dayDuration;
	eventManager->UpdateTime(dayRatio, *city, humans);

	// ==================== 다층 영향 시스템 적용 ====================

	// 현재 도시/지형 상태 가져오기
	CityMetrics cm = city->GetCityMet();
	TerrainType currentTerrain = TerrainType::Wasteland;
	float currentTemperature = 20.0f;

	if (navigation) {
		currentTerrain = navigation->GetCurrentTerrain();
		currentTemperature = navigation->GetCurrentTemperature();
	}

	// 지형 보정값 구조체 생성
	TerrainModifiers terrainMod;
	if (navigation) {
		terrainMod.fatigue = navigation->GetFatigueModifier();
		terrainMod.stress = navigation->GetStressModifier();
		terrainMod.motivation = navigation->GetMotivationModifier();
		terrainMod.safety = navigation->GetSafetyModifier();
		terrainMod.cognition = navigation->GetCognitionModifier();
		terrainMod.temperature = currentTemperature;
	}
	else {
		terrainMod = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 20.0f };
	}

	// 구역별 리더 방문 기록 (간단히 현재 플레이어 구역만 추적)
	// TODO: 구역별 마지막 방문일 저장 구현 시 확장
	static std::unordered_map<int, int> lastLeaderVisitDay;

	// 구역별로 Human 그룹화하여 UpdateContext 생성
	std::unordered_map<Region, std::vector<Human*>> regionHumans;
	for (auto& h : humans) {
		regionHumans[h->GetRegion()].push_back(h.get());
	}

	// 각 Human 업데이트
	for (auto& h : humans) {
		Region humanRegion = h->GetRegion();

		// UpdateContext 생성
		UpdateContext ctx;
		ctx.city = cm;
		ctx.terrain = terrainMod;
		ctx.humanRegion = humanRegion;
		ctx.regionMembers = regionHumans[humanRegion];
		ctx.leaderPresent = (playerRegion == humanRegion);
		ctx.temperature = currentTemperature;

		// 리더 방문 경과 일수 계산
		int regionIdx = static_cast<int>(humanRegion);
		if (ctx.leaderPresent) {
			lastLeaderVisitDay[regionIdx] = currentDay;
			ctx.daysSinceLeaderVisit = 0;
		}
		else {
			auto it = lastLeaderVisitDay.find(regionIdx);
			if (it != lastLeaderVisitDay.end()) {
				ctx.daysSinceLeaderVisit = currentDay - it->second;
			}
			else {
				ctx.daysSinceLeaderVisit = currentDay;  // 한 번도 방문 안 함
			}
		}

		// 다층 영향 시스템을 통한 드라이브 업데이트
		h->UpdateDrive(deltaTime, ctx);
		h->UpdateMentalState();
	}

	// 도시 지표 업데이트 (비선형 효과 적용)
	city->Update(humans, currentTemperature, currentTerrain);
}

EventManager* World::GetEventManager()
{
	return eventManager.get();
}

City* World::GetCity()
{
	return city.get();
}

Navigation* World::GetNavigation()
{
	return navigation.get();
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

// ========== 하부구동부 지시 관리 ==========
void World::IncrementAngleOrderCount()
{
	ResetAngleOrderCountIfNewDay();
	angleOrderCountToday++;

	// 지시 횟수에 따른 하부구동부 인원 스트레스/피로 증가
	auto lowerDriveHumans = GetHumansInRegion(Region::LowerDrive);

	if (angleOrderCountToday >= 3) {
		int stressDelta = 0;
		int fatigueDelta = 0;
		int trustDelta = 0;

		if (angleOrderCountToday == 3) {
			stressDelta = 200;
			fatigueDelta = 100;
		}
		else if (angleOrderCountToday == 4) {
			stressDelta = 400;
			fatigueDelta = 200;
			trustDelta = -100;
		}
		else {  // 5회 이상
			stressDelta = 600;
			fatigueDelta = 300;
			trustDelta = -200;
		}

		for (Human* h : lowerDriveHumans) {
			h->ModifyStressLoad(stressDelta);
			h->ModifyFatigue(fatigueDelta);
			if (trustDelta != 0) {
				h->ModifyInterpersonalTrust(trustDelta);
			}
			h->UpdateMentalState();
		}
	}
}

void World::ResetAngleOrderCountIfNewDay()
{
	if (lastAngleOrderDay != currentDay) {
		angleOrderCountToday = 0;
		lastAngleOrderDay = currentDay;
	}
}
