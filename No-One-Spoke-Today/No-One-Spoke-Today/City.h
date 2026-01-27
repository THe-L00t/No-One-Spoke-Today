#pragma once
#include "pch.h"

class Human;
struct CityMetrics;

class City
{
public:
	City(const std::vector<std::unique_ptr<Human>>&);
	~City();

	void Debug() const;
	const CityMetrics& GetCityMet() const;
private:
	CityMetrics cityMet;

};

