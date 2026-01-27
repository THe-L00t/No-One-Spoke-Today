#pragma once
#include "pch.h"
#include "Time.h"
#include "Scene.h"

class Framework
{
public:
	Framework();
	~Framework();

	void Init();
	void Loop();
	void Destroy();

private:
	std::unique_ptr<Time> timer;
	float startTime;

	std::unordered_map<std::string, Scene*> scenes;
	Scene* currentScene;
};

