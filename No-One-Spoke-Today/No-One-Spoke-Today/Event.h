#pragma once
#include "pch.h"
#include "data.h"

class City;
class Human;

// ===== 열거형 =====
enum class EventCategory { Environment, Personal, Social, CityWide };

enum class ConditionTarget {
	// 도시 지표
	CityMood, CityActivity, CityScarcity,
	// 인간 성향 (고정값, 0~100)
	HumanRationality, HumanAggressiveness, HumanPlanning,
	HumanDependency, HumanRigidity, HumanEmotionalSensitivity,
	// 인간 누적값 (동적, 0~10000)
	HumanStressLoad, HumanEmotionalArousal, HumanFatigue,
	HumanCognitiveCapacity, HumanInterpersonalTrust,
	HumanSocialSafety, HumanSenseOfControl, HumanMotivation,
	// 인간 정신상태 (enum int값)
	HumanArousal, HumanSocial, HumanEnergy, HumanControl
};

enum class CompareOp { Less, LessEq, Greater, GreaterEq, Equal, NotEqual };

enum class EffectType {
	ModifyParticipantDrive,   // 참여자 누적값 변경
	ModifyCityMetric,         // 도시 지표 변경
	KillParticipant,          // 참여자 사망
	AddImmigrant,             // 이주민 추가
	Custom                    // 확장용 (향후 AI 등)
};

enum class DriveField {
	StressLoad, EmotionalArousal, Fatigue, CognitiveCapacity,
	InterpersonalTrust, SocialSafety, SenseOfControl, Motivation
};

enum class MetricField { Mood, Activity, Scarcity };

// ===== 조건 (직렬화 가능) =====
struct EventCondition {
	ConditionTarget target;
	CompareOp op;
	int value;             // 비교 기준값
	int minMatchCount;     // 인간 조건: 최소 충족 인원 (0=도시지표)
};

// ===== 효과 (직렬화 가능) =====
struct EffectData {
	EffectType type;
	int targetField;       // DriveField 또는 MetricField의 int 캐스트
	int delta;             // 변화량 / 사망수 / 이주민수
	std::string customId;  // Custom 타입 식별자
};

// ===== 선택지 =====
struct Choice {
	std::string text;
	std::vector<EffectData> effects;
};

// ===== 이벤트 정의 (바이너리 파일 저장 대상) =====
struct EventDef {
	std::string id;
	std::string name;
	std::string description;
	EventCategory category;
	bool requiresPlayer;
	float baseProbability;       // 0.0~1.0
	int cooldownDays;            // 재발동 대기일
	int participantCount;        // 0=환경, 1=개인, N=사회
	std::string selectorId;     // 참여자 선택 방식 (코드 매핑)
	std::string customTriggerId; // 추가 트리거 함수 (선택, 빈 문자열=없음)
	std::vector<EventCondition> conditions;
	std::vector<Choice> choices;
};

// ===== 활성 이벤트 =====
struct ActiveEvent {
	std::string defId;
	std::string name;
	std::string description;
	bool requiresPlayer;
	bool isActive;
	std::vector<Human*> participants;
	std::vector<Choice> choices;
	int chosenIndex{ -1 };
};

// ===== 타입 별칭 (코드 매핑용) =====
using CustomTriggerFunc = std::function<bool(const CityMetrics&,
	const std::vector<std::unique_ptr<Human>>&)>;
using SelectorFunc = std::function<std::vector<Human*>(
	const std::vector<std::unique_ptr<Human>>&, int count)>;
using CustomEffectFunc = std::function<void(City&,
	std::vector<Human*>&, std::vector<std::unique_ptr<Human>>&)>;

// ===== 이벤트 매니저 =====
class EventManager {
public:
	EventManager();

	// 이벤트 정의 바이너리 파일 로드/저장
	void LoadEventDefs(const std::string& filepath);
	void SaveEventDefs(const std::string& filepath) const;

	// 매일 호출 (World에서 하루 전환 시)
	void ProcessDailyEvents(City& city, const CityMetrics& cityMet,
		std::vector<std::unique_ptr<Human>>& humans, int currentDay);

	// 플레이어 이벤트 처리
	bool HasPendingPlayerEvent() const;
	const ActiveEvent* GetPendingPlayerEvent() const;
	void ApplyPlayerChoice(int choiceIndex, City& city,
		std::vector<std::unique_ptr<Human>>& humans);

	// 게임 상태 저장/로드 (세이브 파일용)
	void SaveState(std::ofstream& out) const;
	void LoadState(std::ifstream& in);

private:
	void RegisterSelectors();
	void RegisterCustomTriggers();
	void RegisterCustomEffects();
	void RegisterDefaultEvents();

	bool EvaluateConditions(const EventDef& def, const CityMetrics& cityMet,
		const std::vector<std::unique_ptr<Human>>& humans) const;
	bool CheckCondition(const EventCondition& cond, const CityMetrics& cityMet,
		const std::vector<std::unique_ptr<Human>>& humans) const;

	ActiveEvent ActivateEvent(const EventDef& def,
		std::vector<std::unique_ptr<Human>>& humans);
	void ProcessAutoEvent(ActiveEvent& event, City& city,
		std::vector<std::unique_ptr<Human>>& humans);
	void ApplyEffects(const std::vector<EffectData>& effects, City& city,
		std::vector<Human*>& participants,
		std::vector<std::unique_ptr<Human>>& humans);

	std::vector<EventDef> definitions;
	std::deque<ActiveEvent> activeEvents;
	std::unordered_map<std::string, int> lastFiredDay; // 쿨다운 추적
	int maxEventsPerDay{ 2 };

	// 코드 매핑 레지스트리 (소수 특수 케이스용)
	std::unordered_map<std::string, SelectorFunc> selectorRegistry;
	std::unordered_map<std::string, CustomTriggerFunc> customTriggerRegistry;
	std::unordered_map<std::string, CustomEffectFunc> customEffectRegistry;

	std::default_random_engine rng;
};
