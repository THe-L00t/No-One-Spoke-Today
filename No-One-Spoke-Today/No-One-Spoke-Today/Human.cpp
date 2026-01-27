#include "Human.h"

std::random_device rd;
std::default_random_engine dre(rd());
std::uniform_int_distribution uid{ 0,100 };

Human::Human()
{
	traits.rationality          = uid(dre);
    traits.aggressiveness       = uid(dre);
    traits.planning 			= uid(dre);
    traits.dependency			= uid(dre);
    traits.rigidity			    = uid(dre);
    traits.emotionalSensitivity = uid(dre);
}

void Human::updateMentalState()
{
    // ---------- ArousalState ----------
        // 상태별 임계값 계산 (정교한 수식 유지)
    double tenseThreshold = 40.0 - traits.emotionalSensitivity * 0.25 + traits.rationality * 0.15;
    double irritableThreshold = 60.0 - traits.emotionalSensitivity * 0.35 + traits.aggressiveness * 0.25;
    double hostileThreshold = 75.0 - traits.emotionalSensitivity * 0.45 + traits.aggressiveness * 0.35;

    if (drives.stressLoad <= tenseThreshold && drives.EmotionalArousal <= tenseThreshold) {
        state.arousal = ArousalState::Calm;
    }
    else if (drives.stressLoad <= irritableThreshold || drives.EmotionalArousal <= irritableThreshold) {
        state.arousal = ArousalState::Tense;
    }
    else if (drives.stressLoad <= hostileThreshold || drives.EmotionalArousal <= hostileThreshold) {
        state.arousal = ArousalState::Irritable;
    }
    else {
        state.arousal = ArousalState::Hostile;
    }

    // ---------- EnergyState ----------
    // 피로 누적 + 인지 능력, 감정 각성 고려
    double fatigueScore = drives.fatigue + drives.EmotionalArousal * 0.2 - drives.cognitiveCapacity * 0.1;
    if (fatigueScore < 50.0)
        state.energy = EnergyState::Normal;
    else if (fatigueScore < 75.0)
        state.energy = EnergyState::Fatigued;
    else
        state.energy = EnergyState::Exhausted;

    // ---------- SocialState ----------
    // 신뢰, 통제감, 감정 각성, 의존성 반영
    double coopScore = drives.interpersonalTrust + drives.motivation * 0.5 - drives.SenseOfControl * 0.3 + traits.dependency * 0.2;
    double withdrawScore = 100 - drives.interpersonalTrust + drives.fatigue * 0.5 + traits.rigidity * 0.3;

    if (coopScore > 65.0)
        state.social = SocialState::Cooperative;
    else if (withdrawScore > 60.0)
        state.social = SocialState::Withdrawn;
    else
        state.social = SocialState::Neutral;

    // ---------- ControlState ----------
    // 통제감, 의존성, 고집, 스트레스 고려
    double dependentScore = traits.dependency * 0.6 + (50 - drives.SenseOfControl) * 0.4;
    double stubbornScore = traits.rigidity * 0.7 + drives.stressLoad * 0.3;

    if (dependentScore > 60.0)
        state.control = ControlState::Dependent;
    else if (stubbornScore > 65.0)
        state.control = ControlState::Stubborn;
    else
        state.control = ControlState::Autonomous;
}

