#pragma once
#include "pch.h"
class Scene
{
public:
	Scene() = default;
	virtual ~Scene() = default;

	virtual void Enter() = 0;
	virtual void Update() = 0;
	virtual void Display() = 0;
	virtual void Exit() = 0;
	virtual void HandleInput(char) = 0;

	bool IsSceneChangeRequested() const { return sceneChangeRequested; }
	std::string GetNextSceneName() const { return nextSceneName; }

protected:
	void RequestSceneChange(const std::string& sceneName) {
		sceneChangeRequested = true;
		nextSceneName = sceneName;
	}

private:
	bool sceneChangeRequested = false;
	std::string nextSceneName;
};

class TitleScene : public Scene 
{
public:

	void Enter();
	void Update();
	void Display();
	void Exit();
	void HandleInput(char);

private:
	std::string introText;
};

class GameScene : public Scene
{
public:

	void Enter();
	void Update();
	void Display();
	void Exit();
	void HandleInput(char);

private:

};

class MenuScene : public Scene
{
public:

	void Enter();
	void Update();
	void Display();
	void Exit();
	void HandleInput(char);

private:

};