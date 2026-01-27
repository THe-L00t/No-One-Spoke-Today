#include "Event.h"
#include "City.h"
#include "Human.h"

// ========== 생성자 ==========
EventManager::EventManager()
{
	RegisterFunctions();
}

// ========== 함수 레지스트리 등록 ==========
void EventManager::RegisterFunctions()
{
	// --- 트리거 함수 등록 ---
	// 예시 키만 등록, 실제 로직은 추후 구현
	triggerRegistry["trigger_high_stress"] = [](const CityMetrics& cm,
		const std::vector<std::unique_ptr<Human>>& humans) -> bool
	{
		// TODO: 실제 트리거 조건 구현
		return false;
	};

	triggerRegistry["trigger_low_mood"] = [](const CityMetrics& cm,
		const std::vector<std::unique_ptr<Human>>& humans) -> bool
	{
		// TODO: 실제 트리거 조건 구현
		return false;
	};

	// --- 효과 함수 등록 ---
	effectRegistry["effect_reduce_stress"] = [](City& city, std::vector<Human*>& participants)
	{
		// TODO: 실제 효과 구현
	};

	effectRegistry["effect_boost_mood"] = [](City& city, std::vector<Human*>& participants)
	{
		// TODO: 실제 효과 구현
	};

	// --- 참여자 선택 함수 등록 ---
	selectorRegistry["select_random_5"] = [](const std::vector<std::unique_ptr<Human>>& humans)
		-> std::vector<Human*>
	{
		// TODO: 실제 선택 로직 구현
		return {};
	};

	selectorRegistry["select_high_stress"] = [](const std::vector<std::unique_ptr<Human>>& humans)
		-> std::vector<Human*>
	{
		// TODO: 실제 선택 로직 구현
		return {};
	};
}

// ========== 이벤트 템플릿 등록 ==========
void EventManager::RegisterEventTemplate(const EventTemplate& tmpl)
{
	templates.push_back(tmpl);
}

// ========== 매일 이벤트 처리 ==========
void EventManager::ProcessDailyEvents(
	City& city,
	const CityMetrics& cityMet,
	std::vector<std::unique_ptr<Human>>& humans,
	int currentDay)
{
	int eventsActivated = 0;

	// 1단계: 트리거 평가 - 모든 템플릿을 순회하며 조건 확인
	for (const auto& tmpl : templates) {
		if (eventsActivated >= maxEventsPerDay) break;

		// 트리거 함수가 없으면 건너뜀
		if (not tmpl.trigger) continue;

		// 트리거 조건 평가
		if (not tmpl.trigger(cityMet, humans)) continue;

		// 2단계: 이벤트 활성화 (참여자 선택 포함)
		ActiveEvent active = ActivateEvent(tmpl, humans);

		// 3단계: 자동 이벤트는 즉시 처리, 플레이어 이벤트는 큐에 추가
		if (not active.requiresPlayer) {
			ProcessAutoEvent(active, city);
		}
		else {
			active.isActive = true;
			activeEvents.push_back(std::move(active));
		}

		++eventsActivated;
	}
}

// ========== 이벤트 활성화 ==========
ActiveEvent EventManager::ActivateEvent(
	const EventTemplate& tmpl,
	const std::vector<std::unique_ptr<Human>>& humans)
{
	ActiveEvent event;
	event.templateId = tmpl.id;
	event.name = tmpl.name;
	event.description = tmpl.description;
	event.requiresPlayer = tmpl.requiresPlayer;
	event.isActive = true;
	event.choices = tmpl.choices;
	event.chosenIndex = -1;

	// 참여자 선택
	if (tmpl.selector) {
		event.participants = tmpl.selector(humans);
	}

	return event;
}

// ========== 자동 이벤트 처리 ==========
void EventManager::ProcessAutoEvent(ActiveEvent& event, City& city)
{
	// 자동 이벤트는 첫 번째 선택지를 자동 적용
	if (not event.choices.empty() and event.choices[0].effect) {
		event.choices[0].effect(city, event.participants);
	}
	event.isActive = false;
}

// ========== 플레이어 이벤트 확인 ==========
bool EventManager::HasPendingPlayerEvent() const
{
	for (const auto& evt : activeEvents) {
		if (evt.isActive and evt.requiresPlayer and evt.chosenIndex < 0) {
			return true;
		}
	}
	return false;
}

// ========== 대기 중인 플레이어 이벤트 반환 ==========
const ActiveEvent* EventManager::GetPendingPlayerEvent() const
{
	for (const auto& evt : activeEvents) {
		if (evt.isActive and evt.requiresPlayer and evt.chosenIndex < 0) {
			return &evt;
		}
	}
	return nullptr;
}

// ========== 플레이어 선택 적용 ==========
void EventManager::ApplyPlayerChoice(int choiceIndex, City& city)
{
	for (auto& evt : activeEvents) {
		if (evt.isActive and evt.requiresPlayer and evt.chosenIndex < 0) {
			if (choiceIndex >= 0 and choiceIndex < static_cast<int>(evt.choices.size())) {
				evt.chosenIndex = choiceIndex;
				if (evt.choices[choiceIndex].effect) {
					evt.choices[choiceIndex].effect(city, evt.participants);
				}
			}
			evt.isActive = false;
			break;
		}
	}
}

// ========== 저장 ==========
void EventManager::SaveEvents(std::ofstream& out) const
{
	// 활성 이벤트 수 저장
	size_t count = activeEvents.size();
	out.write(reinterpret_cast<const char*>(&count), sizeof(count));

	for (const auto& evt : activeEvents) {
		// templateId
		size_t len = evt.templateId.size();
		out.write(reinterpret_cast<const char*>(&len), sizeof(len));
		out.write(evt.templateId.data(), len);

		// name
		len = evt.name.size();
		out.write(reinterpret_cast<const char*>(&len), sizeof(len));
		out.write(evt.name.data(), len);

		// description
		len = evt.description.size();
		out.write(reinterpret_cast<const char*>(&len), sizeof(len));
		out.write(evt.description.data(), len);

		// requiresPlayer, isActive, chosenIndex
		out.write(reinterpret_cast<const char*>(&evt.requiresPlayer), sizeof(evt.requiresPlayer));
		out.write(reinterpret_cast<const char*>(&evt.isActive), sizeof(evt.isActive));
		out.write(reinterpret_cast<const char*>(&evt.chosenIndex), sizeof(evt.chosenIndex));

		// 선택지: id와 text만 저장 (effect 함수는 저장하지 않음)
		size_t choiceCount = evt.choices.size();
		out.write(reinterpret_cast<const char*>(&choiceCount), sizeof(choiceCount));
		for (const auto& c : evt.choices) {
			len = c.id.size();
			out.write(reinterpret_cast<const char*>(&len), sizeof(len));
			out.write(c.id.data(), len);

			len = c.text.size();
			out.write(reinterpret_cast<const char*>(&len), sizeof(len));
			out.write(c.text.data(), len);
		}
		// participants(Human*)는 저장하지 않음 - 로드 시 재선택
	}
}

// ========== 로드 ==========
void EventManager::LoadEvents(std::ifstream& in)
{
	activeEvents.clear();

	size_t count = 0;
	in.read(reinterpret_cast<char*>(&count), sizeof(count));

	for (size_t i = 0; i < count; ++i) {
		ActiveEvent evt;

		// templateId
		size_t len = 0;
		in.read(reinterpret_cast<char*>(&len), sizeof(len));
		evt.templateId.resize(len);
		in.read(evt.templateId.data(), len);

		// name
		in.read(reinterpret_cast<char*>(&len), sizeof(len));
		evt.name.resize(len);
		in.read(evt.name.data(), len);

		// description
		in.read(reinterpret_cast<char*>(&len), sizeof(len));
		evt.description.resize(len);
		in.read(evt.description.data(), len);

		// requiresPlayer, isActive, chosenIndex
		in.read(reinterpret_cast<char*>(&evt.requiresPlayer), sizeof(evt.requiresPlayer));
		in.read(reinterpret_cast<char*>(&evt.isActive), sizeof(evt.isActive));
		in.read(reinterpret_cast<char*>(&evt.chosenIndex), sizeof(evt.chosenIndex));

		// 선택지
		size_t choiceCount = 0;
		in.read(reinterpret_cast<char*>(&choiceCount), sizeof(choiceCount));
		evt.choices.resize(choiceCount);
		for (auto& c : evt.choices) {
			in.read(reinterpret_cast<char*>(&len), sizeof(len));
			c.id.resize(len);
			in.read(c.id.data(), len);

			in.read(reinterpret_cast<char*>(&len), sizeof(len));
			c.text.resize(len);
			in.read(c.text.data(), len);
			// effect는 로드하지 않음 - RebindFunctions에서 복원
		}

		activeEvents.push_back(std::move(evt));
	}
}

// ========== 로드 후 함수 재바인딩 ==========
void EventManager::RebindFunctions()
{
	// 활성 이벤트의 선택지 effect를 레지스트리에서 복원
	for (auto& evt : activeEvents) {
		for (auto& choice : evt.choices) {
			auto it = effectRegistry.find(choice.id);
			if (it != effectRegistry.end()) {
				choice.effect = it->second;
			}
		}
	}

	// 템플릿의 trigger, selector도 레지스트리에서 복원
	for (auto& tmpl : templates) {
		auto trigIt = triggerRegistry.find(tmpl.triggerFuncId);
		if (trigIt != triggerRegistry.end()) {
			tmpl.trigger = trigIt->second;
		}
		auto selIt = selectorRegistry.find(tmpl.selectorFuncId);
		if (selIt != selectorRegistry.end()) {
			tmpl.selector = selIt->second;
		}
	}
}

// ========== 로드 후 참여자 재선택 ==========
void EventManager::ReselectParticipants(const std::vector<std::unique_ptr<Human>>& humans)
{
	for (auto& evt : activeEvents) {
		if (not evt.isActive) continue;

		// 원본 템플릿을 찾아 selector로 참여자 재선택
		for (const auto& tmpl : templates) {
			if (tmpl.id == evt.templateId and tmpl.selector) {
				evt.participants = tmpl.selector(humans);
				break;
			}
		}
	}
}
