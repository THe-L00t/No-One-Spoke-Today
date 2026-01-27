#pragma once
#include "pch.h"

struct CityMetrics {		// 0~10000
	int mood;
	int activity;
	int scarcity;
};

class City
{
public:
	City(const std::vector<std::unique_ptr<Human>>&);
	~City();

private:
	CityMetrics cityMet;

};

