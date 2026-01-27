#include "Framework.h"

Time* Time::Instance = nullptr;

Framework::Framework()
{
	timer = std::make_unique<Time>();
}

Framework::~Framework()
{
	std::cout << " 게임을 종료합니다... " << std::endl;
}

void Framework::Init()
{
    scenes["start"] = std::make_unique<TitleScene>();
    scenes["play"] = std::make_unique<GameScene>();
    scenes["menu"] = std::make_unique<MenuScene>();

}

void Framework::Loop()
{
    startTime = timer.get()->timer;

    currentScene = scenes["start"].get();
    currentScene->Enter();
    while (true) {
        if (_kbhit()) {
            char input = _getch();
            currentScene->HandleInput(input);
        }
        currentScene->Update(timer->deltaTime);
        //currentScene->Display();

        // 씬 전환 체크
        if (currentScene->IsSceneChangeRequested()) {
            std::string nextSceneName = currentScene->GetNextSceneName();
            currentScene->Exit();
            currentScene = scenes[nextSceneName].get();
            currentScene->Enter();
        }
    }
}

void Framework::Destroy()
{
    scenes.clear();
}
