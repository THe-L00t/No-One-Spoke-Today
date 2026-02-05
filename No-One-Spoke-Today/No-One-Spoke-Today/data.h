#pragma once
#include <vector>
#include <algorithm>

// Human ==============================================================================================
struct Trait {
	int rationality;			// 감정보다 이성·논리 우선
	int aggressiveness;			// 위협 시 공격으로 대응하려는 경향	
	int planning;				// 장기 계획·지연 만족 성향
	int dependency;				// 타인·구조에 의존하려는 경향
	int rigidity;				// 신념·규칙 고수, 유연성 부족
	int emotionalSensitivity;	// 감정 자극에 민감한 정도
};

struct Drives {
	int stressLoad;			// 스트레스 누적
	int emotionalArousal;	// 감정 각성
	int fatigue;			// 피로
	int cognitiveCapacity;	// 인지 능력
	int interpersonalTrust;	// 대인 신뢰
	int socialSafety;		// 사회적 안전감
	int senseOfControl;		// 통제감
	int motivation;			// 동기 
};

enum class ArousalState {
	Calm,					// 차분함	긴장이나 불안이 거의 없고, 마음이 안정된 상태. 말과 행동 모두 평온함.
	Tense,					// 긴장		주변 상황에 주의를 기울이는 상태. 작은 자극에도 반응이 증가함.
	Irritable,				// 과민		쉽게 짜증이나 불쾌감을 느끼며, 말과 행동이 날카로움.
	Hostile					// 적대적	공격적이거나 반항적인 태도가 나타남. 대인 갈등 가능성이 높음.
};
enum class SocialState {
	Neutral,				// 중립		특별히 협력하거나 회피하지 않고, 평범한 사회적 태도.
	Cooperative,			// 협력적	주변 사람과 협력하려는 의지가 높고, 상호작용에 적극적.
	Withdrawn				// 철수		다른 사람과의 교류를 피하고 혼자 있으려는 상태.
};
enum class EnergyState {
	Normal,					// 정상		피로가 적고, 판단력과 행동력이 정상 수준.
	Fatigued,				// 피로		육체적·정신적 피로가 쌓여 반응 속도가 느려지고 집중력이 저하됨.
	Exhausted				// 소진		극도로 피로하여 판단과 행동 모두 크게 제한됨.
};
enum class ControlState {
	Autonomous,				// 자율		스스로 판단하고 행동하려는 의지가 강함.
	Dependent,				// 의존		다른 사람이나 구조에 의존하여 행동하려는 경향이 강함.
	Stubborn				// 고집		자신의 신념이나 방식에 집착하여 다른 의견이나 명령을 잘 수용하지 않음.
};

struct MentalState {
	ArousalState arousal;      // 감정 각성
	SocialState social;        // 사회적 태도
	EnergyState energy;        // 에너지/소진
	ControlState control;      // 통제감/의존
};


// Region ===========================================================================================
enum class Region {
	Cockpit,			// 0: 조타실 (플레이어 시작 위치)
	OuterWallMaintenance,	// 1: 외벽정비구역
	Canteen,			// 2: 식당
	RecyclingPlant,		// 3: 순환정제소
	VerticalFarm,		// 4: 수직농장
	LowerDrive,			// 5: 하부구동부
	CentralPowerway,	// 6: 중앙동력로
	ResidentialArea1,	// 7: 거주구역1
	ResidentialArea2,	// 8: 거주구역2
	COUNT				// 구역 수
};

// 구역 이름 반환
inline const char* GetRegionName(Region region) {
	switch (region) {
	case Region::Cockpit:				return "조타실";
	case Region::OuterWallMaintenance:	return "외벽정비구역";
	case Region::Canteen:				return "식당";
	case Region::RecyclingPlant:		return "순환정제소";
	case Region::VerticalFarm:			return "수직농장";
	case Region::LowerDrive:			return "하부구동부";
	case Region::CentralPowerway:		return "중앙동력로";
	case Region::ResidentialArea1:		return "거주구역1";
	case Region::ResidentialArea2:		return "거주구역2";
	default:							return "알 수 없음";
	}
}

// 구역 연결 그래프 (인접 리스트)
// 구조:
//                     [조타실]
//                        │
//                  [외벽정비구역]
//                    /       \
//               [식당]───────[순환정제소]
//               /│ \         / │ \
//              / │  \       /  │  \
//       [거주1] │ [수직농장]   │ [거주2]
//               │  /      \    │
//               │ /        \   │
//            [중앙동력로]──────┘
//                  │
//             [하부구동부]

inline std::vector<Region> GetAdjacentRegions(Region region) {
	switch (region) {
	case Region::Cockpit:
		return { Region::OuterWallMaintenance };
	case Region::OuterWallMaintenance:
		return { Region::Cockpit, Region::Canteen, Region::RecyclingPlant };
	case Region::Canteen:
		return { Region::OuterWallMaintenance, Region::CentralPowerway, Region::VerticalFarm, Region::ResidentialArea1 };
	case Region::RecyclingPlant:
		return { Region::OuterWallMaintenance, Region::CentralPowerway, Region::VerticalFarm, Region::ResidentialArea2 };
	case Region::VerticalFarm:
		return { Region::Canteen, Region::RecyclingPlant, Region::ResidentialArea1, Region::ResidentialArea2 };
	case Region::LowerDrive:
		return { Region::CentralPowerway };
	case Region::CentralPowerway:
		return { Region::Canteen, Region::RecyclingPlant, Region::LowerDrive };
	case Region::ResidentialArea1:
		return { Region::Canteen, Region::VerticalFarm };
	case Region::ResidentialArea2:
		return { Region::RecyclingPlant, Region::VerticalFarm };
	default:
		return {};
	}
}

// 두 구역이 연결되어 있는지 확인
inline bool AreRegionsConnected(Region from, Region to) {
	auto adjacent = GetAdjacentRegions(from);
	return std::find(adjacent.begin(), adjacent.end(), to) != adjacent.end();
}

// 구역별 기본 인구 배정 비율 (200명 기준)
// 조타실: 0, 외벽정비: 10, 식당: 10, 순환정제소: 10, 수직농장: 16
// 하부구동부: 6, 중앙동력로: 8, 거주구역1: 70, 거주구역2: 70
inline int GetRegionPopulationRatio(Region region) {
	switch (region) {
	case Region::Cockpit:				return 0;	// 플레이어만 상주
	case Region::OuterWallMaintenance:	return 10;	// 5%
	case Region::Canteen:				return 10;	// 5%
	case Region::RecyclingPlant:		return 10;	// 5%
	case Region::VerticalFarm:			return 16;	// 8%
	case Region::LowerDrive:			return 6;	// 3%
	case Region::CentralPowerway:		return 8;	// 4%
	case Region::ResidentialArea1:		return 70;	// 35%
	case Region::ResidentialArea2:		return 70;	// 35%
	default:							return 0;
	}
}

// city =============================================================================================
struct CityMetrics {		// 0~10000
	int mood;
	int activity;
	int scarcity;
};


// =============================================================================================
// 다층 영향 시스템 (Multi-Layer Influence System)
// =============================================================================================

// 구역별 환경 특성
struct RegionEnvironment {
	float fatigueRate;      // 피로 증가율 배수
	float stressRate;       // 스트레스 증가율 배수
	float motivationRate;   // 동기 변화율 (음수 = 감소)
	float safetyRate;       // 안전감 변화율
	float trustRate;        // 신뢰 변화율
	float contagionFactor;  // 사회적 전염 강도
	float recoveryRate;     // 회복 속도 배율
};

// 구역별 환경 상수
inline const RegionEnvironment& GetRegionEnvironment(Region region) {
	static const RegionEnvironment environments[] = {
		// Cockpit (조타실) - 고립, 책임감
		{ 0.8f,  1.2f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f },
		// OuterWallMaintenance (외벽정비) - 위험, 고강도
		{ 1.4f,  1.5f, -0.2f,  0.7f,  0.0f, 0.8f, 0.7f },
		// Canteen (식당) - 사교, 휴식
		{ 0.6f,  0.7f,  0.1f,  1.1f,  0.15f, 1.3f, 1.3f },
		// RecyclingPlant (순환정제소) - 단조로움, 오염
		{ 1.2f,  1.1f, -0.15f, 0.9f,  0.0f, 0.6f, 0.9f },
		// VerticalFarm (수직농장) - 자연, 생산적
		{ 1.0f,  0.8f,  0.1f,  1.05f, 0.05f, 0.7f, 1.1f },
		// LowerDrive (하부구동부) - 극한 노동, 소음
		{ 1.6f,  1.4f, -0.25f, 0.8f, -0.1f, 1.0f, 0.6f },
		// CentralPowerway (중앙동력로) - 위험, 필수
		{ 1.3f,  1.3f, -0.1f,  0.85f, 0.0f, 0.5f, 0.8f },
		// ResidentialArea1 (거주구역1) - 일상, 사교
		{ 0.8f,  0.9f,  0.0f,  1.0f,  0.1f, 1.2f, 1.2f },
		// ResidentialArea2 (거주구역2) - 일상, 사교
		{ 0.8f,  0.9f,  0.0f,  1.0f,  0.1f, 1.2f, 1.2f },
	};
	int idx = static_cast<int>(region);
	if (idx < 0 || idx >= static_cast<int>(Region::COUNT)) {
		idx = 0;
	}
	return environments[idx];
}

// 개인 성향 민감도 계수
struct TraitSensitivity {
	float stressSens;       // 스트레스 민감도
	float fatigueSens;      // 피로 민감도
	float motivationSens;   // 동기 민감도
	float safetySens;       // 사회적 안전감 민감도
	float trustSens;        // 신뢰 민감도
	float controlSens;      // 통제감 민감도
	float arousalSens;      // 감정 각성 민감도
	float cognitionSens;    // 인지 능력 민감도

	// 성향으로부터 민감도 계산
	static TraitSensitivity Calculate(const Trait& t) {
		TraitSensitivity s;

		// 스트레스 민감도: 감정민감↑, 이성↓
		s.stressSens = 1.0f
			+ (t.emotionalSensitivity - 50) * 0.008f
			- (t.rationality - 50) * 0.005f;

		// 피로 민감도: 감정민감↑, 계획성↓(페이싱)
		s.fatigueSens = 1.0f
			+ (t.emotionalSensitivity - 50) * 0.004f
			- (t.planning - 50) * 0.003f;

		// 동기 민감도: 감정민감(양날의 검), 경직↓
		s.motivationSens = 1.0f
			+ (t.emotionalSensitivity - 50) * 0.006f
			- (t.rigidity - 50) * 0.004f;

		// 안전감 민감도: 의존↑, 감정민감↑
		s.safetySens = 1.0f
			+ (t.dependency - 50) * 0.008f
			+ (t.emotionalSensitivity - 50) * 0.004f;

		// 신뢰 민감도: 공격성↓, 의존↑
		s.trustSens = 1.0f
			- (t.aggressiveness - 50) * 0.005f
			+ (t.dependency - 50) * 0.004f;

		// 통제감 민감도: 경직↑, 의존↓
		s.controlSens = 1.0f
			+ (t.rigidity - 50) * 0.006f
			- (t.dependency - 50) * 0.005f;

		// 감정 각성 민감도: 감정민감↑, 공격성↑
		s.arousalSens = 1.0f
			+ (t.emotionalSensitivity - 50) * 0.007f
			+ (t.aggressiveness - 50) * 0.004f;

		// 인지 민감도: 이성↓(안정), 감정민감↑
		s.cognitionSens = 1.0f
			- (t.rationality - 50) * 0.004f
			+ (t.emotionalSensitivity - 50) * 0.003f;

		return s;
	}
};

// 지형 보정값
struct TerrainModifiers {
	float fatigue;
	float stress;
	float motivation;
	float safety;
	float cognition;
	float temperature;
};

// 업데이트 컨텍스트 (Human::UpdateDrive에 전달)
class Human;  // 전방 선언

struct UpdateContext {
	CityMetrics city;
	TerrainModifiers terrain;
	Region humanRegion;
	std::vector<Human*> regionMembers;  // 같은 구역 사람들
	bool leaderPresent;                 // 리더(플레이어)가 현재 구역에 있는지
	int daysSinceLeaderVisit;           // 리더가 이 구역 방문 후 경과 일수
	float temperature;                  // 현재 기온

	UpdateContext()
		: city{ 5000, 5000, 5000 }
		, terrain{ 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 20.0f }
		, humanRegion(Region::ResidentialArea1)
		, leaderPresent(false)
		, daysSinceLeaderVisit(0)
		, temperature(20.0f)
	{}
};

// =============================================================================================
// 수식 관련 상수
// =============================================================================================
namespace FormulaConstants {
	// 기본 변화량 (초당)
	constexpr float BASE_STRESS_CHANGE = 0.15f;
	constexpr float BASE_FATIGUE_CHANGE = 0.08f;
	constexpr float BASE_MOTIVATION_CHANGE = 0.10f;
	constexpr float BASE_SAFETY_CHANGE = 0.06f;
	constexpr float BASE_TRUST_CHANGE = 0.04f;
	constexpr float BASE_CONTROL_CHANGE = 0.05f;
	constexpr float BASE_AROUSAL_CHANGE = 0.12f;
	constexpr float BASE_COGNITION_CHANGE = 0.05f;

	// 사회적 전염 계수
	constexpr float CONTAGION_HOSTILE = 0.15f;      // 적대적 사람 한 명당 전염
	constexpr float CONTAGION_IRRITABLE = 0.08f;    // 과민 사람 전염
	constexpr float CONTAGION_HIGH_STRESS = 0.05f;  // 고스트레스 전염
	constexpr float CONTAGION_COOPERATIVE = -0.06f; // 협력적 사람은 음의 전염

	// 사회적 지지/완충 계수
	constexpr float BUFFER_COOPERATIVE_MAX = 0.3f;  // 협력적 동료 최대 완충
	constexpr float BUFFER_LEADER_PRESENT = 0.2f;   // 리더 현재 방문 완충
	constexpr float BUFFER_LEADER_ABSENT_RATE = 0.05f; // 리더 부재 하루당 증가

	// 상호작용 효과 계수
	constexpr float INTERACTION_STRESS_FATIGUE = 0.8f;
	constexpr float INTERACTION_AROUSAL_STRESS = 0.5f;
	constexpr float INTERACTION_FATIGUE_MOTIVATION = 2.0f;

	// 임계점
	constexpr float THRESHOLD_HIGH = 80.0f;    // 고위험 구간
	constexpr float THRESHOLD_LOW = 20.0f;     // 안전 구간
	constexpr float THRESHOLD_MID_LOW = 40.0f; // 안정 구간 하한
	constexpr float THRESHOLD_MID_HIGH = 60.0f;// 안정 구간 상한

	// 회복 비대칭
	constexpr float RECOVERY_RATE = 0.6f;      // 회복은 악화의 60% 속도

	// 기온 관련
	constexpr float OPTIMAL_TEMP = 20.0f;
	constexpr float COMFORT_RANGE = 10.0f;

	// Yerkes-Dodson 최적 각성 구간
	constexpr float YD_OPTIMAL_LOW = 40.0f;
	constexpr float YD_OPTIMAL_HIGH = 60.0f;
	constexpr float YD_BONUS = 1.1f;

	// 결핍-신뢰 붕괴 임계점
	constexpr float SCARCITY_TRUST_THRESHOLD1 = 60.0f;
	constexpr float SCARCITY_TRUST_THRESHOLD2 = 75.0f;
}