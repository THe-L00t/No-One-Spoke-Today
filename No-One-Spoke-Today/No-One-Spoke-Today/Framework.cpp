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
    scenes["save"] = std::make_unique<SaveScene>();

}

void Framework::Loop()
{
    startTime = timer.get()->timer;
    currentScene = scenes["start"].get();
    currentScene->Enter(world);
    while (true) {
        if (_kbhit()) {
            int input = _getch();
            if (input == 0 || input == 224) { // 방향키 또는 기능키
                int dir = _getch();
                currentScene->sHandleInput(dir);
            }
            else {
                currentScene->HandleInput(input);
            }
        }
        currentScene->Update(timer->deltaTime);
        //currentScene->Display();

        // 씬 전환 체크
        if (currentScene->IsSceneChangeRequested()) {
            std::string nextSceneName = currentScene->GetNextSceneName();
            currentScene->Exit();
            currentScene = scenes[nextSceneName].get();
            currentScene->Enter(world);
        }
    }
}

void Framework::Destroy()
{
    scenes.clear();
}
