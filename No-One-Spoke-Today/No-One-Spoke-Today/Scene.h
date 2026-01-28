#pragma once
#include "pch.h"
#include "World.h"

class Scene
{
public:
	Scene() = default;
	virtual ~Scene() = default;

	virtual void Enter(std::unique_ptr<World>&) = 0;
	virtual void Update(float) = 0;
	virtual void Display() = 0;
	virtual void Exit() = 0;
	virtual void HandleInput(char) = 0;
	virtual void sHandleInput(char) = 0;

	bool IsSceneChangeRequested() const { return sceneChangeRequested; }
	std::string GetNextSceneName() const { return nextSceneName; }

protected:
	void RequestSceneChange(const std::string& sceneName) {
		sceneChangeRequested = true;
		nextSceneName = sceneName;
	}
	bool sceneChangeRequested = false;
	static std::string oldScene;

private:
	std::string nextSceneName;
};

class TitleScene : public Scene 
{
public:

	void Enter(std::unique_ptr<World>&);
	void Update(float);
	void Display();
	void Exit();
	void HandleInput(char);
	void sHandleInput(char);

private:
	std::string title;
	std::string intro;
	int option{};
};

class GameScene : public Scene
{
public:

	void Enter(std::unique_ptr<World>&);
	void Update(float);
	void Display();
	void Exit();
	void HandleInput(char);
	void sHandleInput(char);

private:
	World* world{ nullptr };

	std::vector<std::string> dayLog;
	std::unordered_map<std::string, std::vector<std::string>> sentences;
	int lastDay{ -1 };
	bool waitingForChoice{ false };
	float sentenceTimer{ 0.f };
};

class MenuScene : public Scene
{
public:

	void Enter(std::unique_ptr<World>&);
	void Update(float);
	void Display();
	void Exit();
	void HandleInput(char);
	void sHandleInput(char);

private:
	std::string menu;
	int option{};
};

class SaveScene : public Scene
{
public:

	void Enter(std::unique_ptr<World>&);
	void Update(float);
	void Display();
	void Exit();
	void HandleInput(char);
	void sHandleInput(char);

private:
	void LoadMeta();
	void SaveMeta();
	void LoadWorld();
	void SaveWorld();

	struct MetaData {
		unsigned short slotNum{};
		std::string worldName{ "비어있음" };
		int days{};
	};

	World* world{ nullptr };
	std::unique_ptr<World>* worldRef{ nullptr };
	std::vector<MetaData> saveList;
	MetaData foundSave[3]{};
	int pageOffset{};
	bool saveAble{ false };
	int option{};
};