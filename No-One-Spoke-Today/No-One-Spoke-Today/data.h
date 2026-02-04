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