#pragma once
#include "pch.h"
#include "data.h"



class Human
{
public:
	Human();

	void UpdateMentalState();
	void UpdateDrive(float, CityMetrics);

	int GetRationality() const;
	int GetAggressiveness() const;
	int GetPlanning() const;
	int GetDependency() const;
	int GetRigidity() const;
	int GetEmotionalSensitivity() const;
	int GetStressLoad() const;
	int GetEmotionalArousal() const;
	int GetFatigue() const;
	int GetCognitiveCapacity() const;
	int GetInterpersonalTrust() const;
	int GetSocialSafety() const;
	int GetSenseOfControl() const;
	int GetMotivation() const;
	ArousalState GetArousal() const;
	SocialState GetSocial() const;
	EnergyState GetEnergy() const;
	ControlState GetControl() const;

	void ModifyStressLoad(int delta);
	void ModifyEmotionalArousal(int delta);
	void ModifyFatigue(int delta);
	void ModifyCognitiveCapacity(int delta);
	void ModifyInterpersonalTrust(int delta);
	void ModifySocialSafety(int delta);
	void ModifySenseOfControl(int delta);
	void ModifyMotivation(int delta);

	const std::string& GetName() const;
	bool IsMale() const;

private:
	Trait traits;
	Drives drives;
	MentalState state;

	std::string name;
	bool male;
	
};

