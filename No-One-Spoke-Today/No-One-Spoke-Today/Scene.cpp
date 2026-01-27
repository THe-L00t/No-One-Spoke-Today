#include "Scene.h"
#include "Toolkit.h"

void TitleScene::Enter(std::unique_ptr<World>&)
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
		else if (option == 1) RequestSceneChange("save");
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

void GameScene::Enter(std::unique_ptr<World>& w)
{
	if (not w) w = std::make_unique<World>();
		
	world = w.get(); 
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

void MenuScene::Enter(std::unique_ptr<World>&)
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

void MenuScene::sHandleInput(char input)
{
	
}

void SaveScene::Enter(std::unique_ptr<World>& w)
{
	if (not w) saveAble = false;
	else {
		saveAble = true;
		world = w.get();
	}
	LoadMeta();
}

void SaveScene::Update(float deltaTime)
{
	foundSave[0] = {};
	foundSave[1] = {};
	foundSave[2] = {};

	foundSave[0] = saveList[pageOffset];
	if(pageOffset + 1 <= saveList.size()) foundSave[1] = saveList[pageOffset+1];
	if(pageOffset + 2 <= saveList.size()) foundSave[2] = saveList[pageOffset + 2];
}

void SaveScene::Display()
{
	gotoxy(0, 0);
	if (saveList.size() == 0) {
		std::cout << "-------------------------------------" << std::endl;
		std::cout << "        세이브 파일이 없습니다.        " << std::endl;
		std::cout << "-------------------------------------" << std::endl;
	}
	else {
		std::cout << std::endl << std::endl << std::endl;
	}
	for (const auto& f : foundSave) {
		std::println("        |{:>2} {:<15}|", f.slotNum, f.worldName);
		std::println("        |{:16}일|", f.days);
		std::cout << std::endl;
	}
	std::cout << "<이전    S:저장      L:로드     다음>" << std::endl;
}

void SaveScene::Exit()
{
	SaveMeta();
}

void SaveScene::HandleInput(char input)
{
}

void SaveScene::sHandleInput(char input)
{
	switch (input) {
		//case 72: if (option not_eq 0) option -= 1; break;		// 위
		//case 80: if (option not_eq 2) option += 1; break;		// 아래
	case 77: if (pageOffset + 3 <= saveList.size()) pageOffset += 3;; break;		//오
	case 75: if(pageOffset > 2) pageOffset-=3; break;		//왼
	}
	Display();
}

void SaveScene::LoadMeta()
{
	std::ifstream in{ "data/savefile", std::ios::binary };
	if (not in) {
		return;
	}
	MetaData d;
	while (in.read(reinterpret_cast<char*>(&d), sizeof(MetaData))) {
		saveList.push_back(d);
	}

}

void SaveScene::SaveMeta()
{
	std::ofstream out{ "data/savefile", std::ios::binary };
	for (auto& data : saveList) {
		out.write(reinterpret_cast<char*>(&data), sizeof(MetaData));
	}
}

void SaveScene::LoadWorld()
{
}

void SaveScene::SaveWorld()
{
}
