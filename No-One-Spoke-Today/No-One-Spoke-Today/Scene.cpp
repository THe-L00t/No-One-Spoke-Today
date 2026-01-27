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
	typewriter_print(intro);
}

void TitleScene::HandleInput()
{
	char input = _getch();
	if (input == 0 || input == 224) { // 방향키 또는 기능키
		char dir = _getch();
		switch (dir) {
		case 72: if(option not_eq 0) option -= 1; break;		// 위
		case 80: if(option not_eq 2) option += 1; break;		// 아래
		case 75: std::cout << "왼쪽\n"; break;		//왼
		case 77: std::cout << "오른쪽\n"; break;		//오
		}
		std::cout << "입력됨" << std::endl;
	}
	//Display();
	//RequestSceneChange("play");
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

void GameScene::HandleInput()
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

void MenuScene::HandleInput()
{
}
