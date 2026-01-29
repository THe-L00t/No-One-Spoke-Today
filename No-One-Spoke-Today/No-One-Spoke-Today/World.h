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
	Human* GetHumans(int);
	int GetHumansSize() const;
	std::vector<std::unique_ptr<Human>>& GetHumansVector();
	int GetCurrentDay() const;
	int GetMonth() const;
	int GetDay() const;
	float GetAccumulatedTime() const;

	void SetCurrentDay(int d);
	void SetMonth(int m);
	void SetDay(int d);
	void SetAccumulatedTime(float t);
	void ClearHumans();
	void AddHuman(std::unique_ptr<Human> h);
private:
	int currentDay{};
	int month{ 4 };
	int day{ 12 };

	float accumulatedTime{};
	static constexpr float dayDuration = 480.0f; // 8분 = 480초

	std::unique_ptr<City> city;
	std::vector<std::unique_ptr<Human>> humans;
	std::unique_ptr<EventManager> eventManager;
};

