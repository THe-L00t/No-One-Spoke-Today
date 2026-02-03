#pragma once
#include "pch.h"
#include "data.h"

// ============================================================
// [밸런싱 상수] - 베타테스트 시 이 부분만 수정
// ============================================================
namespace TerrainBalance {
	// === 맵 크기 ===
	constexpr int MAP_MIN = 0;
	constexpr int MAP_MAX = 1000;

	// === 적정 기온 ===
	constexpr float OPTIMAL_TEMP = 20.0f;           // 적정 기온 (°C)
	constexpr float COMFORT_RANGE = 10.0f;          // 쾌적 범위 (±10°C)

	// === 기온 영향 계수 ===
	constexpr float TEMP_STRESS_FACTOR = 0.02f;     // 기온차 → 스트레스 (1°C당 2%)
	constexpr float TEMP_FATIGUE_FACTOR = 0.015f;   // 기온차 → 피로 (1°C당 1.5%)
	constexpr float TEMP_COGNITION_FACTOR = 0.01f;  // 기온차 → 인지력 저하 (1°C당 1%)
	constexpr float TEMP_MOTIVATION_FACTOR = 0.01f; // 기온차 → 동기 저하 (1°C당 1%)

	// === 지형별 누적값 보정 배율 ===
	// 피로 (Fatigue)
	constexpr float DESERT_FATIGUE = 1.3f;          // 사막: +30%
	constexpr float MOUNTAIN_FATIGUE = 1.25f;       // 산악: +25%
	constexpr float TUNDRA_FATIGUE = 1.4f;          // 빙원: +40%
	constexpr float RUINS_FATIGUE = 1.1f;           // 폐허: +10%
	constexpr float TOXIC_FATIGUE = 1.35f;          // 독성: +35%
	constexpr float OASIS_FATIGUE = 0.6f;           // 오아시스: -40%
	constexpr float WASTELAND_FATIGUE = 1.0f;       // 황무지: 기본

	// 스트레스 (Stress)
	constexpr float DESERT_STRESS = 1.2f;
	constexpr float MOUNTAIN_STRESS = 1.1f;
	constexpr float TUNDRA_STRESS = 1.3f;
	constexpr float RUINS_STRESS = 1.35f;
	constexpr float TOXIC_STRESS = 1.5f;
	constexpr float OASIS_STRESS = 0.5f;
	constexpr float WASTELAND_STRESS = 1.05f;

	// 동기 (Motivation) - 1.0 미만이면 감소 촉진
	constexpr float DESERT_MOTIVATION = 0.9f;
	constexpr float MOUNTAIN_MOTIVATION = 0.85f;
	constexpr float TUNDRA_MOTIVATION = 0.8f;
	constexpr float RUINS_MOTIVATION = 0.95f;
	constexpr float TOXIC_MOTIVATION = 0.7f;
	constexpr float OASIS_MOTIVATION = 1.3f;
	constexpr float WASTELAND_MOTIVATION = 0.95f;

	// 사회적 안전감 (SocialSafety)
	constexpr float DESERT_SAFETY = 0.95f;
	constexpr float MOUNTAIN_SAFETY = 0.9f;
	constexpr float TUNDRA_SAFETY = 0.85f;
	constexpr float RUINS_SAFETY = 0.8f;
	constexpr float TOXIC_SAFETY = 0.7f;
	constexpr float OASIS_SAFETY = 1.2f;
	constexpr float WASTELAND_SAFETY = 1.0f;

	// 인지 능력 (Cognitive)
	constexpr float DESERT_COGNITION = 0.9f;
	constexpr float MOUNTAIN_COGNITION = 0.95f;
	constexpr float TUNDRA_COGNITION = 0.85f;
	constexpr float RUINS_COGNITION = 1.0f;
	constexpr float TOXIC_COGNITION = 0.75f;
	constexpr float OASIS_COGNITION = 1.1f;
	constexpr float WASTELAND_COGNITION = 1.0f;

	// === 이동 속도 보정 (일수 계산용) ===
	constexpr float DESERT_SPEED = 0.8f;            // 사막: 느림
	constexpr float MOUNTAIN_SPEED = 0.6f;          // 산악: 매우 느림
	constexpr float TUNDRA_SPEED = 0.7f;            // 빙원: 느림
	constexpr float RUINS_SPEED = 0.85f;            // 폐허: 약간 느림
	constexpr float TOXIC_SPEED = 0.75f;            // 독성: 느림
	constexpr float OASIS_SPEED = 1.0f;             // 오아시스: 기본
	constexpr float WASTELAND_SPEED = 1.0f;         // 황무지: 기본

	// === 이벤트 확률 보정 ===
	constexpr float DESERT_EVENT_MOD = 1.3f;        // 모래폭풍 등
	constexpr float MOUNTAIN_EVENT_MOD = 1.2f;      // 낙석 등
	constexpr float TUNDRA_EVENT_MOD = 1.4f;        // 눈보라 등
	constexpr float RUINS_EVENT_MOD = 1.5f;         // 위험 요소
	constexpr float TOXIC_EVENT_MOD = 1.6f;         // 오염 이벤트
	constexpr float OASIS_EVENT_MOD = 0.5f;         // 안전
	constexpr float WASTELAND_EVENT_MOD = 1.0f;     // 기본

	// === 거리당 이동 일수 ===
	constexpr float DISTANCE_PER_DAY = 50.0f;       // 하루에 50 단위 이동

	// === 힌트 발견 확률 ===
	constexpr float HINT_CHANCE_TRAVEL = 0.15f;     // 이동 중 힌트 발견 확률 (일당)
	constexpr float HINT_CHANCE_DIALOGUE = 0.05f;   // 대화 시 힌트 확률
}


// ============================================================
// 지형 타입
// ============================================================
enum class TerrainType {
	Wasteland,   // 황무지: 기본
	Desert,      // 사막: 고온, 건조
	Ruins,       // 폐허: 위험, 불안정
	Mountain,    // 산악: 저온, 고도
	Oasis,       // 오아시스: 휴식
	Tundra,      // 빙원: 극한 추위
	Toxic,       // 독성지대: 오염
	COUNT
};

// 지형 이름 반환
inline const char* GetTerrainName(TerrainType terrain) {
	switch (terrain) {
	case TerrainType::Wasteland: return "황무지";
	case TerrainType::Desert:    return "사막";
	case TerrainType::Ruins:     return "폐허";
	case TerrainType::Mountain:  return "산악";
	case TerrainType::Oasis:     return "오아시스";
	case TerrainType::Tundra:    return "빙원";
	case TerrainType::Toxic:     return "독성지대";
	default:                     return "알 수 없음";
	}
}


// ============================================================
// 맵 지역 정보
// ============================================================
struct MapRegion {
	int id;
	std::string name;
	TerrainType terrain;
	float baseTemperature;       // 기본 기온 (-40 ~ +50°C)
	float temperatureVariation;  // 일교차

	// 좌표 (게임 시작 시 랜덤 배치)
	int x;
	int y;

	// 발견 상태
	bool discovered;             // 위치를 알고 있는지
	bool visited;                // 방문한 적 있는지

	MapRegion()
		: id(-1), terrain(TerrainType::Wasteland)
		, baseTemperature(25.0f), temperatureVariation(10.0f)
		, x(0), y(0), discovered(false), visited(false)
	{}
};


// ============================================================
// 경로 정보
// ============================================================
struct Route {
	int destinationId;           // 목적지 지역 ID
	int totalDistance;           // 총 거리
	int daysRequired;            // 소요 일수
	int requiredAngle;           // 필요 각도 (0-359)

	Route()
		: destinationId(-1), totalDistance(0)
		, daysRequired(0), requiredAngle(0)
	{}
};


// ============================================================
// 위치 힌트 정보
// ============================================================
struct LocationHint {
	int regionId;                // 힌트 대상 지역
	std::string description;     // 힌트 설명
	int approximateX;            // 대략적 X (±50 오차)
	int approximateY;            // 대략적 Y (±50 오차)
	bool isExact;                // 정확한 위치인지

	LocationHint()
		: regionId(-1), approximateX(0), approximateY(0), isExact(false)
	{}
};


// ============================================================
// 네비게이션 시스템 클래스
// ============================================================
class Navigation {
public:
	Navigation();

	// 초기화
	void Initialize();
	void LoadRegionData(const std::string& filepath);
	void RandomizeRegionPositions();

	// 현재 상태
	int GetCurrentX() const { return currentX; }
	int GetCurrentY() const { return currentY; }
	float GetCurrentTemperature() const;
	TerrainType GetCurrentTerrain() const;

	// 지역 정보
	const std::vector<MapRegion>& GetRegions() const { return regions; }
	MapRegion* GetRegion(int id);
	const MapRegion* GetRegion(int id) const;
	std::vector<const MapRegion*> GetDiscoveredRegions() const;

	// 경로 설정 (조타실)
	bool SetDestination(int regionId);
	void CancelRoute();
	bool HasActiveRoute() const { return activeRoute.destinationId >= 0; }
	const Route& GetCurrentRoute() const { return activeRoute; }

	// 각도 설정 (하부구동부)
	void SetMovementAngle(int angle);
	int GetMovementAngle() const { return currentAngle; }
	int GetRequiredAngle() const { return activeRoute.requiredAngle; }
	bool IsAngleCorrect(int tolerance = 5) const;

	// 이동 처리
	void UpdateTravel();
	int GetTravelProgress() const { return travelProgress; }
	int GetRemainingDays() const;
	bool HasArrived() const;
	void CompleteArrival();

	// 정비 모드
	void StartMaintenance(int days);
	void UpdateMaintenance();
	bool IsInMaintenance() const { return inMaintenance; }
	int GetMaintenanceDaysLeft() const { return maintenanceDaysLeft; }

	// 지형 보정값 계산
	float GetFatigueModifier() const;
	float GetStressModifier() const;
	float GetMotivationModifier() const;
	float GetSafetyModifier() const;
	float GetCognitionModifier() const;
	float GetEventModifier() const;
	float GetTemperatureModifier(float factor) const;

	// 힌트 시스템
	LocationHint GenerateHint(int regionId, bool exact = false);
	void DiscoverRegion(int regionId);
	bool TryDiscoverHintDuringTravel();
	bool TryDiscoverHintFromDialogue();

	// 저장/로드
	void SaveState(std::ofstream& out) const;
	void LoadState(std::ifstream& in);

private:
	// 내부 계산
	int CalculateDistance(int x1, int y1, int x2, int y2) const;
	int CalculateAngle(int x1, int y1, int x2, int y2) const;
	int CalculateDaysRequired(int distance, TerrainType terrain) const;
	TerrainType GetTerrainAtPosition(int x, int y) const;

	// 지역 데이터
	std::vector<MapRegion> regions;

	// 현재 위치
	int currentX;
	int currentY;

	// 경로 상태
	Route activeRoute;
	int currentAngle;            // 현재 설정된 각도
	int travelProgress;          // 이동 진행 (일)

	// 정비 상태
	bool inMaintenance;
	int maintenanceDaysLeft;

	// 랜덤
	std::default_random_engine rng;
};


// ============================================================
// 지형별 보정값 헬퍼 함수
// ============================================================
inline float GetTerrainFatigueMod(TerrainType terrain) {
	using namespace TerrainBalance;
	switch (terrain) {
	case TerrainType::Desert:    return DESERT_FATIGUE;
	case TerrainType::Mountain:  return MOUNTAIN_FATIGUE;
	case TerrainType::Tundra:    return TUNDRA_FATIGUE;
	case TerrainType::Ruins:     return RUINS_FATIGUE;
	case TerrainType::Toxic:     return TOXIC_FATIGUE;
	case TerrainType::Oasis:     return OASIS_FATIGUE;
	case TerrainType::Wasteland: return WASTELAND_FATIGUE;
	default:                     return 1.0f;
	}
}

inline float GetTerrainStressMod(TerrainType terrain) {
	using namespace TerrainBalance;
	switch (terrain) {
	case TerrainType::Desert:    return DESERT_STRESS;
	case TerrainType::Mountain:  return MOUNTAIN_STRESS;
	case TerrainType::Tundra:    return TUNDRA_STRESS;
	case TerrainType::Ruins:     return RUINS_STRESS;
	case TerrainType::Toxic:     return TOXIC_STRESS;
	case TerrainType::Oasis:     return OASIS_STRESS;
	case TerrainType::Wasteland: return WASTELAND_STRESS;
	default:                     return 1.0f;
	}
}

inline float GetTerrainMotivationMod(TerrainType terrain) {
	using namespace TerrainBalance;
	switch (terrain) {
	case TerrainType::Desert:    return DESERT_MOTIVATION;
	case TerrainType::Mountain:  return MOUNTAIN_MOTIVATION;
	case TerrainType::Tundra:    return TUNDRA_MOTIVATION;
	case TerrainType::Ruins:     return RUINS_MOTIVATION;
	case TerrainType::Toxic:     return TOXIC_MOTIVATION;
	case TerrainType::Oasis:     return OASIS_MOTIVATION;
	case TerrainType::Wasteland: return WASTELAND_MOTIVATION;
	default:                     return 1.0f;
	}
}

inline float GetTerrainSafetyMod(TerrainType terrain) {
	using namespace TerrainBalance;
	switch (terrain) {
	case TerrainType::Desert:    return DESERT_SAFETY;
	case TerrainType::Mountain:  return MOUNTAIN_SAFETY;
	case TerrainType::Tundra:    return TUNDRA_SAFETY;
	case TerrainType::Ruins:     return RUINS_SAFETY;
	case TerrainType::Toxic:     return TOXIC_SAFETY;
	case TerrainType::Oasis:     return OASIS_SAFETY;
	case TerrainType::Wasteland: return WASTELAND_SAFETY;
	default:                     return 1.0f;
	}
}

inline float GetTerrainCognitionMod(TerrainType terrain) {
	using namespace TerrainBalance;
	switch (terrain) {
	case TerrainType::Desert:    return DESERT_COGNITION;
	case TerrainType::Mountain:  return MOUNTAIN_COGNITION;
	case TerrainType::Tundra:    return TUNDRA_COGNITION;
	case TerrainType::Ruins:     return RUINS_COGNITION;
	case TerrainType::Toxic:     return TOXIC_COGNITION;
	case TerrainType::Oasis:     return OASIS_COGNITION;
	case TerrainType::Wasteland: return WASTELAND_COGNITION;
	default:                     return 1.0f;
	}
}

inline float GetTerrainSpeedMod(TerrainType terrain) {
	using namespace TerrainBalance;
	switch (terrain) {
	case TerrainType::Desert:    return DESERT_SPEED;
	case TerrainType::Mountain:  return MOUNTAIN_SPEED;
	case TerrainType::Tundra:    return TUNDRA_SPEED;
	case TerrainType::Ruins:     return RUINS_SPEED;
	case TerrainType::Toxic:     return TOXIC_SPEED;
	case TerrainType::Oasis:     return OASIS_SPEED;
	case TerrainType::Wasteland: return WASTELAND_SPEED;
	default:                     return 1.0f;
	}
}

inline float GetTerrainEventMod(TerrainType terrain) {
	using namespace TerrainBalance;
	switch (terrain) {
	case TerrainType::Desert:    return DESERT_EVENT_MOD;
	case TerrainType::Mountain:  return MOUNTAIN_EVENT_MOD;
	case TerrainType::Tundra:    return TUNDRA_EVENT_MOD;
	case TerrainType::Ruins:     return RUINS_EVENT_MOD;
	case TerrainType::Toxic:     return TOXIC_EVENT_MOD;
	case TerrainType::Oasis:     return OASIS_EVENT_MOD;
	case TerrainType::Wasteland: return WASTELAND_EVENT_MOD;
	default:                     return 1.0f;
	}
}
