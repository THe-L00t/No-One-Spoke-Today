#include "City.h"
#include "Human.h"


City::City(const std::vector<std::unique_ptr<Human>>& humans)
{
    int moodSum = 0;
    int activitySum = 0;
    int scarcitySum = 0;

    for (const auto& h : humans) {
        // --- Mood 계산 ---
        // 이성(rationality), 공격성(aggressiveness), 감정 민감도(emotionalSensitivity), 스트레스, 사회 안전감
        int mood = 5000;
        
        // 감정 각성
        switch (h->GetArousal()) {
        case ArousalState::Calm: mood += 1000; break;
        case ArousalState::Tense: mood += 300; break;
        case ArousalState::Irritable: mood -= 500; break;
        case ArousalState::Hostile: mood -= 1000; break;
        }

        // 사회적 태도
        switch (h->GetSocial()) {
        case SocialState::Cooperative: mood += 800; break;
        case SocialState::Neutral: mood += 0; break;
        case SocialState::Withdrawn: mood -= 400; break;
        }

        // 성향 영향: 감정 민감도, 공격성, 이성
        mood += (h->GetEmotionalSensitivity() * 2 - h->GetRationality() - h->GetAggressiveness());

        // Drives 영향: 스트레스, 피로, 사회 안전감
        mood -= h->GetStressLoad();
        mood -= h->GetFatigue() / 2;
        mood += h->GetSocialSafety();

        mood = std::clamp(mood, 0, 10000);
        moodSum += mood;

        // --- Activity 계산 ---
        int activity = 5000;

        // 에너지 상태
        switch (h->GetEnergy()) {
        case EnergyState::Normal: activity += 1000; break;
        case EnergyState::Fatigued: activity -= 500; break;
        case EnergyState::Exhausted: activity -= 1000; break;
        }

        // 성향 영향: 계획(planning), 의존(dependency)
        activity += h->GetPlanning() * 5;      // 장기 계획 성향이 높으면 생산성 증가
        activity -= h->GetDependency() * 3;   // 의존 성향 높으면 자율 행동 감소

        // Drives 영향: 동기, 인지 능력
        activity += h->GetMotivation() * 10;
        activity += h->GetCognitiveCapacity() / 2;

        activity = std::clamp(activity, 0, 10000);
        activitySum += activity;

        // --- Scarcity 계산 ---
        int scarcity = 5000;

        // 스트레스, 피로, 통제감, 의존, 고집, 사회 안전감
        scarcity += h->GetStressLoad() / 2;
        scarcity += h->GetFatigue() / 2;
        scarcity -= h->GetSocialSafety() / 2;

        // MentalState control
        switch (h->GetControl()) {
        case ControlState::Dependent: scarcity += 500; break;
        case ControlState::Stubborn: scarcity += 300; break;
        case ControlState::Autonomous: scarcity += 0; break;
        }

        // 성향 영향
        scarcity += h->GetDependency() * 5;
        scarcity += h->GetRigidity() * 3;

        scarcity = std::clamp(scarcity, 0, 10000);
        scarcitySum += scarcity;
    }

    int n = humans.size();
    cityMet.mood = moodSum / n;
    cityMet.activity = activitySum / n;
    cityMet.scarcity = scarcitySum / n;


}

City::~City()
{
}

void City::Update(const std::vector<std::unique_ptr<Human>>& humans)
{
    int moodSum = 0;
    int activitySum = 0;
    int scarcitySum = 0;

    for (const auto& h : humans) {
        // --- Mood 계산 ---
        // 이성(rationality), 공격성(aggressiveness), 감정 민감도(emotionalSensitivity), 스트레스, 사회 안전감
        int mood = 5000;

        // 감정 각성
        switch (h->GetArousal()) {
        case ArousalState::Calm: mood += 1000; break;
        case ArousalState::Tense: mood += 300; break;
        case ArousalState::Irritable: mood -= 500; break;
        case ArousalState::Hostile: mood -= 1000; break;
        }

        // 사회적 태도
        switch (h->GetSocial()) {
        case SocialState::Cooperative: mood += 800; break;
        case SocialState::Neutral: mood += 0; break;
        case SocialState::Withdrawn: mood -= 400; break;
        }

        // 성향 영향: 감정 민감도, 공격성, 이성
        mood += (h->GetEmotionalSensitivity() * 2 - h->GetRationality() - h->GetAggressiveness());

        // Drives 영향: 스트레스, 피로, 사회 안전감
        mood -= h->GetStressLoad();
        mood -= h->GetFatigue() / 2;
        mood += h->GetSocialSafety();

        mood = std::clamp(mood, 0, 10000);
        moodSum += mood;

        // --- Activity 계산 ---
        int activity = 5000;

        // 에너지 상태
        switch (h->GetEnergy()) {
        case EnergyState::Normal: activity += 1000; break;
        case EnergyState::Fatigued: activity -= 500; break;
        case EnergyState::Exhausted: activity -= 1000; break;
        }

        // 성향 영향: 계획(planning), 의존(dependency)
        activity += h->GetPlanning() * 5;      // 장기 계획 성향이 높으면 생산성 증가
        activity -= h->GetDependency() * 3;   // 의존 성향 높으면 자율 행동 감소

        // Drives 영향: 동기, 인지 능력
        activity += h->GetMotivation() * 10;
        activity += h->GetCognitiveCapacity() / 2;

        activity = std::clamp(activity, 0, 10000);
        activitySum += activity;

        // --- Scarcity 계산 ---
        int scarcity = 5000;

        // 스트레스, 피로, 통제감, 의존, 고집, 사회 안전감
        scarcity += h->GetStressLoad() / 2;
        scarcity += h->GetFatigue() / 2;
        scarcity -= h->GetSocialSafety() / 2;

        // MentalState control
        switch (h->GetControl()) {
        case ControlState::Dependent: scarcity += 500; break;
        case ControlState::Stubborn: scarcity += 300; break;
        case ControlState::Autonomous: scarcity += 0; break;
        }

        // 성향 영향
        scarcity += h->GetDependency() * 5;
        scarcity += h->GetRigidity() * 3;

        scarcity = std::clamp(scarcity, 0, 10000);
        scarcitySum += scarcity;
    }

    int n = humans.size();
    cityMet.mood = moodSum / n;
    cityMet.activity = activitySum / n;
    cityMet.scarcity = scarcitySum / n;
}

void City::Debug() const
{
    std::cout << "============== City =================" << std::endl;
    std::cout << "mood : " << cityMet.mood << std::endl;
    std::cout << "activity : " << cityMet.activity << std::endl;
    std::cout << "scarcity : " << cityMet.scarcity << std::endl;
}

const CityMetrics& City::GetCityMet() const
{
    return cityMet;
}

void City::ModifyMood(int delta)
{
    cityMet.mood = std::clamp(cityMet.mood + delta, 0, 10000);
}

void City::ModifyActivity(int delta)
{
    cityMet.activity = std::clamp(cityMet.activity + delta, 0, 10000);
}

void City::ModifyScarcity(int delta)
{
    cityMet.scarcity = std::clamp(cityMet.scarcity + delta, 0, 10000);
}

void City::SetCityMet(const CityMetrics& met)
{
    cityMet = met;
}
