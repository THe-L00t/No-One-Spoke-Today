#pragma once
#include "pch.h"
#include "City.h"
#include "Human.h"


class World
{
public:
	World();

	void Update(float);
	void Debug();
private:
	std::unique_ptr<City> city;
	std::vector<std::unique_ptr<Human>> humans;
};

