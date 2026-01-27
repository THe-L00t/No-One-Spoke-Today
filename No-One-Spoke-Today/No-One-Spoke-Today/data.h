#pragma once
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
