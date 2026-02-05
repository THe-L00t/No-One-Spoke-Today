#include "Event.h"
#include "City.h"
#include "Human.h"
#include <sstream>
#include <algorithm>
#include <cctype>

// 디버그 로그를 파일로 출력
static std::ofstream& GetDebugLog() {
	static std::ofstream debugLog("data/debug.log", std::ios::trunc);
	return debugLog;
}
#define EVENT_LOG(msg) { GetDebugLog() << msg << std::endl; GetDebugLog().flush(); }


// ========== 유틸리티 함수 ==========
namespace {
	// 문자열 트림
	std::string Trim(const std::string& s) {
		size_t start = s.find_first_not_of(" \t\r\n");
		if (start == std::string::npos) return "";
		size_t end = s.find_last_not_of(" \t\r\n");
		return s.substr(start, end - start + 1);
	}

	// 문자열 소문자 변환
	std::string ToLower(const std::string& s) {
		std::string result = s;
		std::transform(result.begin(), result.end(), result.begin(),
			[](unsigned char c) { return std::tolower(c); });
		return result;
	}

	// 16진수 문자열 → uint16_t
	uint16_t ParseHex16(const std::string& s) {
		return static_cast<uint16_t>(std::stoul(s, nullptr, 0));
	}

	// 16진수 문자열 → uint64_t
	uint64_t ParseHex64(const std::string& s) {
		return std::stoull(s, nullptr, 0);
	}

	// 카테고리 문자열 파싱
	EventCategory ParseCategory(const std::string& s) {
		std::string lower = ToLower(s);
		if (lower == "environment") return EventCategory::Environment;
		if (lower == "personal") return EventCategory::Personal;
		if (lower == "social") return EventCategory::Social;
		if (lower == "interpersonal") return EventCategory::Interpersonal;
		if (lower == "citywide") return EventCategory::CityWide;
		return EventCategory::Environment;
	}

	// EffectScope 문자열 파싱
	EffectScope ParseEffectScope(const std::string& s) {
		std::string lower = ToLower(s);
		if (lower == "triggered") return EffectScope::Triggered;
		if (lower == "allhumans") return EffectScope::AllHumans;
		if (lower == "city") return EffectScope::City;
		if (lower == "region") return EffectScope::Region;
		if (lower == "custom") return EffectScope::Custom;
		return EffectScope::Triggered;
	}

	// EffectType 문자열 파싱
	EffectType ParseEffectType(const std::string& s) {
		std::string lower = ToLower(s);
		if (lower == "modifydrive") return EffectType::ModifyDrive;
		if (lower == "modifycitymetric") return EffectType::ModifyCityMetric;
		if (lower == "modifymentalstate") return EffectType::ModifyMentalState;
		if (lower == "kill") return EffectType::Kill;
		if (lower == "addimmigrant") return EffectType::AddImmigrant;
		if (lower == "custom") return EffectType::Custom;
		return EffectType::ModifyDrive;
	}

	// DriveField 문자열 파싱
	int ParseDriveField(const std::string& s) {
		std::string lower = ToLower(s);
		if (lower == "stressload") return static_cast<int>(DriveField::StressLoad);
		if (lower == "emotionalarousal") return static_cast<int>(DriveField::EmotionalArousal);
		if (lower == "fatigue") return static_cast<int>(DriveField::Fatigue);
		if (lower == "cognitivecapacity") return static_cast<int>(DriveField::CognitiveCapacity);
		if (lower == "interpersonaltrust") return static_cast<int>(DriveField::InterpersonalTrust);
		if (lower == "socialsafety") return static_cast<int>(DriveField::SocialSafety);
		if (lower == "senseofcontrol") return static_cast<int>(DriveField::SenseOfControl);
		if (lower == "motivation") return static_cast<int>(DriveField::Motivation);
		return 0;
	}

	// MetricField 문자열 파싱
	int ParseMetricField(const std::string& s) {
		std::string lower = ToLower(s);
		if (lower == "mood") return static_cast<int>(MetricField::Mood);
		if (lower == "activity") return static_cast<int>(MetricField::Activity);
		if (lower == "scarcity") return static_cast<int>(MetricField::Scarcity);
		return 0;
	}

	// MentalField 문자열 파싱
	int ParseMentalField(const std::string& s) {
		std::string lower = ToLower(s);
		if (lower == "arousal") return static_cast<int>(MentalField::Arousal);
		if (lower == "social") return static_cast<int>(MentalField::Social);
		if (lower == "energy") return static_cast<int>(MentalField::Energy);
		if (lower == "control") return static_cast<int>(MentalField::Control);
		return 0;
	}

	// UnresolvedBehavior 문자열 파싱
	UnresolvedBehavior ParseUnresolvedBehavior(const std::string& s) {
		std::string lower = ToLower(s);
		if (lower == "autoresolve") return UnresolvedBehavior::AutoResolve;
		if (lower == "expire") return UnresolvedBehavior::Expire;
		if (lower == "carryover") return UnresolvedBehavior::CarryOver;
		return UnresolvedBehavior::AutoResolve;
	}

	// 효과 문자열 파싱: "EffectType, Field, Delta, Scope"
	EffectData ParseEffect(const std::string& effectStr) {
		EffectData eff;
		std::stringstream ss(effectStr);
		std::string token;
		std::vector<std::string> tokens;

		while (std::getline(ss, token, ',')) {
			tokens.push_back(Trim(token));
		}

		if (tokens.size() >= 3) {
			eff.type = ParseEffectType(tokens[0]);

			// 필드 파싱 (타입에 따라 다름)
			if (eff.type == EffectType::ModifyDrive) {
				eff.targetField = ParseDriveField(tokens[1]);
			}
			else if (eff.type == EffectType::ModifyCityMetric) {
				eff.targetField = ParseMetricField(tokens[1]);
			}
			else if (eff.type == EffectType::ModifyMentalState) {
				eff.targetField = ParseMentalField(tokens[1]);
			}

			eff.delta = std::stoi(tokens[2]);

			// 스코프 (선택적)
			if (tokens.size() >= 4) {
				eff.scope = ParseEffectScope(tokens[3]);
			}
		}

		return eff;
	}

	// 바이너리 I/O 헬퍼
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
}


// ========== 인코딩 함수 ==========
CityCode EncodeCityState(const CityMetrics& metrics) {
	using namespace CityBit;

	uint16_t code = 0;

	int moodLevel = ToLevel5(metrics.mood);
	int activityLevel = ToLevel5(metrics.activity);
	int scarcityLevel = ToLevel5(metrics.scarcity);

	code |= (moodLevel & 0b111) << MOOD_SHIFT;
	code |= (activityLevel & 0b111) << ACTIVITY_SHIFT;
	code |= (scarcityLevel & 0b111) << SCARCITY_SHIFT;

	return CityCode(code);
}

HumanCode EncodeHumanState(const Human& human) {
	using namespace HumanBit;

	uint64_t code = 0;

	// MentalState (2비트씩)
	code |= (static_cast<uint64_t>(human.GetArousal()) & 0b11) << AROUSAL_SHIFT;
	code |= (static_cast<uint64_t>(human.GetSocial()) & 0b11) << SOCIAL_SHIFT;
	code |= (static_cast<uint64_t>(human.GetEnergy()) & 0b11) << ENERGY_SHIFT;
	code |= (static_cast<uint64_t>(human.GetControl()) & 0b11) << CONTROL_SHIFT;

	// Drives (3비트씩, 5단계)
	code |= (static_cast<uint64_t>(ToLevel5(human.GetStressLoad(), 10000)) & 0b111) << STRESS_SHIFT;
	code |= (static_cast<uint64_t>(ToLevel5(human.GetEmotionalArousal(), 10000)) & 0b111) << EMO_AROUSAL_SHIFT;
	code |= (static_cast<uint64_t>(ToLevel5(human.GetFatigue(), 10000)) & 0b111) << FATIGUE_SHIFT;
	code |= (static_cast<uint64_t>(ToLevel5(human.GetCognitiveCapacity(), 10000)) & 0b111) << COGNITIVE_SHIFT;
	code |= (static_cast<uint64_t>(ToLevel5(human.GetInterpersonalTrust(), 10000)) & 0b111) << TRUST_SHIFT;
	code |= (static_cast<uint64_t>(ToLevel5(human.GetSocialSafety(), 10000)) & 0b111) << SAFETY_SHIFT;
	code |= (static_cast<uint64_t>(ToLevel5(human.GetSenseOfControl(), 10000)) & 0b111) << SENSE_CTRL_SHIFT;
	code |= (static_cast<uint64_t>(ToLevel5(human.GetMotivation(), 10000)) & 0b111) << MOTIVATION_SHIFT;

	// Traits (3비트씩, 5단계)
	code |= (static_cast<uint64_t>(ToLevel5(human.GetRationality(), 100)) & 0b111) << RATIONALITY_SHIFT;
	code |= (static_cast<uint64_t>(ToLevel5(human.GetAggressiveness(), 100)) & 0b111) << AGGRESSION_SHIFT;
	code |= (static_cast<uint64_t>(ToLevel5(human.GetPlanning(), 100)) & 0b111) << PLANNING_SHIFT;
	code |= (static_cast<uint64_t>(ToLevel5(human.GetDependency(), 100)) & 0b111) << DEPENDENCY_SHIFT;
	code |= (static_cast<uint64_t>(ToLevel5(human.GetRigidity(), 100)) & 0b111) << RIGIDITY_SHIFT;
	code |= (static_cast<uint64_t>(ToLevel5(human.GetEmotionalSensitivity(), 100)) & 0b111) << EMO_SENS_SHIFT;

	// Region
	code |= (static_cast<uint64_t>(human.GetRegion()) & 0b111) << REGION_SHIFT;

	return HumanCode(code);
}


// ========== EventManager 생성자 ==========
EventManager::EventManager()
	: rng(std::random_device{}())
{
	RegisterCustomEffects();
}


// ========== 텍스트 파일에서 이벤트 정의 로드 ==========
void EventManager::LoadEventDefsFromText(const std::string& filepath) {
	std::ifstream in(filepath);
	if (!in) {
		std::cerr << "이벤트 파일을 열 수 없습니다: " << filepath << std::endl;
		return;
	}

	definitions.clear();

	std::string line;
	EventDef currentDef;
	bool inEvent = false;
	int currentChoiceNum = 0;

	while (std::getline(in, line)) {
		line = Trim(line);

		// 빈 줄이나 주석 무시
		if (line.empty() || line[0] == '#') continue;

		// 이벤트 시작
		if (line == "[EVENT_START]") {
			inEvent = true;
			currentDef = EventDef();
			currentChoiceNum = 0;
			continue;
		}

		// 이벤트 종료
		if (line == "[EVENT_END]") {
			if (inEvent && !currentDef.id.empty()) {
				definitions.push_back(currentDef);
			}
			inEvent = false;
			continue;
		}

		if (!inEvent) continue;

		// key = value 파싱
		size_t eqPos = line.find('=');
		if (eqPos == std::string::npos) continue;

		std::string key = Trim(line.substr(0, eqPos));
		std::string value = Trim(line.substr(eqPos + 1));

		// 기본 정보
		if (key == "id") currentDef.id = value;
		else if (key == "name") currentDef.name = value;
		else if (key == "description") currentDef.description = value;
		else if (key == "category") currentDef.category = ParseCategory(value);

		// 트리거
		else if (key == "trigger_type") {
			currentDef.trigger.isRandom = (ToLower(value) == "random");
		}
		else if (key == "city_mask") {
			currentDef.trigger.cityMask = ParseHex16(value);
			currentDef.trigger.checkCity = (currentDef.trigger.cityMask != 0);
		}
		else if (key == "city_min") currentDef.trigger.cityMin = ParseHex16(value);
		else if (key == "city_max") currentDef.trigger.cityMax = ParseHex16(value);
		else if (key == "human_mask") {
			currentDef.trigger.humanMask = ParseHex64(value);
			currentDef.trigger.checkHuman = (currentDef.trigger.humanMask != 0);
		}
		else if (key == "human_min") currentDef.trigger.humanMin = ParseHex64(value);
		else if (key == "human_max") currentDef.trigger.humanMax = ParseHex64(value);
		else if (key == "min_human_count") currentDef.trigger.minHumanCount = std::stoi(value);
		else if (key == "region_id") currentDef.trigger.regionId = std::stoi(value);

		// 쿨타임
		else if (key == "cooldown_min") currentDef.cooldownMin = std::stoi(value);
		else if (key == "cooldown_max") currentDef.cooldownMax = std::stoi(value);

		// 효과 범위
		else if (key == "effect_scope") currentDef.effectScope = ParseEffectScope(value);
		else if (key == "effect_region_id") currentDef.effectRegionId = std::stoi(value);
		else if (key == "custom_effect_id") currentDef.customEffectId = value;

		// 미방문 시 처리 방식
		else if (key == "unresolved_behavior") currentDef.unresolvedBehavior = ParseUnresolvedBehavior(value);

		// 즉시 효과
		else if (key == "immediate_effect") {
			currentDef.immediateEffects.push_back(ParseEffect(value));
		}

		// 플레이어 선택 필요
		else if (key == "requires_player") {
			currentDef.requiresPlayer = (ToLower(value) == "true" || value == "1");
		}

		// 선택지 파싱
		else if (key.find("choice_") == 0 && key.find("_text") != std::string::npos) {
			// choice_N_text
			int choiceNum = std::stoi(key.substr(7, key.find("_text") - 7));
			while (static_cast<int>(currentDef.choices.size()) < choiceNum) {
				currentDef.choices.push_back(Choice());
			}
			currentDef.choices[choiceNum - 1].text = value;
		}
		else if (key.find("choice_") == 0 && key.find("_effect") != std::string::npos) {
			// choice_N_effect
			int choiceNum = std::stoi(key.substr(7, key.find("_effect") - 7));
			while (static_cast<int>(currentDef.choices.size()) < choiceNum) {
				currentDef.choices.push_back(Choice());
			}
			currentDef.choices[choiceNum - 1].effects.push_back(ParseEffect(value));
		}
	}

	std::cout << "이벤트 " << definitions.size() << "개 로드 완료: " << filepath << std::endl;
}


// ========== 바이너리 저장 ==========
void EventManager::SaveEventDefs(const std::string& filepath) const {
	std::ofstream out(filepath, std::ios::binary);
	if (!out) return;

	uint32_t count = static_cast<uint32_t>(definitions.size());
	out.write(reinterpret_cast<const char*>(&count), sizeof(count));

	for (const auto& def : definitions) {
		WriteString(out, def.id);
		WriteString(out, def.name);
		WriteString(out, def.description);
		out.write(reinterpret_cast<const char*>(&def.category), sizeof(def.category));

		// 트리거
		out.write(reinterpret_cast<const char*>(&def.trigger.isRandom), sizeof(def.trigger.isRandom));
		out.write(reinterpret_cast<const char*>(&def.trigger.cityMask), sizeof(def.trigger.cityMask));
		out.write(reinterpret_cast<const char*>(&def.trigger.cityMin), sizeof(def.trigger.cityMin));
		out.write(reinterpret_cast<const char*>(&def.trigger.cityMax), sizeof(def.trigger.cityMax));
		out.write(reinterpret_cast<const char*>(&def.trigger.checkCity), sizeof(def.trigger.checkCity));
		out.write(reinterpret_cast<const char*>(&def.trigger.humanMask), sizeof(def.trigger.humanMask));
		out.write(reinterpret_cast<const char*>(&def.trigger.humanMin), sizeof(def.trigger.humanMin));
		out.write(reinterpret_cast<const char*>(&def.trigger.humanMax), sizeof(def.trigger.humanMax));
		out.write(reinterpret_cast<const char*>(&def.trigger.checkHuman), sizeof(def.trigger.checkHuman));
		out.write(reinterpret_cast<const char*>(&def.trigger.scope), sizeof(def.trigger.scope));
		out.write(reinterpret_cast<const char*>(&def.trigger.minHumanCount), sizeof(def.trigger.minHumanCount));
		out.write(reinterpret_cast<const char*>(&def.trigger.regionId), sizeof(def.trigger.regionId));

		// 쿨타임
		out.write(reinterpret_cast<const char*>(&def.cooldownMin), sizeof(def.cooldownMin));
		out.write(reinterpret_cast<const char*>(&def.cooldownMax), sizeof(def.cooldownMax));

		// 효과 범위
		out.write(reinterpret_cast<const char*>(&def.effectScope), sizeof(def.effectScope));
		out.write(reinterpret_cast<const char*>(&def.effectRegionId), sizeof(def.effectRegionId));
		WriteString(out, def.customEffectId);

		// 선택지
		out.write(reinterpret_cast<const char*>(&def.requiresPlayer), sizeof(def.requiresPlayer));
		uint32_t choiceCount = static_cast<uint32_t>(def.choices.size());
		out.write(reinterpret_cast<const char*>(&choiceCount), sizeof(choiceCount));
		for (const auto& choice : def.choices) {
			WriteString(out, choice.text);
			uint32_t effectCount = static_cast<uint32_t>(choice.effects.size());
			out.write(reinterpret_cast<const char*>(&effectCount), sizeof(effectCount));
			for (const auto& eff : choice.effects) {
				out.write(reinterpret_cast<const char*>(&eff.type), sizeof(eff.type));
				out.write(reinterpret_cast<const char*>(&eff.targetField), sizeof(eff.targetField));
				out.write(reinterpret_cast<const char*>(&eff.delta), sizeof(eff.delta));
				out.write(reinterpret_cast<const char*>(&eff.scope), sizeof(eff.scope));
				WriteString(out, eff.customId);
			}
		}

		// 즉시 효과
		uint32_t immEffCount = static_cast<uint32_t>(def.immediateEffects.size());
		out.write(reinterpret_cast<const char*>(&immEffCount), sizeof(immEffCount));
		for (const auto& eff : def.immediateEffects) {
			out.write(reinterpret_cast<const char*>(&eff.type), sizeof(eff.type));
			out.write(reinterpret_cast<const char*>(&eff.targetField), sizeof(eff.targetField));
			out.write(reinterpret_cast<const char*>(&eff.delta), sizeof(eff.delta));
			out.write(reinterpret_cast<const char*>(&eff.scope), sizeof(eff.scope));
			WriteString(out, eff.customId);
		}
	}
}


// ========== 바이너리 로드 ==========
void EventManager::LoadEventDefs(const std::string& filepath) {
	std::ifstream in(filepath, std::ios::binary);
	if (!in) return;

	definitions.clear();

	uint32_t count = 0;
	in.read(reinterpret_cast<char*>(&count), sizeof(count));

	for (uint32_t i = 0; i < count; ++i) {
		EventDef def;

		def.id = ReadString(in);
		def.name = ReadString(in);
		def.description = ReadString(in);
		in.read(reinterpret_cast<char*>(&def.category), sizeof(def.category));

		// 트리거
		in.read(reinterpret_cast<char*>(&def.trigger.isRandom), sizeof(def.trigger.isRandom));
		in.read(reinterpret_cast<char*>(&def.trigger.cityMask), sizeof(def.trigger.cityMask));
		in.read(reinterpret_cast<char*>(&def.trigger.cityMin), sizeof(def.trigger.cityMin));
		in.read(reinterpret_cast<char*>(&def.trigger.cityMax), sizeof(def.trigger.cityMax));
		in.read(reinterpret_cast<char*>(&def.trigger.checkCity), sizeof(def.trigger.checkCity));
		in.read(reinterpret_cast<char*>(&def.trigger.humanMask), sizeof(def.trigger.humanMask));
		in.read(reinterpret_cast<char*>(&def.trigger.humanMin), sizeof(def.trigger.humanMin));
		in.read(reinterpret_cast<char*>(&def.trigger.humanMax), sizeof(def.trigger.humanMax));
		in.read(reinterpret_cast<char*>(&def.trigger.checkHuman), sizeof(def.trigger.checkHuman));
		in.read(reinterpret_cast<char*>(&def.trigger.scope), sizeof(def.trigger.scope));
		in.read(reinterpret_cast<char*>(&def.trigger.minHumanCount), sizeof(def.trigger.minHumanCount));
		in.read(reinterpret_cast<char*>(&def.trigger.regionId), sizeof(def.trigger.regionId));

		// 쿨타임
		in.read(reinterpret_cast<char*>(&def.cooldownMin), sizeof(def.cooldownMin));
		in.read(reinterpret_cast<char*>(&def.cooldownMax), sizeof(def.cooldownMax));

		// 효과 범위
		in.read(reinterpret_cast<char*>(&def.effectScope), sizeof(def.effectScope));
		in.read(reinterpret_cast<char*>(&def.effectRegionId), sizeof(def.effectRegionId));
		def.customEffectId = ReadString(in);

		// 선택지
		in.read(reinterpret_cast<char*>(&def.requiresPlayer), sizeof(def.requiresPlayer));
		uint32_t choiceCount = 0;
		in.read(reinterpret_cast<char*>(&choiceCount), sizeof(choiceCount));
		def.choices.resize(choiceCount);
		for (auto& choice : def.choices) {
			choice.text = ReadString(in);
			uint32_t effectCount = 0;
			in.read(reinterpret_cast<char*>(&effectCount), sizeof(effectCount));
			choice.effects.resize(effectCount);
			for (auto& eff : choice.effects) {
				in.read(reinterpret_cast<char*>(&eff.type), sizeof(eff.type));
				in.read(reinterpret_cast<char*>(&eff.targetField), sizeof(eff.targetField));
				in.read(reinterpret_cast<char*>(&eff.delta), sizeof(eff.delta));
				in.read(reinterpret_cast<char*>(&eff.scope), sizeof(eff.scope));
				eff.customId = ReadString(in);
			}
		}

		// 즉시 효과
		uint32_t immEffCount = 0;
		in.read(reinterpret_cast<char*>(&immEffCount), sizeof(immEffCount));
		def.immediateEffects.resize(immEffCount);
		for (auto& eff : def.immediateEffects) {
			in.read(reinterpret_cast<char*>(&eff.type), sizeof(eff.type));
			in.read(reinterpret_cast<char*>(&eff.targetField), sizeof(eff.targetField));
			in.read(reinterpret_cast<char*>(&eff.delta), sizeof(eff.delta));
			in.read(reinterpret_cast<char*>(&eff.scope), sizeof(eff.scope));
			eff.customId = ReadString(in);
		}

		definitions.push_back(std::move(def));
	}
}


// ========== 조건 검사 ==========
bool EventManager::CheckCityCondition(const EventTrigger& trigger, CityCode cityCode) const {
	if (!trigger.checkCity) return true;

	uint16_t masked = cityCode.code & trigger.cityMask;
	return (masked >= (trigger.cityMin & trigger.cityMask)) &&
		   (masked <= (trigger.cityMax & trigger.cityMask));
}

bool EventManager::CheckHumanCondition(const EventTrigger& trigger, HumanCode humanCode) const {
	if (!trigger.checkHuman) return true;

	uint64_t masked = humanCode.code & trigger.humanMask;
	return (masked >= (trigger.humanMin & trigger.humanMask)) &&
		   (masked <= (trigger.humanMax & trigger.humanMask));
}

std::vector<Human*> EventManager::GetTriggeredHumans(
	const EventTrigger& trigger,
	const std::vector<std::unique_ptr<Human>>& humans) const
{
	std::vector<Human*> result;

	// Region 스코프인 경우 해당 구역만 필터링
	bool filterByRegion = (trigger.scope == TriggerScope::Region && trigger.regionId >= 0);

	// 랜덤 이벤트거나 Human 조건이 없으면 (구역 필터링만 적용)
	if (trigger.isRandom || !trigger.checkHuman) {
		for (auto& h : humans) {
			if (filterByRegion && static_cast<int>(h->GetRegion()) != trigger.regionId) {
				continue;
			}
			result.push_back(h.get());
		}
		return result;
	}

	for (auto& h : humans) {
		// 구역 필터링
		if (filterByRegion && static_cast<int>(h->GetRegion()) != trigger.regionId) {
			continue;
		}
		HumanCode code = EncodeHumanState(*h);
		if (CheckHumanCondition(trigger, code)) {
			result.push_back(h.get());
		}
	}

	return result;
}


// ========== 효과 대상 결정 ==========
std::vector<Human*> EventManager::DetermineAffectedHumans(
	const EventDef& def,
	const std::vector<Human*>& triggered,
	std::vector<std::unique_ptr<Human>>& allHumans) const
{
	switch (def.effectScope) {
	case EffectScope::Triggered:
		return triggered;

	case EffectScope::AllHumans:
	{
		std::vector<Human*> all;
		for (auto& h : allHumans) {
			all.push_back(h.get());
		}
		return all;
	}

	case EffectScope::City:
		return {};

	case EffectScope::Region:
	{
		// 특정 지역 Human만 반환
		int regionId = def.effectRegionId;
		if (regionId < 0) {
			// effectRegionId가 -1이면 트리거 구역 사용
			regionId = def.trigger.regionId;
		}
		if (regionId < 0) {
			// 그래도 없으면 트리거된 인간들 반환
			return triggered;
		}
		std::vector<Human*> regionHumans;
		for (auto& h : allHumans) {
			if (static_cast<int>(h->GetRegion()) == regionId) {
				regionHumans.push_back(h.get());
			}
		}
		return regionHumans;
	}

	case EffectScope::Custom:
		// Custom은 커스텀 함수에서 직접 처리
		return triggered;

	default:
		return triggered;
	}
}


// ========== 이벤트 활성화 ==========
ActiveEvent EventManager::ActivateEvent(
	const EventDef& def,
	CityCode cityCode,
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

	// 구역 이벤트 정보 설정
	event.eventRegionId = def.trigger.regionId;
	event.isRegionEvent = (def.trigger.regionId >= 0);
	event.unresolvedBehavior = def.unresolvedBehavior;

	// 발생 시점 랜덤 (0.2 ~ 0.8 사이)
	std::uniform_real_distribution<float> dist(0.2f, 0.8f);
	event.triggerTimeRatio = dist(rng);
	event.hasTriggered = false;

	// 조건 충족자 찾기
	event.triggeredHumans = GetTriggeredHumans(def.trigger, humans);

	// 효과 대상자 결정
	event.affectedHumans = DetermineAffectedHumans(def, event.triggeredHumans, humans);

	return event;
}


// ========== 매일 이벤트 처리 ==========
void EventManager::ProcessDailyEvents(
	City& city,
	const CityMetrics& metrics,
	std::vector<std::unique_ptr<Human>>& humans,
	int currentDay)
{
	scheduledEvents.clear();

	EVENT_LOG("[EVENT] ProcessDailyEvents called, Day=" << currentDay
			  << ", definitions=" << definitions.size());

	// 새로운 날인지 확인
	bool isNewDay = (lastProcessedDay != currentDay);

	if (isNewDay) {
		// 새로운 날: 카운터 리셋
		lastProcessedDay = currentDay;
		eventsTriggeredToday = 0;
		targetEventsToday = 0;  // 아래에서 새로 결정
		EVENT_LOG("[EVENT] New day started, counters reset");
	}

	CityCode cityCode = EncodeCityState(metrics);

	std::vector<const EventDef*> candidates;

	for (const auto& def : definitions) {
		// 쿨다운 체크
		auto it = lastFiredDay.find(def.id);
		if (it != lastFiredDay.end()) {
			if (currentDay < it->second) {
				continue;
			}
		}

		// 랜덤 이벤트는 조건 무시
		if (!def.trigger.isRandom) {
			// 도시 조건 체크
			if (!CheckCityCondition(def.trigger, cityCode)) {
				continue;
			}

			// Human 조건 체크 (최소 인원)
			if (def.trigger.minHumanCount > 0) {
				auto triggered = GetTriggeredHumans(def.trigger, humans);
				if (static_cast<int>(triggered.size()) < def.trigger.minHumanCount) {
					continue;
				}
			}
		}

		candidates.push_back(&def);
	}

	// 후보 중 랜덤 선택
	std::shuffle(candidates.begin(), candidates.end(), rng);

	int candidateCount = static_cast<int>(candidates.size());
	int eventCount = 0;

	if (isNewDay) {
		// 새로운 날: 목표 이벤트 수 결정 (minEventsPerDay ~ maxEventsPerDay)
		int targetMin = (std::min)(minEventsPerDay, candidateCount);
		int targetMax = (std::min)(maxEventsPerDay, candidateCount);
		std::uniform_int_distribution<int> eventCountDist(targetMin, targetMax);
		targetEventsToday = eventCountDist(rng);
		eventCount = targetEventsToday;
	}
	else {
		// 같은 날 (로드 후): 남은 이벤트 수만 스케줄링
		int remainingEvents = targetEventsToday - eventsTriggeredToday;
		eventCount = (std::min)(remainingEvents, candidateCount);
		if (eventCount < 0) eventCount = 0;
	}

	EVENT_LOG("[EVENT] candidates=" << candidates.size() << ", eventCount=" << eventCount);

	for (int i = 0; i < eventCount; ++i) {
		const EventDef* def = candidates[i];
		ActiveEvent event = ActivateEvent(*def, cityCode, humans);

		// 랜덤 쿨타임 설정
		std::uniform_int_distribution<int> cooldownDist(def->cooldownMin, def->cooldownMax);
		lastFiredDay[def->id] = currentDay + cooldownDist(rng);

		scheduledEvents.push_back(std::move(event));
		EVENT_LOG("[EVENT] Scheduled: " << def->name
				  << ", triggerRatio=" << scheduledEvents.back().triggerTimeRatio
				  << ", requiresPlayer=" << scheduledEvents.back().requiresPlayer);
	}

	EVENT_LOG("[EVENT] Total scheduled events: " << scheduledEvents.size());
}


// ========== 시간 업데이트 ==========
void EventManager::UpdateTime(
	float dayRatio,
	City& city,
	std::vector<std::unique_ptr<Human>>& humans)
{
	static float lastLoggedRatio = -1.0f;
	// 10% 단위로 로그 출력
	if (static_cast<int>(dayRatio * 10) != static_cast<int>(lastLoggedRatio * 10)) {
		lastLoggedRatio = dayRatio;
		EVENT_LOG("[EVENT] UpdateTime dayRatio=" << (dayRatio * 100) << "%"
				  << ", scheduledEvents=" << scheduledEvents.size()
				  << ", pendingPlayer=" << pendingPlayerEvents.size());
	}

	for (auto& event : scheduledEvents) {
		if (!event.hasTriggered && dayRatio >= event.triggerTimeRatio) {
			EVENT_LOG("[EVENT] Triggering: " << event.name
					  << " at ratio=" << (dayRatio * 100) << "%");
			event.hasTriggered = true;
			TriggerEvent(event, city, humans);
		}
	}
}


// ========== 이벤트 발동 ==========
void EventManager::TriggerEvent(
	ActiveEvent& event,
	City& city,
	std::vector<std::unique_ptr<Human>>& humans)
{
	const EventDef* def = nullptr;
	for (const auto& d : definitions) {
		if (d.id == event.defId) {
			def = &d;
			break;
		}
	}
	if (!def) return;

	// 이벤트 발생 카운터 증가
	eventsTriggeredToday++;

	// 이벤트 발생 콜백 호출 (힌트 시스템 연결)
	if (onEventTriggered) {
		onEventTriggered();
	}

	if (event.requiresPlayer) {
		pendingPlayerEvents.push_back(event);
		EVENT_LOG("[EVENT] Added to pendingPlayerEvents: " << event.name
				  << ", total pending=" << pendingPlayerEvents.size());
	}
	else {
		EVENT_LOG("[EVENT] Immediate effect applied: " << event.name);
		ApplyEffects(def->immediateEffects, def->effectScope, city,
			event.affectedHumans, humans);
	}
}


// ========== 효과 적용 ==========
void EventManager::ApplyEffects(
	const std::vector<EffectData>& effects,
	EffectScope defaultScope,
	City& city,
	std::vector<Human*>& affected,
	std::vector<std::unique_ptr<Human>>& allHumans)
{
	for (const auto& eff : effects) {
		// 효과별 스코프가 지정되어 있으면 그것을 사용
		EffectScope scope = eff.scope;

		// 스코프에 따라 대상 결정
		std::vector<Human*> targets;
		switch (scope) {
		case EffectScope::Triggered:
			targets = affected;
			break;
		case EffectScope::AllHumans:
			for (auto& h : allHumans) targets.push_back(h.get());
			break;
		case EffectScope::City:
			targets.clear();
			break;
		case EffectScope::Region:
			// Region 스코프는 DetermineAffectedHumans에서 이미 처리됨
			targets = affected;
			break;
		default:
			targets = affected;
		}

		switch (eff.type) {
		case EffectType::ModifyDrive:
		{
			DriveField field = static_cast<DriveField>(eff.targetField);
			for (Human* h : targets) {
				switch (field) {
				case DriveField::StressLoad:        h->ModifyStressLoad(eff.delta); break;
				case DriveField::EmotionalArousal:  h->ModifyEmotionalArousal(eff.delta); break;
				case DriveField::Fatigue:           h->ModifyFatigue(eff.delta); break;
				case DriveField::CognitiveCapacity: h->ModifyCognitiveCapacity(eff.delta); break;
				case DriveField::InterpersonalTrust: h->ModifyInterpersonalTrust(eff.delta); break;
				case DriveField::SocialSafety:      h->ModifySocialSafety(eff.delta); break;
				case DriveField::SenseOfControl:    h->ModifySenseOfControl(eff.delta); break;
				case DriveField::Motivation:        h->ModifyMotivation(eff.delta); break;
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

		case EffectType::ModifyMentalState:
			// TODO: 정신상태 직접 변경 구현
			break;

		case EffectType::Kill:
			// TODO: 사망 처리 구현
			break;

		case EffectType::AddImmigrant:
			// TODO: 이주민 추가 구현
			break;

		case EffectType::Custom:
		{
			auto it = customEffectRegistry.find(eff.customId);
			if (it != customEffectRegistry.end()) {
				it->second(city, affected, targets, allHumans);
			}
			break;
		}
		}
	}
}


// ========== 플레이어 이벤트 처리 ==========
bool EventManager::HasPendingPlayerEvent() const {
	return !pendingPlayerEvents.empty();
}

const ActiveEvent* EventManager::GetPendingPlayerEvent() const {
	if (pendingPlayerEvents.empty()) return nullptr;
	return &pendingPlayerEvents.front();
}

void EventManager::ApplyPlayerChoice(
	int choiceIndex,
	City& city,
	std::vector<std::unique_ptr<Human>>& humans)
{
	if (pendingPlayerEvents.empty()) return;

	ActiveEvent& event = pendingPlayerEvents.front();
	if (choiceIndex < 0 || choiceIndex >= static_cast<int>(event.choices.size())) {
		pendingPlayerEvents.pop_front();
		return;
	}

	event.chosenIndex = choiceIndex;
	const Choice& choice = event.choices[choiceIndex];

	const EventDef* def = nullptr;
	for (const auto& d : definitions) {
		if (d.id == event.defId) {
			def = &d;
			break;
		}
	}

	EffectScope scope = def ? def->effectScope : EffectScope::Triggered;
	ApplyEffects(choice.effects, scope, city, event.affectedHumans, humans);

	pendingPlayerEvents.pop_front();
}

// ========== 구역 이벤트 처리 ==========
bool EventManager::HasPendingRegionEvent(int regionId) const {
	for (const auto& event : pendingRegionPlayerEvents) {
		if (event.eventRegionId == regionId) {
			return true;
		}
	}
	return false;
}

const ActiveEvent* EventManager::GetPendingRegionEvent(int regionId) const {
	for (const auto& event : pendingRegionPlayerEvents) {
		if (event.eventRegionId == regionId) {
			return &event;
		}
	}
	return nullptr;
}

void EventManager::ProcessPendingRegionEvents(int playerRegionId, City& city,
	std::vector<std::unique_ptr<Human>>& humans)
{
	// 플레이어가 방문한 구역의 대기 이벤트를 메인 대기열로 이동
	auto it = pendingRegionPlayerEvents.begin();
	while (it != pendingRegionPlayerEvents.end()) {
		if (it->eventRegionId == playerRegionId) {
			pendingPlayerEvents.push_back(std::move(*it));
			it = pendingRegionPlayerEvents.erase(it);
			EVENT_LOG("[EVENT] Region event moved to pending: regionId=" << playerRegionId);
		}
		else {
			++it;
		}
	}
}

void EventManager::ProcessUnresolvedEvents(City& city,
	std::vector<std::unique_ptr<Human>>& humans)
{
	// 하루 종료 시 미처리 구역 이벤트 처리
	std::deque<ActiveEvent> carryOverEvents;

	for (auto& event : pendingRegionPlayerEvents) {
		switch (event.unresolvedBehavior) {
		case UnresolvedBehavior::AutoResolve:
		{
			// 랜덤 선택지 자동 적용
			if (!event.choices.empty()) {
				std::uniform_int_distribution<size_t> dist(0, event.choices.size() - 1);
				int choiceIndex = static_cast<int>(dist(rng));
				event.chosenIndex = choiceIndex;
				const Choice& choice = event.choices[choiceIndex];

				const EventDef* def = nullptr;
				for (const auto& d : definitions) {
					if (d.id == event.defId) {
						def = &d;
						break;
					}
				}

				EffectScope scope = def ? def->effectScope : EffectScope::Triggered;
				ApplyEffects(choice.effects, scope, city, event.affectedHumans, humans);
				EVENT_LOG("[EVENT] AutoResolve: " << event.name << " -> choice " << choiceIndex);
			}
			break;
		}
		case UnresolvedBehavior::Expire:
			// 효과 없이 소멸
			EVENT_LOG("[EVENT] Expire: " << event.name);
			break;
		case UnresolvedBehavior::CarryOver:
			// 다음 날로 이월
			carryOverEvents.push_back(std::move(event));
			EVENT_LOG("[EVENT] CarryOver: " << event.name);
			break;
		}
	}

	pendingRegionPlayerEvents = std::move(carryOverEvents);
}


// ========== 게임 상태 저장/로드 ==========
void EventManager::SaveState(std::ofstream& out) const {
	// 쿨다운 정보 저장
	uint32_t cooldownCount = static_cast<uint32_t>(lastFiredDay.size());
	out.write(reinterpret_cast<const char*>(&cooldownCount), sizeof(cooldownCount));
	for (const auto& pair : lastFiredDay) {
		WriteString(out, pair.first);
		out.write(reinterpret_cast<const char*>(&pair.second), sizeof(pair.second));
	}

	// 하루 이벤트 진행 상태 저장
	out.write(reinterpret_cast<const char*>(&lastProcessedDay), sizeof(lastProcessedDay));
	out.write(reinterpret_cast<const char*>(&targetEventsToday), sizeof(targetEventsToday));
	out.write(reinterpret_cast<const char*>(&eventsTriggeredToday), sizeof(eventsTriggeredToday));

	// 미방문 구역 이벤트 대기열 저장
	uint32_t pendingRegionCount = static_cast<uint32_t>(pendingRegionPlayerEvents.size());
	out.write(reinterpret_cast<const char*>(&pendingRegionCount), sizeof(pendingRegionCount));
	for (const auto& event : pendingRegionPlayerEvents) {
		WriteString(out, event.defId);
		WriteString(out, event.name);
		WriteString(out, event.description);
		out.write(reinterpret_cast<const char*>(&event.eventRegionId), sizeof(event.eventRegionId));
		out.write(reinterpret_cast<const char*>(&event.isRegionEvent), sizeof(event.isRegionEvent));
		out.write(reinterpret_cast<const char*>(&event.unresolvedBehavior), sizeof(event.unresolvedBehavior));
		out.write(reinterpret_cast<const char*>(&event.requiresPlayer), sizeof(event.requiresPlayer));

		// 선택지 저장
		uint32_t choiceCount = static_cast<uint32_t>(event.choices.size());
		out.write(reinterpret_cast<const char*>(&choiceCount), sizeof(choiceCount));
		for (const auto& choice : event.choices) {
			WriteString(out, choice.text);
			uint32_t effectCount = static_cast<uint32_t>(choice.effects.size());
			out.write(reinterpret_cast<const char*>(&effectCount), sizeof(effectCount));
			for (const auto& eff : choice.effects) {
				out.write(reinterpret_cast<const char*>(&eff.type), sizeof(eff.type));
				out.write(reinterpret_cast<const char*>(&eff.targetField), sizeof(eff.targetField));
				out.write(reinterpret_cast<const char*>(&eff.delta), sizeof(eff.delta));
				out.write(reinterpret_cast<const char*>(&eff.scope), sizeof(eff.scope));
				WriteString(out, eff.customId);
			}
		}
	}
}

void EventManager::LoadState(std::ifstream& in) {
	lastFiredDay.clear();
	scheduledEvents.clear();
	pendingPlayerEvents.clear();
	pendingRegionPlayerEvents.clear();

	// 쿨다운 정보 로드
	uint32_t cooldownCount = 0;
	in.read(reinterpret_cast<char*>(&cooldownCount), sizeof(cooldownCount));
	for (uint32_t i = 0; i < cooldownCount; ++i) {
		std::string id = ReadString(in);
		int day = 0;
		in.read(reinterpret_cast<char*>(&day), sizeof(day));
		lastFiredDay[id] = day;
	}

	// 하루 이벤트 진행 상태 로드
	in.read(reinterpret_cast<char*>(&lastProcessedDay), sizeof(lastProcessedDay));
	in.read(reinterpret_cast<char*>(&targetEventsToday), sizeof(targetEventsToday));
	in.read(reinterpret_cast<char*>(&eventsTriggeredToday), sizeof(eventsTriggeredToday));

	// 미방문 구역 이벤트 대기열 로드 (파일 끝이 아닌 경우만)
	if (in.peek() != EOF) {
		uint32_t pendingRegionCount = 0;
		in.read(reinterpret_cast<char*>(&pendingRegionCount), sizeof(pendingRegionCount));
		for (uint32_t i = 0; i < pendingRegionCount && in.good(); ++i) {
			ActiveEvent event;
			event.defId = ReadString(in);
			event.name = ReadString(in);
			event.description = ReadString(in);
			in.read(reinterpret_cast<char*>(&event.eventRegionId), sizeof(event.eventRegionId));
			in.read(reinterpret_cast<char*>(&event.isRegionEvent), sizeof(event.isRegionEvent));
			in.read(reinterpret_cast<char*>(&event.unresolvedBehavior), sizeof(event.unresolvedBehavior));
			in.read(reinterpret_cast<char*>(&event.requiresPlayer), sizeof(event.requiresPlayer));

			// 선택지 로드
			uint32_t choiceCount = 0;
			in.read(reinterpret_cast<char*>(&choiceCount), sizeof(choiceCount));
			event.choices.resize(choiceCount);
			for (auto& choice : event.choices) {
				choice.text = ReadString(in);
				uint32_t effectCount = 0;
				in.read(reinterpret_cast<char*>(&effectCount), sizeof(effectCount));
				choice.effects.resize(effectCount);
				for (auto& eff : choice.effects) {
					in.read(reinterpret_cast<char*>(&eff.type), sizeof(eff.type));
					in.read(reinterpret_cast<char*>(&eff.targetField), sizeof(eff.targetField));
					in.read(reinterpret_cast<char*>(&eff.delta), sizeof(eff.delta));
					in.read(reinterpret_cast<char*>(&eff.scope), sizeof(eff.scope));
					eff.customId = ReadString(in);
				}
			}

			event.isActive = true;
			pendingRegionPlayerEvents.push_back(std::move(event));
		}
	}
}


// ========== 커스텀 효과 등록 ==========
void EventManager::RegisterCustomEffects() {
	// 예시: 폭동 효과 - 무작위 10% 시민에게 영향
	customEffectRegistry["riot_random_10"] = [this](
		City& city,
		std::vector<Human*>& triggered,
		std::vector<Human*>& affected,
		std::vector<std::unique_ptr<Human>>& allHumans)
	{
		std::vector<Human*> pool;
		for (auto& h : allHumans) pool.push_back(h.get());
		std::shuffle(pool.begin(), pool.end(), rng);

		int count = static_cast<int>(pool.size() * 0.1);
		for (int i = 0; i < count && i < static_cast<int>(pool.size()); ++i) {
			pool[i]->ModifyStressLoad(1500);
			pool[i]->ModifySocialSafety(-800);
		}
		city.ModifyMood(-500);
	};

	// 예시: 희망 전파 - 신뢰도 상위 20%가 주변에 영향
	customEffectRegistry["hope_spread_top20"] = [](
		City& city,
		std::vector<Human*>& triggered,
		std::vector<Human*>& affected,
		std::vector<std::unique_ptr<Human>>& allHumans)
	{
		std::vector<Human*> pool;
		for (auto& h : allHumans) pool.push_back(h.get());

		// 신뢰도 순 정렬
		std::sort(pool.begin(), pool.end(), [](Human* a, Human* b) {
			return a->GetInterpersonalTrust() > b->GetInterpersonalTrust();
		});

		int count = static_cast<int>(pool.size() * 0.2);
		for (int i = 0; i < count; ++i) {
			pool[i]->ModifyMotivation(500);
		}

		// 전체에게 약간의 희망
		for (auto& h : allHumans) {
			h->ModifyMotivation(100);
			h->ModifySocialSafety(50);
		}
		city.ModifyMood(300);
	};
}


// ========== 기본 이벤트 등록 (더 이상 사용 안 함 - 텍스트 파일로 대체) ==========
void EventManager::RegisterDefaultEvents() {
	// 텍스트 파일에서 로드하므로 비워둠
}
