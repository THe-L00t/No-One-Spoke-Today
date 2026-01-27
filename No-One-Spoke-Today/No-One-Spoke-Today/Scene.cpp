#include "Scene.h"
#include "Toolkit.h"

void TitleScene::Enter()
{
	LoadText(title, "title.txt");
	LoadText(intro, "intro.txt");
	system("cls");
	typewriter_print(title, 20);
	std::string menu[3]{ " 새로하기 ", " 이어하기 ", " 종료하기 " };
	for (size_t i = 0; i < 3; i++)
	{
		if (option == i) std::cout << "            >";
		else std::cout << "              ";
		typewriter_print(menu[i], 20);
	}
}

void TitleScene::Update(float deltaTime)
{

}

void TitleScene::Display()
{
	gotoxy(0, 0);
	std::cout << title << std::endl;
	std::string menu[3]{ " 새로하기 ", " 이어하기 ", " 종료하기 " };
	for (size_t i = 0; i < 3; i++)
	{
		if (option == i) std::cout << "            >";
		else std::cout << "              ";
		std::cout << menu[i] << std::endl;
	}
}

void TitleScene::Exit()
{
	sceneChangeRequested = false;
	system("cls");
	if(option == 0) typewriter_print(intro);
}

void TitleScene::HandleInput(char input)
{
	switch (input) {
	case '\r':
		if (option == 0) RequestSceneChange("play");
		else if (option == 1) RequestSceneChange("play");
		else if (option == 2) exit(0);
	}
	Display();

}

void TitleScene::sHandleInput(char input)
{
	switch (input) {
	case 72: if (option not_eq 0) option -= 1; break;		// 위
	case 80: if (option not_eq 2) option += 1; break;		// 아래
		//case 75: std::cout << "왼쪽\n"; break;		//왼
		//case 77: std::cout << "오른쪽\n"; break;		//오
	}
	Display();
}

void GameScene::Enter()
{
	if (not world) {
		world = new World();
	}
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

void GameScene::HandleInput(char input)
{
}

void GameScene::sHandleInput(char input)
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

void MenuScene::HandleInput(char input)
{
}

void MenuScene::sHandleInput(char)
{
}
