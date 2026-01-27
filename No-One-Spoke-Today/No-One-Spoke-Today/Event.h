#pragma once
#include "pch.h"
#include "data.h"

class City;
class Human;

// ========== 타입 별칭 ==========

// 트리거 함수: 도시 지표와 인간 목록으로 이벤트 발생 조건을 판단
using TriggerFunc = std::function<bool(const CityMetrics&, const std::vector<std::unique_ptr<Human>>&)>;

// 효과 함수: 도시와 참여자에게 효과를 적용
using EffectFunc = std::function<void(City&, std::vector<Human*>&)>;

// 참여자 선택 함수: 전체 인간 목록에서 참여자를 선택
using SelectorFunc = std::function<std::vector<Human*>(const std::vector<std::unique_ptr<Human>>&)>;


// ========== 선택지 ==========
struct Choice {
	std::string id;			// 레지스트리 키 (저장/로드용)
	std::string text;		// 선택지 표시 텍스트
	EffectFunc effect;		// 선택 시 적용되는 효과 함수
};


// ========== 이벤트 템플릿 ==========
// 코드에 정의된 이벤트의 원형. 발생 조건과 선택지를 포함.
struct EventTemplate {
	std::string id;					// 고유 식별자 (예: "evt_riot")
	std::string name;				// 이벤트 이름
	std::string description;		// 이벤트 설명
	bool requiresPlayer;			// 플레이어 선택이 필요한지 여부

	std::string triggerFuncId;		// 트리거 함수 레지스트리 키
	std::string selectorFuncId;		// 참여자 선택 함수 레지스트리 키

	TriggerFunc trigger;			// 발동 조건 함수
	SelectorFunc selector;			// 참여자 선택 함수
	std::vector<Choice> choices;	// 선택지 목록
};


// ========== 활성 이벤트 ==========
// 현재 진행 중인 이벤트 인스턴스
struct ActiveEvent {
	std::string templateId;				// 원본 EventTemplate의 id
	std::string name;					// 이벤트 이름 (표시용)
	std::string description;			// 이벤트 설명 (표시용)
	bool requiresPlayer;				// 플레이어 선택 필요 여부
	bool isActive;						// 현재 활성 상태
	std::vector<Human*> participants;	// 선택된 참여자 목록
	std::vector<Choice> choices;		// 선택지 (템플릿에서 복사)
	int chosenIndex{ -1 };				// 플레이어가 선택한 인덱스 (-1 = 미선택)
};


// ========== 이벤트 매니저 ==========
class EventManager {
public:
	EventManager();

	// 매일 호출: 트리거 평가 -> 이벤트 활성화 -> 처리 (하루 최대 maxEventsPerDay개)
	void ProcessDailyEvents(
		City& city,
		const CityMetrics& cityMet,
		std::vector<std::unique_ptr<Human>>& humans,
		int currentDay
	);

	// 플레이어 선택 대기 중인 이벤트가 있는지 확인
	bool HasPendingPlayerEvent() const;

	// 현재 플레이어에게 보여줄 활성 이벤트 반환
	const ActiveEvent* GetPendingPlayerEvent() const;

	// 플레이어의 선택을 적용
	void ApplyPlayerChoice(int choiceIndex, City& city);

	// 이벤트 템플릿 등록 (게임 초기화 시)
	void RegisterEventTemplate(const EventTemplate& tmpl);

	// ===== 저장/로드 =====
	void SaveEvents(std::ofstream& out) const;
	void LoadEvents(std::ifstream& in);

	// 로드 후 함수 포인터 재바인딩
	void RebindFunctions();

	// 로드 후 참여자 포인터 재선택
	void ReselectParticipants(const std::vector<std::unique_ptr<Human>>& humans);

private:
	// 함수 레지스트리 등록 (생성자에서 호출)
	void RegisterFunctions();

	// 이벤트 템플릿에서 ActiveEvent 생성
	ActiveEvent ActivateEvent(
		const EventTemplate& tmpl,
		const std::vector<std::unique_ptr<Human>>& humans
	);

	// 자동 이벤트 처리 (requiresPlayer == false)
	void ProcessAutoEvent(ActiveEvent& event, City& city);

	// ===== 데이터 =====
	std::vector<EventTemplate> templates;				// 등록된 이벤트 템플릿
	std::deque<ActiveEvent> activeEvents;				// 현재 활성 이벤트 큐
	int maxEventsPerDay{ 2 };							// 하루 최대 이벤트 수

	// ===== 함수 레지스트리 =====
	std::unordered_map<std::string, TriggerFunc> triggerRegistry;
	std::unordered_map<std::string, EffectFunc> effectRegistry;
	std::unordered_map<std::string, SelectorFunc> selectorRegistry;
};
