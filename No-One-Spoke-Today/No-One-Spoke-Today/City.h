#pragma once
#include "pch.h"
#include "data.h"

// 전방 선언
class Human;
enum class TerrainType;

class City
{
public:
	City(const std::vector<std::unique_ptr<Human>>&);
	~City();

	// 비선형 업데이트 (지형/기온 반영)
	void Update(const std::vector<std::unique_ptr<Human>>& humans,
		float temperature, TerrainType terrain);

	// 기존 호환용
	void Update(const std::vector<std::unique_ptr<Human>>& humans);

	void Debug() const;
	const CityMetrics& GetCityMet() const;

	void ModifyMood(int delta);
	void ModifyActivity(int delta);
	void ModifyScarcity(int delta);

	void SetCityMet(const CityMetrics& met);

private:
	// 비선형 효과 계산
	float CalculateExtremeStatePenalty(const std::vector<std::unique_ptr<Human>>& humans) const;
	float CalculateExhaustedPenalty(const std::vector<std::unique_ptr<Human>>& humans) const;
	float CalculateCollectiveMorale(const std::vector<std::unique_ptr<Human>>& humans) const;

	CityMetrics cityMet;

};

