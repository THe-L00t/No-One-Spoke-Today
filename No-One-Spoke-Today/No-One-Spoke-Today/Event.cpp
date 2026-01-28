#include "Event.h"
#include "City.h"
#include "Human.h"



const EventCode& EncodingEvent(const City& city, const std::vector<std::unique_ptr<Human>>& humans) {

}
// ========== 바이너리 I/O 헬퍼 ==========
namespace {
	void WriteString(std::ofstream& out, const std::string& s) {
		uint32_t len = static_cast<uint32_t>(s.size());
		out.write(reinterpret_cast<const char*>(&len), sizeof(len));
		out.write(s.data(), len);
	}

	std::string ReadString(std::ifstream& in) {
		uint32_t len = 0;
		in.read(reinterpret_cast<char*>(&len), sizeof(len));
		std::string s(len, '\0');
		in.read(s.data(), len);
		return s;
	}

	bool CompareValues(CompareOp op, int lhs, int rhs) {
		switch (op) {
		case CompareOp::Less:       return lhs < rhs;
		case CompareOp::LessEq:    return lhs <= rhs;
		case CompareOp::Greater:   return lhs > rhs;
		case CompareOp::GreaterEq: return lhs >= rhs;
		case CompareOp::Equal:     return lhs == rhs;
		case CompareOp::NotEqual:  return lhs != rhs;
		}
		return false;
	}

	int GetHumanValue(const Human& h, ConditionTarget target) {
		switch (target) {
		// 성향 (0~100)
		case ConditionTarget::HumanRationality:        return h.GetRationality();
		case ConditionTarget::HumanAggressiveness:     return h.GetAggressiveness();
		case ConditionTarget::HumanPlanning:           return h.GetPlanning();
		case ConditionTarget::HumanDependency:         return h.GetDependency();
		case ConditionTarget::HumanRigidity:           return h.GetRigidity();
		case ConditionTarget::HumanEmotionalSensitivity: return h.GetEmotionalSensitivity();
		// 누적값 (0~10000)
		case ConditionTarget::HumanStressLoad:         return h.GetStressLoad();
		case ConditionTarget::HumanEmotionalArousal:   return h.GetEmotionalArousal();
		case ConditionTarget::HumanFatigue:            return h.GetFatigue();
		case ConditionTarget::HumanCognitiveCapacity:  return h.GetCognitiveCapacity();
		case ConditionTarget::HumanInterpersonalTrust: return h.GetInterpersonalTrust();
		case ConditionTarget::HumanSocialSafety:       return h.GetSocialSafety();
		case ConditionTarget::HumanSenseOfControl:     return h.GetSenseOfControl();
		case ConditionTarget::HumanMotivation:         return h.GetMotivation();
		// 정신상태 (enum int)
		case ConditionTarget::HumanArousal:  return static_cast<int>(h.GetArousal());
		case ConditionTarget::HumanSocial:   return static_cast<int>(h.GetSocial());
		case ConditionTarget::HumanEnergy:   return static_cast<int>(h.GetEnergy());
		case ConditionTarget::HumanControl:  return static_cast<int>(h.GetControl());
		default: return 0;
		}
	}

	// ===== 이벤트 정의 축약 헬퍼 =====
	EventCondition Cond(ConditionTarget t, CompareOp op, int val, int minMatch = 0) {
		return {t, op, val, minMatch};
	}
	EffectData DriveEff(DriveField f, int delta) {
		return {EffectType::ModifyParticipantDrive, static_cast<int>(f), delta, ""};
	}
	EffectData CityEff(MetricField f, int delta) {
		return {EffectType::ModifyCityMetric, static_cast<int>(f), delta, ""};
	}
	EffectData KillEff(int count) {
		return {EffectType::KillParticipant, 0, count, ""};
	}
	EffectData ImmigrantEff(int count) {
		return {EffectType::AddImmigrant, 0, count, ""};
	}
}

// ========== 생성자 ==========
EventManager::EventManager()
	: rng(std::random_device{}())
{
	RegisterSelectors();
	RegisterCustomTriggers();
	RegisterCustomEffects();
	RegisterDefaultEvents();
}

// ========== 셀렉터 등록 ==========
void EventManager::RegisterSelectors()
{
	selectorRegistry["select_random"] = [this](
		const std::vector<std::unique_ptr<Human>>& humans, int count)
		-> std::vector<Human*>
	{
		std::vector<Human*> pool;
		pool.reserve(humans.size());
		for (auto& h : humans) pool.push_back(h.get());

		std::shuffle(pool.begin(), pool.end(), rng);

		int n = (std::min)(count, static_cast<int>(pool.size()));
		pool.resize(n);
		return pool;
	};

	selectorRegistry["select_high_stress"] = [](
		const std::vector<std::unique_ptr<Human>>& humans, int count)
		-> std::vector<Human*>
	{
		std::vector<Human*> pool;
		pool.reserve(humans.size());
		for (auto& h : humans) pool.push_back(h.get());

		std::sort(pool.begin(), pool.end(), [](Human* a, Human* b) {
			return a->GetStressLoad() > b->GetStressLoad();
		});

		int n = (std::min)(count, static_cast<int>(pool.size()));
		pool.resize(n);
		return pool;
	};

	selectorRegistry["select_low_trust"] = [](
		const std::vector<std::unique_ptr<Human>>& humans, int count)
		-> std::vector<Human*>
	{
		std::vector<Human*> pool;
		pool.reserve(humans.size());
		for (auto& h : humans) pool.push_back(h.get());

		std::sort(pool.begin(), pool.end(), [](Human* a, Human* b) {
			return a->GetInterpersonalTrust() < b->GetInterpersonalTrust();
		});

		int n = (std::min)(count, static_cast<int>(pool.size()));
		pool.resize(n);
		return pool;
	};

	selectorRegistry["select_hostile"] = [](
		const std::vector<std::unique_ptr<Human>>& humans, int count)
		-> std::vector<Human*>
	{
		std::vector<Human*> pool;
		for (auto& h : humans) {
			if (h->GetArousal() >= ArousalState::Irritable)
				pool.push_back(h.get());
		}
		std::sort(pool.begin(), pool.end(), [](Human* a, Human* b) {
			return static_cast<int>(a->GetArousal()) > static_cast<int>(b->GetArousal());
		});
		int n = (std::min)(count, static_cast<int>(pool.size()));
		pool.resize(n);
		return pool;
	};

	selectorRegistry["select_cooperative"] = [](
		const std::vector<std::unique_ptr<Human>>& humans, int count)
		-> std::vector<Human*>
	{
		std::vector<Human*> pool;
		for (auto& h : humans) {
			if (h->GetSocial() == SocialState::Cooperative)
				pool.push_back(h.get());
		}
		int n = (std::min)(count, static_cast<int>(pool.size()));
		pool.resize(n);
		return pool;
	};

	selectorRegistry["select_exhausted"] = [](
		const std::vector<std::unique_ptr<Human>>& humans, int count)
		-> std::vector<Human*>
	{
		std::vector<Human*> pool;
		pool.reserve(humans.size());
		for (auto& h : humans) pool.push_back(h.get());
		std::sort(pool.begin(), pool.end(), [](Human* a, Human* b) {
			return a->GetFatigue() > b->GetFatigue();
		});
		int n = (std::min)(count, static_cast<int>(pool.size()));
		pool.resize(n);
		return pool;
	};

	selectorRegistry["select_high_aggression"] = [](
		const std::vector<std::unique_ptr<Human>>& humans, int count)
		-> std::vector<Human*>
	{
		std::vector<Human*> pool;
		pool.reserve(humans.size());
		for (auto& h : humans) pool.push_back(h.get());
		std::sort(pool.begin(), pool.end(), [](Human* a, Human* b) {
			return a->GetAggressiveness() > b->GetAggressiveness();
		});
		int n = (std::min)(count, static_cast<int>(pool.size()));
		pool.resize(n);
		return pool;
	};

	selectorRegistry["select_high_motivation"] = [](
		const std::vector<std::unique_ptr<Human>>& humans, int count)
		-> std::vector<Human*>
	{
		std::vector<Human*> pool;
		pool.reserve(humans.size());
		for (auto& h : humans) pool.push_back(h.get());
		std::sort(pool.begin(), pool.end(), [](Human* a, Human* b) {
			return a->GetMotivation() > b->GetMotivation();
		});
		int n = (std::min)(count, static_cast<int>(pool.size()));
		pool.resize(n);
		return pool;
	};
}

// ========== 커스텀 트리거 등록 ==========
void EventManager::RegisterCustomTriggers()
{
	// 확장용: 특수 트리거 조건 함수 등록
	// 예시:
	// customTriggerRegistry["trigger_special"] = [](const CityMetrics& cm,
	//     const std::vector<std::unique_ptr<Human>>& humans) -> bool { ... };
}

// ========== 커스텀 효과 등록 ==========
void EventManager::RegisterCustomEffects()
{
	// 확장용: 특수 효과 함수 등록
	// 예시:
	// customEffectRegistry["effect_special"] = [](City& city,
	//     std::vector<Human*>& participants,
	//     std::vector<std::unique_ptr<Human>>& humans) { ... };
}

// ========== 조건 평가 ==========
bool EventManager::EvaluateConditions(const EventDef& def, const CityMetrics& cityMet,
	const std::vector<std::unique_ptr<Human>>& humans) const
{
	for (const auto& cond : def.conditions) {
		if (not CheckCondition(cond, cityMet, humans))
			return false;
	}
	return true;
}

bool EventManager::CheckCondition(const EventCondition& cond, const CityMetrics& cityMet,
	const std::vector<std::unique_ptr<Human>>& humans) const
{
	// 도시 지표 조건
	switch (cond.target) {
	case ConditionTarget::CityMood:
		return CompareValues(cond.op, cityMet.mood, cond.value);
	case ConditionTarget::CityActivity:
		return CompareValues(cond.op, cityMet.activity, cond.value);
	case ConditionTarget::CityScarcity:
		return CompareValues(cond.op, cityMet.scarcity, cond.value);
	default:
		break;
	}

	// 인간 조건: 매칭 인원수 카운트
	int matchCount = 0;
	for (const auto& h : humans) {
		int val = GetHumanValue(*h, cond.target);
		if (CompareValues(cond.op, val, cond.value)) {
			++matchCount;
			if (matchCount >= cond.minMatchCount)
				return true;
		}
	}
	return false;
}

// ========== 매일 이벤트 처리 ==========
void EventManager::ProcessDailyEvents(
	City& city,
	const CityMetrics& cityMet,
	std::vector<std::unique_ptr<Human>>& humans,
	int currentDay)
{
	// 1. 조건 충족 이벤트 수집
	std::vector<const EventDef*> candidates;

	for (const auto& def : definitions) {
		// 쿨다운 확인
		auto it = lastFiredDay.find(def.id);
		if (it != lastFiredDay.end()) {
			if (currentDay - it->second < def.cooldownDays)
				continue;
		}

		// 데이터 기반 조건 평가
		if (not EvaluateConditions(def, cityMet, humans))
			continue;

		// 커스텀 트리거 확인
		if (not def.customTriggerId.empty()) {
			auto trigIt = customTriggerRegistry.find(def.customTriggerId);
			if (trigIt != customTriggerRegistry.end()) {
				if (not trigIt->second(cityMet, humans))
					continue;
			}
		}

		candidates.push_back(&def);
	}

	// 2. 셔플
	std::shuffle(candidates.begin(), candidates.end(), rng);

	// 3. 확률 롤 + 최대 2건 활성화
	int eventsActivated = 0;
	std::uniform_real_distribution<float> probDist(0.0f, 1.0f);

	for (const auto* def : candidates) {
		if (eventsActivated >= maxEventsPerDay) break;

		// 확률 롤
		if (probDist(rng) > def->baseProbability) continue;

		// 이벤트 활성화
		ActiveEvent active = ActivateEvent(*def, humans);
		lastFiredDay[def->id] = currentDay;

		if (not active.requiresPlayer) {
			ProcessAutoEvent(active, city, humans);
		}
		else {
			active.isActive = true;
			activeEvents.push_back(std::move(active));
		}

		++eventsActivated;
	}
}

// ========== 이벤트 활성화 ==========
ActiveEvent EventManager::ActivateEvent(const EventDef& def,
	std::vector<std::unique_ptr<Human>>& humans)
{
	ActiveEvent event;
	event.defId = def.id;
	event.name = def.name;
	event.description = def.description;
	event.requiresPlayer = def.requiresPlayer;
	event.isActive = true;
	event.choices = def.choices;
	event.chosenIndex = -1;

	// 참여자 선택
	if (def.participantCount > 0 and not def.selectorId.empty()) {
		auto it = selectorRegistry.find(def.selectorId);
		if (it != selectorRegistry.end()) {
			event.participants = it->second(humans, def.participantCount);
		}
	}

	return event;
}

// ========== 자동 이벤트 처리 ==========
void EventManager::ProcessAutoEvent(ActiveEvent& event, City& city,
	std::vector<std::unique_ptr<Human>>& humans)
{
	// 자동 이벤트는 첫 번째 선택지를 자동 적용
	if (not event.choices.empty()) {
		ApplyEffects(event.choices[0].effects, city, event.participants, humans);
	}
	event.isActive = false;
}

// ========== 효과 적용 ==========
void EventManager::ApplyEffects(const std::vector<EffectData>& effects, City& city,
	std::vector<Human*>& participants,
	std::vector<std::unique_ptr<Human>>& humans)
{
	for (const auto& eff : effects) {
		switch (eff.type) {
		case EffectType::ModifyParticipantDrive:
		{
			DriveField field = static_cast<DriveField>(eff.targetField);
			for (auto* h : participants) {
				switch (field) {
				case DriveField::StressLoad:       h->ModifyStressLoad(eff.delta); break;
				case DriveField::EmotionalArousal: h->ModifyEmotionalArousal(eff.delta); break;
				case DriveField::Fatigue:          h->ModifyFatigue(eff.delta); break;
				case DriveField::CognitiveCapacity:h->ModifyCognitiveCapacity(eff.delta); break;
				case DriveField::InterpersonalTrust:h->ModifyInterpersonalTrust(eff.delta); break;
				case DriveField::SocialSafety:     h->ModifySocialSafety(eff.delta); break;
				case DriveField::SenseOfControl:   h->ModifySenseOfControl(eff.delta); break;
				case DriveField::Motivation:       h->ModifyMotivation(eff.delta); break;
				}
			}
			break;
		}
		case EffectType::ModifyCityMetric:
		{
			MetricField field = static_cast<MetricField>(eff.targetField);
			switch (field) {
			case MetricField::Mood:     city.ModifyMood(eff.delta); break;
			case MetricField::Activity: city.ModifyActivity(eff.delta); break;
			case MetricField::Scarcity: city.ModifyScarcity(eff.delta); break;
			}
			break;
		}
		case EffectType::KillParticipant:
		{
			// delta = 사망 수 (참여자 중 앞에서부터)
			int killCount = (std::min)(eff.delta, static_cast<int>(participants.size()));
			for (int i = 0; i < killCount; ++i) {
				Human* target = participants[i];
				// humans 벡터에서 해당 포인터를 찾아 erase
				auto it = std::find_if(humans.begin(), humans.end(),
					[target](const std::unique_ptr<Human>& h) {
						return h.get() == target;
					});
				if (it != humans.end()) {
					humans.erase(it);
				}
			}
			// 참여자 목록에서도 제거
			if (killCount > 0 and killCount <= static_cast<int>(participants.size())) {
				participants.erase(participants.begin(),
					participants.begin() + killCount);
			}
			break;
		}
		case EffectType::AddImmigrant:
		{
			// delta = 이주민 수
			for (int i = 0; i < eff.delta; ++i) {
				humans.emplace_back(std::make_unique<Human>());
			}
			break;
		}
		case EffectType::Custom:
		{
			if (not eff.customId.empty()) {
				auto it = customEffectRegistry.find(eff.customId);
				if (it != customEffectRegistry.end()) {
					it->second(city, participants, humans);
				}
			}
			break;
		}
		}
	}
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
void EventManager::ApplyPlayerChoice(int choiceIndex, City& city,
	std::vector<std::unique_ptr<Human>>& humans)
{
	for (auto& evt : activeEvents) {
		if (evt.isActive and evt.requiresPlayer and evt.chosenIndex < 0) {
			if (choiceIndex >= 0 and choiceIndex < static_cast<int>(evt.choices.size())) {
				evt.chosenIndex = choiceIndex;
				ApplyEffects(evt.choices[choiceIndex].effects, city,
					evt.participants, humans);
			}
			evt.isActive = false;
			break;
		}
	}
}

// ========== 이벤트 정의 바이너리 저장 ==========
void EventManager::SaveEventDefs(const std::string& filepath) const
{
	std::ofstream out(filepath, std::ios::binary);
	if (not out.is_open()) return;

	// 버전
	uint32_t version = 1;
	out.write(reinterpret_cast<const char*>(&version), sizeof(version));

	// 이벤트 수
	uint32_t count = static_cast<uint32_t>(definitions.size());
	out.write(reinterpret_cast<const char*>(&count), sizeof(count));

	for (const auto& def : definitions) {
		WriteString(out, def.id);
		WriteString(out, def.name);
		WriteString(out, def.description);

		uint8_t cat = static_cast<uint8_t>(def.category);
		out.write(reinterpret_cast<const char*>(&cat), sizeof(cat));

		out.write(reinterpret_cast<const char*>(&def.requiresPlayer), sizeof(def.requiresPlayer));
		out.write(reinterpret_cast<const char*>(&def.baseProbability), sizeof(def.baseProbability));

		int32_t cd = def.cooldownDays;
		out.write(reinterpret_cast<const char*>(&cd), sizeof(cd));
		int32_t pc = def.participantCount;
		out.write(reinterpret_cast<const char*>(&pc), sizeof(pc));

		WriteString(out, def.selectorId);
		WriteString(out, def.customTriggerId);

		// 조건
		uint32_t condCount = static_cast<uint32_t>(def.conditions.size());
		out.write(reinterpret_cast<const char*>(&condCount), sizeof(condCount));
		for (const auto& cond : def.conditions) {
			uint8_t target = static_cast<uint8_t>(cond.target);
			uint8_t op = static_cast<uint8_t>(cond.op);
			out.write(reinterpret_cast<const char*>(&target), sizeof(target));
			out.write(reinterpret_cast<const char*>(&op), sizeof(op));
			int32_t val = cond.value;
			out.write(reinterpret_cast<const char*>(&val), sizeof(val));
			int32_t mmc = cond.minMatchCount;
			out.write(reinterpret_cast<const char*>(&mmc), sizeof(mmc));
		}

		// 선택지
		uint32_t choiceCount = static_cast<uint32_t>(def.choices.size());
		out.write(reinterpret_cast<const char*>(&choiceCount), sizeof(choiceCount));
		for (const auto& choice : def.choices) {
			WriteString(out, choice.text);

			uint32_t effCount = static_cast<uint32_t>(choice.effects.size());
			out.write(reinterpret_cast<const char*>(&effCount), sizeof(effCount));
			for (const auto& eff : choice.effects) {
				uint8_t type = static_cast<uint8_t>(eff.type);
				out.write(reinterpret_cast<const char*>(&type), sizeof(type));
				int32_t tf = eff.targetField;
				out.write(reinterpret_cast<const char*>(&tf), sizeof(tf));
				int32_t delta = eff.delta;
				out.write(reinterpret_cast<const char*>(&delta), sizeof(delta));
				WriteString(out, eff.customId);
			}
		}
	}
}

// ========== 이벤트 정의 바이너리 로드 ==========
void EventManager::LoadEventDefs(const std::string& filepath)
{
	std::ifstream in(filepath, std::ios::binary);
	if (not in.is_open()) return;

	uint32_t version = 0;
	in.read(reinterpret_cast<char*>(&version), sizeof(version));

	uint32_t count = 0;
	in.read(reinterpret_cast<char*>(&count), sizeof(count));

	definitions.clear();
	definitions.reserve(count);

	for (uint32_t i = 0; i < count; ++i) {
		EventDef def;
		def.id = ReadString(in);
		def.name = ReadString(in);
		def.description = ReadString(in);

		uint8_t cat = 0;
		in.read(reinterpret_cast<char*>(&cat), sizeof(cat));
		def.category = static_cast<EventCategory>(cat);

		in.read(reinterpret_cast<char*>(&def.requiresPlayer), sizeof(def.requiresPlayer));
		in.read(reinterpret_cast<char*>(&def.baseProbability), sizeof(def.baseProbability));

		int32_t cd = 0;
		in.read(reinterpret_cast<char*>(&cd), sizeof(cd));
		def.cooldownDays = cd;
		int32_t pc = 0;
		in.read(reinterpret_cast<char*>(&pc), sizeof(pc));
		def.participantCount = pc;

		def.selectorId = ReadString(in);
		def.customTriggerId = ReadString(in);

		// 조건
		uint32_t condCount = 0;
		in.read(reinterpret_cast<char*>(&condCount), sizeof(condCount));
		def.conditions.resize(condCount);
		for (auto& cond : def.conditions) {
			uint8_t target = 0, op = 0;
			in.read(reinterpret_cast<char*>(&target), sizeof(target));
			in.read(reinterpret_cast<char*>(&op), sizeof(op));
			cond.target = static_cast<ConditionTarget>(target);
			cond.op = static_cast<CompareOp>(op);
			int32_t val = 0;
			in.read(reinterpret_cast<char*>(&val), sizeof(val));
			cond.value = val;
			int32_t mmc = 0;
			in.read(reinterpret_cast<char*>(&mmc), sizeof(mmc));
			cond.minMatchCount = mmc;
		}

		// 선택지
		uint32_t choiceCount = 0;
		in.read(reinterpret_cast<char*>(&choiceCount), sizeof(choiceCount));
		def.choices.resize(choiceCount);
		for (auto& choice : def.choices) {
			choice.text = ReadString(in);

			uint32_t effCount = 0;
			in.read(reinterpret_cast<char*>(&effCount), sizeof(effCount));
			choice.effects.resize(effCount);
			for (auto& eff : choice.effects) {
				uint8_t type = 0;
				in.read(reinterpret_cast<char*>(&type), sizeof(type));
				eff.type = static_cast<EffectType>(type);
				int32_t tf = 0;
				in.read(reinterpret_cast<char*>(&tf), sizeof(tf));
				eff.targetField = tf;
				int32_t delta = 0;
				in.read(reinterpret_cast<char*>(&delta), sizeof(delta));
				eff.delta = delta;
				eff.customId = ReadString(in);
			}
		}

		definitions.push_back(std::move(def));
	}
}

// ========== 게임 상태 저장 (세이브 파일용) ==========
void EventManager::SaveState(std::ofstream& out) const
{
	// 쿨다운 맵
	uint32_t cooldownCount = static_cast<uint32_t>(lastFiredDay.size());
	out.write(reinterpret_cast<const char*>(&cooldownCount), sizeof(cooldownCount));
	for (const auto& [id, day] : lastFiredDay) {
		WriteString(out, id);
		int32_t d = day;
		out.write(reinterpret_cast<const char*>(&d), sizeof(d));
	}

	// 활성 이벤트
	uint32_t activeCount = static_cast<uint32_t>(activeEvents.size());
	out.write(reinterpret_cast<const char*>(&activeCount), sizeof(activeCount));

	for (const auto& evt : activeEvents) {
		WriteString(out, evt.defId);
		WriteString(out, evt.name);
		WriteString(out, evt.description);
		out.write(reinterpret_cast<const char*>(&evt.requiresPlayer), sizeof(evt.requiresPlayer));
		out.write(reinterpret_cast<const char*>(&evt.isActive), sizeof(evt.isActive));
		int32_t ci = evt.chosenIndex;
		out.write(reinterpret_cast<const char*>(&ci), sizeof(ci));

		// 선택지 (effects 포함)
		uint32_t choiceCount = static_cast<uint32_t>(evt.choices.size());
		out.write(reinterpret_cast<const char*>(&choiceCount), sizeof(choiceCount));
		for (const auto& choice : evt.choices) {
			WriteString(out, choice.text);
			uint32_t effCount = static_cast<uint32_t>(choice.effects.size());
			out.write(reinterpret_cast<const char*>(&effCount), sizeof(effCount));
			for (const auto& eff : choice.effects) {
				uint8_t type = static_cast<uint8_t>(eff.type);
				out.write(reinterpret_cast<const char*>(&type), sizeof(type));
				int32_t tf = eff.targetField;
				out.write(reinterpret_cast<const char*>(&tf), sizeof(tf));
				int32_t delta = eff.delta;
				out.write(reinterpret_cast<const char*>(&delta), sizeof(delta));
				WriteString(out, eff.customId);
			}
		}
		// participants는 저장하지 않음 - 로드 시 재선택
	}
}

// ========== 게임 상태 로드 (세이브 파일용) ==========
void EventManager::LoadState(std::ifstream& in)
{
	// 쿨다운 맵
	lastFiredDay.clear();
	uint32_t cooldownCount = 0;
	in.read(reinterpret_cast<char*>(&cooldownCount), sizeof(cooldownCount));
	for (uint32_t i = 0; i < cooldownCount; ++i) {
		std::string id = ReadString(in);
		int32_t day = 0;
		in.read(reinterpret_cast<char*>(&day), sizeof(day));
		lastFiredDay[id] = day;
	}

	// 활성 이벤트
	activeEvents.clear();
	uint32_t activeCount = 0;
	in.read(reinterpret_cast<char*>(&activeCount), sizeof(activeCount));

	for (uint32_t i = 0; i < activeCount; ++i) {
		ActiveEvent evt;
		evt.defId = ReadString(in);
		evt.name = ReadString(in);
		evt.description = ReadString(in);
		in.read(reinterpret_cast<char*>(&evt.requiresPlayer), sizeof(evt.requiresPlayer));
		in.read(reinterpret_cast<char*>(&evt.isActive), sizeof(evt.isActive));
		int32_t ci = 0;
		in.read(reinterpret_cast<char*>(&ci), sizeof(ci));
		evt.chosenIndex = ci;

		// 선택지
		uint32_t choiceCount = 0;
		in.read(reinterpret_cast<char*>(&choiceCount), sizeof(choiceCount));
		evt.choices.resize(choiceCount);
		for (auto& choice : evt.choices) {
			choice.text = ReadString(in);
			uint32_t effCount = 0;
			in.read(reinterpret_cast<char*>(&effCount), sizeof(effCount));
			choice.effects.resize(effCount);
			for (auto& eff : choice.effects) {
				uint8_t type = 0;
				in.read(reinterpret_cast<char*>(&type), sizeof(type));
				eff.type = static_cast<EffectType>(type);
				int32_t tf = 0;
				in.read(reinterpret_cast<char*>(&tf), sizeof(tf));
				eff.targetField = tf;
				int32_t delta = 0;
				in.read(reinterpret_cast<char*>(&delta), sizeof(delta));
				eff.delta = delta;
				eff.customId = ReadString(in);
			}
		}

		activeEvents.push_back(std::move(evt));
	}
}

// ==========================================================================
//   기본 이벤트 등록 (58개)
// ==========================================================================
void EventManager::RegisterDefaultEvents()
{
	definitions.clear();
	definitions.reserve(60);

	// ========================
	//   환경 이벤트 (8)
	// ========================

	// [1] 폭풍우 - 순수 랜덤
	definitions.push_back({"evt_storm", "폭풍우",
		"거센 폭풍이 도시를 휩쓸고 지나갑니다. 시민들이 불안해합니다.",
		EventCategory::Environment, false, 0.12f, 3, 10, "select_random", "",
		{},
		{{"폭풍이 지나갑니다.", {CityEff(MetricField::Mood, -300),
		  DriveEff(DriveField::StressLoad, 500), DriveEff(DriveField::Fatigue, 200)}}}
	});

	// [2] 폭염
	definitions.push_back({"evt_heatwave", "폭염",
		"극심한 더위가 도시를 덮칩니다. 사람들이 지쳐갑니다.",
		EventCategory::Environment, false, 0.10f, 5, 15, "select_random", "",
		{},
		{{"더위가 계속됩니다.", {CityEff(MetricField::Activity, -400),
		  DriveEff(DriveField::Fatigue, 600), DriveEff(DriveField::StressLoad, 300)}}}
	});

	// [3] 폭우
	definitions.push_back({"evt_heavy_rain", "폭우",
		"쏟아지는 비로 도시 곳곳이 침수됩니다.",
		EventCategory::Environment, false, 0.15f, 2, 8, "select_random", "",
		{},
		{{"비가 쏟아집니다.", {CityEff(MetricField::Activity, -300),
		  CityEff(MetricField::Scarcity, 100), DriveEff(DriveField::Fatigue, 200)}}}
	});

	// [4] 한파
	definitions.push_back({"evt_cold_snap", "한파",
		"매서운 추위가 몰아칩니다. 난방과 식량 수요가 급증합니다.",
		EventCategory::Environment, false, 0.10f, 5, 10, "select_random", "",
		{},
		{{"추위가 계속됩니다.", {CityEff(MetricField::Scarcity, 300),
		  DriveEff(DriveField::StressLoad, 400), DriveEff(DriveField::Fatigue, 300)}}}
	});

	// [5] 맑은 날씨
	definitions.push_back({"evt_clear_sky", "맑은 날씨",
		"화창한 날씨가 이어집니다. 사람들의 기분이 한결 나아집니다.",
		EventCategory::Environment, false, 0.20f, 2, 10, "select_random", "",
		{},
		{{"좋은 날씨가 이어집니다.", {CityEff(MetricField::Mood, 200),
		  DriveEff(DriveField::StressLoad, -200), DriveEff(DriveField::Motivation, 100)}}}
	});

	// [6] 지진 (플레이어)
	definitions.push_back({"evt_earthquake", "지진",
		"강한 지진이 도시를 뒤흔듭니다. 건물이 무너지고 사람들이 공포에 빠집니다.",
		EventCategory::Environment, true, 0.03f, 30, 15, "select_random", "",
		{},
		{{"긴급 대피 지시", {CityEff(MetricField::Activity, -500),
		  DriveEff(DriveField::StressLoad, 300), CityEff(MetricField::Scarcity, 200)}},
		 {"구조대 즉시 파견", {CityEff(MetricField::Scarcity, 300),
		  DriveEff(DriveField::InterpersonalTrust, 200), DriveEff(DriveField::SocialSafety, 100)}},
		 {"자체 판단에 맡김", {KillEff(2), CityEff(MetricField::Mood, -500),
		  DriveEff(DriveField::InterpersonalTrust, -300)}}}
	});

	// [7] 홍수
	definitions.push_back({"evt_flood", "홍수",
		"폭우로 인한 홍수가 발생했습니다. 저지대가 침수되었습니다.",
		EventCategory::Environment, false, 0.05f, 14, 10, "select_random", "",
		{Cond(ConditionTarget::CityScarcity, CompareOp::GreaterEq, 4000)},
		{{"물이 차오릅니다.", {CityEff(MetricField::Scarcity, 500),
		  CityEff(MetricField::Activity, -300), DriveEff(DriveField::StressLoad, 600), KillEff(1)}}}
	});

	// [8] 산불
	definitions.push_back({"evt_wildfire", "산불",
		"인근 산에서 대형 화재가 발생했습니다. 연기가 도시를 뒤덮습니다.",
		EventCategory::Environment, false, 0.04f, 20, 5, "select_random", "",
		{},
		{{"산불이 번집니다.", {CityEff(MetricField::Mood, -400),
		  CityEff(MetricField::Scarcity, 300), DriveEff(DriveField::StressLoad, 500)}}}
	});

	// ========================
	//   인프라 이벤트 (5)
	// ========================

	// [9] 정전 - 높은 활동량일 때
	definitions.push_back({"evt_power_outage", "정전",
		"도시 전역에 정전이 발생했습니다. 일상이 마비됩니다.",
		EventCategory::Environment, false, 0.08f, 7, 10, "select_random", "",
		{Cond(ConditionTarget::CityActivity, CompareOp::GreaterEq, 7000)},
		{{"전기가 끊겼습니다.", {CityEff(MetricField::Activity, -600),
		  CityEff(MetricField::Mood, -200), DriveEff(DriveField::StressLoad, 400)}}}
	});

	// [10] 수자원 부족
	definitions.push_back({"evt_water_shortage", "수자원 부족",
		"깨끗한 물이 부족해지고 있습니다.",
		EventCategory::Environment, false, 0.10f, 10, 10, "select_random", "",
		{Cond(ConditionTarget::CityScarcity, CompareOp::GreaterEq, 6000)},
		{{"물 부족이 심화됩니다.", {CityEff(MetricField::Scarcity, 400),
		  DriveEff(DriveField::StressLoad, 500), DriveEff(DriveField::SocialSafety, -300)}}}
	});

	// [11] 건물 붕괴 (플레이어)
	definitions.push_back({"evt_building_collapse", "건물 붕괴",
		"노후된 건물이 붕괴되었습니다. 사람들이 매몰되었을 수 있습니다.",
		EventCategory::Environment, true, 0.05f, 14, 5, "select_random", "",
		{Cond(ConditionTarget::CityScarcity, CompareOp::GreaterEq, 6000),
		 Cond(ConditionTarget::CityActivity, CompareOp::Less, 4000)},
		{{"즉각 구조 작업", {CityEff(MetricField::Scarcity, 200),
		  DriveEff(DriveField::Fatigue, 500), DriveEff(DriveField::InterpersonalTrust, 200)}},
		 {"안전 확보 우선", {CityEff(MetricField::Activity, -300),
		  DriveEff(DriveField::StressLoad, 300)}},
		 {"방관", {KillEff(1), CityEff(MetricField::Mood, -400),
		  DriveEff(DriveField::SocialSafety, -300)}}}
	});

	// [12] 자원 발견
	definitions.push_back({"evt_resource_found", "자원 발견",
		"사용할 수 있는 물자가 발견되었습니다!",
		EventCategory::Environment, false, 0.08f, 10, 0, "", "",
		{},
		{{"물자를 확보했습니다.", {CityEff(MetricField::Scarcity, -300),
		  CityEff(MetricField::Mood, 100)}}}
	});

	// [13] 화재
	definitions.push_back({"evt_fire", "화재",
		"건물에서 화재가 발생했습니다!",
		EventCategory::Environment, false, 0.06f, 7, 5, "select_random", "",
		{Cond(ConditionTarget::CityScarcity, CompareOp::GreaterEq, 5000)},
		{{"불길이 번집니다.", {CityEff(MetricField::Mood, -300),
		  CityEff(MetricField::Scarcity, 200), DriveEff(DriveField::StressLoad, 600), KillEff(1)}}}
	});

	// ========================
	//   개인 이벤트 (12)
	// ========================

	// [14] 정신적 붕괴 - 극심한 스트레스 + 높은 감정민감도
	definitions.push_back({"evt_mental_breakdown", "정신적 붕괴",
		"극심한 스트레스를 견디지 못한 사람이 무너져 내립니다.",
		EventCategory::Personal, false, 0.25f, 5, 1, "select_high_stress", "",
		{Cond(ConditionTarget::HumanStressLoad, CompareOp::GreaterEq, 8000, 1),
		 Cond(ConditionTarget::HumanEmotionalSensitivity, CompareOp::GreaterEq, 60, 1)},
		{{"정신이 무너집니다.", {DriveEff(DriveField::StressLoad, 2000),
		  DriveEff(DriveField::CognitiveCapacity, -1000), DriveEff(DriveField::SenseOfControl, -800)}}}
	});

	// [15] 창의적 영감 - 높은 인지 + 높은 동기 + 차분
	definitions.push_back({"evt_creative_breakthrough", "창의적 영감",
		"누군가가 놀라운 아이디어를 떠올렸습니다.",
		EventCategory::Personal, false, 0.15f, 7, 1, "select_high_motivation", "",
		{Cond(ConditionTarget::HumanCognitiveCapacity, CompareOp::GreaterEq, 7000, 1),
		 Cond(ConditionTarget::HumanMotivation, CompareOp::GreaterEq, 7000, 1),
		 Cond(ConditionTarget::HumanArousal, CompareOp::Equal, 0, 1)},
		{{"영감이 떠오릅니다.", {DriveEff(DriveField::CognitiveCapacity, 500),
		  DriveEff(DriveField::Motivation, 800), CityEff(MetricField::Activity, 100)}}}
	});

	// [16] 감정 폭발 - 높은 감정각성 + 높은 감정민감도 + 과민 이상
	definitions.push_back({"evt_emotional_outburst", "감정 폭발",
		"누군가가 감정을 주체하지 못하고 폭발합니다.",
		EventCategory::Personal, false, 0.20f, 3, 1, "select_high_stress", "",
		{Cond(ConditionTarget::HumanEmotionalArousal, CompareOp::GreaterEq, 7000, 1),
		 Cond(ConditionTarget::HumanEmotionalSensitivity, CompareOp::GreaterEq, 65, 1),
		 Cond(ConditionTarget::HumanArousal, CompareOp::GreaterEq, 2, 1)},
		{{"감정이 폭발합니다.", {DriveEff(DriveField::EmotionalArousal, 1000),
		  DriveEff(DriveField::InterpersonalTrust, -400), DriveEff(DriveField::SocialSafety, -300)}}}
	});

	// [17] 번아웃 - 높은 피로 + 낮은 동기
	definitions.push_back({"evt_burnout", "번아웃",
		"지속된 과로로 누군가가 완전히 지쳐버렸습니다.",
		EventCategory::Personal, false, 0.20f, 7, 1, "select_exhausted", "",
		{Cond(ConditionTarget::HumanFatigue, CompareOp::GreaterEq, 8000, 1),
		 Cond(ConditionTarget::HumanMotivation, CompareOp::Less, 2000, 1)},
		{{"의욕이 사라집니다.", {DriveEff(DriveField::Motivation, -1500),
		  DriveEff(DriveField::Fatigue, 1000), DriveEff(DriveField::CognitiveCapacity, -500)}}}
	});

	// [18] 깨달음 - 높은 인지 + 자율 + 차분
	definitions.push_back({"evt_epiphany", "깨달음",
		"고요한 순간, 누군가가 삶에 대한 깊은 깨달음을 얻습니다.",
		EventCategory::Personal, false, 0.08f, 14, 1, "select_random", "",
		{Cond(ConditionTarget::HumanCognitiveCapacity, CompareOp::GreaterEq, 7000, 1),
		 Cond(ConditionTarget::HumanControl, CompareOp::Equal, 0, 1),
		 Cond(ConditionTarget::HumanArousal, CompareOp::Equal, 0, 1)},
		{{"깨달음을 얻습니다.", {DriveEff(DriveField::SenseOfControl, 600),
		  DriveEff(DriveField::Motivation, 500), DriveEff(DriveField::StressLoad, -400)}}}
	});

	// [19] 도피 행동 - 높은 스트레스 + 낮은 통제감 + 의존적
	definitions.push_back({"evt_escape_behavior", "도피 행동",
		"현실을 감당하지 못한 누군가가 도피 행동에 빠집니다.",
		EventCategory::Personal, false, 0.18f, 5, 1, "select_high_stress", "",
		{Cond(ConditionTarget::HumanStressLoad, CompareOp::GreaterEq, 7000, 1),
		 Cond(ConditionTarget::HumanSenseOfControl, CompareOp::Less, 3000, 1),
		 Cond(ConditionTarget::HumanControl, CompareOp::Equal, 1, 1)},
		{{"현실에서 도피합니다.", {DriveEff(DriveField::StressLoad, -500),
		  DriveEff(DriveField::CognitiveCapacity, -800), DriveEff(DriveField::SocialSafety, -400)}}}
	});

	// [20] 신체 쓰러짐 - 극심한 피로 + 높은 스트레스
	definitions.push_back({"evt_physical_collapse", "신체 쓰러짐",
		"과로와 스트레스로 누군가가 쓰러졌습니다.",
		EventCategory::Personal, false, 0.15f, 7, 1, "select_exhausted", "",
		{Cond(ConditionTarget::HumanFatigue, CompareOp::GreaterEq, 9000, 1),
		 Cond(ConditionTarget::HumanStressLoad, CompareOp::GreaterEq, 7000, 1)},
		{{"쓰러집니다.", {DriveEff(DriveField::Fatigue, 1500),
		  DriveEff(DriveField::CognitiveCapacity, -1000), CityEff(MetricField::Mood, -100)}}}
	});

	// [21] 분노 발작 - 적대적 + 높은 공격성 + 높은 스트레스
	definitions.push_back({"evt_rage_episode", "분노 발작",
		"참을 수 없는 분노가 폭발합니다.",
		EventCategory::Personal, false, 0.20f, 3, 1, "select_hostile", "",
		{Cond(ConditionTarget::HumanArousal, CompareOp::GreaterEq, 3, 1),
		 Cond(ConditionTarget::HumanAggressiveness, CompareOp::GreaterEq, 65, 1),
		 Cond(ConditionTarget::HumanStressLoad, CompareOp::GreaterEq, 7000, 1)},
		{{"분노가 폭발합니다.", {DriveEff(DriveField::StressLoad, 800),
		  DriveEff(DriveField::EmotionalArousal, 1200), DriveEff(DriveField::InterpersonalTrust, -500)}}}
	});

	// [22] 우울 증세 - 높은 피로 + 낮은 동기 + 철수
	definitions.push_back({"evt_depression", "우울 증세",
		"깊은 무기력감에 빠진 사람이 있습니다.",
		EventCategory::Personal, false, 0.18f, 5, 1, "select_exhausted", "",
		{Cond(ConditionTarget::HumanFatigue, CompareOp::GreaterEq, 6000, 1),
		 Cond(ConditionTarget::HumanMotivation, CompareOp::Less, 3000, 1),
		 Cond(ConditionTarget::HumanSocial, CompareOp::Equal, 2, 1)},
		{{"무기력에 빠집니다.", {DriveEff(DriveField::Motivation, -800),
		  DriveEff(DriveField::SocialSafety, -400), DriveEff(DriveField::InterpersonalTrust, -300)}}}
	});

	// [23] 불안 발작 - 긴장 + 높은 감정각성 + 높은 감정민감도
	definitions.push_back({"evt_anxiety_attack", "불안 발작",
		"극심한 불안에 시달리는 사람이 있습니다.",
		EventCategory::Personal, false, 0.18f, 3, 1, "select_high_stress", "",
		{Cond(ConditionTarget::HumanArousal, CompareOp::GreaterEq, 1, 1),
		 Cond(ConditionTarget::HumanEmotionalArousal, CompareOp::GreaterEq, 7000, 1),
		 Cond(ConditionTarget::HumanEmotionalSensitivity, CompareOp::GreaterEq, 70, 1)},
		{{"불안이 엄습합니다.", {DriveEff(DriveField::StressLoad, 600),
		  DriveEff(DriveField::CognitiveCapacity, -500), DriveEff(DriveField::SenseOfControl, -400)}}}
	});

	// [24] 자연 회복 - 차분 + 정상 에너지
	definitions.push_back({"evt_natural_recovery", "자연 회복",
		"안정을 되찾은 누군가가 서서히 회복하고 있습니다.",
		EventCategory::Personal, false, 0.25f, 3, 1, "select_random", "",
		{Cond(ConditionTarget::HumanArousal, CompareOp::Equal, 0, 1),
		 Cond(ConditionTarget::HumanEnergy, CompareOp::Equal, 0, 1)},
		{{"회복합니다.", {DriveEff(DriveField::StressLoad, -500),
		  DriveEff(DriveField::Fatigue, -400), DriveEff(DriveField::Motivation, 300)}}}
	});

	// [25] 집중력 향상 - 높은 인지 + 차분 + 높은 이성
	definitions.push_back({"evt_focus_boost", "집중력 향상",
		"맑은 정신으로 놀라운 집중력을 보여주는 사람이 있습니다.",
		EventCategory::Personal, false, 0.12f, 5, 1, "select_random", "",
		{Cond(ConditionTarget::HumanCognitiveCapacity, CompareOp::GreaterEq, 6000, 1),
		 Cond(ConditionTarget::HumanArousal, CompareOp::Equal, 0, 1),
		 Cond(ConditionTarget::HumanRationality, CompareOp::GreaterEq, 65, 1)},
		{{"집중합니다.", {DriveEff(DriveField::CognitiveCapacity, 600),
		  DriveEff(DriveField::Motivation, 400), CityEff(MetricField::Activity, 50)}}}
	});

	// ========================
	//   대인 갈등 이벤트 (8)
	// ========================

	// [26] 거리 싸움 - 높은 공격성 + 적대적/과민 + 높은 스트레스
	definitions.push_back({"evt_street_fight", "거리 싸움",
		"스트레스가 폭발한 사람들 사이에 주먹다짐이 벌어졌습니다.",
		EventCategory::Social, false, 0.20f, 3, 3, "select_hostile", "",
		{Cond(ConditionTarget::HumanAggressiveness, CompareOp::GreaterEq, 65, 2),
		 Cond(ConditionTarget::HumanArousal, CompareOp::GreaterEq, 2, 2),
		 Cond(ConditionTarget::HumanStressLoad, CompareOp::GreaterEq, 6000, 2)},
		{{"싸움이 벌어집니다.", {DriveEff(DriveField::StressLoad, 800),
		  DriveEff(DriveField::InterpersonalTrust, -500), CityEff(MetricField::Mood, -200), KillEff(1)}}}
	});

	// [27] 말다툼 - 높은 고집 + 긴장 + 서로 다른 의견
	definitions.push_back({"evt_argument", "말다툼",
		"완고한 성격의 사람들이 격렬하게 다투고 있습니다.",
		EventCategory::Social, false, 0.25f, 2, 2, "select_hostile", "",
		{Cond(ConditionTarget::HumanRigidity, CompareOp::GreaterEq, 60, 2),
		 Cond(ConditionTarget::HumanArousal, CompareOp::GreaterEq, 1, 2)},
		{{"고성이 오갑니다.", {DriveEff(DriveField::StressLoad, 500),
		  DriveEff(DriveField::InterpersonalTrust, -300), DriveEff(DriveField::EmotionalArousal, 400)}}}
	});

	// [28] 괴롭힘 (플레이어) - 높은 공격성 가해자
	definitions.push_back({"evt_bullying", "괴롭힘",
		"강한 사람이 약한 사람을 괴롭히고 있습니다.",
		EventCategory::Social, true, 0.15f, 5, 2, "select_high_aggression", "",
		{Cond(ConditionTarget::HumanAggressiveness, CompareOp::GreaterEq, 70, 1),
		 Cond(ConditionTarget::HumanSocialSafety, CompareOp::Less, 3000, 1)},
		{{"직접 개입하여 제지", {DriveEff(DriveField::InterpersonalTrust, 300),
		  DriveEff(DriveField::SocialSafety, 400), CityEff(MetricField::Mood, 100)}},
		 {"규칙을 강화", {DriveEff(DriveField::SocialSafety, 200),
		  DriveEff(DriveField::SenseOfControl, -200), CityEff(MetricField::Activity, -100)}},
		 {"무시", {DriveEff(DriveField::SocialSafety, -500),
		  DriveEff(DriveField::InterpersonalTrust, -300), CityEff(MetricField::Mood, -200)}}}
	});

	// [29] 도둑질 (플레이어) - 높은 스트레스 + 높은 결핍
	definitions.push_back({"evt_theft", "도둑질",
		"굶주린 누군가가 다른 사람의 물자를 훔쳤습니다.",
		EventCategory::Social, true, 0.15f, 5, 2, "select_high_stress", "",
		{Cond(ConditionTarget::HumanStressLoad, CompareOp::GreaterEq, 6000, 1),
		 Cond(ConditionTarget::CityScarcity, CompareOp::GreaterEq, 5000)},
		{{"범인 처벌", {DriveEff(DriveField::SocialSafety, 300),
		  DriveEff(DriveField::InterpersonalTrust, -100), CityEff(MetricField::Mood, -100)}},
		 {"원인 해결 (자원 분배)", {CityEff(MetricField::Scarcity, -200),
		  DriveEff(DriveField::InterpersonalTrust, 200), DriveEff(DriveField::SocialSafety, 100)}},
		 {"무시", {DriveEff(DriveField::SocialSafety, -400),
		  DriveEff(DriveField::InterpersonalTrust, -200)}}}
	});

	// [30] 협박 - 높은 공격성 + 높은 통제감 + 적대적
	definitions.push_back({"evt_intimidation", "협박",
		"힘으로 다른 사람을 위협하고 복종을 강요하는 사람이 나타났습니다.",
		EventCategory::Social, false, 0.12f, 5, 2, "select_high_aggression", "",
		{Cond(ConditionTarget::HumanAggressiveness, CompareOp::GreaterEq, 70, 1),
		 Cond(ConditionTarget::HumanSenseOfControl, CompareOp::GreaterEq, 7000, 1),
		 Cond(ConditionTarget::HumanArousal, CompareOp::GreaterEq, 3, 1)},
		{{"위협합니다.", {DriveEff(DriveField::SocialSafety, -600),
		  DriveEff(DriveField::InterpersonalTrust, -400), CityEff(MetricField::Mood, -150)}}}
	});

	// [31] 영역 다툼 - 높은 고집 + 높은 공격성 + 높은 스트레스
	definitions.push_back({"evt_territory_dispute", "영역 다툼",
		"한정된 공간을 두고 사람들이 다투고 있습니다.",
		EventCategory::Social, false, 0.15f, 5, 4, "select_hostile", "",
		{Cond(ConditionTarget::HumanRigidity, CompareOp::GreaterEq, 55, 2),
		 Cond(ConditionTarget::HumanAggressiveness, CompareOp::GreaterEq, 55, 2),
		 Cond(ConditionTarget::CityScarcity, CompareOp::GreaterEq, 5000)},
		{{"영역 다툼이 벌어집니다.", {DriveEff(DriveField::StressLoad, 600),
		  DriveEff(DriveField::InterpersonalTrust, -400), DriveEff(DriveField::SocialSafety, -300)}}}
	});

	// [32] 복수 행위 - 적대적 + 고집 + 높은 스트레스
	definitions.push_back({"evt_revenge", "복수",
		"과거의 원한을 품은 누군가가 복수를 감행합니다.",
		EventCategory::Social, false, 0.08f, 10, 2, "select_hostile", "",
		{Cond(ConditionTarget::HumanArousal, CompareOp::GreaterEq, 3, 1),
		 Cond(ConditionTarget::HumanRigidity, CompareOp::GreaterEq, 65, 1),
		 Cond(ConditionTarget::HumanInterpersonalTrust, CompareOp::Less, 2000, 1)},
		{{"복수합니다.", {DriveEff(DriveField::StressLoad, 1000),
		  DriveEff(DriveField::InterpersonalTrust, -600), CityEff(MetricField::Mood, -200), KillEff(1)}}}
	});

	// [33] 스토킹 - 높은 의존 + 과민 + 높은 감정각성
	definitions.push_back({"evt_stalking", "스토킹",
		"집착이 심한 누군가가 특정인을 따라다닙니다.",
		EventCategory::Social, false, 0.10f, 7, 2, "select_high_stress", "",
		{Cond(ConditionTarget::HumanDependency, CompareOp::GreaterEq, 70, 1),
		 Cond(ConditionTarget::HumanArousal, CompareOp::GreaterEq, 2, 1),
		 Cond(ConditionTarget::HumanEmotionalArousal, CompareOp::GreaterEq, 7000, 1)},
		{{"추적합니다.", {DriveEff(DriveField::SocialSafety, -500),
		  DriveEff(DriveField::StressLoad, 600), DriveEff(DriveField::InterpersonalTrust, -400)}}}
	});

	// ========================
	//   대인 협력 이벤트 (8)
	// ========================

	// [34] 상호 부조 - 협력적 + 높은 대인신뢰 + 높은 동기
	definitions.push_back({"evt_mutual_aid", "상호 부조",
		"뜻이 맞는 사람들이 서로 돕기 시작합니다.",
		EventCategory::Social, false, 0.18f, 3, 5, "select_cooperative", "",
		{Cond(ConditionTarget::HumanSocial, CompareOp::Equal, 1, 3),
		 Cond(ConditionTarget::HumanInterpersonalTrust, CompareOp::GreaterEq, 6000, 3),
		 Cond(ConditionTarget::HumanMotivation, CompareOp::GreaterEq, 5000, 3)},
		{{"서로 돕습니다.", {DriveEff(DriveField::InterpersonalTrust, 400),
		  DriveEff(DriveField::SocialSafety, 300), DriveEff(DriveField::StressLoad, -300),
		  CityEff(MetricField::Mood, 100)}}}
	});

	// [35] 멘토링 - 높은 인지능력 + 협력적
	definitions.push_back({"evt_mentorship", "멘토링",
		"경험 많은 사람이 다른 이에게 지식과 경험을 나눠줍니다.",
		EventCategory::Social, false, 0.12f, 5, 2, "select_cooperative", "",
		{Cond(ConditionTarget::HumanCognitiveCapacity, CompareOp::GreaterEq, 7000, 1),
		 Cond(ConditionTarget::HumanSocial, CompareOp::Equal, 1, 1),
		 Cond(ConditionTarget::HumanInterpersonalTrust, CompareOp::GreaterEq, 6000, 1)},
		{{"가르침을 나눕니다.", {DriveEff(DriveField::CognitiveCapacity, 400),
		  DriveEff(DriveField::InterpersonalTrust, 300), DriveEff(DriveField::Motivation, 200)}}}
	});

	// [36] 물물교환 - 높은 계획 + 협력적 + 높은 대인신뢰
	definitions.push_back({"evt_trade", "물물교환",
		"계획적인 사람들이 서로 필요한 물자를 교환합니다.",
		EventCategory::Social, false, 0.15f, 3, 3, "select_cooperative", "",
		{Cond(ConditionTarget::HumanPlanning, CompareOp::GreaterEq, 60, 2),
		 Cond(ConditionTarget::HumanSocial, CompareOp::Equal, 1, 2),
		 Cond(ConditionTarget::HumanInterpersonalTrust, CompareOp::GreaterEq, 5000, 2)},
		{{"교환합니다.", {CityEff(MetricField::Scarcity, -150),
		  DriveEff(DriveField::InterpersonalTrust, 300), DriveEff(DriveField::Motivation, 200)}}}
	});

	// [37] 공동 식사 - 협력적 + 낮은 결핍
	definitions.push_back({"evt_community_meal", "공동 식사",
		"사람들이 모여 함께 식사를 나눕니다.",
		EventCategory::Social, false, 0.15f, 3, 6, "select_cooperative", "",
		{Cond(ConditionTarget::HumanSocial, CompareOp::Equal, 1, 3),
		 Cond(ConditionTarget::CityScarcity, CompareOp::Less, 5000)},
		{{"함께 식사합니다.", {DriveEff(DriveField::InterpersonalTrust, 400),
		  DriveEff(DriveField::StressLoad, -300), DriveEff(DriveField::SocialSafety, 200),
		  CityEff(MetricField::Mood, 100)}}}
	});

	// [38] 축하 행사 - 차분 + 협력적 + 높은 동기 + 높은 도시분위기
	definitions.push_back({"evt_celebration", "축하 행사",
		"기쁜 일을 맞아 사람들이 함께 축하합니다.",
		EventCategory::Social, false, 0.10f, 7, 8, "select_cooperative", "",
		{Cond(ConditionTarget::HumanArousal, CompareOp::Equal, 0, 5),
		 Cond(ConditionTarget::HumanSocial, CompareOp::Equal, 1, 5),
		 Cond(ConditionTarget::CityMood, CompareOp::GreaterEq, 6000)},
		{{"축하합니다.", {DriveEff(DriveField::StressLoad, -500),
		  DriveEff(DriveField::Motivation, 400), DriveEff(DriveField::InterpersonalTrust, 300),
		  CityEff(MetricField::Mood, 200)}}}
	});

	// [39] 학습 모임 - 높은 인지 + 높은 계획 + 협력적
	definitions.push_back({"evt_study_group", "학습 모임",
		"지적 호기심이 강한 사람들이 모여 배움을 나눕니다.",
		EventCategory::Social, false, 0.10f, 5, 4, "select_cooperative", "",
		{Cond(ConditionTarget::HumanCognitiveCapacity, CompareOp::GreaterEq, 6000, 3),
		 Cond(ConditionTarget::HumanPlanning, CompareOp::GreaterEq, 55, 3),
		 Cond(ConditionTarget::HumanSocial, CompareOp::Equal, 1, 3)},
		{{"함께 배웁니다.", {DriveEff(DriveField::CognitiveCapacity, 500),
		  DriveEff(DriveField::Motivation, 300), DriveEff(DriveField::InterpersonalTrust, 200)}}}
	});

	// [40] 갈등 중재 - 높은 이성 + 협력적 + 높은 대인신뢰
	definitions.push_back({"evt_mediation", "갈등 중재",
		"이성적인 중재자가 나서서 갈등을 해소하려 합니다.",
		EventCategory::Social, false, 0.12f, 5, 3, "select_cooperative", "",
		{Cond(ConditionTarget::HumanRationality, CompareOp::GreaterEq, 70, 1),
		 Cond(ConditionTarget::HumanSocial, CompareOp::Equal, 1, 1),
		 Cond(ConditionTarget::HumanInterpersonalTrust, CompareOp::GreaterEq, 7000, 1)},
		{{"중재합니다.", {DriveEff(DriveField::InterpersonalTrust, 400),
		  DriveEff(DriveField::StressLoad, -300), DriveEff(DriveField::SocialSafety, 300),
		  CityEff(MetricField::Mood, 100)}}}
	});

	// [41] 예술 활동 - 높은 감정민감도 + 높은 인지 + 높은 동기
	definitions.push_back({"evt_art_activity", "예술 활동",
		"감성이 풍부한 사람들이 모여 예술 활동을 합니다.",
		EventCategory::Social, false, 0.10f, 5, 3, "select_high_motivation", "",
		{Cond(ConditionTarget::HumanEmotionalSensitivity, CompareOp::GreaterEq, 65, 2),
		 Cond(ConditionTarget::HumanCognitiveCapacity, CompareOp::GreaterEq, 5000, 2),
		 Cond(ConditionTarget::HumanMotivation, CompareOp::GreaterEq, 6000, 2)},
		{{"예술을 합니다.", {DriveEff(DriveField::StressLoad, -400),
		  DriveEff(DriveField::EmotionalArousal, -300), DriveEff(DriveField::Motivation, 300),
		  CityEff(MetricField::Mood, 150)}}}
	});

	// ========================
	//   집단 형성 이벤트 (5)
	// ========================

	// [42] 갱단 형성 - 적대적 + 높은 공격성 + 낮은 사회안전감
	definitions.push_back({"evt_gang_formation", "갱단 형성",
		"불안한 환경에서 공격적인 사람들이 무리를 지어 세력을 형성합니다.",
		EventCategory::Social, false, 0.08f, 14, 5, "select_high_aggression", "",
		{Cond(ConditionTarget::HumanArousal, CompareOp::GreaterEq, 2, 3),
		 Cond(ConditionTarget::HumanAggressiveness, CompareOp::GreaterEq, 65, 3),
		 Cond(ConditionTarget::HumanSocialSafety, CompareOp::Less, 3000, 3)},
		{{"갱단이 형성됩니다.", {DriveEff(DriveField::SocialSafety, -500),
		  DriveEff(DriveField::InterpersonalTrust, -400), CityEff(MetricField::Mood, -300),
		  CityEff(MetricField::Scarcity, 200)}}}
	});

	// [43] 종교 모임 - 높은 의존 + 높은 고집 + 협력적
	definitions.push_back({"evt_religious_gathering", "종교 모임",
		"신앙을 가진 사람들이 모여 의지하고 기도합니다.",
		EventCategory::Social, false, 0.12f, 5, 6, "select_cooperative", "",
		{Cond(ConditionTarget::HumanDependency, CompareOp::GreaterEq, 60, 3),
		 Cond(ConditionTarget::HumanRigidity, CompareOp::GreaterEq, 55, 3)},
		{{"모여 기도합니다.", {DriveEff(DriveField::StressLoad, -400),
		  DriveEff(DriveField::SenseOfControl, 300), DriveEff(DriveField::SocialSafety, 200)}}}
	});

	// [44] 지지 그룹 - 높은 감정민감도 + 협력적 + 높은 대인신뢰
	definitions.push_back({"evt_support_group", "지지 그룹",
		"감정적으로 힘든 시기를 함께 나누는 사람들이 모입니다.",
		EventCategory::Social, false, 0.10f, 7, 4, "select_cooperative", "",
		{Cond(ConditionTarget::HumanEmotionalSensitivity, CompareOp::GreaterEq, 60, 3),
		 Cond(ConditionTarget::HumanSocial, CompareOp::Equal, 1, 3),
		 Cond(ConditionTarget::HumanInterpersonalTrust, CompareOp::GreaterEq, 5000, 3)},
		{{"서로 위로합니다.", {DriveEff(DriveField::StressLoad, -500),
		  DriveEff(DriveField::EmotionalArousal, -400), DriveEff(DriveField::InterpersonalTrust, 400),
		  DriveEff(DriveField::SocialSafety, 300)}}}
	});

	// [45] 자경단 결성 - 높은 공격성 + 자율 + 낮은 사회안전감
	definitions.push_back({"evt_vigilante", "자경단 결성",
		"치안이 불안해지자 시민들이 스스로 자경단을 조직합니다.",
		EventCategory::Social, false, 0.08f, 10, 5, "select_high_aggression", "",
		{Cond(ConditionTarget::HumanAggressiveness, CompareOp::GreaterEq, 55, 3),
		 Cond(ConditionTarget::HumanControl, CompareOp::Equal, 0, 3),
		 Cond(ConditionTarget::HumanSocialSafety, CompareOp::Less, 3000, 5)},
		{{"자경단이 결성됩니다.", {DriveEff(DriveField::SocialSafety, 300),
		  DriveEff(DriveField::SenseOfControl, 400), CityEff(MetricField::Mood, -100),
		  DriveEff(DriveField::InterpersonalTrust, -200)}}}
	});

	// [46] 암시장 (플레이어) - 높은 결핍 + 낮은 사회안전감 + 높은 계획
	definitions.push_back({"evt_black_market", "암시장",
		"자원이 부족해지자 비공식적인 거래 시장이 생겨났습니다.",
		EventCategory::Social, true, 0.10f, 14, 5, "select_random", "",
		{Cond(ConditionTarget::CityScarcity, CompareOp::GreaterEq, 6000),
		 Cond(ConditionTarget::HumanSocialSafety, CompareOp::Less, 4000, 5),
		 Cond(ConditionTarget::HumanPlanning, CompareOp::GreaterEq, 60, 3)},
		{{"단속 및 폐쇄", {CityEff(MetricField::Scarcity, 300),
		  DriveEff(DriveField::SocialSafety, 200), DriveEff(DriveField::InterpersonalTrust, -200)}},
		 {"묵인하되 감시", {CityEff(MetricField::Scarcity, -200),
		  DriveEff(DriveField::SocialSafety, -100)}},
		 {"공식 시장으로 전환", {CityEff(MetricField::Scarcity, -400),
		  CityEff(MetricField::Activity, 300), DriveEff(DriveField::InterpersonalTrust, 200)}}}
	});

	// ========================
	//   도시 전체 이벤트 (12)
	// ========================

	// [47] 폭동 (플레이어) - 낮은 도시분위기 + 많은 적대적
	definitions.push_back({"evt_riot", "폭동",
		"분노한 시민들이 거리로 쏟아져 나와 폭동이 일어났습니다.",
		EventCategory::CityWide, true, 0.25f, 14, 20, "select_hostile", "",
		{Cond(ConditionTarget::CityMood, CompareOp::Less, 3000),
		 Cond(ConditionTarget::HumanArousal, CompareOp::GreaterEq, 3, 10)},
		{{"강경 진압", {CityEff(MetricField::Mood, -500),
		  DriveEff(DriveField::StressLoad, 800), KillEff(2), DriveEff(DriveField::SocialSafety, 300)}},
		 {"대화와 협상", {CityEff(MetricField::Mood, 200),
		  DriveEff(DriveField::InterpersonalTrust, 300), DriveEff(DriveField::StressLoad, -200)}},
		 {"방관", {CityEff(MetricField::Mood, -300),
		  CityEff(MetricField::Activity, -500), CityEff(MetricField::Scarcity, 400)}}}
	});

	// [48] 시위 (플레이어) - 낮은 사회안전감 + 높은 동기
	definitions.push_back({"evt_protest", "시위",
		"불만이 쌓인 시민들이 조직적으로 시위를 벌입니다.",
		EventCategory::CityWide, true, 0.20f, 10, 15, "select_high_motivation", "",
		{Cond(ConditionTarget::HumanSocialSafety, CompareOp::Less, 3000, 15),
		 Cond(ConditionTarget::HumanMotivation, CompareOp::GreaterEq, 6000, 10),
		 Cond(ConditionTarget::HumanArousal, CompareOp::GreaterEq, 1, 15)},
		{{"요구 수용", {CityEff(MetricField::Mood, 400),
		  DriveEff(DriveField::InterpersonalTrust, 300), CityEff(MetricField::Scarcity, 200)}},
		 {"강제 해산", {CityEff(MetricField::Mood, -400),
		  DriveEff(DriveField::StressLoad, 600), DriveEff(DriveField::SocialSafety, -200)}},
		 {"대화 채널 개설", {CityEff(MetricField::Mood, 100),
		  DriveEff(DriveField::InterpersonalTrust, 200), DriveEff(DriveField::StressLoad, -100)}}}
	});

	// [49] 집단 공포 - 낮은 분위기 + 낮은 사회안전감
	definitions.push_back({"evt_mass_panic", "집단 공포",
		"공포가 전염되어 사람들이 이성을 잃고 도망칩니다.",
		EventCategory::CityWide, false, 0.15f, 10, 25, "select_high_stress", "",
		{Cond(ConditionTarget::CityMood, CompareOp::Less, 2500),
		 Cond(ConditionTarget::HumanSocialSafety, CompareOp::Less, 2500, 20),
		 Cond(ConditionTarget::HumanArousal, CompareOp::GreaterEq, 1, 20)},
		{{"공포가 퍼집니다.", {DriveEff(DriveField::StressLoad, 1000),
		  DriveEff(DriveField::SocialSafety, -500), DriveEff(DriveField::InterpersonalTrust, -300),
		  CityEff(MetricField::Mood, -400), KillEff(1)}}}
	});

	// [50] 문화 축제 - 높은 분위기 + 높은 활동 + 많은 협력적
	definitions.push_back({"evt_festival", "문화 축제",
		"활기찬 도시에서 자발적인 축제가 열립니다.",
		EventCategory::CityWide, false, 0.12f, 14, 20, "select_cooperative", "",
		{Cond(ConditionTarget::CityMood, CompareOp::GreaterEq, 6000),
		 Cond(ConditionTarget::CityActivity, CompareOp::GreaterEq, 6000),
		 Cond(ConditionTarget::HumanSocial, CompareOp::Equal, 1, 15)},
		{{"축제가 열립니다.", {DriveEff(DriveField::StressLoad, -600),
		  DriveEff(DriveField::Motivation, 500), DriveEff(DriveField::InterpersonalTrust, 400),
		  CityEff(MetricField::Mood, 300)}}}
	});

	// [51] 경제 호황
	definitions.push_back({"evt_economic_boom", "경제 호황",
		"활발한 생산과 교역으로 도시가 번영합니다.",
		EventCategory::CityWide, false, 0.08f, 14, 0, "", "",
		{Cond(ConditionTarget::CityActivity, CompareOp::GreaterEq, 7000),
		 Cond(ConditionTarget::CityScarcity, CompareOp::Less, 3000),
		 Cond(ConditionTarget::CityMood, CompareOp::GreaterEq, 6000)},
		{{"경제가 번영합니다.", {CityEff(MetricField::Activity, 500),
		  CityEff(MetricField::Scarcity, -500), CityEff(MetricField::Mood, 300)}}}
	});

	// [52] 경제 붕괴
	definitions.push_back({"evt_economic_collapse", "경제 붕괴",
		"생산이 멈추고 물자가 고갈되어 경제가 무너집니다.",
		EventCategory::CityWide, false, 0.10f, 14, 15, "select_random", "",
		{Cond(ConditionTarget::CityActivity, CompareOp::Less, 2500),
		 Cond(ConditionTarget::CityScarcity, CompareOp::GreaterEq, 7000),
		 Cond(ConditionTarget::CityMood, CompareOp::Less, 3000)},
		{{"경제가 무너집니다.", {CityEff(MetricField::Activity, -500),
		  CityEff(MetricField::Scarcity, 500), DriveEff(DriveField::StressLoad, 800),
		  DriveEff(DriveField::Motivation, -600)}}}
	});

	// [53] 전염병 (플레이어) - 많은 소진 + 높은 결핍
	definitions.push_back({"evt_epidemic", "전염병",
		"질병이 빠르게 퍼져 도시 전체가 위험에 빠졌습니다.",
		EventCategory::CityWide, true, 0.08f, 21, 20, "select_exhausted", "",
		{Cond(ConditionTarget::HumanEnergy, CompareOp::GreaterEq, 1, 20),
		 Cond(ConditionTarget::CityScarcity, CompareOp::GreaterEq, 6000)},
		{{"전면 격리", {CityEff(MetricField::Activity, -800),
		  DriveEff(DriveField::Fatigue, 500), DriveEff(DriveField::SocialSafety, 200)}},
		 {"의료 자원 투입", {CityEff(MetricField::Scarcity, 500),
		  DriveEff(DriveField::Fatigue, -300), DriveEff(DriveField::InterpersonalTrust, 200)}},
		 {"자연 면역에 의존", {KillEff(5), CityEff(MetricField::Mood, -500),
		  DriveEff(DriveField::StressLoad, 1000)}}}
	});

	// [54] 기근 (플레이어) - 매우 높은 결핍
	definitions.push_back({"evt_famine", "기근",
		"식량이 바닥났습니다. 사람들이 굶주리고 있습니다.",
		EventCategory::CityWide, true, 0.15f, 14, 15, "select_exhausted", "",
		{Cond(ConditionTarget::CityScarcity, CompareOp::GreaterEq, 8000)},
		{{"비상 식량 배급", {CityEff(MetricField::Scarcity, -300),
		  DriveEff(DriveField::Motivation, 200), DriveEff(DriveField::SocialSafety, 100)}},
		 {"외부 지원 요청", {ImmigrantEff(5), CityEff(MetricField::Scarcity, -200)}},
		 {"자율 배분", {DriveEff(DriveField::InterpersonalTrust, -300),
		  CityEff(MetricField::Scarcity, 200), KillEff(1)}}}
	});

	// [55] 집단 이주 - 매우 높은 스트레스 + 매우 낮은 분위기
	definitions.push_back({"evt_mass_exodus", "집단 이주",
		"더 이상 견딜 수 없는 사람들이 도시를 떠나기 시작합니다.",
		EventCategory::CityWide, false, 0.10f, 14, 10, "select_high_stress", "",
		{Cond(ConditionTarget::HumanStressLoad, CompareOp::GreaterEq, 8000, 15),
		 Cond(ConditionTarget::CityMood, CompareOp::Less, 2000),
		 Cond(ConditionTarget::HumanPlanning, CompareOp::GreaterEq, 50, 10)},
		{{"사람들이 떠납니다.", {KillEff(5), CityEff(MetricField::Activity, -300),
		  CityEff(MetricField::Mood, -200)}}}
	});

	// [56] 이민 유입 - 낮은 결핍 + 높은 분위기
	definitions.push_back({"evt_immigration", "이민 유입",
		"안정된 도시를 찾아 새로운 사람들이 들어옵니다.",
		EventCategory::CityWide, false, 0.10f, 14, 0, "", "",
		{Cond(ConditionTarget::CityScarcity, CompareOp::Less, 3000),
		 Cond(ConditionTarget::CityMood, CompareOp::GreaterEq, 6000)},
		{{"새 이주민이 도착합니다.", {ImmigrantEff(10), CityEff(MetricField::Activity, 200),
		  CityEff(MetricField::Scarcity, 100)}}}
	});

	// [57] 혁명 (플레이어) - 많은 적대적 + 높은 동기 + 낮은 분위기
	definitions.push_back({"evt_revolution", "혁명",
		"기존 질서에 대한 불만이 극에 달해 혁명의 불꽃이 타오릅니다.",
		EventCategory::CityWide, true, 0.05f, 30, 30, "select_hostile", "",
		{Cond(ConditionTarget::CityMood, CompareOp::Less, 2000),
		 Cond(ConditionTarget::HumanArousal, CompareOp::GreaterEq, 2, 20),
		 Cond(ConditionTarget::HumanMotivation, CompareOp::GreaterEq, 6000, 15)},
		{{"개혁 수용", {CityEff(MetricField::Mood, 500),
		  DriveEff(DriveField::SocialSafety, 300), CityEff(MetricField::Activity, -200)}},
		 {"강력 진압", {KillEff(5), CityEff(MetricField::Mood, -800),
		  DriveEff(DriveField::SocialSafety, 400), DriveEff(DriveField::InterpersonalTrust, -500)}},
		 {"협상 시도", {CityEff(MetricField::Mood, 200),
		  DriveEff(DriveField::InterpersonalTrust, 300), DriveEff(DriveField::StressLoad, -200)}}}
	});

	// [58] 통행금지 - 낮은 사회안전감 + 낮은 분위기
	definitions.push_back({"evt_curfew", "통행금지",
		"치안 불안으로 야간 통행이 금지됩니다.",
		EventCategory::CityWide, false, 0.12f, 10, 10, "select_random", "",
		{Cond(ConditionTarget::HumanSocialSafety, CompareOp::Less, 2500, 20),
		 Cond(ConditionTarget::CityMood, CompareOp::Less, 3000)},
		{{"통행이 금지됩니다.", {CityEff(MetricField::Activity, -400),
		  DriveEff(DriveField::SocialSafety, 200), DriveEff(DriveField::SenseOfControl, -300),
		  CityEff(MetricField::Mood, -200)}}}
	});
}
