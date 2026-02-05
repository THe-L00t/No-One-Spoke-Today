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

	// === 이동 속도 밸런싱 ===
	constexpr float BASE_SPEED = 14.0f;             // 기본 속도 (좌표/일)
	constexpr float MAX_SPEED = 25.0f;              // 최대 속도 (최적 상태)
	constexpr float MIN_SPEED = 5.0f;               // 최소 속도 (최악 상태)

	// 노동효율 계수
	constexpr float FATIGUE_PENALTY_MAX = 0.4f;     // 피로 최대 감소율 (1.0 → 0.6)
	constexpr float MOTIVATION_BASE = 0.7f;         // 동기 기본값
	constexpr float MOTIVATION_BONUS_MAX = 0.3f;    // 동기 최대 보너스
	constexpr float STRESS_THRESHOLD = 7000.0f;     // 스트레스 급락 임계점
	constexpr float STRESS_PENALTY_HIGH = 0.6f;     // 고스트레스 시 효율

	// 협력계수
	constexpr float TRUST_BASE = 0.8f;              // 신뢰 기본값
	constexpr float TRUST_BONUS_MAX = 0.2f;         // 신뢰 최대 보너스
	constexpr float HOSTILE_THRESHOLD = 0.15f;      // 적대비율 임계점 (15%)
	constexpr float HOSTILE_PENALTY_MULT = 2.0f;    // 임계 초과 시 페널티 배율
	constexpr float HOSTILE_PENALTY_MIN = 0.6f;     // 적대 페널티 최소값

	// 인프라계수
	constexpr float SCARCITY_THRESHOLD = 7000.0f;   // 물자부족 임계점
	constexpr float MOOD_BASE = 0.9f;               // 분위기 기본값
	constexpr float MOOD_BONUS_MAX = 0.1f;          // 분위기 최대 보너스

	// === 힌트 발견 확률 ===
	constexpr float HINT_CHANCE_ON_EVENT = 0.50f;   // 이벤트 발생 시 힌트 확률 (50%)
	constexpr float SANCTUARY_HINT_CHANCE = 0.05f;  // 힌트 중 최종목적지 확률 (5%)
	constexpr float HINT_CHANCE_TRAVEL = 0.15f;     // 이동 중 힌트 발견 확률 (일당) - 기존 유지
	constexpr float HINT_CHANCE_DIALOGUE = 0.05f;   // 대화 시 힌트 확률 - 기존 유지
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
// 속도 계산용 도시/시민 상태
// ============================================================
struct SpeedContext {
	// 시민 평균 상태 (0~10000)
	int avgFatigue;
	int avgMotivation;
	int avgStress;
	int avgTrust;

	// 적대적 시민 비율 (0.0~1.0)
	float hostileRatio;

	// 도시 지표 (0~10000)
	int cityMood;
	int cityScarcity;

	SpeedContext()
		: avgFatigue(5000), avgMotivation(5000), avgStress(5000), avgTrust(5000)
		, hostileRatio(0.0f), cityMood(5000), cityScarcity(5000)
	{}
};


// ============================================================
// 경로 정보 (좌표 기반)
// ============================================================
struct Route {
	int targetX;                 // 목표 X 좌표
	int targetY;                 // 목표 Y 좌표
	int totalDistance;           // 총 거리
	int estimatedDays;           // 예상 소요 일수
	int requiredAngle;           // 필요 각도 (0-359)
	bool isSet;                  // 경로가 설정되었는지

	Route()
		: targetX(0), targetY(0), totalDistance(0)
		, estimatedDays(0), requiredAngle(0), isSet(false)
	{}
};


// ============================================================
// 힌트 유형
// ============================================================
enum class HintType {
	Coordinate,      // 좌표 제공 (지역 발견됨)
	Direction,       // 방향 제공 (몇 도 방향)
	Characteristic   // 특성 제공 (지형 정보)
};

// ============================================================
// 위치 힌트 정보
// ============================================================
struct LocationHint {
	int regionId;                // 힌트 대상 지역 (-1이면 Sanctuary)
	std::string discoveryMessage; // 쪽지 발견 대사
	std::string description;     // 힌트 내용
	int approximateX;            // 대략적 X (±50 오차)
	int approximateY;            // 대략적 Y (±50 오차)
	int directionAngle;          // 방향 힌트일 때 각도
	std::string terrainName;     // 특성 힌트일 때 지형 이름
	std::string regionName;      // 지역 이름
	HintType hintType;           // 힌트 유형
	bool isExact;                // 정확한 위치인지
	bool isSanctuary;            // 최종 목적지 힌트인지

	LocationHint()
		: regionId(-1), approximateX(0), approximateY(0), directionAngle(0)
		, hintType(HintType::Coordinate), isExact(false), isSanctuary(false)
	{}
};


// ============================================================
// 최종 목적지 (안정지대) 정보
// ============================================================
struct Sanctuary {
	int x;                       // 정확한 X 좌표
	int y;                       // 정확한 Y 좌표
	int hintLevel;               // 힌트 레벨: 0=미발견, 1=방향, 2=대략좌표, 3=정확좌표
	std::string currentHint;     // 현재 힌트 설명
	bool discovered;             // 발견 여부 (도착 시 true)

	Sanctuary()
		: x(0), y(0), hintLevel(0), discovered(false)
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

	// 경로 계산 (조타실) - 정보 제공만, 실제 이동 방향 변경 안 함
	Route CalculateRoute(int targetX, int targetY) const;
	void SetTargetCoordinates(int targetX, int targetY);  // 목표 좌표 저장 (참고용)
	void ClearTarget();
	bool HasTarget() const { return targetRoute.isSet; }
	const Route& GetTargetRoute() const { return targetRoute; }

	// 각도 설정 (하부구동부) - 실제 이동 방향 변경
	void SetMovementAngle(int angle);
	int GetMovementAngle() const { return currentAngle; }

	// 이동 처리 (매일 현재 각도 방향으로 이동)
	void UpdateTravel(const SpeedContext& ctx);  // 다층 시스템 기반 속도
	void UpdateTravel();  // 기본 속도 (하위 호환)
	int GetTraveledDays() const { return traveledDays; }
	int CalculateDailySpeed(const SpeedContext& ctx) const;  // 이동 속도 계산 (다층 시스템)
	int GetLastCalculatedSpeed() const { return lastCalculatedSpeed; }  // UI 표시용

	// 지역 도착 체크
	int CheckNearbyRegion(int proximityThreshold = 30) const;  // 근처 지역 ID 반환 (-1: 없음)
	void OnRegionArrival(int regionId);  // 지역 도착 처리

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

	// 힌트 시스템 (좌표 기반)
	LocationHint GenerateHint(int regionId, bool exact = false);
	LocationHint GenerateSanctuaryHint();  // 최종 목적지 힌트 생성
	void DiscoverRegion(int regionId);
	std::pair<bool, LocationHint> TryDiscoverHintOnEvent();  // 이벤트 발생 시 힌트 (50% 확률, 그 중 5% 최종목적지)
	std::pair<bool, LocationHint> TryDiscoverHintDuringTravel();  // 이동 중 힌트
	std::pair<bool, LocationHint> TryDiscoverHintFromDialogue();  // 대화 시 힌트
	std::vector<LocationHint> GetDiscoveredHints() const;  // 발견된 좌표 목록

	// UI 알림용 힌트 접근
	bool HasPendingHintNotification() const { return !pendingHintNotifications.empty(); }
	const std::vector<LocationHint>& GetPendingHintNotifications() const { return pendingHintNotifications; }
	void ClearPendingHintNotifications() { pendingHintNotifications.clear(); }

	// 최종 목적지 (Sanctuary) 시스템
	void InitializeSanctuary();  // 랜덤 좌표 생성
	const Sanctuary& GetSanctuary() const { return sanctuary; }
	int GetDistanceToSanctuary() const;
	int GetAngleToSanctuary() const;
	bool CheckSanctuaryArrival(int threshold = 30);  // 도착 체크
	void UpgradeSanctuaryHint();  // 힌트 레벨 상승
	bool IsSanctuaryDiscovered() const { return sanctuary.discovered; }

	// 저장/로드
	void SaveState(std::ofstream& out) const;
	void LoadState(std::ifstream& in);

private:
	// 내부 계산
	int CalculateDistance(int x1, int y1, int x2, int y2) const;
	int CalculateAngle(int x1, int y1, int x2, int y2) const;
	int CalculateDaysRequired(int distance, TerrainType terrain) const;
	TerrainType GetTerrainAtPosition(int x, int y) const;
	void MoveInDirection(int angle, int distance);  // 각도 방향으로 이동

	// 지역 데이터
	std::vector<MapRegion> regions;

	// 현재 위치
	int currentX;
	int currentY;

	// 목표 좌표 (조타실에서 설정, 참고용)
	Route targetRoute;

	// 이동 상태
	int currentAngle;            // 현재 설정된 이동 각도
	int traveledDays;            // 총 이동 일수
	int lastCalculatedSpeed{ 0 };  // 마지막 계산된 속도 (UI 표시용)

	// 발견된 힌트 목록
	std::vector<LocationHint> discoveredHints;

	// UI 알림용 대기 힌트 (획득 시 추가, UI 표시 후 제거)
	std::vector<LocationHint> pendingHintNotifications;

	// 최종 목적지
	Sanctuary sanctuary;

	// 정비 상태
	bool inMaintenance;
	int maintenanceDaysLeft;

	// 힌트 메시지 (외부 파일에서 로드)
	std::vector<std::string> discoveryMessages;      // 쪽지 발견 대사
	std::vector<std::string> coordinateHintMessages; // 좌표 힌트 문구
	std::vector<std::string> directionHintMessages;  // 방향 힌트 문구
	std::vector<std::string> characteristicHintMessages; // 특성 힌트 문구

	// 힌트 메시지 로드
	void LoadHintMessages(const std::string& filepath);
	std::string GetRandomDiscoveryMessage();
	std::string FormatHintMessage(HintType type, const std::string& name, int x, int y, int angle, const std::string& terrain);

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
