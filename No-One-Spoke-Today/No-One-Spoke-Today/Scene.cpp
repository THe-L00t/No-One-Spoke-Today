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
	showingMoveMenu = false;
	showingRegionMap = false;
	showingCitizenInfo = false;
	showingNavigationMenu = false;
	showingAngleMenu = false;
	citizenInfoMode = 0;
	angleInputBuffer = 0;
	statusUpdateTimer = 0.f;
	dialogueTimer = 0.f;
	nextDialogueInterval = GetRandomDialogueInterval();
}

void GameScene::Update(float deltaTime)
{
	if (not world) return;

	// 메뉴 표시 중이면 업데이트 중지
	if (showingMoveMenu || showingRegionMap || showingCitizenInfo ||
		showingNavigationMenu || showingAngleMenu) {
		return;
	}

	// 이벤트 선택 대기 중이면 시뮬레이션 일시정지
	if (!waitingForChoice) {
		world->Update(deltaTime);
	}

	int today = world->GetCurrentDay();
	if (today != lastDay) {
		// 하루 전환 연출
		if (lastDay >= 0 && !dayTransitionShown) {
			DisplayDayEnd();  // 하루 엔딩 문구 후 바로 다음 날 시작
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
	// 이동 메뉴 처리
	if (showingMoveMenu) {
		HandleRegionMove(input);
		return;
	}

	// 구역 맵 표시 중이면 아무 키나 누르면 닫기
	if (showingRegionMap) {
		showingRegionMap = false;
		system("cls");
		DisplayDayStart();
		for (const auto& d : dayLog) {
			std::println("    \"{}\"", d);
		}
		return;
	}

	// 시민 정보 표시 중
	if (showingCitizenInfo) {
		HandleCitizenInfoInput(input);
		return;
	}

	// 네비게이션 메뉴 표시 중 (조타실)
	if (showingNavigationMenu) {
		HandleNavigationInput(input);
		return;
	}

	// 각도 메뉴 표시 중 (하부구동부)
	if (showingAngleMenu) {
		HandleAngleInput(input);
		return;
	}

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
				std::string eventName = event->name;
				std::string chosenText = event->choices[choiceIndex].text;

				// 이벤트 기록을 dayLog에 추가
				std::string eventLog = "[" + eventName + "] → " + chosenText;
				dayLog.push_back(eventLog);

				// 선택 적용
				em->ApplyPlayerChoice(choiceIndex, *world->GetCity(), world->GetHumansVector());

				waitingForChoice = false;
				eventDisplayed = false;
				system("cls");
				DisplayDayStart();

				// 저장된 대사들 다시 출력
				for (const auto& d : dayLog) {
					std::println("    \"{}\"", d);
				}

				// 선택 결과 표시
				std::println("");
				std::println("    >> 선택 완료: {}", chosenText);
				std::println("");
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
		// 시민 정보 표시
		if (!waitingForChoice) {
			showingCitizenInfo = true;
			citizenInfoMode = 0;  // 성향부터 시작
			DisplayCitizenInfo();
		}
		break;
	case 'm':
	case 'M':
		// 이동 메뉴 열기
		if (!waitingForChoice) {
			showingMoveMenu = true;
			DisplayMoveMenu();
		}
		break;
	case 'r':
	case 'R':
		// 구역 맵 표시
		if (!waitingForChoice) {
			showingRegionMap = true;
			DisplayRegionMap();
		}
		break;
	case 'n':
	case 'N':
		// 네비게이션 메뉴 (조타실에서만)
		if (!waitingForChoice && world->GetPlayerRegion() == Region::Cockpit) {
			showingNavigationMenu = true;
			system("cls");
			DisplayNavigationMenu();
		}
		break;
	case 'a':
	case 'A':
		// 각도 조정 메뉴 (하부구동부에서만)
		if (!waitingForChoice && world->GetPlayerRegion() == Region::LowerDrive) {
			showingAngleMenu = true;
			angleInputBuffer = 0;
			system("cls");
			DisplayAngleMenu();
		}
		break;
	}
}

void GameScene::sHandleInput(char input)
{
	// 시민 정보 표시 중 방향키 처리
	if (showingCitizenInfo) {
		HandleCitizenInfoArrow(input);
		return;
	}

	// 각도 조정 중 방향키 처리
	if (showingAngleMenu && world && world->GetNavigation()) {
		Navigation* nav = world->GetNavigation();
		switch (input) {
		case 75:  // 왼쪽
			nav->SetMovementAngle(nav->GetMovementAngle() - 1);
			DisplayAngleMenu();
			break;
		case 77:  // 오른쪽
			nav->SetMovementAngle(nav->GetMovementAngle() + 1);
			DisplayAngleMenu();
			break;
		}
	}
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

		// 구역 (버전 2 이상에서만)
		if (version >= 2) {
			uint8_t regionVal;
			in.read(reinterpret_cast<char*>(&regionVal), sizeof(regionVal));
			h->SetRegion(static_cast<Region>(regionVal));
		}

		world->AddHuman(std::move(h));
	}

	// 플레이어 위치 로드 (버전 2 이상에서만)
	if (version >= 2) {
		uint8_t playerRegionVal;
		in.read(reinterpret_cast<char*>(&playerRegionVal), sizeof(playerRegionVal));
		world->SetPlayerRegion(static_cast<Region>(playerRegionVal));
	}

	// 이벤트 매니저 상태 로드
	world->GetEventManager()->LoadState(in);

	// 네비게이션 상태 로드 (버전 3 이상에서만)
	if (version >= 3 && world->GetNavigation()) {
		world->GetNavigation()->LoadState(in);
	}

	in.close();
	saveAble = true;
	std::cout << "로드 완료: data/" << fileName << ".bin" << std::endl;
}

void SaveScene::SaveWorld()
{
	if (!saveAble || !world) return;

	// 기존 슬롯에 덮어쓰기인지 확인
	bool isOverwrite = !foundSave[option].worldName.empty() && foundSave[option].worldName != "비어있음";
	int existingSlotIndex = -1;

	if (isOverwrite) {
		// saveList에서 해당 슬롯 찾기
		for (size_t i = 0; i < saveList.size(); ++i) {
			if (saveList[i].slotNum == foundSave[option].slotNum) {
				existingSlotIndex = static_cast<int>(i);
				break;
			}
		}
	}

	std::string fileName;
	std::cout << "저장할 이름을 작성해주세요 : ";
	std::cin >> fileName;
	CreateDirectoryA("data", NULL);

	// 덮어쓰기인 경우 기존 파일 삭제
	if (isOverwrite && existingSlotIndex >= 0) {
		std::string oldFileName = saveList[existingSlotIndex].worldName;
		if (oldFileName != fileName) {
			std::remove(("data/" + oldFileName + ".bin").c_str());
		}
	}

	std::ofstream out{ "data/" + fileName + ".bin", std::ios::binary };
	if (not out) return;

	// 버전 (3: 네비게이션 시스템 추가)
	uint32_t version = 3;
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

		// 구역 (버전 2에서 추가)
		uint8_t regionVal = static_cast<uint8_t>(h->GetRegion());
		out.write(reinterpret_cast<const char*>(&regionVal), sizeof(regionVal));
	}
	std::cout << "인간 저장";

	// 플레이어 위치 저장 (버전 2에서 추가)
	uint8_t playerRegionVal = static_cast<uint8_t>(world->GetPlayerRegion());
	out.write(reinterpret_cast<const char*>(&playerRegionVal), sizeof(playerRegionVal));
	std::cout << "플레이어 위치 저장";

	// 이벤트 매니저 상태
	world->GetEventManager()->SaveState(out);

	// 네비게이션 상태 저장 (버전 3에서 추가)
	if (world->GetNavigation()) {
		world->GetNavigation()->SaveState(out);
		std::cout << "네비게이션 저장";
	}

	out.close();
	std::cout << "저장 완료: data/" << fileName << ".bin" << std::endl;

	// 메타데이터 업데이트
	if (isOverwrite && existingSlotIndex >= 0) {
		// 기존 슬롯 업데이트
		saveList[existingSlotIndex].worldName = fileName;
		saveList[existingSlotIndex].days = world->GetCurrentDay();
	}
	else {
		// 새 슬롯 추가
		MetaData meta;
		meta.slotNum = static_cast<unsigned short>(saveList.size() + 1);
		meta.worldName = fileName;
		meta.days = world->GetCurrentDay();
		saveList.push_back(meta);
	}

	// 화면에 표시되는 foundSave도 업데이트
	Update(0);
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

	// 현재 구역 정보
	Region currentRegion = world->GetPlayerRegion();
	std::string regionName = GetRegionName(currentRegion);
	int regionPopulation = world->GetHumanCountInRegion(currentRegion);

	// 하루 단위로 일지 메시지 설정
	currentGreeting = GetRandomMessage("greeting");
	currentWheel = GetRandomMessage("wheel");

	// 미확인 이벤트가 있는 구역 수
	int unseenCount = static_cast<int>(world->GetUnseenEventRegions().size());
	std::string alertText = unseenCount > 0 ? std::format("(!{})", unseenCount) : "";

	// 대시보드 스타일 출력 (std::print 사용)
	std::println("");
	std::println("  ╔════════════════════════════════════════════════╗");
	std::println("  ║  2156년 {:>2}월 {:>2}일                      Day {:>3} ║", month, dayOfMonth, totalDay);
	std::println("  ╠════════════════════════════════════════════════╣");
	std::println("  ║  [현재 위치] {:<12} (인원: {:>3}명)        ║", regionName, regionPopulation);
	std::println("  ╠════════════════════════════════════════════════╣");
	std::println("  ║  [도시 상태]             [시민 상태]           ║");
	std::println("  ║  > 분위기: {:<8}     > 인구: {:>4}명         ║", moodText, population);
	std::println("  ║  > 활동량: {:<12} > 평균 스트레스: {:>3}%  ║", activityText, avgStress / 100);
	std::println("  ║  > 결핍도: {:<12} > 평균 피로도: {:>3}%    ║", scarcityText, avgFatigue / 100);
	std::println("  ╠════════════════════════════════════════════════╣");
	std::println("  ║  [오늘의 일지]                                 ║");
	std::println("  ║  {:44}  ║", currentGreeting);
	std::println("  ║  {:44}  ║", currentWheel);
	std::println("  ╠════════════════════════════════════════════════╣");
	std::println("  ║  [진행률] ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░   0%  ║");
	std::println("  ║            0분 /  8분                          ║");
	std::println("  ╚════════════════════════════════════════════════╝");

	// 기본 키 + 구역별 특수 키
	std::string keyHints = std::format("[ESC] 메뉴  [S] 상세  [M] 이동 {}  [R] 맵", alertText);

	// 조타실에서는 [N] 항로 추가
	if (currentRegion == Region::Cockpit) {
		keyHints += "  [N] 항로";
	}
	// 하부구동부에서는 [A] 각도 추가
	else if (currentRegion == Region::LowerDrive) {
		keyHints += "  [A] 각도";
	}

	std::println("    {}", keyHints);
	std::println("");
}

void GameScene::DisplayEvent()
{
	if (!world) return;

	EventManager* em = world->GetEventManager();
	if (!em || !em->HasPendingPlayerEvent()) return;

	const ActiveEvent* event = em->GetPendingPlayerEvent();
	if (!event) return;

	// 화면 갱신: 대시보드 + 대사들 + 이벤트 순서로 출력
	system("cls");

	// 대시보드 (간소화 버전)
	int month = world->GetMonth();
	int dayOfMonth = world->GetDay();
	int totalDay = world->GetCurrentDay();
	int population = world->GetHumansSize();
	const CityMetrics& cm = world->GetCity()->GetCityMet();

	std::string moodText = GetMoodText(cm.mood);
	int avgStress = CalculateAverageStress();

	std::println("");
	std::println("  ╔════════════════════════════════════════════════╗");
	std::println("  ║  2156년 {:>2}월 {:>2}일                      Day {:>3} ║", month, dayOfMonth, totalDay );
	std::println("  ║  분위기: {:<8}  인구: {:>4}명  스트레스: {:>3}% ║", moodText, population, avgStress / 100);
	std::println("  ╚════════════════════════════════════════════════╝");

	// 저장된 대사들 출력
	for (const auto& d : dayLog) {
		std::println("    \"{}\"", d);
	}

	// 이벤트 구분선
	std::println("");
	std::println("  ──────────────── [이벤트 발생] ────────────────");
	std::println("");

	// 이벤트 제목 강조 (박스로 감싸기)
	std::println("  ┌────────────────────────────────────────┐");
	std::println("  │{:^40}│", ">> " + event->name + " <<");
	std::println("  └────────────────────────────────────────┘");

	std::println("");
	std::println("  {}", event->description);
	std::println("");

	std::println("  어떻게 하시겠습니까?");
	std::println("");

	// 선택지 표시
	for (size_t i = 0; i < event->choices.size(); ++i) {
		if (i == 0) {
			std::println("  > {}. {}", i + 1, event->choices[i].text);
		}
		else {
			std::println("    {}. {}", i + 1, event->choices[i].text);
		}
	}

	std::println("");
	std::println("  _");
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

	// 현재 구역 정보
	Region currentRegion = world->GetPlayerRegion();
	std::string regionName = GetRegionName(currentRegion);
	int regionPopulation = world->GetHumanCountInRegion(currentRegion);

	float accTime = world->GetAccumulatedTime();
	float ratio = accTime / 480.0f;
	std::string progressBar = GetProgressBar(ratio, 30);
	int minutes = static_cast<int>(accTime / 60.0f);
	int totalMinutes = 8;

	// 미확인 이벤트가 있는 구역 수
	int unseenCount = static_cast<int>(world->GetUnseenEventRegions().size());
	std::string alertText = unseenCount > 0 ? std::format("(!{})", unseenCount) : "";

	std::println("");
	std::println("  ╔════════════════════════════════════════════════╗");
	std::println("  ║  2156년 {:>2}월 {:>2}일                      Day {:>3} ║", month, dayOfMonth, totalDay);
	std::println("  ╠════════════════════════════════════════════════╣");
	std::println("  ║  [현재 위치] {:<12} (인원: {:>3}명)         ║", regionName, regionPopulation);
	std::println("  ╠════════════════════════════════════════════════╣");
	std::println("  ║  [도시 상태]             [시민 상태]           ║");
	std::println("  ║  > 분위기: {:<8}     > 인구: {:>4}명         ║", moodText, population);
	std::println("  ║  > 활동량: {:<12} > 평균 스트레스: {:>3}%  ║", activityText, avgStress / 100);
	std::println("  ║  > 결핍도: {:<12} > 평균 피로도: {:>3}%    ║", scarcityText, avgFatigue / 100);
	std::println("  ╠════════════════════════════════════════════════╣");
	std::println("  ║  [오늘의 일지]                                 ║");
	std::println("  ║  {:44}  ║", currentGreeting);
	std::println("  ║  {:44}  ║", currentWheel);
	std::println("  ╠════════════════════════════════════════════════╣");
	std::println("  ║  [진행률] {} {:>3}%  ║", progressBar, static_cast<int>(ratio * 100));
	std::println("  ║           {:>2}분 / {:>2}분                          ║", minutes, totalMinutes);
	std::println("  ╚════════════════════════════════════════════════╝");

	// 기본 키 + 구역별 특수 키
	std::string keyHints = std::format("[ESC] 메뉴  [S] 상세  [M] 이동 {}  [R] 맵", alertText);

	// 조타실에서는 [N] 항로 추가
	if (currentRegion == Region::Cockpit) {
		keyHints += "  [N] 항로";
	}
	// 하부구동부에서는 [A] 각도 추가
	else if (currentRegion == Region::LowerDrive) {
		keyHints += "  [A] 각도";
	}

	std::println("    {}", keyHints);
	std::println("");

	// 저장된 대사들 다시 출력
	for (const auto& d : dayLog) {
		std::println("    \"{}\"\n", d);
	}
}

void GameScene::DisplayDayEnd()
{
	if (!world) return;

	const CityMetrics& cm = world->GetCity()->GetCityMet();
	int avgStress = CalculateAverageStress();
	int avgFatigue = CalculateAverageFatigue();

	// 상태에 따른 엔딩 문구 카테고리 결정
	std::string category;

	// 우선순위: 결핍 > 피로 > 스트레스 > 분위기
	if (cm.scarcity >= 6000) {
		category = "ending_scarce";
	}
	else if (avgFatigue >= 6000) {
		category = "ending_exhausted";
	}
	else if (avgStress >= 6000) {
		category = "ending_stressed";
	}
	else if (cm.mood >= 6000) {
		category = "ending_good_mood";
	}
	else if (cm.mood >= 4000) {
		category = "ending_normal_mood";
	}
	else {
		category = "ending_bad_mood";
	}

	std::string endingMessage = GetDialogueForState(category);
	if (endingMessage.empty()) {
		endingMessage = "오늘 하루도 살아남았습니다.";
	}

	system("cls");
	std::println("");
	std::println("");
	std::println("");
	std::println("");
	std::println("                    {}", endingMessage);
	std::println("");

	std::this_thread::sleep_for(std::chrono::milliseconds(1500));
}

void GameScene::DisplayDayTransition()
{
	system("cls");
	std::println("");
	std::println("");
	std::println("");
	std::println("");
	typewriter_print("                    ...", 100);
	std::println("");
	typewriter_print("               다음 날이 밝았습니다.", 30);
	std::println("");
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

	// 하루 엔딩 문구
	dialogues["ending_good_mood"] = loadSentences("data/dialogues/ending_good_mood.txt");
	dialogues["ending_normal_mood"] = loadSentences("data/dialogues/ending_normal_mood.txt");
	dialogues["ending_bad_mood"] = loadSentences("data/dialogues/ending_bad_mood.txt");
	dialogues["ending_scarce"] = loadSentences("data/dialogues/ending_scarce.txt");
	dialogues["ending_exhausted"] = loadSentences("data/dialogues/ending_exhausted.txt");
	dialogues["ending_stressed"] = loadSentences("data/dialogues/ending_stressed.txt");
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

// ========== 구역 관련 함수 ==========
std::string GameScene::GetRegionStatusIcon(Region region)
{
	if (!world) return " ";
	if (world->GetPlayerRegion() == region) return "●";  // 현재 위치
	if (world->HasUnseenEventInRegion(region)) return "!";  // 미확인 이벤트
	return "○";
}

void GameScene::DisplayRegionMap()
{
	if (!world) return;

	system("cls");

	std::println("");
	std::println("  ╔═══════════════════════════════════════════════════╗");
	std::println("  ║                    [구역 맵]                      ║");
	std::println("  ╠═══════════════════════════════════════════════════╣");
	std::println("  ║                                                   ║");
	std::println("  ║                  [{}] 조타실                      ║", GetRegionStatusIcon(Region::Cockpit));
	std::println("  ║                       │                           ║");
	std::println("  ║                [{}] 외벽정비구역                  ║", GetRegionStatusIcon(Region::OuterWallMaintenance));
	std::println("  ║                    /  │  \\                        ║");
	std::println("  ║                   /   │   \\                       ║");
	std::println("  ║          [{}] 식당───[{}] 순환정제소              ║",
		GetRegionStatusIcon(Region::Canteen), GetRegionStatusIcon(Region::RecyclingPlant));
	std::println("  ║           / |  \\    /  |                          ║");
	std::println("  ║          /  |   \\  /   |                          ║");
	std::println("  ║   [{}] 거주1  [{}] 수직농장  [{}] 거주2           ║",
		GetRegionStatusIcon(Region::ResidentialArea1),
		GetRegionStatusIcon(Region::VerticalFarm),
		GetRegionStatusIcon(Region::ResidentialArea2));
	std::println("  ║        |                                          ║");
	std::println("  ║   [{}] 중앙동력로                                 ║", GetRegionStatusIcon(Region::CentralPowerway));
	std::println("  ║        │                                          ║");
	std::println("  ║   [{}] 하부구동부                                 ║", GetRegionStatusIcon(Region::LowerDrive));
	std::println("  ║                                                   ║");
	std::println("  ╠═══════════════════════════════════════════════════╣");
	std::println("  ║  ● 현재 위치    ! 미확인 이벤트    ○ 일반        ║");
	std::println("  ╠═══════════════════════════════════════════════════╣");

	// 구역별 인구 표시
	std::println("  ║  [구역별 인원]                                    ║");
	std::println("  ║  조타실:{:>3}  외벽정비:{:>3}  식당:{:>3}  순환정제:{:>3}  ║",
		world->GetHumanCountInRegion(Region::Cockpit),
		world->GetHumanCountInRegion(Region::OuterWallMaintenance),
		world->GetHumanCountInRegion(Region::Canteen),
		world->GetHumanCountInRegion(Region::RecyclingPlant));
	std::println("  ║  수직농장:{:>3}  거주1:{:>3}  거주2:{:>3}             ║",
		world->GetHumanCountInRegion(Region::VerticalFarm),
		world->GetHumanCountInRegion(Region::ResidentialArea1),
		world->GetHumanCountInRegion(Region::ResidentialArea2));
	std::println("  ║  중앙동력로:{:>3}  하부구동부:{:>3}                    ║",
		world->GetHumanCountInRegion(Region::CentralPowerway),
		world->GetHumanCountInRegion(Region::LowerDrive));

	std::println("  ╚═══════════════════════════════════════════════════╝");
	std::println("    아무 키나 누르면 돌아갑니다...");
}

void GameScene::DisplayMoveMenu()
{
	if (!world) return;

	Region currentRegion = world->GetPlayerRegion();
	std::vector<Region> adjacent = world->GetAccessibleRegions();

	// 대사 출력 하단에 인라인으로 이동 메뉴 표시
	std::println("");
	std::println("  ──────────────── [구역 이동] ────────────────");
	std::print("  이동 가능: ");

	for (size_t i = 0; i < adjacent.size(); ++i) {
		Region r = adjacent[i];
		std::string icon = world->HasUnseenEventInRegion(r) ? "!" : "";
		if (i > 0) std::print(" | ");
		std::print("[{}]{}{}", i + 1, GetRegionName(r), icon);
	}
	std::println("");
	std::println("  숫자 키로 이동, ESC로 취소");
	std::print("  > ");
}

void GameScene::HandleRegionMove(char input)
{
	if (!world) return;

	if (input == 27) {  // ESC
		showingMoveMenu = false;
		std::println("\n  이동을 취소했습니다.");
		return;
	}

	std::vector<Region> adjacent = world->GetAccessibleRegions();
	int choice = input - '1';

	if (choice >= 0 && choice < static_cast<int>(adjacent.size())) {
		Region targetRegion = adjacent[choice];
		bool moved = world->MovePlayerToRegion(targetRegion);

		if (moved) {
			// 구역 이벤트 처리 (플레이어가 방문한 구역)
			EventManager* em = world->GetEventManager();
			if (em) {
				em->ProcessPendingRegionEvents(static_cast<int>(targetRegion), *world->GetCity(), world->GetHumansVector());
			}

			// 이동 메시지 출력
			std::println("\n  >> {} 구역으로 이동했습니다.", GetRegionName(targetRegion));

			// 이동 메시지 로그 추가
			std::string moveLog = std::format("[이동] {} 구역으로 이동했습니다.", GetRegionName(targetRegion));
			dayLog.push_back(moveLog);
		}

		showingMoveMenu = false;
	}
}

void GameScene::DisplayRegionEventAlert(Region region, const std::string& eventName)
{
	std::string regionName = GetRegionName(region);
	std::string alertLog = std::format("[알림] {}에서 '{}' 이벤트 발생!", regionName, eventName);
	dayLog.push_back(alertLog);
	std::println("    {}", alertLog);
}

// ========== 시민 정보 관련 함수 ==========
void GameScene::DisplayCitizenInfo()
{
	if (!world) return;

	system("cls");

	// 모드에 따른 탭 표시
	std::string modeNames[3] = { "성향", "누적값", "상태" };
	std::string tabs = "";
	for (int i = 0; i < 3; ++i) {
		if (i == citizenInfoMode) {
			tabs += std::format(" [{}] ", modeNames[i]);
		}
		else {
			tabs += std::format("  {}  ", modeNames[i]);
		}
	}

	std::println("");
	std::println("  ╔════════════════════════════════════════════════════════════════╗");
	std::println("  ║                        [시민 정보]                             ║");
	std::println("  ╠════════════════════════════════════════════════════════════════╣");
	std::println("  ║  {}                              ║", tabs);
	std::println("  ╠════════════════════════════════════════════════════════════════╣");

	// 구역별로 시민 정보 출력
	for (int r = 0; r < static_cast<int>(Region::COUNT); ++r) {
		Region region = static_cast<Region>(r);
		std::vector<Human*> humansInRegion = world->GetHumansInRegion(region);

		if (humansInRegion.empty()) continue;

		std::println("  ║  [{:<12}] ({:>3}명)                                       ║",
			GetRegionName(region), humansInRegion.size());

		// 한 줄에 4명씩 표시
		for (size_t i = 0; i < humansInRegion.size(); i += 4) {
			std::string line = "  ║  ";

			for (size_t j = i; j < i + 4 && j < humansInRegion.size(); ++j) {
				Human* h = humansInRegion[j];
				std::string gender = h->IsMale() ? "♂" : "♀";
				std::string name = h->GetName();

				std::string info;
				switch (citizenInfoMode) {
				case 0:  // 성향
				{
					int rat = h->GetRationality() / 10;
					int agg = h->GetAggressiveness() / 10;
					int pln = h->GetPlanning() / 10;
					info = std::format("{}{} R{}A{}P{}", name, gender, rat, agg, pln);
					break;
				}
				case 1:  // 누적값
				{
					int str = h->GetStressLoad() / 100;
					int fat = h->GetFatigue() / 100;
					int mot = h->GetMotivation() / 100;
					info = std::format("{}{} S{}F{}M{}", name, gender, str, fat, mot);
					break;
				}
				case 2:  // 상태
				{
					std::string arousal;
					switch (h->GetArousal()) {
					case ArousalState::Calm: arousal = "차분"; break;
					case ArousalState::Tense: arousal = "긴장"; break;
					case ArousalState::Irritable: arousal = "과민"; break;
					case ArousalState::Hostile: arousal = "적대"; break;
					}
					std::string energy;
					switch (h->GetEnergy()) {
					case EnergyState::Normal: energy = "정상"; break;
					case EnergyState::Fatigued: energy = "피로"; break;
					case EnergyState::Exhausted: energy = "소진"; break;
					}
					info = std::format("{}{} {}/{}", name, gender, arousal, energy);
					break;
				}
				}

				line += std::format("{:<16}", info);
			}

			// 줄 맞춤
			while (line.size() < 68) line += " ";
			line += "║";
			std::println("{}", line);
		}

		std::println("  ║                                                                  ║");
	}

	std::println("  ╚════════════════════════════════════════════════════════════════╝");
	std::println("    [←/→] 모드 전환    [ESC] 닫기");
}

void GameScene::HandleCitizenInfoInput(char input)
{
	if (input == 27) {  // ESC
		showingCitizenInfo = false;
		system("cls");
		DisplayDayStart();
		for (const auto& d : dayLog) {
			std::println("    \"{}\"", d);
		}
	}
}

void GameScene::HandleCitizenInfoArrow(int dir)
{
	switch (dir) {
	case 75:  // 왼쪽
		citizenInfoMode = (citizenInfoMode + 2) % 3;  // -1 mod 3
		DisplayCitizenInfo();
		break;
	case 77:  // 오른쪽
		citizenInfoMode = (citizenInfoMode + 1) % 3;
		DisplayCitizenInfo();
		break;
	}
}

// ========== 네비게이션 관련 함수 ==========
void GameScene::DisplayNavigationStatus()
{
	if (!world || !world->GetNavigation()) return;

	Navigation* nav = world->GetNavigation();

	std::println("  ──────────────── [항행 상태] ────────────────");
	std::println("  현재 위치: ({}, {})", nav->GetCurrentX(), nav->GetCurrentY());
	std::println("  현재 지형: {}", GetTerrainName(nav->GetCurrentTerrain()));
	std::println("  현재 기온: {:.1f}°C", nav->GetCurrentTemperature());
	std::println("");

	if (nav->HasActiveRoute()) {
		const Route& route = nav->GetCurrentRoute();
		const MapRegion* dest = nav->GetRegion(route.destinationId);
		std::string destName = dest ? dest->name : "알 수 없음";

		std::println("  [목적지] {}", destName);
		std::println("  총 거리: {} 단위", route.totalDistance);
		std::println("  필요 각도: {}°", route.requiredAngle);
		std::println("  현재 각도: {}° {}", nav->GetMovementAngle(),
			nav->IsAngleCorrect() ? "(정확)" : "(조정 필요!)");
		std::println("  남은 일수: {}일 / 총 {}일", nav->GetRemainingDays(), route.daysRequired);
	}
	else if (nav->IsInMaintenance()) {
		std::println("  [정비 중] 남은 일수: {}일", nav->GetMaintenanceDaysLeft());
	}
	else {
		std::println("  경로가 설정되지 않았습니다.");
	}
	std::println("");
}

void GameScene::DisplayNavigationMenu()
{
	if (!world || !world->GetNavigation()) return;

	Navigation* nav = world->GetNavigation();

	std::println("");
	std::println("  ══════════════ [조타실 - 목적지 설정] ══════════════");
	std::println("");

	// 현재 상태 표시
	DisplayNavigationStatus();

	// 발견된 지역 목록 표시
	auto discovered = nav->GetDiscoveredRegions();

	if (discovered.empty()) {
		std::println("  발견된 지역이 없습니다.");
		std::println("  이동 중이나 대화를 통해 힌트를 얻으세요.");
	}
	else {
		std::println("  [발견된 지역]");
		int idx = 1;
		for (const MapRegion* region : discovered) {
			std::string visitedMark = region->visited ? "[방문]" : "[미방문]";
			std::println("    {}. {} - {} ({})", idx, region->name,
				GetTerrainName(region->terrain), visitedMark);
			idx++;
		}
		std::println("");
		std::println("  숫자로 목적지 선택, [C] 경로 취소, [ESC] 닫기");
	}

	std::print("  > ");
}

void GameScene::HandleNavigationInput(char input)
{
	if (!world || !world->GetNavigation()) return;

	Navigation* nav = world->GetNavigation();

	if (input == 27) {  // ESC
		showingNavigationMenu = false;
		system("cls");
		DisplayDayStart();
		for (const auto& d : dayLog) {
			std::println("    \"{}\"", d);
		}
		return;
	}

	if (input == 'c' || input == 'C') {
		nav->CancelRoute();
		std::println("\n  경로가 취소되었습니다.");
		DisplayNavigationMenu();
		return;
	}

	// 숫자 입력 처리
	auto discovered = nav->GetDiscoveredRegions();
	int choice = input - '1';

	if (choice >= 0 && choice < static_cast<int>(discovered.size())) {
		int regionId = discovered[choice]->id;
		if (nav->SetDestination(regionId)) {
			const MapRegion* dest = nav->GetRegion(regionId);
			std::string logMsg = std::format("[항로] {}로 목적지 설정 (각도: {}°, 예상 {}일)",
				dest->name, nav->GetRequiredAngle(), nav->GetCurrentRoute().daysRequired);
			dayLog.push_back(logMsg);

			std::println("\n  >> {}로 목적지가 설정되었습니다.", dest->name);
			std::println("     하부구동부에서 각도를 {}°로 맞춰주세요.", nav->GetRequiredAngle());
		}
		else {
			std::println("\n  해당 지역으로 경로를 설정할 수 없습니다.");
		}
		DisplayNavigationMenu();
	}
}

void GameScene::DisplayAngleMenu()
{
	if (!world || !world->GetNavigation()) return;

	Navigation* nav = world->GetNavigation();

	std::println("");
	std::println("  ══════════════ [하부구동부 - 각도 조정] ══════════════");
	std::println("");

	// 현재 항행 상태
	DisplayNavigationStatus();

	// 각도 조정 UI
	std::println("  [각도 조정]");
	std::println("  현재 각도: {}°", nav->GetMovementAngle());

	if (nav->HasActiveRoute()) {
		std::println("  필요 각도: {}°", nav->GetRequiredAngle());
		if (nav->IsAngleCorrect()) {
			std::println("  >> 각도가 정확합니다. 이동 중...");
		}
		else {
			int diff = nav->GetRequiredAngle() - nav->GetMovementAngle();
			if (diff > 180) diff -= 360;
			if (diff < -180) diff += 360;
			std::println("  >> 조정 필요: {}° {}", std::abs(diff),
				diff > 0 ? "(시계방향)" : "(반시계방향)");
		}
	}
	else {
		std::println("  목적지가 설정되지 않았습니다.");
	}

	std::println("");
	std::println("  [←/→] 1° 조정  [A/D] 10° 조정  [숫자 입력] 직접 설정");
	std::println("  [ESC] 닫기");
	std::print("  > ");
}

void GameScene::HandleAngleInput(char input)
{
	if (!world || !world->GetNavigation()) return;

	Navigation* nav = world->GetNavigation();

	if (input == 27) {  // ESC
		showingAngleMenu = false;
		angleInputBuffer = 0;
		system("cls");
		DisplayDayStart();
		for (const auto& d : dayLog) {
			std::println("    \"{}\"", d);
		}
		return;
	}

	// 숫자 입력 (직접 각도 설정)
	if (input >= '0' && input <= '9') {
		angleInputBuffer = angleInputBuffer * 10 + (input - '0');
		if (angleInputBuffer > 359) {
			angleInputBuffer = angleInputBuffer % 360;
		}
		std::print("{}", input);
		return;
	}

	// Enter로 각도 확정
	if (input == '\r' && angleInputBuffer > 0) {
		nav->SetMovementAngle(angleInputBuffer);
		std::println("\n  >> 각도를 {}°로 설정했습니다.", angleInputBuffer);
		angleInputBuffer = 0;
		DisplayAngleMenu();
		return;
	}

	// A/D로 10도 조정
	if (input == 'a' || input == 'A') {
		int newAngle = nav->GetMovementAngle() - 10;
		nav->SetMovementAngle(newAngle);
		DisplayAngleMenu();
		return;
	}
	if (input == 'd' || input == 'D') {
		int newAngle = nav->GetMovementAngle() + 10;
		nav->SetMovementAngle(newAngle);
		DisplayAngleMenu();
		return;
	}
}

// 방향키 처리를 위한 sHandleInput에서 각도 조정
// Scene.cpp의 sHandleInput 함수 내에서 처리
