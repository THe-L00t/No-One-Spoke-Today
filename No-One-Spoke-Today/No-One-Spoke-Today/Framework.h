#pragma once
#include "pch.h"
#include "Time.h"
#include "Scene.h"
#include "World.h"

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

	std::unordered_map<std::string, std::unique_ptr<Scene>> scenes;
	Scene* currentScene;

	std::unique_ptr<World> world;
};

