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

void Human::UpdateDrive(float deltaTime, const UpdateContext& ctx)
{
    using namespace FormulaConstants;

    // 성향 민감도 계산
    TraitSensitivity sens = GetTraitSensitivity();

    // 구역 환경 가져오기
    const RegionEnvironment& regionEnv = GetRegionEnvironment(ctx.humanRegion);

    // 사회적 효과 계산
    float contagion = CalculateSocialContagion(ctx.regionMembers);
    float buffer = CalculateSocialBuffer(ctx.regionMembers, ctx.leaderPresent, ctx.daysSinceLeaderVisit);
    float interaction = CalculateInteractionEffect();
    float tempEffect = CalculateTemperatureEffect(ctx.temperature);
    float ydEffect = CalculateYerkesDodson();
    float scarcityTrustFactor = CalculateScarcityTrustCollapse(static_cast<float>(ctx.city.scarcity) / 100.0f);

    // 도시 상태 정규화 (0~1)
    float cityMoodNorm = ctx.city.mood / 10000.0f;
    float cityActivityNorm = ctx.city.activity / 10000.0f;
    float cityScarcityNorm = ctx.city.scarcity / 10000.0f;

    // ==================== STRESS ====================
    {
        // 기본 변화: 도시 분위기 나쁠수록 증가
        float baseChange = BASE_STRESS_CHANGE * (1.0f - cityMoodNorm) * 100.0f;

        // 지형 + 구역 + 기온 보정
        float envMod = ctx.terrain.stress * regionEnv.stressRate * tempEffect;

        // 개인 민감도
        float personalDelta = baseChange * envMod * sens.stressSens;

        // 사회적 전염 추가
        float socialDelta = contagion * regionEnv.contagionFactor * 10.0f;

        // 상태 보정
        float stateMod = 1.0f;
        switch (state.arousal) {
        case ArousalState::Calm: stateMod = 0.5f; break;
        case ArousalState::Tense: stateMod = 1.0f; break;
        case ArousalState::Irritable: stateMod = 1.3f; break;
        case ArousalState::Hostile: stateMod = 1.5f; break;
        }

        // 통합
        float totalDelta = (personalDelta + socialDelta) * buffer * interaction * stateMod;

        // 임계점 효과
        totalDelta = CalculateThresholdEffect(static_cast<float>(drives.stressLoad) / 100.0f, totalDelta);

        // 회복 비대칭
        totalDelta = ApplyRecoveryAsymmetry(totalDelta, static_cast<float>(drives.stressLoad) / 100.0f);

        drives.stressLoad = std::clamp(drives.stressLoad + static_cast<int>(totalDelta * deltaTime), 0, 10000);
    }

    // ==================== FATIGUE ====================
    {
        // 기본 변화: 활동성 낮을수록 피로 증가 (쉬지 못함)
        float baseChange = BASE_FATIGUE_CHANGE * (1.0f - cityActivityNorm * 0.5f) * 100.0f;

        // 지형 + 구역 보정
        float envMod = ctx.terrain.fatigue * regionEnv.fatigueRate * tempEffect;

        // 개인 민감도
        float personalDelta = baseChange * envMod * sens.fatigueSens;

        // 에너지 상태 보정
        float stateMod = 1.0f;
        switch (state.energy) {
        case EnergyState::Normal: stateMod = 0.8f; break;
        case EnergyState::Fatigued: stateMod = 1.2f; break;
        case EnergyState::Exhausted: stateMod = 1.5f; break;
        }

        float totalDelta = personalDelta * interaction * stateMod;

        // 구역 회복률 적용 (음수면 회복)
        if (totalDelta < 0) {
            totalDelta *= regionEnv.recoveryRate;
        }

        totalDelta = CalculateThresholdEffect(static_cast<float>(drives.fatigue) / 100.0f, totalDelta);
        totalDelta = ApplyRecoveryAsymmetry(totalDelta, static_cast<float>(drives.fatigue) / 100.0f);

        drives.fatigue = std::clamp(drives.fatigue + static_cast<int>(totalDelta * deltaTime), 0, 10000);
    }

    // ==================== EMOTIONAL AROUSAL ====================
    {
        // 기본 변화: 분위기 나쁠수록 각성 증가
        float baseChange = BASE_AROUSAL_CHANGE * (1.0f - cityMoodNorm) * 100.0f;

        // 지형 + 기온
        float envMod = ctx.terrain.stress * tempEffect;  // 스트레스 지형이 각성에도 영향

        // 개인 민감도
        float personalDelta = baseChange * envMod * sens.arousalSens;

        // 사회적 전염 (감정 각성은 전염이 강함)
        float socialDelta = contagion * regionEnv.contagionFactor * 15.0f;

        float totalDelta = (personalDelta + socialDelta) * buffer;

        totalDelta = CalculateThresholdEffect(static_cast<float>(drives.emotionalArousal) / 100.0f, totalDelta);
        totalDelta = ApplyRecoveryAsymmetry(totalDelta, static_cast<float>(drives.emotionalArousal) / 100.0f);

        drives.emotionalArousal = std::clamp(drives.emotionalArousal + static_cast<int>(totalDelta * deltaTime), 0, 10000);
    }

    // ==================== MOTIVATION ====================
    {
        // 기본 변화: 활동성 높을수록 동기 증가
        float baseChange = BASE_MOTIVATION_CHANGE * (cityActivityNorm - 0.5f) * 100.0f;

        // 지형 + 구역 보정
        float envMod = ctx.terrain.motivation;
        if (regionEnv.motivationRate < 0) {
            baseChange += regionEnv.motivationRate * 50.0f;  // 구역 동기 감소 효과
        }
        else {
            baseChange += regionEnv.motivationRate * 30.0f;  // 구역 동기 증가 효과
        }

        // 개인 민감도 + Yerkes-Dodson
        float personalDelta = baseChange * envMod * sens.motivationSens * ydEffect;

        // 에너지 상태 보정
        if (state.energy == EnergyState::Fatigued) personalDelta *= 0.7f;
        if (state.energy == EnergyState::Exhausted) personalDelta *= 0.4f;

        float totalDelta = personalDelta * buffer;

        totalDelta = CalculateThresholdEffect(static_cast<float>(drives.motivation) / 100.0f, totalDelta);
        totalDelta = ApplyRecoveryAsymmetry(totalDelta, static_cast<float>(drives.motivation) / 100.0f);

        drives.motivation = std::clamp(drives.motivation + static_cast<int>(totalDelta * deltaTime), 0, 10000);
    }

    // ==================== COGNITIVE CAPACITY ====================
    {
        // 기본 변화: 활동성 높을수록 인지 유지
        float baseChange = BASE_COGNITION_CHANGE * (cityActivityNorm - 0.3f) * 100.0f;

        // 지형 + 기온
        float envMod = ctx.terrain.cognition * tempEffect;

        // 개인 민감도 + Yerkes-Dodson
        float personalDelta = baseChange * envMod * sens.cognitionSens * ydEffect;

        // 피로/스트레스 상호작용
        personalDelta *= (1.0f / interaction);  // 상호작용 높으면 인지 저하

        float totalDelta = personalDelta;

        totalDelta = CalculateThresholdEffect(static_cast<float>(drives.cognitiveCapacity) / 100.0f, totalDelta);
        totalDelta = ApplyRecoveryAsymmetry(totalDelta, static_cast<float>(drives.cognitiveCapacity) / 100.0f);

        drives.cognitiveCapacity = std::clamp(drives.cognitiveCapacity + static_cast<int>(totalDelta * deltaTime), 0, 10000);
    }

    // ==================== INTERPERSONAL TRUST ====================
    {
        // 기본 변화: 분위기 좋을수록 신뢰 증가
        float baseChange = BASE_TRUST_CHANGE * (cityMoodNorm - 0.5f) * 100.0f;

        // 구역 보정
        baseChange += regionEnv.trustRate * 30.0f;

        // 결핍-신뢰 붕괴
        if (cityScarcityNorm > 0.6f) {
            baseChange -= scarcityTrustFactor * 20.0f;
        }

        // 개인 민감도
        float personalDelta = baseChange * sens.trustSens;

        // 협력적 동료 비율에 따른 보너스
        int cooperativeCount = 0;
        for (Human* other : ctx.regionMembers) {
            if (other != this && other->GetSocial() == SocialState::Cooperative) {
                cooperativeCount++;
            }
        }
        float coopRatio = ctx.regionMembers.empty() ? 0.0f :
            static_cast<float>(cooperativeCount) / ctx.regionMembers.size();
        personalDelta += coopRatio * 5.0f;

        float totalDelta = personalDelta * buffer;

        totalDelta = CalculateThresholdEffect(static_cast<float>(drives.interpersonalTrust) / 100.0f, totalDelta);
        totalDelta = ApplyRecoveryAsymmetry(totalDelta, static_cast<float>(drives.interpersonalTrust) / 100.0f);

        drives.interpersonalTrust = std::clamp(drives.interpersonalTrust + static_cast<int>(totalDelta * deltaTime), 0, 10000);
    }

    // ==================== SOCIAL SAFETY ====================
    {
        // 기본 변화: 결핍 낮을수록 안전감 증가
        float baseChange = BASE_SAFETY_CHANGE * (1.0f - cityScarcityNorm) * 100.0f;

        // 지형 + 구역 보정
        float envMod = ctx.terrain.safety * regionEnv.safetyRate;

        // 개인 민감도
        float personalDelta = baseChange * envMod * sens.safetySens;

        // 상태 보정
        if (state.control == ControlState::Dependent) personalDelta *= 0.7f;

        float totalDelta = personalDelta * buffer;

        totalDelta = CalculateThresholdEffect(static_cast<float>(drives.socialSafety) / 100.0f, totalDelta);
        totalDelta = ApplyRecoveryAsymmetry(totalDelta, static_cast<float>(drives.socialSafety) / 100.0f);

        drives.socialSafety = std::clamp(drives.socialSafety + static_cast<int>(totalDelta * deltaTime), 0, 10000);
    }

    // ==================== SENSE OF CONTROL ====================
    {
        // 기본 변화: 결핍 낮을수록 통제감 증가
        float baseChange = BASE_CONTROL_CHANGE * (1.0f - cityScarcityNorm) * 100.0f;

        // 리더 방문 효과
        if (ctx.leaderPresent) {
            baseChange += 10.0f;  // 리더 있으면 통제감 증가
        }

        // 개인 민감도
        float personalDelta = baseChange * sens.controlSens;

        // 상태 보정
        if (state.control == ControlState::Stubborn) personalDelta *= 0.7f;  // 고집은 변화에 둔감

        float totalDelta = personalDelta * buffer;

        totalDelta = CalculateThresholdEffect(static_cast<float>(drives.senseOfControl) / 100.0f, totalDelta);
        totalDelta = ApplyRecoveryAsymmetry(totalDelta, static_cast<float>(drives.senseOfControl) / 100.0f);

        drives.senseOfControl = std::clamp(drives.senseOfControl + static_cast<int>(totalDelta * deltaTime), 0, 10000);
    }
}

// ==================== 다층 영향 시스템 함수 구현 ====================

float Human::CalculateSocialContagion(const std::vector<Human*>& regionMembers) const
{
    using namespace FormulaConstants;

    if (regionMembers.empty()) return 0.0f;

    float contagionSum = 0.0f;
    int count = 0;

    for (Human* other : regionMembers) {
        if (other == this) continue;

        // 감정 각성 상태 전염
        switch (other->GetArousal()) {
        case ArousalState::Hostile:
            contagionSum += CONTAGION_HOSTILE;
            break;
        case ArousalState::Irritable:
            contagionSum += CONTAGION_IRRITABLE;
            break;
        default:
            break;
        }

        // 고스트레스 전염
        if (other->GetStressLoad() > 7000) {
            contagionSum += CONTAGION_HIGH_STRESS;
        }

        // 협력적 사람은 음의 전염 (안정화)
        if (other->GetSocial() == SocialState::Cooperative) {
            contagionSum += CONTAGION_COOPERATIVE;
        }

        count++;
    }

    // 집단 크기 보정 (작은 집단일수록 전염 강함)
    float groupSizeFactor = 10.0f / (count + 10.0f);

    // 본인의 전염 저항력 (이성 높으면 저항)
    float resistance = 1.0f - (traits.rationality * 0.005f);

    return (std::max)(0.0f, contagionSum * groupSizeFactor * resistance);
}

float Human::CalculateSocialBuffer(const std::vector<Human*>& regionMembers,
    bool leaderPresent, int daysSinceLeaderVisit) const
{
    using namespace FormulaConstants;

    float buffer = 1.0f;

    // 협력적 동료 비율
    if (!regionMembers.empty()) {
        int cooperativeCount = 0;
        for (Human* other : regionMembers) {
            if (other != this && other->GetSocial() == SocialState::Cooperative) {
                cooperativeCount++;
            }
        }
        float cooperativeRatio = static_cast<float>(cooperativeCount) / regionMembers.size();
        buffer -= cooperativeRatio * BUFFER_COOPERATIVE_MAX;
    }

    // 리더 현재 방문 중
    if (leaderPresent) {
        buffer -= BUFFER_LEADER_PRESENT;
        // 의존성 높은 사람은 리더 효과 더 큼
        buffer -= (traits.dependency - 50) * 0.003f;
    }

    // 리더 오래 부재 시 불안 증가
    if (daysSinceLeaderVisit > 3) {
        buffer += (daysSinceLeaderVisit - 3) * BUFFER_LEADER_ABSENT_RATE;
    }

    return std::clamp(buffer, 0.3f, 1.5f);
}

float Human::CalculateInteractionEffect() const
{
    using namespace FormulaConstants;

    float stress = drives.stressLoad / 10000.0f;
    float fatigue = drives.fatigue / 10000.0f;
    float arousal = drives.emotionalArousal / 10000.0f;
    float motivation = drives.motivation / 10000.0f;

    // 스트레스 × 피로 상호작용
    float stressFatigueInt = 1.0f + (stress * fatigue * INTERACTION_STRESS_FATIGUE);

    // 고각성 + 고스트레스 = 폭발 위험
    float arousalStressInt = 1.0f + (arousal * stress * INTERACTION_AROUSAL_STRESS);

    // 피로 + 저동기 = 무기력 악순환
    float fatigueMotivationInt = 1.0f;
    if (fatigue > 0.6f && motivation < 0.4f) {
        fatigueMotivationInt = 1.0f + ((fatigue - 0.6f) * (0.4f - motivation) * INTERACTION_FATIGUE_MOTIVATION);
    }

    return stressFatigueInt * arousalStressInt * fatigueMotivationInt;
}

float Human::CalculateThresholdEffect(float currentValue, float delta) const
{
    using namespace FormulaConstants;

    // currentValue는 0~100 스케일로 정규화됨

    // 80% 이상: 악화 가속
    if (currentValue > THRESHOLD_HIGH && delta > 0) {
        return delta * (1.0f + (currentValue - THRESHOLD_HIGH) * 0.05f);
    }

    // 20% 이하: 회복 둔화 (이미 좋으면 더 좋아지기 어려움)
    if (currentValue < THRESHOLD_LOW && delta < 0) {
        return delta * 0.5f;
    }

    // 40~60% 구간: 안정 (변화 저항)
    if (currentValue > THRESHOLD_MID_LOW && currentValue < THRESHOLD_MID_HIGH) {
        return delta * 0.8f;
    }

    return delta;
}

float Human::ApplyRecoveryAsymmetry(float delta, float currentValue) const
{
    using namespace FormulaConstants;

    if (delta > 0) {
        // 악화: 기본 속도
        return delta;
    }
    else {
        // 회복: 현재 상태가 나쁠수록 회복도 느림 (악순환)
        float recoveryPenalty = 1.0f - (currentValue / 100.0f) * 0.5f;
        return delta * RECOVERY_RATE * (std::max)(0.3f, recoveryPenalty);
    }
}

float Human::CalculateYerkesDodson() const
{
    using namespace FormulaConstants;

    float arousal = drives.emotionalArousal / 100.0f;  // 0~100

    // 최적 구간 (40~60)
    if (arousal >= YD_OPTIMAL_LOW && arousal <= YD_OPTIMAL_HIGH) {
        return YD_BONUS;  // 10% 보너스
    }

    // 최적에서 멀어질수록 페널티
    float distance = 0.0f;
    if (arousal < YD_OPTIMAL_LOW) {
        distance = YD_OPTIMAL_LOW - arousal;
    }
    else {
        distance = arousal - YD_OPTIMAL_HIGH;
    }

    if (distance < 15.0f) {
        return 1.0f;  // 정상 범위
    }

    return (std::max)(0.6f, 1.0f - (distance - 15.0f) * 0.02f);
}

float Human::CalculateScarcityTrustCollapse(float scarcity) const
{
    using namespace FormulaConstants;

    // scarcity는 0~100 스케일
    if (scarcity < SCARCITY_TRUST_THRESHOLD1) return 1.0f;

    if (scarcity < SCARCITY_TRUST_THRESHOLD2) {
        return 1.0f + (scarcity - SCARCITY_TRUST_THRESHOLD1) * 0.02f;  // 완만한 증가
    }

    // 75% 이상: 급격한 신뢰 붕괴
    float excess = scarcity - SCARCITY_TRUST_THRESHOLD2;
    return 1.0f + (excess * excess) * 0.01f;
}

float Human::CalculateTemperatureEffect(float temperature) const
{
    using namespace FormulaConstants;

    float diff = std::abs(temperature - OPTIMAL_TEMP);

    // 쾌적 범위 내: 영향 없음
    if (diff <= COMFORT_RANGE) {
        return 1.0f;
    }

    // 쾌적 범위 초과: 제곱 함수로 급격히 증가
    float excess = diff - COMFORT_RANGE;
    float modifier = 1.0f + (excess * excess) * 0.003f;

    return (std::min)(modifier, 2.0f);  // 최대 2배
}

TraitSensitivity Human::GetTraitSensitivity() const
{
    return TraitSensitivity::Calculate(traits);
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

const std::string& Human::GetName() const
{
    return name;
}

bool Human::IsMale() const
{
    return male;
}

Region Human::GetRegion() const
{
    return region;
}

void Human::SetName(const std::string& n)
{
    name = n;
}

void Human::SetMale(bool m)
{
    male = m;
}

void Human::SetRegion(Region r)
{
    region = r;
}

void Human::SetTraits(const Trait& t)
{
    traits = t;
}

void Human::SetDrives(const Drives& d)
{
    drives = d;
}

void Human::SetMentalState(const MentalState& ms)
{
    state = ms;
}

