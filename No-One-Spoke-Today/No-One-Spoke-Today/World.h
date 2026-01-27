#pragma once
#include "pch.h"
#include "City.h"
#include "Human.h"


class World
{
public:
	World();

	void Update(float);
	void Display() const;
	void Debug();
private:
	struct Event {

	};
	std::unique_ptr<City> city;
	std::vector<std::unique_ptr<Human>> humans;
	int currentDay{};
	std::deque<Event> eventQueue;
};

