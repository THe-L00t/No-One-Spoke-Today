#include "Scene.h"
#include "Toolkit.h"

void TitleScene::Enter()
{
	LoadText(title, "title.txt");
	LoadText(introText, "intro.txt");
	typewriter_print(title);
}

void TitleScene::Update()
{

}

void TitleScene::Display()
{

}

void TitleScene::Exit()
{

}

void TitleScene::HandleInput(char)
{

}

void GameScene::Enter()
{

}

void GameScene::Update()
{

}

void GameScene::Display()
{

}

void GameScene::Exit()
{

}

void GameScene::HandleInput(char)
{
}

void MenuScene::Enter()
{
}

void MenuScene::Update()
{
}

void MenuScene::Display()
{
}

void MenuScene::Exit()
{
}

void MenuScene::HandleInput(char)
{
}
