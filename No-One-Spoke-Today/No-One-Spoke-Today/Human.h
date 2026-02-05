#pragma once
#include "pch.h"
#include "data.h"



class Human
{
public:
	Human();

	// 핵심 업데이트 함수
	void UpdateMentalState();
	void UpdateDrive(float deltaTime, const UpdateContext& ctx);

	// 다층 영향 시스템 함수들
	float CalculateSocialContagion(const std::vector<Human*>& regionMembers) const;
	float CalculateSocialBuffer(const std::vector<Human*>& regionMembers,
		bool leaderPresent, int daysSinceLeaderVisit) const;
	float CalculateInteractionEffect() const;
	float CalculateThresholdEffect(float currentValue, float delta) const;
	float ApplyRecoveryAsymmetry(float delta, float currentValue) const;
	float CalculateYerkesDodson() const;
	float CalculateScarcityTrustCollapse(float scarcity) const;
	float CalculateTemperatureEffect(float temperature) const;

	// 성향 민감도 계산
	TraitSensitivity GetTraitSensitivity() const;

	// Getters - Trait
	int GetRationality() const;
	int GetAggressiveness() const;
	int GetPlanning() const;
	int GetDependency() const;
	int GetRigidity() const;
	int GetEmotionalSensitivity() const;
	const Trait& GetTraits() const { return traits; }

	// Getters - Drives
	int GetStressLoad() const;
	int GetEmotionalArousal() const;
	int GetFatigue() const;
	int GetCognitiveCapacity() const;
	int GetInterpersonalTrust() const;
	int GetSocialSafety() const;
	int GetSenseOfControl() const;
	int GetMotivation() const;

	// Getters - MentalState
	ArousalState GetArousal() const;
	SocialState GetSocial() const;
	EnergyState GetEnergy() const;
	ControlState GetControl() const;

	// Modifiers
	void ModifyStressLoad(int delta);
	void ModifyEmotionalArousal(int delta);
	void ModifyFatigue(int delta);
	void ModifyCognitiveCapacity(int delta);
	void ModifyInterpersonalTrust(int delta);
	void ModifySocialSafety(int delta);
	void ModifySenseOfControl(int delta);
	void ModifyMotivation(int delta);

	// Identity
	const std::string& GetName() const;
	bool IsMale() const;
	Region GetRegion() const;

	// Setters
	void SetName(const std::string& n);
	void SetMale(bool m);
	void SetRegion(Region r);
	void SetTraits(const Trait& t);
	void SetDrives(const Drives& d);
	void SetMentalState(const MentalState& ms);

private:
	Trait traits;
	Drives drives;
	MentalState state;

	std::string name;
	bool male;
	Region region{ Region::ResidentialArea1 };	// 소속 구역

};

