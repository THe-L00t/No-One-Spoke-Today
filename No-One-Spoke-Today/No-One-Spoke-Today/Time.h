#pragma once
#include <chrono>
class Time
{
public:
	static Time* Instance;

public:
	Time()
	{
		if (Instance == nullptr) Instance = this;
		startTime = std::chrono::high_resolution_clock::now();
		prevTime = startTime;
	}
	void Update()
	{
		auto end = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - prevTime);
		deltaTime = static_cast<float>(duration.count()) / 1000;
		timer += deltaTime;
		prevTime = end;
	}

private:
	std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
	std::chrono::time_point<std::chrono::high_resolution_clock> prevTime;
public:
	float timer = 0;
	float deltaTime = 0;
};

