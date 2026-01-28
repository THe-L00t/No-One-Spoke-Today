#include "Scene.h"
#include "Toolkit.h"

std::string Scene::oldScene;

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
	//if(option == 0) typewriter_print(intro);
	oldScene = "start";
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
	system("cls");
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
	oldScene = "play";
}

void GameScene::HandleInput(char input)
{
	switch (input) {
	case 27:					//esc
		RequestSceneChange("menu");
	}
}

void GameScene::sHandleInput(char input)
{
}

void MenuScene::Enter(std::unique_ptr<World>&)
{
	system("cls");
	LoadText(menu, "menu.txt");
	typewriter_print(menu, 20);
	std::string menu[3]{ " 계속하기 ", " 저장하기 ", " 처음으로 " };
	for (size_t i = 0; i < 3; i++)
	{
		if (option == i) std::cout << "            >";
		else std::cout << "              ";
		typewriter_print(menu[i], 20);
	}
}

void MenuScene::Update(float deltaTime)
{
}

void MenuScene::Display()
{
	gotoxy(0, 0);
	std::cout << menu << std::endl;
	std::string menu[3]{ " 계속하기 ", " 저장하기 ", " 처음으로 " };
	for (size_t i = 0; i < 3; i++)
	{
		if (option == i) std::cout << "            >";
		else std::cout << "              ";
		std::cout << menu[i] << std::endl;
	}
}

void MenuScene::Exit()
{
	bool sceneChangeRequested = false;
	oldScene = "menu";
}

void MenuScene::HandleInput(char input)
{
	switch (input) {
	case '\r':
		if (option == 0) RequestSceneChange("play");
		else if (option == 1) RequestSceneChange("save");
		else if (option == 2) RequestSceneChange("start");
	}
	Display();
}

void MenuScene::sHandleInput(char input)
{
	switch (input) {
	case 72: if (option not_eq 0) option -= 1; break;		// 위
	case 80: if (option not_eq 2) option += 1; break;		// 아래
		//case 75: std::cout << "왼쪽\n"; break;		//왼
		//case 77: std::cout << "오른쪽\n"; break;		//오
	}
	Display();
}

void SaveScene::Enter(std::unique_ptr<World>& w)
{
	system("cls");
	if (not w) saveAble = false;
	else {
		saveAble = true;
		world = w.get();
	}
	LoadMeta();
	Display();
}

void SaveScene::Update(float deltaTime)
{
	foundSave[0] = {};
	foundSave[1] = {};
	foundSave[2] = {};

	if(saveList.size() not_eq 0) foundSave[0] = saveList[pageOffset];
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
	for (size_t i = 0; i < 3; ++i)
	{
		if (i == option) {
			std::cout << "       >";
			std::println("|{:>2} {:<15}|", foundSave[i].slotNum, foundSave[i].worldName);
			std::cout << "       >";
			std::println("|{:16}일|", foundSave[i].days);
		}
		else {
			std::println("        |{:>2} {:<15}|", foundSave[i].slotNum, foundSave[i].worldName);
			std::println("        |{:16}일|", foundSave[i].days);
		}
		std::cout << std::endl;
	}
	std::cout << "<이전    S:저장      L:로드     다음>" << std::endl;
}

void SaveScene::Exit()
{
	SaveMeta();
	oldScene = "save";
}

void SaveScene::HandleInput(char input)
{
	switch (input) {
	case 's':

		break;
	case 'l':

		break;
	case 27:
		RequestSceneChange(oldScene);
		break;
	}
}

void SaveScene::sHandleInput(char input)
{
	switch (input) {
	case 72: if (option not_eq 0) option -= 1; break;		// 위
	case 80: if (option not_eq 2) option += 1; break;		// 아래
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
	if (!saveAble || !world) return;

	std::string fileName;
	std::cout << "저장할 이름을 작성해주세요 : ";
	std::cin >> fileName;
	std::ofstream out{ "data/" + fileName + ".bin", std::ios::binary };
	if (!out.is_open()) return;

	// 버전
	uint32_t version = 1;
	out.write(reinterpret_cast<const char*>(&version), sizeof(version));

	// 월드 시간 정보
	int32_t curDay = world->GetCurrentDay();
	int32_t curMonth = world->GetMonth();
	int32_t curDayOfMonth = world->GetDay();
	float accTime = world->GetAccumulatedTime();
	out.write(reinterpret_cast<const char*>(&curDay), sizeof(curDay));
	out.write(reinterpret_cast<const char*>(&curMonth), sizeof(curMonth));
	out.write(reinterpret_cast<const char*>(&curDayOfMonth), sizeof(curDayOfMonth));
	out.write(reinterpret_cast<const char*>(&accTime), sizeof(accTime));

	// 도시 지표
	const CityMetrics& cm = world->GetCity()->GetCityMet();
	int32_t mood = cm.mood;
	int32_t activity = cm.activity;
	int32_t scarcity = cm.scarcity;
	out.write(reinterpret_cast<const char*>(&mood), sizeof(mood));
	out.write(reinterpret_cast<const char*>(&activity), sizeof(activity));
	out.write(reinterpret_cast<const char*>(&scarcity), sizeof(scarcity));

	// 인간 수
	uint32_t humanCount = static_cast<uint32_t>(world->GetHumansSize());
	out.write(reinterpret_cast<const char*>(&humanCount), sizeof(humanCount));

	// 각 인간 데이터
	for (int i = 0; i < static_cast<int>(humanCount); ++i) {
		Human* h = world->GetHumans(i);

		// 이름
		const std::string& hname = h->GetName();
		uint32_t nameLen = static_cast<uint32_t>(hname.size());
		out.write(reinterpret_cast<const char*>(&nameLen), sizeof(nameLen));
		out.write(hname.data(), nameLen);

		// 성별
		uint8_t gender = h->IsMale() ? 1 : 0;
		out.write(reinterpret_cast<const char*>(&gender), sizeof(gender));

		// 성향 6개 (0~100)
		int32_t val;
		val = h->GetRationality();          out.write(reinterpret_cast<const char*>(&val), sizeof(val));
		val = h->GetAggressiveness();       out.write(reinterpret_cast<const char*>(&val), sizeof(val));
		val = h->GetPlanning();             out.write(reinterpret_cast<const char*>(&val), sizeof(val));
		val = h->GetDependency();           out.write(reinterpret_cast<const char*>(&val), sizeof(val));
		val = h->GetRigidity();             out.write(reinterpret_cast<const char*>(&val), sizeof(val));
		val = h->GetEmotionalSensitivity(); out.write(reinterpret_cast<const char*>(&val), sizeof(val));

		// 누적값 8개 (0~10000)
		val = h->GetStressLoad();           out.write(reinterpret_cast<const char*>(&val), sizeof(val));
		val = h->GetEmotionalArousal();     out.write(reinterpret_cast<const char*>(&val), sizeof(val));
		val = h->GetFatigue();              out.write(reinterpret_cast<const char*>(&val), sizeof(val));
		val = h->GetCognitiveCapacity();    out.write(reinterpret_cast<const char*>(&val), sizeof(val));
		val = h->GetInterpersonalTrust();   out.write(reinterpret_cast<const char*>(&val), sizeof(val));
		val = h->GetSocialSafety();         out.write(reinterpret_cast<const char*>(&val), sizeof(val));
		val = h->GetSenseOfControl();       out.write(reinterpret_cast<const char*>(&val), sizeof(val));
		val = h->GetMotivation();           out.write(reinterpret_cast<const char*>(&val), sizeof(val));

		// 정신상태 4개 (enum -> uint8)
		uint8_t st;
		st = static_cast<uint8_t>(h->GetArousal());  out.write(reinterpret_cast<const char*>(&st), sizeof(st));
		st = static_cast<uint8_t>(h->GetSocial());   out.write(reinterpret_cast<const char*>(&st), sizeof(st));
		st = static_cast<uint8_t>(h->GetEnergy());   out.write(reinterpret_cast<const char*>(&st), sizeof(st));
		st = static_cast<uint8_t>(h->GetControl());  out.write(reinterpret_cast<const char*>(&st), sizeof(st));
	}

	// 이벤트 매니저 상태
	world->GetEventManager()->SaveState(out);

	out.close();
	std::cout << "저장 완료: data/" << fileName << ".bin" << std::endl;
}
