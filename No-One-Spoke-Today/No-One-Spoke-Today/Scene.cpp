#include "Scene.h"
#include "Toolkit.h"

void TitleScene::Enter()
{
	LoadText(title, "title.txt");
	LoadText(intro, "intro.txt");
	system("cls");
	typewriter_print(title, 20);
}

void TitleScene::Update(float deltaTime)
{

}

void TitleScene::Display()
{

}

void TitleScene::Exit()
{
	sceneChangeRequested = false;
	system("cls");
	typewriter_print(intro);
}

void TitleScene::HandleInput(char input)
{
	RequestSceneChange("play");
}

void GameScene::Enter()
{

}

void GameScene::Update(float deltaTime)
{

}

void GameScene::Display()
{

}

void GameScene::Exit()
{
	sceneChangeRequested = false;
}

void GameScene::HandleInput(char)
{
}

void MenuScene::Enter()
{
}

void MenuScene::Update(float deltaTime)
{
}

void MenuScene::Display()
{
}

void MenuScene::Exit()
{
	bool sceneChangeRequested = false;
}

void MenuScene::HandleInput(char)
{
}
