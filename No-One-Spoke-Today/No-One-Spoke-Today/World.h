#pragma once
#include "pch.h"
#include "City.h"
#include "Human.h"
#include "Event.h"


class World
{
public:
	World();

	void Update(float);
	void Display() const;
	void Debug();

	EventManager* GetEventManager();
	City* GetCity();

private:
	std::unique_ptr<City> city;
	std::vector<std::unique_ptr<Human>> humans;
	std::unique_ptr<EventManager> eventManager;
	int currentDay{};

	float accumulatedTime{};
	static constexpr float dayDuration = 480.0f; // 8분 = 480초
};

