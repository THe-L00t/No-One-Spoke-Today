#pragma once
#include "pch.h"
#include "data.h"

class City;
class Human;


// ===== 비트 레이아웃 상수 =====
// CityCode (16비트 사용, uint16_t)
// [15:12] 예약
// [11:9]  scarcity (3비트, 5단계)
// [8:6]   activity (3비트, 5단계)
// [5:3]   mood (3비트, 5단계)
// [2:0]   예약

namespace CityBit {
	constexpr int MOOD_SHIFT = 3;
	constexpr int ACTIVITY_SHIFT = 6;
	constexpr int SCARCITY_SHIFT = 9;

	constexpr uint16_t MOOD_MASK = 0b111 << MOOD_SHIFT;
	constexpr uint16_t ACTIVITY_MASK = 0b111 << ACTIVITY_SHIFT;
	constexpr uint16_t SCARCITY_MASK = 0b111 << SCARCITY_SHIFT;

	// 5단계 변환: 0~10000 → 0~4
	inline int ToLevel5(int value, int maxVal = 10000) {
		if (value <= 0) return 0;
		if (value >= maxVal) return 4;
		return (value * 5) / (maxVal + 1);
	}
}

// HumanCode (64비트 사용, uint64_t)
// [63:53] 예약
// [52:50] Region (3비트, 소속 구역)
// [49:32] Trait (18비트: 6개 × 3비트)
// [31:8]  Drives (24비트: 8개 × 3비트)
// [7:0]   MentalState (8비트: 4개 × 2비트)

namespace HumanBit {
	// MentalState (8비트)
	constexpr int AROUSAL_SHIFT = 0;
	constexpr int SOCIAL_SHIFT = 2;
	constexpr int ENERGY_SHIFT = 4;
	constexpr int CONTROL_SHIFT = 6;

	constexpr uint64_t AROUSAL_MASK = 0b11ULL << AROUSAL_SHIFT;
	constexpr uint64_t SOCIAL_MASK = 0b11ULL << SOCIAL_SHIFT;
	constexpr uint64_t ENERGY_MASK = 0b11ULL << ENERGY_SHIFT;
	constexpr uint64_t CONTROL_MASK = 0b11ULL << CONTROL_SHIFT;

	// Drives (24비트, 비트 8~31)
	constexpr int STRESS_SHIFT = 8;
	constexpr int EMO_AROUSAL_SHIFT = 11;
	constexpr int FATIGUE_SHIFT = 14;
	constexpr int COGNITIVE_SHIFT = 17;
	constexpr int TRUST_SHIFT = 20;
	constexpr int SAFETY_SHIFT = 23;
	constexpr int SENSE_CTRL_SHIFT = 26;
	constexpr int MOTIVATION_SHIFT = 29;

	constexpr uint64_t STRESS_MASK = 0b111ULL << STRESS_SHIFT;
	constexpr uint64_t EMO_AROUSAL_MASK = 0b111ULL << EMO_AROUSAL_SHIFT;
	constexpr uint64_t FATIGUE_MASK = 0b111ULL << FATIGUE_SHIFT;
	constexpr uint64_t COGNITIVE_MASK = 0b111ULL << COGNITIVE_SHIFT;
	constexpr uint64_t TRUST_MASK = 0b111ULL << TRUST_SHIFT;
	constexpr uint64_t SAFETY_MASK = 0b111ULL << SAFETY_SHIFT;
	constexpr uint64_t SENSE_CTRL_MASK = 0b111ULL << SENSE_CTRL_SHIFT;
	constexpr uint64_t MOTIVATION_MASK = 0b111ULL << MOTIVATION_SHIFT;

	// Trait (18비트, 비트 32~49)
	constexpr int RATIONALITY_SHIFT = 32;
	constexpr int AGGRESSION_SHIFT = 35;
	constexpr int PLANNING_SHIFT = 38;
	constexpr int DEPENDENCY_SHIFT = 41;
	constexpr int RIGIDITY_SHIFT = 44;
	constexpr int EMO_SENS_SHIFT = 47;

	constexpr uint64_t RATIONALITY_MASK = 0b111ULL << RATIONALITY_SHIFT;
	constexpr uint64_t AGGRESSION_MASK = 0b111ULL << AGGRESSION_SHIFT;
	constexpr uint64_t PLANNING_MASK = 0b111ULL << PLANNING_SHIFT;
	constexpr uint64_t DEPENDENCY_MASK = 0b111ULL << DEPENDENCY_SHIFT;
	constexpr uint64_t RIGIDITY_MASK = 0b111ULL << RIGIDITY_SHIFT;
	constexpr uint64_t EMO_SENS_MASK = 0b111ULL << EMO_SENS_SHIFT;

	// Region (3비트, 비트 50~52)
	constexpr int REGION_SHIFT = 50;
	constexpr uint64_t REGION_MASK = 0b111ULL << REGION_SHIFT;

	// 5단계 변환
	inline int ToLevel5(int value, int maxVal) {
		if (value <= 0) return 0;
		if (value >= maxVal) return 4;
		return (value * 5) / (maxVal + 1);
	}
}


// ===== 상태 코드 구조체 =====
struct CityCode {
	uint16_t code;

	CityCode() : code(0) {}
	explicit CityCode(uint16_t c) : code(c) {}
};

struct HumanCode {
	uint64_t code;

	HumanCode() : code(0) {}
	explicit HumanCode(uint64_t c) : code(c) {}
};


// ===== 인코딩 함수 선언 =====
CityCode EncodeCityState(const CityMetrics& metrics);
HumanCode EncodeHumanState(const Human& human);


// ===== 열거형 =====
enum class EventCategory {
	Environment,	// 환경 (날씨, 자연재해, 외부 요인)
	Personal,		// 개인 (정신상태 변화)
	Social,			// 사회 (집단 역학)
	Interpersonal,	// 대인관계
	CityWide		// 도시 전체
};

enum class TriggerScope {
	City,			// 도시 전체에서 N명 충족
	Region			// 특정 지역에서 N명 충족 (추후 확장)
};

enum class EffectScope {
	Triggered,		// 조건 충족자만
	AllHumans,		// 도시 전체 인간
	City,			// 도시 지표만
	Region,			// 특정 지역 인간 (추후 확장)
	Custom			// 커스텀 로직
};

enum class EffectType {
	ModifyDrive,		// 누적값 변경
	ModifyCityMetric,	// 도시 지표 변경
	ModifyMentalState,	// 정신상태 변경
	Kill,				// 사망
	AddImmigrant,		// 이주민 추가
	Custom				// 확장용
};

// 미방문 구역 이벤트 처리 방식
enum class UnresolvedBehavior {
	AutoResolve,	// 하루 종료 시 랜덤 선택지 자동 적용
	Expire,			// 효과 없이 소멸
	CarryOver		// 다음 날로 이월
};

enum class DriveField {
	StressLoad, EmotionalArousal, Fatigue, CognitiveCapacity,
	InterpersonalTrust, SocialSafety, SenseOfControl, Motivation
};

enum class MetricField { Mood, Activity, Scarcity };

enum class MentalField { Arousal, Social, Energy, Control };


// ===== 이벤트 트리거 (비트 마스킹) =====
struct EventTrigger {
	// 트리거 타입
	bool isRandom;				// true면 조건 무시, 랜덤 발생

	// 도시 조건
	uint16_t cityMask;			// 검사할 비트 마스크
	uint16_t cityMin;			// 최소값 (각 필드별)
	uint16_t cityMax;			// 최대값 (각 필드별)
	bool checkCity;				// 도시 조건 검사 여부

	// Human 조건
	uint64_t humanMask;			// 검사할 비트 마스크
	uint64_t humanMin;			// 최소값
	uint64_t humanMax;			// 최대값
	bool checkHuman;			// Human 조건 검사 여부

	// 범위 및 인원
	TriggerScope scope;			// City or Region
	int minHumanCount;			// 최소 충족 인원 (0 = 인원 무관)
	int regionId;				// Region 스코프일 때 지역 ID (추후 확장, -1 = 무관)

	EventTrigger()
		: isRandom(false)
		, cityMask(0), cityMin(0), cityMax(0xFFFF), checkCity(false)
		, humanMask(0), humanMin(0), humanMax(0xFFFFFFFFFFFFFFFFULL), checkHuman(false)
		, scope(TriggerScope::City), minHumanCount(0), regionId(-1)
	{}
};


// ===== 효과 데이터 =====
struct EffectData {
	EffectType type;
	int targetField;		// DriveField, MetricField, MentalField의 int 캐스트
	int delta;				// 변화량 (정신상태는 새 상태값)
	EffectScope scope;		// 효과 적용 범위 (선택지별로 다를 수 있음)
	std::string customId;	// Custom 타입 식별자

	EffectData()
		: type(EffectType::ModifyDrive), targetField(0), delta(0)
		, scope(EffectScope::Triggered), customId("")
	{}
	EffectData(EffectType t, int field, int d, EffectScope s = EffectScope::Triggered, const std::string& custom = "")
		: type(t), targetField(field), delta(d), scope(s), customId(custom)
	{}
};


// ===== 선택지 =====
struct Choice {
	std::string text;
	std::vector<EffectData> effects;
};


// ===== 이벤트 정의 =====
struct EventDef {
	// 기본 정보
	std::string id;
	std::string name;
	std::string description;
	EventCategory category;

	// 발동 조건
	EventTrigger trigger;

	// 쿨타임 (랜덤 범위)
	int cooldownMin;			// 최소 쿨타임 (일)
	int cooldownMax;			// 최대 쿨타임 (일)

	// 효과 범위
	EffectScope effectScope;
	int effectRegionId;			// Region 스코프일 때 지역 ID (-1 = 트리거와 동일)
	std::string customEffectId;	// Custom 효과 식별자

	// 선택지 (비어있으면 즉시 적용)
	bool requiresPlayer;
	std::vector<Choice> choices;
	std::vector<EffectData> immediateEffects;	// 선택지 없을 때

	// 미방문 시 처리 방식
	UnresolvedBehavior unresolvedBehavior;

	EventDef()
		: category(EventCategory::Environment)
		, cooldownMin(1), cooldownMax(7)
		, effectScope(EffectScope::Triggered)
		, effectRegionId(-1)
		, requiresPlayer(false)
		, unresolvedBehavior(UnresolvedBehavior::AutoResolve)
	{}
};


// ===== 활성 이벤트 =====
struct ActiveEvent {
	std::string defId;
	std::string name;
	std::string description;
	bool requiresPlayer;
	bool isActive;
	float triggerTimeRatio;			// 하루 중 발생 시점 (0.0~1.0)
	bool hasTriggered;				// 이미 발생했는지

	std::vector<Human*> triggeredHumans;	// 조건 충족자
	std::vector<Human*> affectedHumans;		// 효과 대상자

	std::vector<Choice> choices;
	int chosenIndex;

	// 구역 이벤트 관련
	int eventRegionId;				// 이벤트 발생 구역 ID (-1 = 전역 이벤트)
	bool isRegionEvent;				// 구역 특정 이벤트 여부
	UnresolvedBehavior unresolvedBehavior;	// 미방문 시 처리 방식

	ActiveEvent()
		: requiresPlayer(false), isActive(false)
		, triggerTimeRatio(0.0f), hasTriggered(false)
		, chosenIndex(-1)
		, eventRegionId(-1), isRegionEvent(false)
		, unresolvedBehavior(UnresolvedBehavior::AutoResolve)
	{}
};


// ===== 타입 별칭 =====
using CustomEffectFunc = std::function<void(
	City&,
	std::vector<Human*>& triggered,
	std::vector<Human*>& affected,
	std::vector<std::unique_ptr<Human>>& allHumans
)>;

// 이벤트 발생 시 콜백 (힌트 시스템 등 연결용)
using OnEventTriggeredCallback = std::function<void()>;


// ===== 이벤트 매니저 =====
class EventManager {
public:
	EventManager();

	// 이벤트 정의 파일 로드/저장
	void LoadEventDefs(const std::string& filepath);			// 바이너리
	void LoadEventDefsFromText(const std::string& filepath);	// 텍스트
	void SaveEventDefs(const std::string& filepath) const;		// 바이너리

	// 매일 호출: 후보군 선정 및 이벤트 스케줄링
	void ProcessDailyEvents(City& city, const CityMetrics& metrics,
		std::vector<std::unique_ptr<Human>>& humans, int currentDay);

	// 시간 경과 시 호출: 스케줄된 이벤트 발생 체크
	void UpdateTime(float dayRatio, City& city,
		std::vector<std::unique_ptr<Human>>& humans);

	// 플레이어 이벤트 처리
	bool HasPendingPlayerEvent() const;
	const ActiveEvent* GetPendingPlayerEvent() const;
	void ApplyPlayerChoice(int choiceIndex, City& city,
		std::vector<std::unique_ptr<Human>>& humans);

	// 구역 이벤트 처리
	bool HasPendingRegionEvent(int regionId) const;
	const ActiveEvent* GetPendingRegionEvent(int regionId) const;
	void ProcessPendingRegionEvents(int playerRegionId, City& city,
		std::vector<std::unique_ptr<Human>>& humans);
	void ProcessUnresolvedEvents(City& city,
		std::vector<std::unique_ptr<Human>>& humans);	// 하루 종료 시 미처리 이벤트 처리

	// 게임 상태 저장/로드
	void SaveState(std::ofstream& out) const;
	void LoadState(std::ifstream& in);

	// 이벤트 정의 접근
	const std::vector<EventDef>& GetDefinitions() const { return definitions; }
	void AddEventDef(const EventDef& def) { definitions.push_back(def); }

	// 이벤트 발생 콜백 설정 (힌트 시스템 연결)
	void SetOnEventTriggeredCallback(OnEventTriggeredCallback callback) { onEventTriggered = callback; }

private:
	void RegisterCustomEffects();
	void RegisterDefaultEvents();

	// 조건 검사
	bool CheckCityCondition(const EventTrigger& trigger, CityCode cityCode) const;
	bool CheckHumanCondition(const EventTrigger& trigger, HumanCode humanCode) const;
	std::vector<Human*> GetTriggeredHumans(const EventTrigger& trigger,
		const std::vector<std::unique_ptr<Human>>& humans) const;

	// 효과 대상 결정
	std::vector<Human*> DetermineAffectedHumans(const EventDef& def,
		const std::vector<Human*>& triggered,
		std::vector<std::unique_ptr<Human>>& allHumans) const;

	// 이벤트 발동
	ActiveEvent ActivateEvent(const EventDef& def, CityCode cityCode,
		std::vector<std::unique_ptr<Human>>& humans);
	void TriggerEvent(ActiveEvent& event, City& city,
		std::vector<std::unique_ptr<Human>>& humans);
	void ApplyEffects(const std::vector<EffectData>& effects,
		EffectScope scope, City& city,
		std::vector<Human*>& affected,
		std::vector<std::unique_ptr<Human>>& allHumans);

	std::vector<EventDef> definitions;
	std::deque<ActiveEvent> scheduledEvents;	// 오늘 발생 예정
	std::deque<ActiveEvent> pendingPlayerEvents;
	std::deque<ActiveEvent> pendingRegionPlayerEvents;	// 미방문 구역의 선택형 이벤트 대기열
	std::unordered_map<std::string, int> lastFiredDay;
	int minEventsPerDay{ 2 };
	int maxEventsPerDay{ 6 };

	// 하루 이벤트 진행 상태 (저장/로드용)
	int lastProcessedDay{ -1 };		// 마지막으로 이벤트 처리한 날
	int targetEventsToday{ 0 };		// 오늘 목표 이벤트 수
	int eventsTriggeredToday{ 0 };	// 오늘 이미 발생한 이벤트 수

	std::unordered_map<std::string, CustomEffectFunc> customEffectRegistry;

	std::default_random_engine rng;

	// 이벤트 발생 콜백
	OnEventTriggeredCallback onEventTriggered;
};
