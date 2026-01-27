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
    scenes["start"] = new TitleScene();
    scenes["play"] = new GameScene();
    scenes["menu"] = new MenuScene();

}

void Framework::Loop()
{
    startTime = timer.get()->timer;

    currentScene = scenes["start"];
    currentScene->Enter();
    while (true) {
        if (_kbhit()) {
            char input = _getch();

        }

    }
}

void Framework::Destroy()
{
    scenes.clear();
}
