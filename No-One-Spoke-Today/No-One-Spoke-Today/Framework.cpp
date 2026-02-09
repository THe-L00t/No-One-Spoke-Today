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
    scenes["ending"] = std::make_unique<EndingScene>();
    scenes["gameover"] = std::make_unique<GameOverScene>();
}

void Framework::Loop()
{
    startTime = timer.get()->timer;
    currentScene = scenes["start"].get();
    currentScene->Enter(world);
    while (true) {
        // 시간 업데이트 (deltaTime 계산)
        timer->Update();

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

        // 씬 전환 체크
        if (currentScene->IsSceneChangeRequested()) {
            std::string nextSceneName = currentScene->GetNextSceneName();
            currentScene->Exit();

            // 엔딩/게임오버 씬일 경우 타입 설정
            if (nextSceneName == "ending" && world) {
                EndingScene* endingScene = dynamic_cast<EndingScene*>(scenes["ending"].get());
                if (endingScene) {
                    endingScene->SetEndingType(world->GetCurrentGameEndState());
                }
            }
            else if (nextSceneName == "gameover" && world) {
                GameOverScene* gameOverScene = dynamic_cast<GameOverScene*>(scenes["gameover"].get());
                if (gameOverScene) {
                    gameOverScene->SetGameOverType(world->GetCurrentGameEndState());
                }
            }

            currentScene = scenes[nextSceneName].get();
            currentScene->Enter(world);

            // 타이틀로 돌아갈 때 월드 리셋
            if (nextSceneName == "start") {
                world.reset();
            }
        }

        // CPU 과부하 방지를 위한 짧은 sleep
        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
    }
}

void Framework::Destroy()
{
    scenes.clear();
}
