#include "Human.h"

std::random_device rd;
std::default_random_engine dre(rd());
std::uniform_int_distribution rtrait{ 0,100 };
std::uniform_int_distribution rdrive{ 40,60 };

Human::Human()
{
	traits.rationality          = rtrait(dre);
    traits.aggressiveness       = rtrait(dre);
    traits.planning 			= rtrait(dre);
    traits.dependency			= rtrait(dre);
    traits.rigidity			    = rtrait(dre);
    traits.emotionalSensitivity = rtrait(dre);

    drives.stressLoad           = rdrive(dre);
    drives.emotionalArousal     = rdrive(dre);
    drives.fatigue              = rdrive(dre);
    drives.cognitiveCapacity    = rdrive(dre);
    drives.interpersonalTrust   = rdrive(dre);
    drives.socialSafety         = rdrive(dre);
    drives.senseOfControl       = rdrive(dre);
    drives.motivation           = rdrive(dre);

    {
        int temp = rtrait(dre);
        state.arousal = (temp < 35) ? ArousalState::Calm :
            (temp < 75) ? ArousalState::Tense :
            (temp < 95) ? ArousalState::Irritable : ArousalState::Hostile;
    }
    {
        int temp = rtrait(dre);
        state.social = (temp < 50) ? SocialState::Neutral :
            (temp < 85) ? SocialState::Cooperative : SocialState::Withdrawn;
    }
    {
        int temp = rtrait(dre);
        state.energy = (temp < 60) ? EnergyState::Normal :
            (temp < 90) ? EnergyState::Fatigued : EnergyState::Exhausted;
    }
    {
        int temp = rtrait(dre);
        state.control = (temp < 55) ? ControlState::Autonomous :
            (temp < 85) ? ControlState::Dependent : ControlState::Stubborn;
    }
}

void Human::UpdateMentalState()
{
    // ---------- ArousalState ----------
        // 상태별 임계값 계산 (정교한 수식 유지)
    double tenseThreshold = 40.0 - traits.emotionalSensitivity * 0.25 + traits.rationality * 0.15;
    double irritableThreshold = 60.0 - traits.emotionalSensitivity * 0.35 + traits.aggressiveness * 0.25;
    double hostileThreshold = 75.0 - traits.emotionalSensitivity * 0.45 + traits.aggressiveness * 0.35;

    if (drives.stressLoad <= tenseThreshold && drives.emotionalArousal <= tenseThreshold) {
        state.arousal = ArousalState::Calm;
    }
    else if (drives.stressLoad <= irritableThreshold || drives.emotionalArousal <= irritableThreshold) {
        state.arousal = ArousalState::Tense;
    }
    else if (drives.stressLoad <= hostileThreshold || drives.emotionalArousal <= hostileThreshold) {
        state.arousal = ArousalState::Irritable;
    }
    else {
        state.arousal = ArousalState::Hostile;
    }

    // ---------- EnergyState ----------
    // 피로 누적 + 인지 능력, 감정 각성 고려
    double fatigueScore = drives.fatigue + drives.emotionalArousal * 0.2 - drives.cognitiveCapacity * 0.1;
    if (fatigueScore < 50.0)
        state.energy = EnergyState::Normal;
    else if (fatigueScore < 75.0)
        state.energy = EnergyState::Fatigued;
    else
        state.energy = EnergyState::Exhausted;

    // ---------- SocialState ----------
    // 신뢰, 통제감, 감정 각성, 의존성 반영
    double coopScore = drives.interpersonalTrust + drives.motivation * 0.5 - drives.senseOfControl * 0.3 + traits.dependency * 0.2;
    double withdrawScore = 100 - drives.interpersonalTrust + drives.fatigue * 0.5 + traits.rigidity * 0.3;

    if (coopScore > 65.0)
        state.social = SocialState::Cooperative;
    else if (withdrawScore > 60.0)
        state.social = SocialState::Withdrawn;
    else
        state.social = SocialState::Neutral;

    // ---------- ControlState ----------
    // 통제감, 의존성, 고집, 스트레스 고려
    double dependentScore = traits.dependency * 0.6 + (50 - drives.senseOfControl) * 0.4;
    double stubbornScore = traits.rigidity * 0.7 + drives.stressLoad * 0.3;

    if (dependentScore > 60.0)
        state.control = ControlState::Dependent;
    else if (stubbornScore > 65.0)
        state.control = ControlState::Stubborn;
    else
        state.control = ControlState::Autonomous;
}

void Human::UpdateDrive(float deltaTime, CityMetrics city)
{
    // ΔTime을 초 단위로 받음
    // 최대 변화량 상수
    const int MAX_STRESS_DELTA = 50;     // 1초 기준 최대 변화량
    const int MAX_MOTIVATION_DELTA = 30;
    const int MAX_SOCIAL_DELTA = 20;

    // --- StressLoad ---
    int stressDelta = MAX_STRESS_DELTA * (10000 - city.mood) / 10000;
    // 상태 보정
    switch (state.arousal) {
    case ArousalState::Calm: stressDelta *= 0.5; break;
    case ArousalState::Tense: stressDelta *= 1.0; break;
    case ArousalState::Irritable: stressDelta *= 1.3; break;
    case ArousalState::Hostile: stressDelta *= 1.5; break;
    }
    drives.stressLoad = std::clamp(drives.stressLoad + stressDelta * deltaTime, 0.f, 10000.f);

    // --- Motivation & Cognitive Capacity ---
    int motivationDelta = MAX_MOTIVATION_DELTA * city.activity / 10000;
    if (state.energy == EnergyState::Fatigued) motivationDelta *= 0.7;
    if (state.energy == EnergyState::Exhausted) motivationDelta *= 0.4;
    drives.motivation = std::clamp(drives.motivation + motivationDelta * deltaTime, 0.f, 10000.f);

    int cognitiveDelta = MAX_MOTIVATION_DELTA * city.activity / 10000;
    drives.cognitiveCapacity = std::clamp(drives.cognitiveCapacity + cognitiveDelta * deltaTime, 0.f, 10000.f);

    // --- SocialSafety & SenseOfControl ---
    int socialDelta = MAX_SOCIAL_DELTA * (10000 - city.scarcity) / 10000;
    if (state.control == ControlState::Dependent) socialDelta *= 0.7;
    if (state.control == ControlState::Stubborn) socialDelta *= 0.9;
    drives.socialSafety = std::clamp(drives.socialSafety + socialDelta * deltaTime, 0.f, 10000.f);
    drives.senseOfControl = std::clamp(drives.senseOfControl + socialDelta * deltaTime, 0.f, 10000.f);

    // --- Fatigue & EmotionalArousal ---
    int fatigueDelta = MAX_STRESS_DELTA * (10000 - city.activity) / 10000;
    drives.fatigue = std::clamp(drives.fatigue + fatigueDelta * deltaTime, 0.f, 10000.f);

    int arousalDelta = MAX_SOCIAL_DELTA * (10000 - city.mood) / 10000;
    drives.emotionalArousal = std::clamp(drives.emotionalArousal + arousalDelta * deltaTime, 0.f, 10000.f);
}

int Human::GetRationality() const
{
    return traits.rationality;
}

int Human::GetAggressiveness() const
{
    return traits.aggressiveness;
}

int Human::GetPlanning() const
{
    return traits.planning;
}

int Human::GetDependency() const
{
    return traits.dependency;
}

int Human::GetRigidity() const
{
    return traits.rigidity;
}

int Human::GetEmotionalSensitivity() const
{
    return traits.emotionalSensitivity;
}

int Human::GetStressLoad() const
{
    return drives.stressLoad;
}

int Human::GetEmotionalArousal() const
{
    return drives.emotionalArousal;
}

int Human::GetFatigue() const
{
    return drives.fatigue;
}

int Human::GetCognitiveCapacity() const
{
    return drives.cognitiveCapacity;
}

int Human::GetInterpersonalTrust() const
{
    return drives.interpersonalTrust;
}

int Human::GetSocialSafety() const
{
    return drives.socialSafety;
}

int Human::GetSenseOfControl() const
{
    return drives.senseOfControl;
}

int Human::GetMotivation() const
{
    return drives.motivation;
}

ArousalState Human::GetArousal() const
{
    return state.arousal;
}

SocialState Human::GetSocial() const
{
    return state.social;
}

EnergyState Human::GetEnergy() const
{
    return state.energy;
}

ControlState Human::GetControl() const
{
    return state.control;
}

void Human::ModifyStressLoad(int delta)
{
    drives.stressLoad = std::clamp(drives.stressLoad + delta, 0, 10000);
}

void Human::ModifyEmotionalArousal(int delta)
{
    drives.emotionalArousal = std::clamp(drives.emotionalArousal + delta, 0, 10000);
}

void Human::ModifyFatigue(int delta)
{
    drives.fatigue = std::clamp(drives.fatigue + delta, 0, 10000);
}

void Human::ModifyCognitiveCapacity(int delta)
{
    drives.cognitiveCapacity = std::clamp(drives.cognitiveCapacity + delta, 0, 10000);
}

void Human::ModifyInterpersonalTrust(int delta)
{
    drives.interpersonalTrust = std::clamp(drives.interpersonalTrust + delta, 0, 10000);
}

void Human::ModifySocialSafety(int delta)
{
    drives.socialSafety = std::clamp(drives.socialSafety + delta, 0, 10000);
}

void Human::ModifySenseOfControl(int delta)
{
    drives.senseOfControl = std::clamp(drives.senseOfControl + delta, 0, 10000);
}

void Human::ModifyMotivation(int delta)
{
    drives.motivation = std::clamp(drives.motivation + delta, 0, 10000);
}

