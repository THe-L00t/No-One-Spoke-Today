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
	, currentAngle(0), travelProgress(0)
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
// 경로 설정 (조타실)
// ============================================================
bool Navigation::SetDestination(int regionId) {
	const MapRegion* dest = GetRegion(regionId);
	if (!dest || !dest->discovered) {
		return false;  // 발견되지 않은 지역
	}

	activeRoute.destinationId = regionId;
	activeRoute.totalDistance = CalculateDistance(currentX, currentY, dest->x, dest->y);
	activeRoute.requiredAngle = CalculateAngle(currentX, currentY, dest->x, dest->y);
	activeRoute.daysRequired = CalculateDaysRequired(activeRoute.totalDistance, dest->terrain);

	travelProgress = 0;

	return true;
}

void Navigation::CancelRoute() {
	activeRoute.destinationId = -1;
	activeRoute.totalDistance = 0;
	activeRoute.daysRequired = 0;
	activeRoute.requiredAngle = 0;
	travelProgress = 0;
}


// ============================================================
// 각도 설정 (하부구동부)
// ============================================================
void Navigation::SetMovementAngle(int angle) {
	currentAngle = ((angle % 360) + 360) % 360;  // 0-359 범위로 정규화
}

bool Navigation::IsAngleCorrect(int tolerance) const {
	if (!HasActiveRoute()) return false;

	int diff = std::abs(currentAngle - activeRoute.requiredAngle);
	if (diff > 180) diff = 360 - diff;  // 각도 차이 최소화

	return diff <= tolerance;
}


// ============================================================
// 이동 처리
// ============================================================
void Navigation::UpdateTravel() {
	if (!HasActiveRoute()) return;
	if (!IsAngleCorrect()) return;  // 각도가 맞지 않으면 이동 안 함

	travelProgress++;

	// 위치 업데이트 (목적지 방향으로 이동)
	const MapRegion* dest = GetRegion(activeRoute.destinationId);
	if (dest) {
		float progress = static_cast<float>(travelProgress) / activeRoute.daysRequired;
		progress = std::min(progress, 1.0f);

		int startX = currentX;
		int startY = currentY;

		// 이동 시작 위치에서 목적지까지 보간
		// (실제로는 경로 시작 시점의 위치를 저장해야 하지만 간단하게 구현)
		if (progress < 1.0f) {
			float invProgress = 1.0f - progress;
			// currentX, currentY는 이동 중간 위치로 업데이트하지 않음
			// 도착 시에만 업데이트
		}
	}
}

int Navigation::GetRemainingDays() const {
	if (!HasActiveRoute()) return 0;
	return std::max(0, activeRoute.daysRequired - travelProgress);
}

bool Navigation::HasArrived() const {
	if (!HasActiveRoute()) return false;
	return travelProgress >= activeRoute.daysRequired;
}

void Navigation::CompleteArrival() {
	if (!HasArrived()) return;

	const MapRegion* dest = GetRegion(activeRoute.destinationId);
	if (dest) {
		currentX = dest->x;
		currentY = dest->y;

		// 방문 표시
		MapRegion* destMut = GetRegion(activeRoute.destinationId);
		if (destMut) {
			destMut->visited = true;
		}
	}

	CancelRoute();
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

	return std::min(modifier, 2.0f);  // 최대 2배 제한
}


// ============================================================
// 힌트 시스템
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

		hint.description = region->name + "은(는) 대략 " + direction + "쪽에 있다고 합니다.";
	}

	return hint;
}

void Navigation::DiscoverRegion(int regionId) {
	MapRegion* region = GetRegion(regionId);
	if (region) {
		region->discovered = true;
	}
}

bool Navigation::TryDiscoverHintDuringTravel() {
	using namespace TerrainBalance;

	std::uniform_real_distribution<float> chanceDist(0.0f, 1.0f);
	if (chanceDist(rng) > HINT_CHANCE_TRAVEL) {
		return false;
	}

	// 미발견 지역 중 하나 선택
	std::vector<int> undiscovered;
	for (const auto& r : regions) {
		if (!r.discovered) {
			undiscovered.push_back(r.id);
		}
	}

	if (undiscovered.empty()) return false;

	std::uniform_int_distribution<size_t> regionDist(0, undiscovered.size() - 1);
	int selectedId = undiscovered[regionDist(rng)];

	// 발견!
	DiscoverRegion(selectedId);
	return true;
}

bool Navigation::TryDiscoverHintFromDialogue() {
	using namespace TerrainBalance;

	std::uniform_real_distribution<float> chanceDist(0.0f, 1.0f);
	if (chanceDist(rng) > HINT_CHANCE_DIALOGUE) {
		return false;
	}

	// 미발견 지역 중 하나 선택
	std::vector<int> undiscovered;
	for (const auto& r : regions) {
		if (!r.discovered) {
			undiscovered.push_back(r.id);
		}
	}

	if (undiscovered.empty()) return false;

	std::uniform_int_distribution<size_t> regionDist(0, undiscovered.size() - 1);
	int selectedId = undiscovered[regionDist(rng)];

	DiscoverRegion(selectedId);
	return true;
}


// ============================================================
// 저장/로드
// ============================================================
void Navigation::SaveState(std::ofstream& out) const {
	// 현재 위치
	out.write(reinterpret_cast<const char*>(&currentX), sizeof(currentX));
	out.write(reinterpret_cast<const char*>(&currentY), sizeof(currentY));

	// 경로 상태
	out.write(reinterpret_cast<const char*>(&activeRoute.destinationId), sizeof(activeRoute.destinationId));
	out.write(reinterpret_cast<const char*>(&activeRoute.totalDistance), sizeof(activeRoute.totalDistance));
	out.write(reinterpret_cast<const char*>(&activeRoute.daysRequired), sizeof(activeRoute.daysRequired));
	out.write(reinterpret_cast<const char*>(&activeRoute.requiredAngle), sizeof(activeRoute.requiredAngle));
	out.write(reinterpret_cast<const char*>(&currentAngle), sizeof(currentAngle));
	out.write(reinterpret_cast<const char*>(&travelProgress), sizeof(travelProgress));

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
}

void Navigation::LoadState(std::ifstream& in) {
	// 현재 위치
	in.read(reinterpret_cast<char*>(&currentX), sizeof(currentX));
	in.read(reinterpret_cast<char*>(&currentY), sizeof(currentY));

	// 경로 상태
	in.read(reinterpret_cast<char*>(&activeRoute.destinationId), sizeof(activeRoute.destinationId));
	in.read(reinterpret_cast<char*>(&activeRoute.totalDistance), sizeof(activeRoute.totalDistance));
	in.read(reinterpret_cast<char*>(&activeRoute.daysRequired), sizeof(activeRoute.daysRequired));
	in.read(reinterpret_cast<char*>(&activeRoute.requiredAngle), sizeof(activeRoute.requiredAngle));
	in.read(reinterpret_cast<char*>(&currentAngle), sizeof(currentAngle));
	in.read(reinterpret_cast<char*>(&travelProgress), sizeof(travelProgress));

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
	return std::max(1, days);  // 최소 1일
}
