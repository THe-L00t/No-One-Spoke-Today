#include "Scene.h"
#include "Toolkit.h"
#include <iomanip>

std::string Scene::oldScene;

void TitleScene::Enter(std::unique_ptr<World>&)
{
	LoadText(title, "title.txt");
	LoadText(intro, "intro.txt");
	system("cls");

	if (firstVisit) {
		typewriter_print(title, 20);
		std::string menu[3]{ " 새로하기 ", " 이어하기 ", " 종료하기 " };
		for (size_t i = 0; i < 3; i++)
		{
			if (option == i) std::cout << "            >";
			else std::cout << "              ";
			typewriter_print(menu[i], 20);
		}
		firstVisit = false;
	}
	else {
		Display();
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

	// 월드가 바뀌면 대사 로그 클리어
	if (world != w.get()) {
		dayLog.clear();
	}

	world = w.get();
	InitDayMessages();

	if (!dialoguesLoaded) {
		LoadDialogueFiles();
		dialoguesLoaded = true;
	}

	lastDay = world->GetCurrentDay();
	dayStartDisplayed = false;
	waitingForChoice = false;
	eventDisplayed = false;
	statusUpdateTimer = 0.f;
	dialogueTimer = 0.f;
	nextDialogueInterval = GetRandomDialogueInterval();
}

void GameScene::Update(float deltaTime)
{
	if (not world) return;

	// 이벤트 선택 대기 중이면 시뮬레이션 일시정지
	if (!waitingForChoice) {
		world->Update(deltaTime);
	}

	int today = world->GetCurrentDay();
	if (today != lastDay) {
		// 하루 전환 연출
		if (lastDay >= 0 && !dayTransitionShown) {
			DisplayDayTransition();
			dayTransitionShown = true;
		}
		lastDay = today;
		dayLog.clear();
		dayStartDisplayed = false;
		eventDisplayed = false;
		dayTransitionShown = false;
		dialogueTimer = 0.f;
	}

	// 하루 시작 문구 표시
	if (!dayStartDisplayed) {
		DisplayDayStart();
		// 저장된 대사들 다시 출력
		for (const auto& d : dayLog) {
			std::println("    \"{}\"\n", d);
		}
		dayStartDisplayed = true;
	}

	// 이벤트 체크 및 표시
	EventManager* em = world->GetEventManager();
	if (em && em->HasPendingPlayerEvent()) {
		if (!waitingForChoice) {
			waitingForChoice = true;
			eventDisplayed = false;
		}
		if (!eventDisplayed) {
			DisplayEvent();
			eventDisplayed = true;
		}
	}
	else {
		// 이벤트가 없으면 시민 대사 표시
		if (waitingForChoice) {
			waitingForChoice = false;
			system("cls");
			DisplayDayStart();
			// 저장된 대사들 다시 출력
			for (const auto& d : dayLog) {
				std::println("    \"{}\"", d);
			}
		}

		statusUpdateTimer += deltaTime;
		if (statusUpdateTimer >= STATUS_UPDATE_INTERVAL) {
			statusUpdateTimer = 0.f;
			DisplayStatus();
		}

		dialogueTimer += deltaTime;
		if (dialogueTimer >= nextDialogueInterval) {
			dialogueTimer = 0.f;
			nextDialogueInterval = GetRandomDialogueInterval();
			DisplayCitizenDialogue();
		}
	}
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
	// 이벤트 선택 처리
	if (waitingForChoice && world) {
		EventManager* em = world->GetEventManager();
		if (em && em->HasPendingPlayerEvent()) {
			const ActiveEvent* event = em->GetPendingPlayerEvent();
			int choiceIndex = -1;

			if (input >= '1' && input <= '9') {
				choiceIndex = input - '1';
			}

			if (choiceIndex >= 0 && choiceIndex < static_cast<int>(event->choices.size())) {
				// 선택한 텍스트 저장 (ApplyPlayerChoice 후 event가 사라지므로)
				std::string chosenText = event->choices[choiceIndex].text;

				// 선택 적용
				em->ApplyPlayerChoice(choiceIndex, *world->GetCity(), world->GetHumansVector());

				waitingForChoice = false;
				eventDisplayed = false;
				system("cls");
				DisplayDayStart();

				// 선택 결과 표시
				std::cout << std::endl;
				std::cout << "    >> 선택 완료: " << chosenText << std::endl;
				std::cout << std::endl;
				return;
			}
		}
	}

	switch (input) {
	case 27:					//esc
		RequestSceneChange("menu");
		break;
	case 's':
	case 'S':
		// 빠른 상태 확인
		DisplayStatus();
		break;
	}
}

void GameScene::sHandleInput(char input)
{
}

void MenuScene::Enter(std::unique_ptr<World>&)
{
	system("cls");
	LoadText(menu, "menu.txt");

	if (firstVisit) {
		typewriter_print(menu, 20);
		std::string menuItems[3]{ " 계속하기 ", " 저장하기 ", " 처음으로 " };
		for (size_t i = 0; i < 3; i++)
		{
			if (option == i) std::cout << "            >";
			else std::cout << "              ";
			typewriter_print(menuItems[i], 20);
		}
		firstVisit = false;
	}
	else {
		Display();
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
	Update(0);  // foundSave 초기화
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
		if (saveAble) {
			// 이미 저장된 슬롯인지 확인
			if (!foundSave[option].worldName.empty() && foundSave[option].worldName != "비어있음") {
				std::cout << "이미 저장된 데이터가 있습니다. 덮어쓰시겠습니까? (y/n): ";
				char confirm = _getch();
				if (confirm != 'y' && confirm != 'Y') {
					std::cout << "취소됨" << std::endl;
					break;
				}
			}
			SaveWorld();
			system("cls");
			Display();
		}
		break;
	case 'l':
		// 빈 슬롯에서 로드 방지
		if (foundSave[option].worldName.empty() || foundSave[option].worldName == "비어있음") {
			std::cout << "해당 슬롯에 저장된 데이터가 없습니다." << std::endl;
			break;
		}
		std::cout << "'" << foundSave[option].worldName << "' 을(를) 로드하시겠습니까? (y/n): ";
		{
			char confirm = _getch();
			if (confirm != 'y' && confirm != 'Y') {
				std::cout << "취소됨" << std::endl;
				break;
			}
		}
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
	if (!in.is_open()) {
		std::cout << "[DEBUG] data/savefile 파일을 열 수 없습니다." << std::endl;
		return;
	}
	while (in.good() && in.peek() != EOF) {
		MetaData d;
		in.read(reinterpret_cast<char*>(&d.slotNum), sizeof(d.slotNum));
		if (!in.good()) break;
		uint32_t nameLen;
		in.read(reinterpret_cast<char*>(&nameLen), sizeof(nameLen));
		if (!in.good() || nameLen > 1000) break;
		d.worldName.resize(nameLen);
		in.read(d.worldName.data(), nameLen);
		if (!in.good()) break;
		in.read(reinterpret_cast<char*>(&d.days), sizeof(d.days));
		if (in.good()) saveList.push_back(d);
	}
	in.close();
	std::cout << "[DEBUG] 로드된 세이브 수: " << saveList.size() << std::endl;
}

void SaveScene::SaveMeta()
{
	std::cout << "[DEBUG] SaveMeta 호출, saveList.size() = " << saveList.size() << std::endl;
	CreateDirectoryA("data", NULL);
	std::ofstream out{ "data/savefile", std::ios::binary | std::ios::trunc };
	if (!out.is_open()) {
		std::cout << "[DEBUG] data/savefile 쓰기 실패" << std::endl;
		return;
	}
	for (auto& data : saveList) {
		out.write(reinterpret_cast<const char*>(&data.slotNum), sizeof(data.slotNum));
		uint32_t nameLen = static_cast<uint32_t>(data.worldName.size());
		out.write(reinterpret_cast<const char*>(&nameLen), sizeof(nameLen));
		out.write(data.worldName.data(), nameLen);
		out.write(reinterpret_cast<const char*>(&data.days), sizeof(data.days));
	}
	out.close();
	std::cout << "[DEBUG] SaveMeta 완료" << std::endl;
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


// ========== GameScene 하루 시작 관련 함수 ==========
void GameScene::InitDayMessages()
{
	// 첫 번째 줄: 하루 시작 인사
	dayMessages["greeting"] = {
		"오늘 하루가 시작됩니다.",
		"새로운 하루가 밝았습니다.",
		"또 하루가 시작되었습니다.",
		"아침이 찾아왔습니다.",
		"눈을 뜨니 또 하루입니다.",
		"어둠이 걷히고 하루가 시작됩니다.",
		"시민들이 잠에서 깨어납니다.",
		"엔진 소리와 함께 하루가 열립니다.",
		"강철 도시에 아침이 왔습니다.",
		"살아있는 자들의 하루가 시작됩니다."
	};

	// 두 번째 줄: 바퀴/이동 관련
	dayMessages["wheel"] = {
		"바퀴는 멈추지 않습니다.",
		"도시는 오늘도 굴러갑니다.",
		"멈춤은 곧 죽음입니다.",
		"우리는 계속 나아갑니다.",
		"쉬는 것은 사치입니다.",
		"엔진이 쉴 틈은 없습니다.",
		"바퀴 소리가 오늘의 심장박동입니다.",
		"도시의 맥박이 뛰고 있습니다.",
		"움직임만이 생존입니다.",
		"지평선을 향해 굴러갑니다.",
		"먼지 폭풍 너머로 나아갑니다.",
		"오늘도 죽음을 뒤로하고 달립니다.",
		"살기 위해 멈출 수 없습니다.",
		"정지는 선택지에 없습니다.",
		"강철 심장은 쉬지 않습니다."
	};
}

std::string GameScene::GetRandomMessage(const std::string& category)
{
	auto it = dayMessages.find(category);
	if (it == dayMessages.end() || it->second.empty()) {
		return "";
	}

	std::uniform_int_distribution<size_t> dist(0, it->second.size() - 1);
	return it->second[dist(rng)];
}

std::string GameScene::GetMoodText(int mood)
{
	if (mood >= 8000) return "희망참";
	if (mood >= 6000) return "평온함";
	if (mood >= 4000) return "불안함";
	if (mood >= 2000) return "침울함";
	return "절망적";
}

void GameScene::DisplayDayStart()
{
	if (!world) return;

	system("cls");

	int month = world->GetMonth();
	int dayOfMonth = world->GetDay();
	int totalDay = world->GetCurrentDay();
	int population = world->GetHumansSize();
	const CityMetrics& cm = world->GetCity()->GetCityMet();

	std::string moodText = GetMoodText(cm.mood);
	std::string activityText = GetActivityText(cm.activity);
	std::string scarcityText = GetScarcityText(cm.scarcity);

	int avgStress = CalculateAverageStress();
	int avgFatigue = CalculateAverageFatigue();

	// 하루 단위로 일지 메시지 설정
	currentGreeting = GetRandomMessage("greeting");
	currentWheel = GetRandomMessage("wheel");

	// 대시보드 스타일 출력 (std::print 사용)
	std::println("");
	std::println("  ╔════════════════════════════════════════════════╗");
	std::println("  ║  2156년 {:>2}월 {:>2}일                      Day {:>3} ║", month, dayOfMonth, totalDay+1);
	std::println("  ╠════════════════════════════════════════════════╣");
	std::println("  ║  [도시 상태]             [시민 상태]           ║");
	std::println("  ║  > 분위기: {:<8}     > 인구: {:>4}명         ║", moodText, population);
	std::println("  ║  > 활동량: {:<10}   > 평균 스트레스: {:>3}%  ║", activityText, avgStress / 100);
	std::println("  ║  > 결핍도: {:<8}     > 평균 피로도: {:>3}%    ║", scarcityText, avgFatigue / 100);
	std::println("  ╠════════════════════════════════════════════════╣");
	std::println("  ║  [오늘의 일지]                                 ║");
	std::println("  ║  {:44}  ║", currentGreeting);
	std::println("  ║  {:44}  ║", currentWheel);
	std::println("  ╠════════════════════════════════════════════════╣");
	std::println("  ║  [진행률] ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░   0%  ║");
	std::println("  ║            0분 /  8분                          ║");
	std::println("  ╚════════════════════════════════════════════════╝");
	std::println("    [ESC] 메뉴  [S] 상세 상태");
	std::println("");
}

void GameScene::DisplayEvent()
{
	if (!world) return;

	EventManager* em = world->GetEventManager();
	if (!em || !em->HasPendingPlayerEvent()) return;

	const ActiveEvent* event = em->GetPendingPlayerEvent();
	if (!event) return;

	system("cls");

	// 타자기 스타일 (옵션 C) - 이벤트 제목 강조
	std::cout << std::endl;
	std::cout << std::endl;
	typewriter_print("  ...전방에서 이상 징후가 감지되었습니다.", 15);
	std::cout << std::endl;
	std::this_thread::sleep_for(std::chrono::milliseconds(300));

	// 이벤트 제목 강조 (박스로 감싸기)
	std::cout << "  ┌────────────────────────────────────────┐" << std::endl;
	std::cout << "  │";
	// 제목 중앙 정렬
	int titleLen = event->name.length();
	int padding = (40 - titleLen) / 2;
	for (int i = 0; i < padding; ++i) std::cout << " ";
	std::cout << ">> " << event->name << " <<";
	for (int i = 0; i < 40 - padding - titleLen - 6; ++i) std::cout << " ";
	std::cout << "│" << std::endl;
	std::cout << "  └────────────────────────────────────────┘" << std::endl;

	std::cout << std::endl;
	typewriter_print("  " + event->description, 12);
	std::cout << std::endl;
	std::this_thread::sleep_for(std::chrono::milliseconds(200));

	std::cout << std::endl;
	typewriter_print("  어떻게 하시겠습니까?", 20);
	std::cout << std::endl;

	// 선택지 표시
	for (size_t i = 0; i < event->choices.size(); ++i) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		if (i == 0) {
			std::cout << "  > " << (i + 1) << ". " << event->choices[i].text << std::endl;
		}
		else {
			std::cout << "    " << (i + 1) << ". " << event->choices[i].text << std::endl;
		}
	}

	std::cout << std::endl;
	std::cout << "  _" << std::endl;
}

void GameScene::DisplayStatus()
{
	if (!world) return;

	system("cls");

	// 대시보드 상단 (고정 부분)
	int month = world->GetMonth();
	int dayOfMonth = world->GetDay();
	int totalDay = world->GetCurrentDay();
	int population = world->GetHumansSize();
	const CityMetrics& cm = world->GetCity()->GetCityMet();

	std::string moodText = GetMoodText(cm.mood);
	std::string activityText = GetActivityText(cm.activity);
	std::string scarcityText = GetScarcityText(cm.scarcity);
	int avgStress = CalculateAverageStress();
	int avgFatigue = CalculateAverageFatigue();

	float accTime = world->GetAccumulatedTime();
	float ratio = accTime / 480.0f;
	std::string progressBar = GetProgressBar(ratio, 30);
	int minutes = static_cast<int>(accTime / 60.0f);
	int totalMinutes = 8;

	std::println("");
	std::println("  ╔════════════════════════════════════════════════╗");
	std::println("  ║  2156년 {:>2}월 {:>2}일                      Day {:>3} ║", month, dayOfMonth, totalDay+1);
	std::println("  ╠════════════════════════════════════════════════╣");
	std::println("  ║  [도시 상태]             [시민 상태]           ║");
	std::println("  ║  > 분위기: {:<8}     > 인구: {:>4}명         ║", moodText, population);
	std::println("  ║  > 활동량: {:<10}   > 평균 스트레스: {:>3}%  ║", activityText, avgStress / 100);
	std::println("  ║  > 결핍도: {:<8}     > 평균 피로도: {:>3}%    ║", scarcityText, avgFatigue / 100);
	std::println("  ╠════════════════════════════════════════════════╣");
	std::println("  ║  [오늘의 일지]                                 ║");
	std::println("  ║  {:44}  ║", currentGreeting);
	std::println("  ║  {:44}  ║", currentWheel);
	std::println("  ╠════════════════════════════════════════════════╣");
	std::println("  ║  [진행률] {} {:>3}%  ║", progressBar, static_cast<int>(ratio * 100));
	std::println("  ║           {:>2}분 / {:>2}분                          ║", minutes, totalMinutes);
	std::println("  ╚════════════════════════════════════════════════╝");
	std::println("    [ESC] 메뉴  [S] 상세 상태");
	std::println("");

	// 저장된 대사들 다시 출력
	for (const auto& d : dayLog) {
		std::println("    \"{}\"\n", d);
	}
}

void GameScene::DisplayDayTransition()
{
	system("cls");
	std::cout << std::endl;
	std::cout << std::endl;
	std::cout << std::endl;
	std::cout << std::endl;
	typewriter_print("                    ...", 100);
	std::cout << std::endl;
	typewriter_print("               다음 날이 밝았습니다.", 30);
	std::cout << std::endl;
	std::this_thread::sleep_for(std::chrono::milliseconds(800));
}

std::string GameScene::GetProgressBar(float ratio, int width)
{
	ratio = std::clamp(ratio, 0.0f, 1.0f);
	int filled = static_cast<int>(ratio * width);
	std::string bar;
	for (int i = 0; i < filled; ++i) bar += "█";
	for (int i = filled; i < width; ++i) bar += "░";
	return bar;
}

int GameScene::CalculateAverageStress()
{
	if (!world || world->GetHumansSize() == 0) return 0;

	int total = 0;
	int count = world->GetHumansSize();
	for (int i = 0; i < count; ++i) {
		total += world->GetHumans(i)->GetStressLoad();
	}
	return total / count;
}

int GameScene::CalculateAverageFatigue()
{
	if (!world || world->GetHumansSize() == 0) return 0;

	int total = 0;
	int count = world->GetHumansSize();
	for (int i = 0; i < count; ++i) {
		total += world->GetHumans(i)->GetFatigue();
	}
	return total / count;
}

std::string GameScene::GetActivityText(int activity)
{
	if (activity >= 8000) return "매우 활발";
	if (activity >= 6000) return "활발함";
	if (activity >= 4000) return "보통";
	if (activity >= 2000) return "침체";
	return "정지 상태";
}

std::string GameScene::GetScarcityText(int scarcity)
{
	if (scarcity >= 8000) return "심각한 부족";
	if (scarcity >= 6000) return "부족함";
	if (scarcity >= 4000) return "보통";
	if (scarcity >= 2000) return "여유";
	return "풍족함";
}

void GameScene::LoadDialogueFiles()
{
	// 정신 상태별 대사 파일 로드
	// ArousalState
	dialogues["Calm"] = loadSentences("data/dialogues/Calm.txt");
	dialogues["Tense"] = loadSentences("data/dialogues/Tense.txt");
	dialogues["Irritable"] = loadSentences("data/dialogues/Irritable.txt");
	dialogues["Hostile"] = loadSentences("data/dialogues/Hostile.txt");

	// SocialState
	dialogues["Neutral"] = loadSentences("data/dialogues/Neutral.txt");
	dialogues["Cooperative"] = loadSentences("data/dialogues/Cooperative.txt");
	dialogues["Withdrawn"] = loadSentences("data/dialogues/Withdrawn.txt");

	// EnergyState
	dialogues["Normal"] = loadSentences("data/dialogues/Normal.txt");
	dialogues["Fatigued"] = loadSentences("data/dialogues/Fatigued.txt");
	dialogues["Exhausted"] = loadSentences("data/dialogues/Exhausted.txt");

	// ControlState
	dialogues["Autonomous"] = loadSentences("data/dialogues/Autonomous.txt");
	dialogues["Dependent"] = loadSentences("data/dialogues/Dependent.txt");
	dialogues["Stubborn"] = loadSentences("data/dialogues/Stubborn.txt");

	// 성향 기반
	dialogues["Rationality"] = loadSentences("data/dialogues/Rationality.txt");
	dialogues["Aggressiveness"] = loadSentences("data/dialogues/Aggressiveness.txt");
	dialogues["Planning"] = loadSentences("data/dialogues/Planning.txt");
	dialogues["Dependency"] = loadSentences("data/dialogues/Dependency.txt");
	dialogues["Rigidity"] = loadSentences("data/dialogues/Rigidity.txt");
	dialogues["Emotional Sensitivity"] = loadSentences("data/dialogues/Emotional Sensitivity.txt");

	// 누적값 기반 (높음/낮음)
	dialogues["High Stress"] = loadSentences("data/dialogues/High Stress.txt");
	dialogues["Low Stress"] = loadSentences("data/dialogues/Low Stress.txt");
	dialogues["High Fatigue"] = loadSentences("data/dialogues/High Fatigue.txt");
	dialogues["Low Fatigue"] = loadSentences("data/dialogues/Low Fatigue.txt");
	dialogues["High Motivation"] = loadSentences("data/dialogues/High Motivation.txt");
	dialogues["Low Motivation"] = loadSentences("data/dialogues/Low Motivation.txt");
	dialogues["High Arousal"] = loadSentences("data/dialogues/High Arousal.txt");
	dialogues["Low Arousal"] = loadSentences("data/dialogues/Low Arousal.txt");
	dialogues["High Cognitive"] = loadSentences("data/dialogues/High Cognitive.txt");
	dialogues["Low Cognitive"] = loadSentences("data/dialogues/Low Cognitive.txt");
	dialogues["High Trust"] = loadSentences("data/dialogues/High Trust.txt");
	dialogues["Low Trust"] = loadSentences("data/dialogues/Low Trust.txt");
	dialogues["High Safety"] = loadSentences("data/dialogues/High Safety.txt");
	dialogues["Low Safety"] = loadSentences("data/dialogues/Low Safety.txt");
	dialogues["High Control"] = loadSentences("data/dialogues/High Control.txt");
	dialogues["Low Control"] = loadSentences("data/dialogues/Low Control.txt");
}

std::vector<std::string> GameScene::GetMatchingDialogueKeys(Human* h)
{
	std::vector<std::string> keys;

	// 상태 기반
	switch (h->GetArousal()) {
	case ArousalState::Calm: keys.push_back("Calm"); break;
	case ArousalState::Tense: keys.push_back("Tense"); break;
	case ArousalState::Irritable: keys.push_back("Irritable"); break;
	case ArousalState::Hostile: keys.push_back("Hostile"); break;
	}
	switch (h->GetSocial()) {
	case SocialState::Neutral: keys.push_back("Neutral"); break;
	case SocialState::Cooperative: keys.push_back("Cooperative"); break;
	case SocialState::Withdrawn: keys.push_back("Withdrawn"); break;
	}
	switch (h->GetEnergy()) {
	case EnergyState::Normal: keys.push_back("Normal"); break;
	case EnergyState::Fatigued: keys.push_back("Fatigued"); break;
	case EnergyState::Exhausted: keys.push_back("Exhausted"); break;
	}
	switch (h->GetControl()) {
	case ControlState::Autonomous: keys.push_back("Autonomous"); break;
	case ControlState::Dependent: keys.push_back("Dependent"); break;
	case ControlState::Stubborn: keys.push_back("Stubborn"); break;
	}

	// 누적값 기반 (극단적인 경우만)
	if (h->GetStressLoad() > 7000) keys.push_back("High Stress");
	else if (h->GetStressLoad() < 3000) keys.push_back("Low Stress");

	if (h->GetFatigue() > 7000) keys.push_back("High Fatigue");
	else if (h->GetFatigue() < 3000) keys.push_back("Low Fatigue");

	if (h->GetMotivation() > 7000) keys.push_back("High Motivation");
	else if (h->GetMotivation() < 3000) keys.push_back("Low Motivation");

	if (h->GetEmotionalArousal() > 7000) keys.push_back("High Arousal");
	else if (h->GetEmotionalArousal() < 3000) keys.push_back("Low Arousal");

	if (h->GetCognitiveCapacity() > 7000) keys.push_back("High Cognitive");
	else if (h->GetCognitiveCapacity() < 3000) keys.push_back("Low Cognitive");

	if (h->GetInterpersonalTrust() > 7000) keys.push_back("High Trust");
	else if (h->GetInterpersonalTrust() < 3000) keys.push_back("Low Trust");

	if (h->GetSocialSafety() > 7000) keys.push_back("High Safety");
	else if (h->GetSocialSafety() < 3000) keys.push_back("Low Safety");

	if (h->GetSenseOfControl() > 7000) keys.push_back("High Control");
	else if (h->GetSenseOfControl() < 3000) keys.push_back("Low Control");

	return keys;
}

std::string GameScene::GetDialogueForState(const std::string& stateKey)
{
	auto it = dialogues.find(stateKey);
	if (it == dialogues.end() || it->second.empty()) {
		return "";
	}

	std::uniform_int_distribution<size_t> dist(0, it->second.size() - 1);
	return it->second[dist(rng)];
}

float GameScene::GetRandomDialogueInterval()
{
	std::uniform_real_distribution<float> dist(3.0f, 8.0f);
	return dist(rng);
}

void GameScene::DisplayCitizenDialogue()
{
	if (!world || world->GetHumansSize() == 0) return;

	// 랜덤 시민 선택
	std::uniform_int_distribution<int> personDist(0, world->GetHumansSize() - 1);
	Human* speaker = world->GetHumans(personDist(rng));
	if (!speaker) return;

	// 해당 시민의 성향/누적값/상태에 맞는 대사 키들 수집
	std::vector<std::string> keys = GetMatchingDialogueKeys(speaker);
	if (keys.empty()) return;

	// 키 중 랜덤 선택
	std::uniform_int_distribution<size_t> keyDist(0, keys.size() - 1);
	std::string stateKey = keys[keyDist(rng)];

	// 해당 키의 대사 가져오기
	std::string dialogue = GetDialogueForState(stateKey);
	if (dialogue.empty()) {
		dialogue = "...";
	}

	// 대사 저장 및 출력
	dayLog.push_back(dialogue);
	std::println("    \"{}\"", dialogue);
}
