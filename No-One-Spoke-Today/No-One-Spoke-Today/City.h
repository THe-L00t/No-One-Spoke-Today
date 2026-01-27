#pragma once
#include "pch.h"
#include "data.h"
class Human;


class City
{
public:
	City(const std::vector<std::unique_ptr<Human>>&);
	~City();

	void Update(const std::vector<std::unique_ptr<Human>>&);
	void Debug() const;
	const CityMetrics& GetCityMet() const;
private:
	CityMetrics cityMet;

};

