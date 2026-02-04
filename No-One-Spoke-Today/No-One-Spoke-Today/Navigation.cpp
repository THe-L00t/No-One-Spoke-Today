#include "Navigation.h"
#include <cmath>
#include <sstream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ============================================================
// 생성자
// ============================================================
Navigation::Navigation()
	: currentX(500), currentY(500)  // 맵 중앙에서 시작
	, currentAngle(0), traveledDays(0)
	, inMaintenance(false), maintenanceDaysLeft(0)
	, rng(std::random_device{}())
{
}


// ============================================================
// 초기화
// ============================================================
void Navigation::Initialize() {
	LoadRegionData("data/regions.txt");
	RandomizeRegionPositions();
}

void Navigation::LoadRegionData(const std::string& filepath) {
	std::ifstream in(filepath);
	if (!in) {
		// 파일이 없으면 기본 지역 생성
		regions.clear();

		// 기본 지역들 추가
		MapRegion r;

		r.id = 0; r.name = "폐허도시 알파"; r.terrain = TerrainType::Ruins;
		r.baseTemperature = 28.0f; r.temperatureVariation = 12.0f;
		regions.push_back(r);

		r.id = 1; r.name = "사하라 잔해"; r.terrain = TerrainType::Desert;
		r.baseTemperature = 42.0f; r.temperatureVariation = 18.0f;
		regions.push_back(r);

		r.id = 2; r.name = "얼음 협곡"; r.terrain = TerrainType::Tundra;
		r.baseTemperature = -20.0f; r.temperatureVariation = 8.0f;
		regions.push_back(r);

		r.id = 3; r.name = "녹색 피난처"; r.terrain = TerrainType::Oasis;
		r.baseTemperature = 22.0f; r.temperatureVariation = 6.0f;
		regions.push_back(r);

		r.id = 4; r.name = "독성 늪지대"; r.terrain = TerrainType::Toxic;
		r.baseTemperature = 32.0f; r.temperatureVariation = 10.0f;
		regions.push_back(r);

		r.id = 5; r.name = "철벽 산맥"; r.terrain = TerrainType::Mountain;
		r.baseTemperature = 5.0f; r.temperatureVariation = 15.0f;
		regions.push_back(r);

		r.id = 6; r.name = "황야의 교차로"; r.terrain = TerrainType::Wasteland;
		r.baseTemperature = 25.0f; r.temperatureVariation = 12.0f;
		regions.push_back(r);

		r.id = 7; r.name = "붉은 사막"; r.terrain = TerrainType::Desert;
		r.baseTemperature = 38.0f; r.temperatureVariation = 20.0f;
		regions.push_back(r);

		r.id = 8; r.name = "잊혀진 도시"; r.terrain = TerrainType::Ruins;
		r.baseTemperature = 26.0f; r.temperatureVariation = 14.0f;
		regions.push_back(r);

		r.id = 9; r.name = "동토의 끝"; r.terrain = TerrainType::Tundra;
		r.baseTemperature = -35.0f; r.temperatureVariation = 5.0f;
		regions.push_back(r);

		r.id = 10; r.name = "오염된 골짜기"; r.terrain = TerrainType::Toxic;
		r.baseTemperature = 30.0f; r.temperatureVariation = 8.0f;
		regions.push_back(r);

		r.id = 11; r.name = "숨겨진 샘"; r.terrain = TerrainType::Oasis;
		r.baseTemperature = 24.0f; r.temperatureVariation = 5.0f;
		regions.push_back(r);

		r.id = 12; r.name = "검은 봉우리"; r.terrain = TerrainType::Mountain;
		r.baseTemperature = -5.0f; r.temperatureVariation = 18.0f;
		regions.push_back(r);

		return;
	}

	regions.clear();
	std::string line;

	while (std::getline(in, line)) {
		// 주석이나 빈 줄 무시
		if (line.empty() || line[0] == '#') continue;

		std::stringstream ss(line);
		MapRegion r;
		std::string terrainStr;
		char comma;

		ss >> r.id >> comma;
		std::getline(ss, r.name, ',');
		// 이름 앞뒤 공백 제거
		size_t start = r.name.find_first_not_of(" \t");
		size_t end = r.name.find_last_not_of(" \t");
		if (start != std::string::npos) {
			r.name = r.name.substr(start, end - start + 1);
		}

		std::getline(ss, terrainStr, ',');
		start = terrainStr.find_first_not_of(" \t");
		end = terrainStr.find_last_not_of(" \t");
		if (start != std::string::npos) {
			terrainStr = terrainStr.substr(start, end - start + 1);
		}

		// 지형 타입 파싱
		if (terrainStr == "Desert") r.terrain = TerrainType::Desert;
		else if (terrainStr == "Ruins") r.terrain = TerrainType::Ruins;
		else if (terrainStr == "Mountain") r.terrain = TerrainType::Mountain;
		else if (terrainStr == "Oasis") r.terrain = TerrainType::Oasis;
		else if (terrainStr == "Tundra") r.terrain = TerrainType::Tundra;
		else if (terrainStr == "Toxic") r.terrain = TerrainType::Toxic;
		else r.terrain = TerrainType::Wasteland;

		ss >> r.baseTemperature >> comma >> r.temperatureVariation;

		regions.push_back(r);
	}
}

void Navigation::RandomizeRegionPositions() {
	using namespace TerrainBalance;

	std::uniform_int_distribution<int> posDist(MAP_MIN + 50, MAP_MAX - 50);

	// 각 지역에 랜덤 좌표 배정 (서로 너무 가깝지 않게)
	for (auto& region : regions) {
		bool validPosition = false;
		int attempts = 0;

		while (!validPosition && attempts < 100) {
			region.x = posDist(rng);
			region.y = posDist(rng);

			// 다른 지역과 최소 거리 확인 (100 이상)
			validPosition = true;
			for (const auto& other : regions) {
				if (other.id == region.id) continue;
				if (other.x == 0 && other.y == 0) continue;  // 아직 배치 안 됨

				int dist = CalculateDistance(region.x, region.y, other.x, other.y);
				if (dist < 100) {
					validPosition = false;
					break;
				}
			}

			// 현재 위치와도 거리 확인
			int distFromCurrent = CalculateDistance(region.x, region.y, currentX, currentY);
			if (distFromCurrent < 80) {
				validPosition = false;
			}

			attempts++;
		}

		region.discovered = false;
		region.visited = false;
	}
}


// ============================================================
// 현재 상태
// ============================================================
float Navigation::GetCurrentTemperature() const {
	// 현재 위치의 지형에 따른 기온 (이동 중이면 경로상 지형 고려)
	TerrainType terrain = GetCurrentTerrain();

	// 기본 기온 (지형별)
	float baseTemp = 25.0f;
	switch (terrain) {
	case TerrainType::Desert:    baseTemp = 40.0f; break;
	case TerrainType::Tundra:    baseTemp = -25.0f; break;
	case TerrainType::Mountain:  baseTemp = 0.0f; break;
	case TerrainType::Oasis:     baseTemp = 22.0f; break;
	case TerrainType::Toxic:     baseTemp = 30.0f; break;
	case TerrainType::Ruins:     baseTemp = 28.0f; break;
	default:                     baseTemp = 25.0f; break;
	}

	return baseTemp;
}

TerrainType Navigation::GetCurrentTerrain() const {
	return GetTerrainAtPosition(currentX, currentY);
}

TerrainType Navigation::GetTerrainAtPosition(int x, int y) const {
	// 가장 가까운 지역의 지형 반환
	float minDist = 999999.0f;
	TerrainType closestTerrain = TerrainType::Wasteland;

	for (const auto& region : regions) {
		float dist = static_cast<float>(CalculateDistance(x, y, region.x, region.y));
		if (dist < minDist) {
			minDist = dist;
			closestTerrain = region.terrain;
		}
	}

	// 어느 지역과도 가깝지 않으면 황무지
	if (minDist > 150.0f) {
		return TerrainType::Wasteland;
	}

	return closestTerrain;
}


// ============================================================
// 지역 정보
// ============================================================
MapRegion* Navigation::GetRegion(int id) {
	for (auto& r : regions) {
		if (r.id == id) return &r;
	}
	return nullptr;
}

const MapRegion* Navigation::GetRegion(int id) const {
	for (const auto& r : regions) {
		if (r.id == id) return &r;
	}
	return nullptr;
}

std::vector<const MapRegion*> Navigation::GetDiscoveredRegions() const {
	std::vector<const MapRegion*> result;
	for (const auto& r : regions) {
		if (r.discovered) {
			result.push_back(&r);
		}
	}
	return result;
}


// ============================================================
// 경로 계산 (조타실) - 정보 제공만
// ============================================================
Route Navigation::CalculateRoute(int targetX, int targetY) const {
	Route route;
	route.targetX = targetX;
	route.targetY = targetY;
	route.totalDistance = CalculateDistance(currentX, currentY, targetX, targetY);
	route.requiredAngle = CalculateAngle(currentX, currentY, targetX, targetY);

	// 경로상 지형 고려하여 예상 일수 계산
	TerrainType terrain = GetTerrainAtPosition(targetX, targetY);
	route.estimatedDays = CalculateDaysRequired(route.totalDistance, terrain);
	route.isSet = true;

	return route;
}

void Navigation::SetTargetCoordinates(int targetX, int targetY) {
	targetRoute = CalculateRoute(targetX, targetY);
}

void Navigation::ClearTarget() {
	targetRoute = Route();
}


// ============================================================
// 각도 설정 (하부구동부) - 실제 이동 방향 변경
// ============================================================
void Navigation::SetMovementAngle(int angle) {
	currentAngle = ((angle % 360) + 360) % 360;  // 0-359 범위로 정규화
}


// ============================================================
// 이동 처리 (매일 현재 각도 방향으로 이동)
// ============================================================
void Navigation::UpdateTravel() {
	using namespace TerrainBalance;

	if (inMaintenance) return;

	// 현재 지형의 속도 보정
	TerrainType terrain = GetCurrentTerrain();
	float speedMod = GetTerrainSpeedMod(terrain);

	// 하루 이동 거리 계산
	int dailyDistance = static_cast<int>(DISTANCE_PER_DAY * speedMod);

	// 현재 각도 방향으로 이동
	MoveInDirection(currentAngle, dailyDistance);

	traveledDays++;
}

void Navigation::MoveInDirection(int angle, int distance) {
	using namespace TerrainBalance;

	// 각도를 라디안으로 변환 (북쪽 = 0도, 시계방향)
	double radians = angle * M_PI / 180.0;

	// 이동량 계산
	int dx = static_cast<int>(std::sin(radians) * distance);
	int dy = static_cast<int>(std::cos(radians) * distance);

	// 새 위치 계산 (맵 범위 제한)
	currentX = std::clamp(currentX + dx, MAP_MIN, MAP_MAX);
	currentY = std::clamp(currentY + dy, MAP_MIN, MAP_MAX);
}

int Navigation::CheckNearbyRegion(int proximityThreshold) const {
	for (const auto& region : regions) {
		int dist = CalculateDistance(currentX, currentY, region.x, region.y);
		if (dist <= proximityThreshold) {
			return region.id;
		}
	}
	return -1;
}

void Navigation::OnRegionArrival(int regionId) {
	MapRegion* region = GetRegion(regionId);
	if (region) {
		region->discovered = true;
		region->visited = true;

		// 해당 지역 좌표로 위치 보정
		currentX = region->x;
		currentY = region->y;
	}
}


// ============================================================
// 정비 모드
// ============================================================
void Navigation::StartMaintenance(int days) {
	inMaintenance = true;
	maintenanceDaysLeft = days;
}

void Navigation::UpdateMaintenance() {
	if (!inMaintenance) return;

	maintenanceDaysLeft--;
	if (maintenanceDaysLeft <= 0) {
		inMaintenance = false;
		maintenanceDaysLeft = 0;
	}
}


// ============================================================
// 지형 보정값 계산
// ============================================================
float Navigation::GetFatigueModifier() const {
	TerrainType terrain = GetCurrentTerrain();
	float terrainMod = GetTerrainFatigueMod(terrain);
	float tempMod = GetTemperatureModifier(TerrainBalance::TEMP_FATIGUE_FACTOR);
	return terrainMod * tempMod;
}

float Navigation::GetStressModifier() const {
	TerrainType terrain = GetCurrentTerrain();
	float terrainMod = GetTerrainStressMod(terrain);
	float tempMod = GetTemperatureModifier(TerrainBalance::TEMP_STRESS_FACTOR);
	return terrainMod * tempMod;
}

float Navigation::GetMotivationModifier() const {
	TerrainType terrain = GetCurrentTerrain();
	float terrainMod = GetTerrainMotivationMod(terrain);
	float tempMod = GetTemperatureModifier(TerrainBalance::TEMP_MOTIVATION_FACTOR);
	return terrainMod * tempMod;
}

float Navigation::GetSafetyModifier() const {
	TerrainType terrain = GetCurrentTerrain();
	return GetTerrainSafetyMod(terrain);
}

float Navigation::GetCognitionModifier() const {
	TerrainType terrain = GetCurrentTerrain();
	float terrainMod = GetTerrainCognitionMod(terrain);
	float tempMod = GetTemperatureModifier(TerrainBalance::TEMP_COGNITION_FACTOR);
	return terrainMod * tempMod;
}

float Navigation::GetEventModifier() const {
	TerrainType terrain = GetCurrentTerrain();
	return GetTerrainEventMod(terrain);
}

float Navigation::GetTemperatureModifier(float factor) const {
	using namespace TerrainBalance;

	float currentTemp = GetCurrentTemperature();
	float diff = std::abs(currentTemp - OPTIMAL_TEMP);

	// 쾌적 범위 내: 영향 없음
	if (diff <= COMFORT_RANGE) {
		return 1.0f;
	}

	// 쾌적 범위 초과: 제곱 함수로 급격히 증가
	float excess = diff - COMFORT_RANGE;
	float modifier = 1.0f + (excess * excess) * factor * 0.001f;

	return (std::min)(modifier, 2.0f);  // 최대 2배 제한
}


// ============================================================
// 힌트 시스템 (좌표 기반)
// ============================================================
LocationHint Navigation::GenerateHint(int regionId, bool exact) {
	LocationHint hint;
	hint.regionId = regionId;

	const MapRegion* region = GetRegion(regionId);
	if (!region) return hint;

	if (exact) {
		hint.approximateX = region->x;
		hint.approximateY = region->y;
		hint.isExact = true;
		hint.description = "정확한 좌표: (" + std::to_string(region->x) + ", " + std::to_string(region->y) + ")";
	}
	else {
		// 대략적인 위치 (±50 오차)
		std::uniform_int_distribution<int> errorDist(-50, 50);
		hint.approximateX = region->x + errorDist(rng);
		hint.approximateY = region->y + errorDist(rng);
		hint.isExact = false;

		// 방향 설명 생성
		std::string direction;
		if (region->y > currentY + 100) direction += "북";
		else if (region->y < currentY - 100) direction += "남";
		if (region->x > currentX + 100) direction += "동";
		else if (region->x < currentX - 100) direction += "서";
		if (direction.empty()) direction = "근처";

		hint.description = direction + "쪽 어딘가, 대략 (" +
			std::to_string(hint.approximateX) + ", " + std::to_string(hint.approximateY) + ") 부근";
	}

	return hint;
}

void Navigation::DiscoverRegion(int regionId) {
	MapRegion* region = GetRegion(regionId);
	if (region) {
		region->discovered = true;
	}
}

std::pair<bool, LocationHint> Navigation::TryDiscoverHintDuringTravel() {
	using namespace TerrainBalance;

	std::uniform_real_distribution<float> chanceDist(0.0f, 1.0f);
	if (chanceDist(rng) > HINT_CHANCE_TRAVEL) {
		return { false, LocationHint() };
	}

	// 아직 힌트를 얻지 못한 지역 중 하나 선택
	std::vector<int> undiscoveredIds;
	for (const auto& r : regions) {
		// 이미 힌트 목록에 있는지 확인
		bool alreadyHinted = false;
		for (const auto& h : discoveredHints) {
			if (h.regionId == r.id) {
				alreadyHinted = true;
				break;
			}
		}
		if (!alreadyHinted && !r.visited) {
			undiscoveredIds.push_back(r.id);
		}
	}

	if (undiscoveredIds.empty()) return { false, LocationHint() };

	std::uniform_int_distribution<size_t> regionDist(0, undiscoveredIds.size() - 1);
	int selectedId = undiscoveredIds[regionDist(rng)];

	// 힌트 생성 및 저장
	LocationHint hint = GenerateHint(selectedId, false);  // 대략적 좌표
	discoveredHints.push_back(hint);

	return { true, hint };
}

std::pair<bool, LocationHint> Navigation::TryDiscoverHintFromDialogue() {
	using namespace TerrainBalance;

	std::uniform_real_distribution<float> chanceDist(0.0f, 1.0f);
	if (chanceDist(rng) > HINT_CHANCE_DIALOGUE) {
		return { false, LocationHint() };
	}

	// 아직 힌트를 얻지 못한 지역 중 하나 선택
	std::vector<int> undiscoveredIds;
	for (const auto& r : regions) {
		bool alreadyHinted = false;
		for (const auto& h : discoveredHints) {
			if (h.regionId == r.id) {
				alreadyHinted = true;
				break;
			}
		}
		if (!alreadyHinted && !r.visited) {
			undiscoveredIds.push_back(r.id);
		}
	}

	if (undiscoveredIds.empty()) return { false, LocationHint() };

	std::uniform_int_distribution<size_t> regionDist(0, undiscoveredIds.size() - 1);
	int selectedId = undiscoveredIds[regionDist(rng)];

	// 힌트 생성 및 저장
	LocationHint hint = GenerateHint(selectedId, false);
	discoveredHints.push_back(hint);

	return { true, hint };
}

std::vector<LocationHint> Navigation::GetDiscoveredHints() const {
	return discoveredHints;
}


// ============================================================
// 저장/로드
// ============================================================
void Navigation::SaveState(std::ofstream& out) const {
	// 현재 위치
	out.write(reinterpret_cast<const char*>(&currentX), sizeof(currentX));
	out.write(reinterpret_cast<const char*>(&currentY), sizeof(currentY));

	// 이동 상태
	out.write(reinterpret_cast<const char*>(&currentAngle), sizeof(currentAngle));
	out.write(reinterpret_cast<const char*>(&traveledDays), sizeof(traveledDays));

	// 목표 좌표
	out.write(reinterpret_cast<const char*>(&targetRoute.targetX), sizeof(targetRoute.targetX));
	out.write(reinterpret_cast<const char*>(&targetRoute.targetY), sizeof(targetRoute.targetY));
	out.write(reinterpret_cast<const char*>(&targetRoute.isSet), sizeof(targetRoute.isSet));

	// 정비 상태
	out.write(reinterpret_cast<const char*>(&inMaintenance), sizeof(inMaintenance));
	out.write(reinterpret_cast<const char*>(&maintenanceDaysLeft), sizeof(maintenanceDaysLeft));

	// 지역 상태
	uint32_t regionCount = static_cast<uint32_t>(regions.size());
	out.write(reinterpret_cast<const char*>(&regionCount), sizeof(regionCount));

	for (const auto& r : regions) {
		out.write(reinterpret_cast<const char*>(&r.id), sizeof(r.id));
		out.write(reinterpret_cast<const char*>(&r.x), sizeof(r.x));
		out.write(reinterpret_cast<const char*>(&r.y), sizeof(r.y));
		out.write(reinterpret_cast<const char*>(&r.discovered), sizeof(r.discovered));
		out.write(reinterpret_cast<const char*>(&r.visited), sizeof(r.visited));
	}

	// 발견된 힌트 목록
	uint32_t hintCount = static_cast<uint32_t>(discoveredHints.size());
	out.write(reinterpret_cast<const char*>(&hintCount), sizeof(hintCount));

	for (const auto& hint : discoveredHints) {
		out.write(reinterpret_cast<const char*>(&hint.regionId), sizeof(hint.regionId));
		out.write(reinterpret_cast<const char*>(&hint.approximateX), sizeof(hint.approximateX));
		out.write(reinterpret_cast<const char*>(&hint.approximateY), sizeof(hint.approximateY));
		out.write(reinterpret_cast<const char*>(&hint.isExact), sizeof(hint.isExact));

		uint32_t descLen = static_cast<uint32_t>(hint.description.size());
		out.write(reinterpret_cast<const char*>(&descLen), sizeof(descLen));
		out.write(hint.description.data(), descLen);
	}
}

void Navigation::LoadState(std::ifstream& in) {
	// 현재 위치
	in.read(reinterpret_cast<char*>(&currentX), sizeof(currentX));
	in.read(reinterpret_cast<char*>(&currentY), sizeof(currentY));

	// 이동 상태
	in.read(reinterpret_cast<char*>(&currentAngle), sizeof(currentAngle));
	in.read(reinterpret_cast<char*>(&traveledDays), sizeof(traveledDays));

	// 목표 좌표
	in.read(reinterpret_cast<char*>(&targetRoute.targetX), sizeof(targetRoute.targetX));
	in.read(reinterpret_cast<char*>(&targetRoute.targetY), sizeof(targetRoute.targetY));
	in.read(reinterpret_cast<char*>(&targetRoute.isSet), sizeof(targetRoute.isSet));

	// 목표가 설정되어 있으면 경로 재계산
	if (targetRoute.isSet) {
		targetRoute = CalculateRoute(targetRoute.targetX, targetRoute.targetY);
	}

	// 정비 상태
	in.read(reinterpret_cast<char*>(&inMaintenance), sizeof(inMaintenance));
	in.read(reinterpret_cast<char*>(&maintenanceDaysLeft), sizeof(maintenanceDaysLeft));

	// 지역 상태
	uint32_t regionCount = 0;
	in.read(reinterpret_cast<char*>(&regionCount), sizeof(regionCount));

	for (uint32_t i = 0; i < regionCount && in.good(); ++i) {
		int id;
		in.read(reinterpret_cast<char*>(&id), sizeof(id));

		MapRegion* region = GetRegion(id);
		if (region) {
			in.read(reinterpret_cast<char*>(&region->x), sizeof(region->x));
			in.read(reinterpret_cast<char*>(&region->y), sizeof(region->y));
			in.read(reinterpret_cast<char*>(&region->discovered), sizeof(region->discovered));
			in.read(reinterpret_cast<char*>(&region->visited), sizeof(region->visited));
		}
		else {
			// 해당 ID가 없으면 스킵
			int dummy;
			bool dummyBool;
			in.read(reinterpret_cast<char*>(&dummy), sizeof(dummy));
			in.read(reinterpret_cast<char*>(&dummy), sizeof(dummy));
			in.read(reinterpret_cast<char*>(&dummyBool), sizeof(dummyBool));
			in.read(reinterpret_cast<char*>(&dummyBool), sizeof(dummyBool));
		}
	}

	// 발견된 힌트 목록 로드
	discoveredHints.clear();
	if (in.peek() != EOF) {
		uint32_t hintCount = 0;
		in.read(reinterpret_cast<char*>(&hintCount), sizeof(hintCount));

		for (uint32_t i = 0; i < hintCount && in.good(); ++i) {
			LocationHint hint;
			in.read(reinterpret_cast<char*>(&hint.regionId), sizeof(hint.regionId));
			in.read(reinterpret_cast<char*>(&hint.approximateX), sizeof(hint.approximateX));
			in.read(reinterpret_cast<char*>(&hint.approximateY), sizeof(hint.approximateY));
			in.read(reinterpret_cast<char*>(&hint.isExact), sizeof(hint.isExact));

			uint32_t descLen = 0;
			in.read(reinterpret_cast<char*>(&descLen), sizeof(descLen));
			hint.description.resize(descLen);
			in.read(hint.description.data(), descLen);

			discoveredHints.push_back(hint);
		}
	}
}


// ============================================================
// 내부 계산
// ============================================================
int Navigation::CalculateDistance(int x1, int y1, int x2, int y2) const {
	int dx = x2 - x1;
	int dy = y2 - y1;
	return static_cast<int>(std::sqrt(dx * dx + dy * dy));
}

int Navigation::CalculateAngle(int x1, int y1, int x2, int y2) const {
	int dx = x2 - x1;
	int dy = y2 - y1;

	// atan2로 각도 계산 (북쪽 = 0도, 시계방향)
	double radians = std::atan2(dx, dy);  // x, y 순서 주의 (북쪽 기준)
	double degrees = radians * 180.0 / M_PI;

	int angle = static_cast<int>(degrees);
	return ((angle % 360) + 360) % 360;  // 0-359 범위로 정규화
}

int Navigation::CalculateDaysRequired(int distance, TerrainType terrain) const {
	using namespace TerrainBalance;

	float speedMod = GetTerrainSpeedMod(terrain);
	float effectiveDistance = distance / speedMod;

	int days = static_cast<int>(std::ceil(effectiveDistance / DISTANCE_PER_DAY));
	return (std::max)(1, days);  // 최소 1일
}
