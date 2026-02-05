#include "City.h"
#include "Human.h"
#include "Navigation.h"
#include <cmath>


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

    size_t n = humans.size();
    if (n > 0) {
        cityMet.mood = moodSum / static_cast<int>(n);
        cityMet.activity = activitySum / static_cast<int>(n);
        cityMet.scarcity = scarcitySum / static_cast<int>(n);
    }
    else {
        cityMet.mood = 5000;
        cityMet.activity = 5000;
        cityMet.scarcity = 5000;
    }


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

    size_t n = humans.size();
    if (n > 0) {
        cityMet.mood = moodSum / static_cast<int>(n);
        cityMet.activity = activitySum / static_cast<int>(n);
        cityMet.scarcity = scarcitySum / static_cast<int>(n);
    }
}

// ==================== 비선형 업데이트 (지형/기온 반영) ====================
void City::Update(const std::vector<std::unique_ptr<Human>>& humans,
    float temperature, TerrainType terrain)
{
    if (humans.empty()) return;

    int moodSum = 0;
    int activitySum = 0;
    int scarcitySum = 0;

    // 극단적 상태 카운트
    int hostileCount = 0;
    int irritableCount = 0;
    int exhaustedCount = 0;
    int fatiguedCount = 0;
    int withdrawnCount = 0;
    int cooperativeCount = 0;

    for (const auto& h : humans) {
        // --- 상태 카운트 ---
        switch (h->GetArousal()) {
        case ArousalState::Hostile: hostileCount++; break;
        case ArousalState::Irritable: irritableCount++; break;
        default: break;
        }
        switch (h->GetEnergy()) {
        case EnergyState::Exhausted: exhaustedCount++; break;
        case EnergyState::Fatigued: fatiguedCount++; break;
        default: break;
        }
        switch (h->GetSocial()) {
        case SocialState::Withdrawn: withdrawnCount++; break;
        case SocialState::Cooperative: cooperativeCount++; break;
        default: break;
        }

        // --- Mood 계산 ---
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

        // 성향 영향
        mood += (h->GetEmotionalSensitivity() * 2 - h->GetRationality() - h->GetAggressiveness());

        // Drives 영향
        mood -= h->GetStressLoad();
        mood -= h->GetFatigue() / 2;
        mood += h->GetSocialSafety();

        mood = std::clamp(mood, 0, 10000);
        moodSum += mood;

        // --- Activity 계산 ---
        int activity = 5000;

        switch (h->GetEnergy()) {
        case EnergyState::Normal: activity += 1000; break;
        case EnergyState::Fatigued: activity -= 500; break;
        case EnergyState::Exhausted: activity -= 1000; break;
        }

        activity += h->GetPlanning() * 5;
        activity -= h->GetDependency() * 3;
        activity += h->GetMotivation() * 10;
        activity += h->GetCognitiveCapacity() / 2;

        activity = std::clamp(activity, 0, 10000);
        activitySum += activity;

        // --- Scarcity 계산 ---
        int scarcity = 5000;

        scarcity += h->GetStressLoad() / 2;
        scarcity += h->GetFatigue() / 2;
        scarcity -= h->GetSocialSafety() / 2;

        switch (h->GetControl()) {
        case ControlState::Dependent: scarcity += 500; break;
        case ControlState::Stubborn: scarcity += 300; break;
        case ControlState::Autonomous: scarcity += 0; break;
        }

        scarcity += h->GetDependency() * 5;
        scarcity += h->GetRigidity() * 3;

        scarcity = std::clamp(scarcity, 0, 10000);
        scarcitySum += scarcity;
    }

    size_t n = humans.size();
    float nf = static_cast<float>(n);

    // 기본 평균값
    int baseMood = moodSum / static_cast<int>(n);
    int baseActivity = activitySum / static_cast<int>(n);
    int baseScarcity = scarcitySum / static_cast<int>(n);

    // ==================== 비선형 효과 적용 ====================

    // 1. 적대적/과민 비율에 따른 분위기 급락 (집단 불안)
    float hostileRatio = hostileCount / nf;
    float irritableRatio = irritableCount / nf;
    float negativeRatio = hostileRatio + irritableRatio * 0.5f;

    if (negativeRatio > 0.15f) {
        // 15% 초과 시 비선형 급락
        float excess = negativeRatio - 0.15f;
        float penalty = std::pow(excess, 2) * 50000.0f;  // 제곱 함수
        baseMood -= static_cast<int>(penalty);
    }
    else if (negativeRatio > 0.05f) {
        // 5~15%: 선형 감소
        baseMood -= static_cast<int>((negativeRatio - 0.05f) * 2000.0f);
    }

    // 2. 소진자 비율에 따른 활동성 급감
    float exhaustedRatio = exhaustedCount / nf;
    float fatiguedRatio = fatiguedCount / nf;
    float tiredRatio = exhaustedRatio + fatiguedRatio * 0.4f;

    if (tiredRatio > 0.1f) {
        float excess = tiredRatio - 0.1f;
        baseActivity = static_cast<int>(baseActivity * (1.0f - excess * 2.0f));
    }

    // 3. 협력적 비율에 따른 보너스
    float cooperativeRatio = cooperativeCount / nf;
    if (cooperativeRatio > 0.3f) {
        float bonus = (cooperativeRatio - 0.3f) * 1000.0f;
        baseMood += static_cast<int>(bonus);
        baseActivity += static_cast<int>(bonus * 0.5f);
    }

    // 4. 철수자 비율에 따른 결핍 증가
    float withdrawnRatio = withdrawnCount / nf;
    if (withdrawnRatio > 0.2f) {
        float excess = withdrawnRatio - 0.2f;
        baseScarcity += static_cast<int>(excess * 3000.0f);
    }

    // 5. 지형 영향
    float terrainMoodMod = 1.0f;
    float terrainActivityMod = 1.0f;
    float terrainScarcityMod = 1.0f;

    switch (terrain) {
    case TerrainType::Oasis:
        terrainMoodMod = 1.1f;
        terrainActivityMod = 1.05f;
        terrainScarcityMod = 0.9f;
        break;
    case TerrainType::Toxic:
        terrainMoodMod = 0.85f;
        terrainActivityMod = 0.9f;
        terrainScarcityMod = 1.2f;
        break;
    case TerrainType::Tundra:
        terrainMoodMod = 0.9f;
        terrainActivityMod = 0.85f;
        terrainScarcityMod = 1.15f;
        break;
    case TerrainType::Desert:
        terrainMoodMod = 0.92f;
        terrainActivityMod = 0.9f;
        terrainScarcityMod = 1.1f;
        break;
    case TerrainType::Ruins:
        terrainMoodMod = 0.88f;
        terrainActivityMod = 0.95f;
        terrainScarcityMod = 1.1f;
        break;
    case TerrainType::Mountain:
        terrainMoodMod = 0.95f;
        terrainActivityMod = 0.85f;
        terrainScarcityMod = 1.05f;
        break;
    default:  // Wasteland
        break;
    }

    // 6. 기온 영향
    float tempDiff = std::abs(temperature - 20.0f);
    float tempMod = 1.0f;
    if (tempDiff > 10.0f) {
        float excess = tempDiff - 10.0f;
        tempMod = 1.0f - (excess * 0.01f);  // 10도 초과 시 1%씩 감소
        tempMod = (std::max)(0.7f, tempMod);
    }

    // 최종 적용
    cityMet.mood = std::clamp(
        static_cast<int>(baseMood * terrainMoodMod * tempMod), 0, 10000);
    cityMet.activity = std::clamp(
        static_cast<int>(baseActivity * terrainActivityMod * tempMod), 0, 10000);
    cityMet.scarcity = std::clamp(
        static_cast<int>(baseScarcity * terrainScarcityMod), 0, 10000);
}

// ==================== 비선형 효과 헬퍼 함수 ====================

float City::CalculateExtremeStatePenalty(const std::vector<std::unique_ptr<Human>>& humans) const
{
    if (humans.empty()) return 0.0f;

    int hostileCount = 0;
    int irritableCount = 0;

    for (const auto& h : humans) {
        if (h->GetArousal() == ArousalState::Hostile) hostileCount++;
        else if (h->GetArousal() == ArousalState::Irritable) irritableCount++;
    }

    float ratio = (hostileCount + irritableCount * 0.5f) / humans.size();

    if (ratio > 0.15f) {
        return std::pow(ratio - 0.15f, 2) * 100.0f;
    }
    return 0.0f;
}

float City::CalculateExhaustedPenalty(const std::vector<std::unique_ptr<Human>>& humans) const
{
    if (humans.empty()) return 0.0f;

    int exhaustedCount = 0;
    for (const auto& h : humans) {
        if (h->GetEnergy() == EnergyState::Exhausted) exhaustedCount++;
    }

    float ratio = static_cast<float>(exhaustedCount) / humans.size();

    if (ratio > 0.1f) {
        return (ratio - 0.1f) * 2.0f;  // 10% 초과 시 배율
    }
    return 0.0f;
}

float City::CalculateCollectiveMorale(const std::vector<std::unique_ptr<Human>>& humans) const
{
    if (humans.empty()) return 1.0f;

    int cooperativeCount = 0;
    for (const auto& h : humans) {
        if (h->GetSocial() == SocialState::Cooperative) cooperativeCount++;
    }

    float ratio = static_cast<float>(cooperativeCount) / humans.size();

    if (ratio > 0.3f) {
        return 1.0f + (ratio - 0.3f) * 0.5f;  // 30% 초과 시 보너스
    }
    return 1.0f;
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
