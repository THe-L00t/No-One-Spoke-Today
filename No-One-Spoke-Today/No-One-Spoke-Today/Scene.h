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
	bool firstVisit{ true };
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
	void DisplayDayStart();
	void DisplayEvent();
	void DisplayStatus();
	void DisplayDayTransition();
	void DisplayCitizenDialogue();
	void InitDayMessages();
	void LoadDialogueFiles();
	std::string GetRandomMessage(const std::string& category);
	std::string GetMoodText(int mood);
	std::string GetActivityText(int activity);
	std::string GetScarcityText(int scarcity);
	std::string GetProgressBar(float ratio, int width = 20);
	std::string GetDialogueForState(const std::string& stateKey);
	int CalculateAverageStress();
	int CalculateAverageFatigue();
	std::vector<std::string> GetMatchingDialogueKeys(Human* h);
	float GetRandomDialogueInterval();

	World* world{ nullptr };

	std::vector<std::string> dayLog;
	std::unordered_map<std::string, std::vector<std::string>> dayMessages;
	std::unordered_map<std::string, std::vector<std::string>> dialogues;  // 상태별 대사
	std::string currentGreeting;  // 하루 단위 고정
	std::string currentWheel;     // 하루 단위 고정
	int lastDay{ -1 };
	bool waitingForChoice{ false };
	bool eventDisplayed{ false };
	float statusUpdateTimer{ 0.f };
	float dialogueTimer{ 0.f };
	float nextDialogueInterval{ 5.0f };
	static constexpr float STATUS_UPDATE_INTERVAL = 3.0f;
	bool dayStartDisplayed{ false };
	bool dayTransitionShown{ false };
	bool firstVisit{ true };
	bool dialoguesLoaded{ false };
	std::default_random_engine rng{ std::random_device{}() };
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
	bool firstVisit{ true };
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