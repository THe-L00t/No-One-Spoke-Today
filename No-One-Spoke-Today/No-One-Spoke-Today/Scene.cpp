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
	sceneChangeRequested = false;
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
	worldRef = &w;
	LoadMeta();
	Display();
}

void SaveScene::Update(float deltaTime)
{
	foundSave[0] = {};
	foundSave[1] = {};
	foundSave[2] = {};

	if(pageOffset < saveList.size()) foundSave[0] = saveList[pageOffset];
	if(pageOffset + 1 < saveList.size()) foundSave[1] = saveList[pageOffset+1];
	if(pageOffset + 2 < saveList.size()) foundSave[2] = saveList[pageOffset + 2];
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
		std::cout << "-------------------------------------" << std::endl;
		std::cout << "             세이브 파일           " << std::endl;
		std::cout << "-------------------------------------" << std::endl;
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
	sceneChangeRequested = false;
}

void SaveScene::HandleInput(char input)
{
	switch (input) {
	case 's':
		if (saveAble) SaveWorld();
		break;
	case 'l':
		LoadWorld();
		RequestSceneChange("play");
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
	saveList.clear();
	std::ifstream in{ "data/savefile", std::ios::binary };
	if (not in) {
		return;
	}
	while (in.peek() != EOF) {
		MetaData d;
		in.read(reinterpret_cast<char*>(&d.slotNum), sizeof(d.slotNum));
		uint32_t nameLen;
		in.read(reinterpret_cast<char*>(&nameLen), sizeof(nameLen));
		d.worldName.resize(nameLen);
		in.read(d.worldName.data(), nameLen);
		in.read(reinterpret_cast<char*>(&d.days), sizeof(d.days));
		if (in) saveList.push_back(d);
	}
}

void SaveScene::SaveMeta()
{
	CreateDirectoryA("data", NULL);
	std::ofstream out{ "data/savefile", std::ios::binary };
	for (auto& data : saveList) {
		out.write(reinterpret_cast<const char*>(&data.slotNum), sizeof(data.slotNum));
		uint32_t nameLen = static_cast<uint32_t>(data.worldName.size());
		out.write(reinterpret_cast<const char*>(&nameLen), sizeof(nameLen));
		out.write(data.worldName.data(), nameLen);
		out.write(reinterpret_cast<const char*>(&data.days), sizeof(data.days));
	}
}

void SaveScene::LoadWorld()
{
	// 빈 슬롯 체크
	if (foundSave[option].worldName == "비어있음" || foundSave[option].worldName.empty()) {
		std::cout << "해당 슬롯에 저장된 정보가 없습니다." << std::endl;
		return;
	}

	std::string fileName = foundSave[option].worldName;
	std::ifstream in{ "data/" + fileName + ".bin", std::ios::binary };
	if (!in.is_open()) {
		std::cout << "파일을 열 수 없습니다." << std::endl;
		return;
	}

	// World 생성
	if (!worldRef) return;
	*worldRef = std::make_unique<World>();
	world = worldRef->get();

	// 버전
	uint32_t version;
	in.read(reinterpret_cast<char*>(&version), sizeof(version));

	// 시간 정보
	int32_t curDay, curMonth, curDayOfMonth;
	float accTime;
	in.read(reinterpret_cast<char*>(&curDay), sizeof(curDay));
	in.read(reinterpret_cast<char*>(&curMonth), sizeof(curMonth));
	in.read(reinterpret_cast<char*>(&curDayOfMonth), sizeof(curDayOfMonth));
	in.read(reinterpret_cast<char*>(&accTime), sizeof(accTime));
	world->SetCurrentDay(curDay);
	world->SetMonth(curMonth);
	world->SetDay(curDayOfMonth);
	world->SetAccumulatedTime(accTime);

	// 도시 지표
	int32_t mood, activity, scarcity;
	in.read(reinterpret_cast<char*>(&mood), sizeof(mood));
	in.read(reinterpret_cast<char*>(&activity), sizeof(activity));
	in.read(reinterpret_cast<char*>(&scarcity), sizeof(scarcity));
	CityMetrics cm;
	cm.mood = mood;
	cm.activity = activity;
	cm.scarcity = scarcity;
	world->GetCity()->SetCityMet(cm);

	// 인간 수
	uint32_t humanCount;
	in.read(reinterpret_cast<char*>(&humanCount), sizeof(humanCount));

	// 기존 인간 제거 후 로드
	world->ClearHumans();

	for (uint32_t i = 0; i < humanCount; ++i) {
		auto h = std::make_unique<Human>();

		// 이름
		uint32_t nameLen;
		in.read(reinterpret_cast<char*>(&nameLen), sizeof(nameLen));
		std::string hname(nameLen, '\0');
		in.read(hname.data(), nameLen);
		h->SetName(hname);

		// 성별
		uint8_t gender;
		in.read(reinterpret_cast<char*>(&gender), sizeof(gender));
		h->SetMale(gender == 1);

		// 성향 6개
		Trait t{};
		int32_t val;
		in.read(reinterpret_cast<char*>(&val), sizeof(val)); t.rationality = val;
		in.read(reinterpret_cast<char*>(&val), sizeof(val)); t.aggressiveness = val;
		in.read(reinterpret_cast<char*>(&val), sizeof(val)); t.planning = val;
		in.read(reinterpret_cast<char*>(&val), sizeof(val)); t.dependency = val;
		in.read(reinterpret_cast<char*>(&val), sizeof(val)); t.rigidity = val;
		in.read(reinterpret_cast<char*>(&val), sizeof(val)); t.emotionalSensitivity = val;
		h->SetTraits(t);

		// 누적값 8개
		Drives d{};
		in.read(reinterpret_cast<char*>(&val), sizeof(val)); d.stressLoad = val;
		in.read(reinterpret_cast<char*>(&val), sizeof(val)); d.emotionalArousal = val;
		in.read(reinterpret_cast<char*>(&val), sizeof(val)); d.fatigue = val;
		in.read(reinterpret_cast<char*>(&val), sizeof(val)); d.cognitiveCapacity = val;
		in.read(reinterpret_cast<char*>(&val), sizeof(val)); d.interpersonalTrust = val;
		in.read(reinterpret_cast<char*>(&val), sizeof(val)); d.socialSafety = val;
		in.read(reinterpret_cast<char*>(&val), sizeof(val)); d.senseOfControl = val;
		in.read(reinterpret_cast<char*>(&val), sizeof(val)); d.motivation = val;
		h->SetDrives(d);

		// 정신상태 4개
		MentalState ms{};
		uint8_t st;
		in.read(reinterpret_cast<char*>(&st), sizeof(st)); ms.arousal = static_cast<ArousalState>(st);
		in.read(reinterpret_cast<char*>(&st), sizeof(st)); ms.social = static_cast<SocialState>(st);
		in.read(reinterpret_cast<char*>(&st), sizeof(st)); ms.energy = static_cast<EnergyState>(st);
		in.read(reinterpret_cast<char*>(&st), sizeof(st)); ms.control = static_cast<ControlState>(st);
		h->SetMentalState(ms);

		world->AddHuman(std::move(h));
	}

	// 이벤트 매니저 상태 로드
	world->GetEventManager()->LoadState(in);

	in.close();
	saveAble = true;
	std::cout << "로드 완료: data/" << fileName << ".bin" << std::endl;
}

void SaveScene::SaveWorld()
{
	if (!saveAble || !world) return;

	std::string fileName;
	std::cout << "저장할 이름을 작성해주세요 : ";
	std::cin >> fileName;
	CreateDirectoryA("data", NULL);
	std::ofstream out{ "data/" + fileName + ".bin", std::ios::binary };
	if (not out) return;

	// 버전
	uint32_t version = 1;
	out.write(reinterpret_cast<const char*>(&version), sizeof(version));
	std::cout << "버전 저장";
	// 월드 시간 정보
	int32_t curDay = world->GetCurrentDay();
	int32_t curMonth = world->GetMonth();
	int32_t curDayOfMonth = world->GetDay();
	float accTime = world->GetAccumulatedTime();
	out.write(reinterpret_cast<const char*>(&curDay), sizeof(curDay));
	out.write(reinterpret_cast<const char*>(&curMonth), sizeof(curMonth));
	out.write(reinterpret_cast<const char*>(&curDayOfMonth), sizeof(curDayOfMonth));
	out.write(reinterpret_cast<const char*>(&accTime), sizeof(accTime));
	std::cout << "월드 시간";
	// 도시 지표
	const CityMetrics& cm = world->GetCity()->GetCityMet();
	int32_t mood = cm.mood;
	int32_t activity = cm.activity;
	int32_t scarcity = cm.scarcity;
	out.write(reinterpret_cast<const char*>(&mood), sizeof(mood));
	out.write(reinterpret_cast<const char*>(&activity), sizeof(activity));
	out.write(reinterpret_cast<const char*>(&scarcity), sizeof(scarcity));
	std::cout << "도시 지표";
	// 인간 수
	uint32_t humanCount = static_cast<uint32_t>(world->GetHumansSize());
	out.write(reinterpret_cast<const char*>(&humanCount), sizeof(humanCount));
	std::cout << "인간 수";
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
	std::cout << "인간 저장";
	// 이벤트 매니저 상태
	world->GetEventManager()->SaveState(out);

	out.close();
	std::cout << "저장 완료: data/" << fileName << ".bin" << std::endl;

	// 메타데이터 업데이트
	MetaData meta;
	meta.slotNum = static_cast<unsigned short>(saveList.size() + 1);
	meta.worldName = fileName;
	meta.days = world->GetCurrentDay();
	saveList.push_back(meta);
}
