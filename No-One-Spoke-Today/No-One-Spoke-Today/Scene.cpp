#include "Scene.h"
#include "Toolkit.h"
#include <iomanip>
#include <sstream>

std::string Scene::oldScene;

void TitleScene::Enter(std::unique_ptr<World>&)
{
	LoadText(title, "data/title.txt");
	LoadText(intro, "data/intro.txt");
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
	navigationInputBuffer.clear();
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

		// 게임 종료 상태 체크
		GameEndState endState = world->CheckGameEndState();
		if (endState != GameEndState::None) {
			// 승리 조건
			if (endState == GameEndState::Victory_Good ||
				endState == GameEndState::Victory_Normal ||
				endState == GameEndState::Victory_Bad) {
				RequestSceneChange("ending");
			}
			// 게임오버 조건
			else {
				RequestSceneChange("gameover");
			}
			return;
		}
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
				std::println("    \"{}\"\n", d);
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

				// 힌트 체크 및 표시 (이벤트 콜백으로 힌트가 생성되었을 수 있음)
				CheckAndDisplayHints();

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
	LoadText(menu, "data/menu.txt");

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
	std::ifstream in{ GetFullPath("data/savefile"), std::ios::binary };
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
	CreateDirectoryA(GetFullPath("data").c_str(), NULL);
	std::ofstream out{ GetFullPath("data/savefile"), std::ios::binary | std::ios::trunc };
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
	std::ifstream in{ GetFullPath("data/" + fileName + ".bin"), std::ios::binary };
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

	// 네비게이션 상태 로드 (버전 4 이상에서만, 구버전 세이브는 새로 초기화)
	if (version >= 4 && world->GetNavigation()) {
		world->GetNavigation()->LoadState(in);
	}

	// 하부구동부 지시 횟수 로드 (버전 5 이상)
	if (version >= 5) {
		int32_t angleOrderCount;
		in.read(reinterpret_cast<char*>(&angleOrderCount), sizeof(angleOrderCount));
		world->SetAngleOrderCountToday(angleOrderCount);
		world->SetLastAngleOrderDay(world->GetCurrentDay());
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
			std::remove(GetFullPath("data/" + oldFileName + ".bin").c_str());
		}
	}

	std::ofstream out{ GetFullPath("data/" + fileName + ".bin"), std::ios::binary };
	if (not out) return;

	// 버전 (5: 하부구동부 지시 시스템 추가)
	uint32_t version = 5;
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

	// 네비게이션 상태 저장 (버전 4: 좌표 기반)
	if (world->GetNavigation()) {
		world->GetNavigation()->SaveState(out);
		std::cout << "네비게이션 저장";
	}

	// 하부구동부 지시 횟수 저장 (버전 5)
	int32_t angleOrderCount = world->GetAngleOrderCountToday();
	out.write(reinterpret_cast<const char*>(&angleOrderCount), sizeof(angleOrderCount));
	std::cout << "지시횟수 저장";

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
	std::println("  ╔══════════════════════════════════════════════════════════════╗");
	std::println("  ║                          [구역 맵]                           ║");
	std::println("  ╠══════════════════════════════════════════════════════════════╣");
	std::println("  ║                                                              ║");
	std::println("  ║  <상부>  [{}] 조타실 (외벽정비↓)                              ║", GetRegionStatusIcon(Region::Cockpit));
	std::println("  ║                                                              ║");
	std::println("  ║  <중부>  [{}] 외벽정비─────────[{}] 식당                       ║", GetRegionStatusIcon(Region::OuterWallMaintenance), GetRegionStatusIcon(Region::Canteen));
	std::println("  ║               │              /     │   \\                     ║");
	std::println("  ║               │             /      │    \\                    ║");
	std::println("  ║               │    [{}] 중앙동력로  │ [{}] 거주1               ║", GetRegionStatusIcon(Region::CentralPowerway), GetRegionStatusIcon(Region::ResidentialArea1));
	std::println("  ║               │        /           │     /                   ║");
	std::println("  ║               │       /            │    /                    ║");
	std::println("  ║          [{}] 순환정제소───────[{}] 수직농장                   ║", GetRegionStatusIcon(Region::RecyclingPlant), GetRegionStatusIcon(Region::VerticalFarm));
	std::println("  ║                    \\           /                             ║");
	std::println("  ║                     \\         /                              ║");
	std::println("  ║                       [{}] 거주2                              ║", GetRegionStatusIcon(Region::ResidentialArea2));
	std::println("  ║                                                              ║");
	std::println("  ║   <하부>  [{}] 하부구동부 (중앙동력로↑)                       ║", GetRegionStatusIcon(Region::LowerDrive));
	std::println("  ║                                                              ║");
	std::println("  ╠══════════════════════════════════════════════════════════════╣");
	std::println("  ║  ● 현재 위치    ! 미확인 이벤트    ○ 일반                    ║");
	std::println("  ╠══════════════════════════════════════════════════════════════╣");
	std::println("  ║  [구역별 인원]                                               ║");
	std::println("  ║  조타실:{:>3}   외벽정비:{:>3}   식당:{:>3}   순환정제:{:>3}         ║",
		world->GetHumanCountInRegion(Region::Cockpit),
		world->GetHumanCountInRegion(Region::OuterWallMaintenance),
		world->GetHumanCountInRegion(Region::Canteen),
		world->GetHumanCountInRegion(Region::RecyclingPlant));
	std::println("  ║  수직농장:{:>3}  거주1:{:>3}     거주2:{:>3}                       ║",
		world->GetHumanCountInRegion(Region::VerticalFarm),
		world->GetHumanCountInRegion(Region::ResidentialArea1),
		world->GetHumanCountInRegion(Region::ResidentialArea2));
	std::println("  ║  중앙동력로:{:>3}  하부구동부:{:>3}                              ║",
		world->GetHumanCountInRegion(Region::CentralPowerway),
		world->GetHumanCountInRegion(Region::LowerDrive));
	std::println("  ╚══════════════════════════════════════════════════════════════╝");
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

// ========== 시민 정보 관련 헬퍼 함수 ==========
// UTF-8 문자열의 콘솔 표시 너비 계산
int GetDisplayWidth(const std::string& str) {
	int width = 0;
	size_t i = 0;
	while (i < str.size()) {
		unsigned char c = str[i];
		if ((c & 0x80) == 0) {
			// ASCII (1바이트) - 너비 1
			width += 1;
			i += 1;
		}
		else if ((c & 0xE0) == 0xC0) {
			// 2바이트 문자 - 너비 1
			width += 1;
			i += 2;
		}
		else if ((c & 0xF0) == 0xE0) {
			// 3바이트 문자 (한글, 화살표 등)
			// 한글 범위 (U+AC00 ~ U+D7A3) 체크
			if (i + 2 < str.size()) {
				unsigned char c2 = str[i + 1];
				unsigned char c3 = str[i + 2];
				// UTF-8 디코딩: ((c & 0x0F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F)
				int codepoint = ((c & 0x0F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F);
				// 한글 범위 또는 전각 문자
				if ((codepoint >= 0xAC00 && codepoint <= 0xD7A3) ||  // 한글
					(codepoint >= 0x3000 && codepoint <= 0x303F) ||  // CJK 기호
					(codepoint >= 0xFF00 && codepoint <= 0xFFEF)) {  // 전각 문자
					width += 2;
				}
				else {
					width += 1;  // 화살표, 성별 기호 등
				}
			}
			i += 3;
		}
		else if ((c & 0xF8) == 0xF0) {
			// 4바이트 문자 - 너비 2
			width += 2;
			i += 4;
		}
		else {
			i += 1;
		}
	}
	return width;
}

// 고정 너비로 패딩된 문자열 생성
std::string PadToWidth(const std::string& str, int targetWidth) {
	int currentWidth = GetDisplayWidth(str);
	std::string result = str;
	while (currentWidth < targetWidth) {
		result += ' ';
		currentWidth++;
	}
	return result;
}

// 성향 방향 기호 (0-100 → ↑/→/↓)
std::string GetTraitArrow(int value) {
	if (value >= 67) return "↑";
	if (value >= 34) return "→";
	return "↓";
}

// 누적값 5단계 (0-10000 → 0-4)
int GetDriveLevel(int value) {
	if (value >= 8000) return 4;
	if (value >= 6000) return 3;
	if (value >= 4000) return 2;
	if (value >= 2000) return 1;
	return 0;
}

// 상태 태그 문자
char GetArousalTag(ArousalState state) {
	switch (state) {
	case ArousalState::Calm: return 'C';
	case ArousalState::Tense: return 'T';
	case ArousalState::Irritable: return 'I';
	case ArousalState::Hostile: return 'H';
	default: return '?';
	}
}

char GetSocialTag(SocialState state) {
	switch (state) {
	case SocialState::Neutral: return 'N';
	case SocialState::Cooperative: return 'C';
	case SocialState::Withdrawn: return 'W';
	default: return '?';
	}
}

char GetEnergyTag(EnergyState state) {
	switch (state) {
	case EnergyState::Normal: return 'N';
	case EnergyState::Fatigued: return 'F';
	case EnergyState::Exhausted: return 'E';
	default: return '?';
	}
}

char GetControlTag(ControlState state) {
	switch (state) {
	case ControlState::Autonomous: return 'A';
	case ControlState::Dependent: return 'D';
	case ControlState::Stubborn: return 'S';
	default: return '?';
	}
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
	std::println("  ╔════════════════════════════════════════════════════════════════════╗");
	std::println("  ║                          [시민 정보]                               ║");
	std::println("  ╠════════════════════════════════════════════════════════════════════╣");
	std::println("  ║  {}                                        ║", tabs);
	std::println("  ╠════════════════════════════════════════════════════════════════════╣");

	// 모드별 범례 출력
	switch (citizenInfoMode) {
	case 0:  // 성향
		std::println("  ║  [범례] R:이성 A:공격 P:계획 D:의존 G:고집 E:감정민감              ║");
		std::println("  ║         ↑:높음(67+)  →:보통(34-66)  ↓:낮음(0-33)                   ║");
		break;
	case 1:  // 누적값
		std::println("  ║  [범례] S:스트레스 A:감정각성 F:피로 C:인지력                     ║");
		std::println("  ║         T:신뢰 Y:안전감 O:통제감 M:동기  (0~4 단계)               ║");
		break;
	case 2:  // 상태
		std::println("  ║  [범례] 감정: C차분/T긴장/I과민/H적대  사회: N중립/C협력/W철수    ║");
		std::println("  ║         에너지: N정상/F피로/E소진  통제: A자율/D의존/S고집        ║");
		break;
	}
	std::println("  ╠════════════════════════════════════════════════════════════════════╣");

	// 구역별로 시민 정보 출력
	for (int r = 0; r < static_cast<int>(Region::COUNT); ++r) {
		Region region = static_cast<Region>(r);
		std::vector<Human*> humansInRegion = world->GetHumansInRegion(region);

		if (humansInRegion.empty()) continue;

		std::println("  ║  [{:<12}] ({:>3}명)                                            ║",
			GetRegionName(region), humansInRegion.size());

		// 한 줄에 3명씩 표시 (더 넓은 정보)
		for (size_t i = 0; i < humansInRegion.size(); i += 3) {
			std::string line = "  ║  ";

			for (size_t j = i; j < i + 3 && j < humansInRegion.size(); ++j) {
				Human* h = humansInRegion[j];
				std::string gender = h->IsMale() ? "♂" : "♀";
				std::string name = h->GetName();

				std::string info;
				switch (citizenInfoMode) {
				case 0:  // 성향 (방향 기호)
				{
					std::string traits = std::format("R{}A{}P{}D{}G{}E{}",
						GetTraitArrow(h->GetRationality()),
						GetTraitArrow(h->GetAggressiveness()),
						GetTraitArrow(h->GetPlanning()),
						GetTraitArrow(h->GetDependency()),
						GetTraitArrow(h->GetRigidity()),
						GetTraitArrow(h->GetEmotionalSensitivity()));
					info = std::format("{}{} {}", name, gender, traits);
					break;
				}
				case 1:  // 누적값 (0-4 단계)
				{
					std::string drives = std::format("S{}A{}F{}C{}T{}Y{}O{}M{}",
						GetDriveLevel(h->GetStressLoad()),
						GetDriveLevel(h->GetEmotionalArousal()),
						GetDriveLevel(h->GetFatigue()),
						GetDriveLevel(h->GetCognitiveCapacity()),
						GetDriveLevel(h->GetInterpersonalTrust()),
						GetDriveLevel(h->GetSocialSafety()),
						GetDriveLevel(h->GetSenseOfControl()),
						GetDriveLevel(h->GetMotivation()));
					info = std::format("{}{} {}", name, gender, drives);
					break;
				}
				case 2:  // 상태 (태그)
				{
					std::string states = std::format("{}{}{}{}",
						GetArousalTag(h->GetArousal()),
						GetSocialTag(h->GetSocial()),
						GetEnergyTag(h->GetEnergy()),
						GetControlTag(h->GetControl()));
					info = std::format("{}{} [{}]", name, gender, states);
					break;
				}
				}

				line += PadToWidth(info, 22);
			}

			// 줄 맞춤 (표시 너비 기준)
			while (GetDisplayWidth(line) < 71) line += " ";
			line += "║";
			std::println("{}", line);
		}

		std::println("  ║                                                                    ║");
	}

	std::println("  ╚════════════════════════════════════════════════════════════════════╝");
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
	std::println("  현재 이동 각도: {}°", nav->GetMovementAngle());
	std::println("  총 이동 일수: {}일", nav->GetTraveledDays());
	std::println("");

	if (nav->HasTarget()) {
		const Route& target = nav->GetTargetRoute();
		std::println("  [목표 좌표] ({}, {})", target.targetX, target.targetY);
		std::println("  예상 거리: {} 단위", target.totalDistance);
		std::println("  필요 각도: {}°", target.requiredAngle);
		std::println("  예상 소요: {}일", target.estimatedDays);

		int angleDiff = std::abs(nav->GetMovementAngle() - target.requiredAngle);
		if (angleDiff > 180) angleDiff = 360 - angleDiff;
		if (angleDiff <= 5) {
			std::println("  >> 현재 각도가 목표와 일치합니다.");
		}
		else {
			std::println("  >> 하부구동부에서 각도를 {}°로 맞춰야 합니다!", target.requiredAngle);
		}
	}
	else if (nav->IsInMaintenance()) {
		std::println("  [정비 중] 남은 일수: {}일", nav->GetMaintenanceDaysLeft());
	}
	else {
		std::println("  목표 좌표가 설정되지 않았습니다.");
	}
	std::println("");
}

void GameScene::DisplayNavigationMenu()
{
	if (!world || !world->GetNavigation()) return;

	Navigation* nav = world->GetNavigation();

	std::println("");
	std::println("  ══════════════ [조타실 - 항로 계산] ══════════════");
	std::println("");

	// 현재 상태 표시
	DisplayNavigationStatus();

	// 발견된 힌트 좌표 목록 표시
	auto hints = nav->GetDiscoveredHints();

	std::println("  ──────────────── [알려진 좌표] ────────────────");
	if (hints.empty()) {
		std::println("  알려진 좌표가 없습니다.");
		std::println("  이동 중이나 대화를 통해 좌표 정보를 얻으세요.");
	}
	else {
		for (size_t i = 0; i < hints.size(); ++i) {
			const auto& hint = hints[i];
			std::string exactMark = hint.isExact ? "[정확]" : "[대략]";
			std::println("    {}. ({}, {}) {}", i + 1, hint.approximateX, hint.approximateY, exactMark);
		}
	}

	std::println("");
	std::println("  ──────────────── [좌표 입력] ────────────────");
	std::println("  목표 좌표를 입력하세요 (형식: X Y)");
	std::println("  예: 750 320");
	std::println("");
	std::println("  [C] 목표 해제  [ESC] 닫기");
	std::print("  > ");
}

void GameScene::HandleNavigationInput(char input)
{
	if (!world || !world->GetNavigation()) return;

	Navigation* nav = world->GetNavigation();

	if (input == 27) {  // ESC
		showingNavigationMenu = false;
		navigationInputBuffer.clear();
		system("cls");
		DisplayDayStart();
		for (const auto& d : dayLog) {
			std::println("    \"{}\"\n", d);
		}
		return;
	}

	if (input == 'c' || input == 'C') {
		nav->ClearTarget();
		std::println("\n  목표 좌표가 해제되었습니다.");
		navigationInputBuffer.clear();
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		system("cls");
		DisplayNavigationMenu();
		return;
	}

	// 숫자, 공백, 마이너스 입력 처리
	if ((input >= '0' && input <= '9') || input == ' ' || input == '-') {
		navigationInputBuffer += input;
		std::print("{}", input);
		return;
	}

	// 백스페이스
	if (input == 8 && !navigationInputBuffer.empty()) {
		navigationInputBuffer.pop_back();
		std::print("\b \b");
		return;
	}

	// Enter로 좌표 확정
	if (input == '\r' && !navigationInputBuffer.empty()) {
		// 좌표 파싱 (X Y 형식)
		std::stringstream ss(navigationInputBuffer);
		int targetX, targetY;

		if (ss >> targetX >> targetY) {
			// 좌표 범위 체크
			if (targetX < 0 || targetX > 1000 || targetY < 0 || targetY > 1000) {
				std::println("\n  좌표 범위 오류! (0~1000 사이 값을 입력하세요)");
				navigationInputBuffer.clear();
				std::this_thread::sleep_for(std::chrono::milliseconds(800));
				system("cls");
				DisplayNavigationMenu();
				return;
			}

			// 목표 좌표 설정
			nav->SetTargetCoordinates(targetX, targetY);
			const Route& route = nav->GetTargetRoute();

			std::string logMsg = std::format("[항로] 목표 좌표 ({}, {}) 설정 - 필요 각도: {}°, 예상 거리: {}, 예상 {}일",
				targetX, targetY, route.requiredAngle, route.totalDistance, route.estimatedDays);
			dayLog.push_back(logMsg);

			std::println("");
			std::println("");
			std::println("  ════════════════════════════════════════");
			std::println("  목표 좌표: ({}, {})", targetX, targetY);
			std::println("  필요 각도: {}°", route.requiredAngle);
			std::println("  예상 거리: {} 단위", route.totalDistance);
			std::println("  예상 소요: {}일", route.estimatedDays);
			std::println("  ════════════════════════════════════════");
			std::println("");
			std::println("  ※ 하부구동부에서 각도를 {}°로 설정해야 합니다.", route.requiredAngle);
			std::println("  ※ 현재 각도: {}°", nav->GetMovementAngle());
			std::println("");

			navigationInputBuffer.clear();
			std::this_thread::sleep_for(std::chrono::milliseconds(2000));
			system("cls");
			DisplayNavigationMenu();
		}
		else {
			std::println("\n  입력 형식 오류! (예: 750 320)");
			navigationInputBuffer.clear();
			std::this_thread::sleep_for(std::chrono::milliseconds(800));
			system("cls");
			DisplayNavigationMenu();
		}
		return;
	}
}

// ========== 하부구동부 대화 헬퍼 함수 ==========
Human* GameScene::SelectLowerDriveLeader()
{
	if (!world) return nullptr;

	auto lowerDriveHumans = world->GetHumansInRegion(Region::LowerDrive);
	if (lowerDriveHumans.empty()) return nullptr;

	// 이성↑ & 계획↑인 사람 우선 선택, 없으면 첫 번째 사람
	for (Human* h : lowerDriveHumans) {
		if (h->GetRationality() >= 50 && h->GetPlanning() >= 50) {
			return h;
		}
	}
	return lowerDriveHumans[0];
}

std::string GameScene::GetFatigueBar(Human* h)
{
	if (!h) return "□□□□□";

	int level = h->GetFatigue() / 2000;  // 0-10000 → 0-5
	if (level > 5) level = 5;

	std::string bar;
	for (int i = 0; i < level; ++i) bar += "■";
	for (int i = level; i < 5; ++i) bar += "□";
	return bar;
}

std::string GameScene::GetPlayerAngleDialogue(int angle, int orderCount)
{
	// 지시 횟수에 따라 대사 선택
	std::vector<std::string> dialogues;

	if (orderCount <= 1) {
		// 첫 지시: 정중한 대사
		dialogues = {
			std::format("이동 각도를 {}도로 맞춰주시오.", angle),
			std::format("방향을 {}도로 틀어야 합니다.", angle),
			std::format("{}도로 조정 부탁드립니다.", angle),
			std::format("계산 결과, {}도가 필요합니다.", angle),
			std::format("수고스럽겠지만 {}도로 부탁합니다.", angle)
		};
	}
	else if (orderCount == 2) {
		// 두 번째: 약간 긴급
		dialogues = {
			std::format("지금 바로 {}도로 변경해주십시오.", angle),
			std::format("{}도로 조정 부탁드립니다.", angle),
			std::format("다시 한번 확인했습니다. {}도입니다.", angle)
		};
	}
	else if (orderCount == 3) {
		// 세 번째: 재지시/사과
		dialogues = {
			std::format("잠깐, 방금 틀었는데... {}도로 다시.", angle),
			std::format("미안하지만 또 바꿔야 할 것 같습니다. {}도로.", angle),
			std::format("죄송합니다, 다시 {}도로 맞춰주십시오.", angle)
		};
	}
	else {
		// 4회 이상: 확신/번복
		dialogues = {
			std::format("이번이 마지막입니다. {}도로.", angle),
			std::format("아까 말한 건 취소하고, {}도로 가겠습니다.", angle),
			std::format("...{}도입니다. 부탁합니다.", angle)
		};
	}

	std::uniform_int_distribution<int> dist(0, static_cast<int>(dialogues.size()) - 1);
	return dialogues[dist(rng)];
}

std::string GameScene::GetWorkerResponse(Human* leader, int angle, int orderCount)
{
	if (!leader) return std::format("...{}도, 알겠습니다.", angle);

	ArousalState arousal = leader->GetArousal();
	int stress = leader->GetStressLoad();
	int fatigue = leader->GetFatigue();

	std::vector<std::string> responses;

	// 적대적 상태
	if (arousal == ArousalState::Hostile) {
		responses = {
			"맨날 바꾸고 바꾸고... 직접 하시죠?",
			std::format("또요? 아까 그 각도 아니었어요?"),
			std::format("그 각도가 맞긴 한 겁니까? ...{}도.", angle)
		};
	}
	// 과민 상태 또는 스트레스 높음
	else if (arousal == ArousalState::Irritable || stress >= 7000) {
		responses = {
			"벌써 몇 번째입니까?",
			"제대로 계산하고 오신 겁니까?",
			std::format("...됐습니다. {}도.", angle)
		};
	}
	// 피로 상태
	else if (fatigue >= 6000) {
		responses = {
			std::format("...네. {}도요.", angle),
			"(한숨) 알겠습니다.",
			std::format("또요? 아, 네. {}도.", angle)
		};
	}
	// 긴장 상태
	else if (arousal == ArousalState::Tense) {
		responses = {
			std::format("네, {}도... 맞습니까?", angle),
			std::format("{}도, 확인했습니다.", angle),
			"알겠습니다. 바로 조정하겠습니다."
		};
	}
	// 정상 상태
	else {
		if (orderCount <= 2) {
			responses = {
				std::format("알겠습니다. {}도로 조정하겠습니다.", angle),
				"네, 바로 맞추겠습니다.",
				std::format("{}도, 확인했습니다.", angle)
			};
		}
		else {
			responses = {
				std::format("네... {}도요.", angle),
				std::format("알겠습니다. {}도로 다시 맞추겠습니다.", angle),
				"...알겠습니다."
			};
		}
	}

	std::uniform_int_distribution<int> dist(0, static_cast<int>(responses.size()) - 1);
	return responses[dist(rng)];
}

void GameScene::DisplayAngleMenu()
{
	if (!world || !world->GetNavigation()) return;

	system("cls");

	Navigation* nav = world->GetNavigation();

	// 작업반장 선택 (없으면 선택)
	if (!lowerDriveLeader) {
		lowerDriveLeader = SelectLowerDriveLeader();
	}

	std::println("");
	std::println("  ╔══════════════════════════════════════════════════════════╗");
	std::println("  ║            [하부구동부 - 이동 각도 지시]                  ║");
	std::println("  ╠══════════════════════════════════════════════════════════╣");
	std::println("");

	// 현재 상태 정보
	std::println("    현재 위치: ({}, {})    현재 각도: {}°",
		nav->GetCurrentX(), nav->GetCurrentY(), nav->GetMovementAngle());
	std::println("    오늘 지시 횟수: {}회", world->GetAngleOrderCountToday());
	std::println("");

	// 대화 상태에 따른 화면
	if (angleDialogueState == AngleDialogueState::Idle) {
		// 작업반장 정보 표시
		if (lowerDriveLeader) {
			std::string gender = lowerDriveLeader->IsMale() ? "♂" : "♀";
			std::println("    ┌────────────────────────────────────────────────┐");
			std::println("    │ 작업반장 {}{} (피로: {})                │",
				PadToWidth(lowerDriveLeader->GetName(), 8), gender,
				GetFatigueBar(lowerDriveLeader));
			std::println("    │                                                │");

			// 상태에 따른 대기 대사
			std::string waitingMsg;
			ArousalState arousal = lowerDriveLeader->GetArousal();
			if (arousal == ArousalState::Hostile) {
				waitingMsg = "\"...뭡니까.\"";
			}
			else if (arousal == ArousalState::Irritable) {
				waitingMsg = "\"무슨 일이십니까.\"";
			}
			else if (lowerDriveLeader->GetFatigue() >= 6000) {
				waitingMsg = "\"...네, 말씀하십시오.\"";
			}
			else {
				waitingMsg = "\"무엇을 도와드릴까요?\"";
			}
			std::println("    │ {}                          │", PadToWidth(waitingMsg, 30));
			std::println("    └────────────────────────────────────────────────┘");
		}

		std::println("");
		std::println("    ┌────────────────────────────────────────────────┐");
		std::println("    │              방향 안내                         │");
		std::println("    │                 0° (북)                        │");
		std::println("    │                   │                            │");
		std::println("    │        270° ──────┼────── 90°                  │");
		std::println("    │        (서)       │       (동)                 │");
		std::println("    │                 180°                           │");
		std::println("    │                 (남)                           │");
		std::println("    └────────────────────────────────────────────────┘");
		std::println("");

		// 입력 안내
		if (angleInputBuffer > 0) {
			std::println("    입력 중: {}°", angleInputBuffer);
		}
		else {
			std::println("    지시할 각도를 입력하세요.");
		}
		std::println("");
		std::println("    [0-9] 각도 입력    [Enter] 지시    [ESC] 나가기");
		std::println("    [C] 입력 초기화");
	}
	else if (angleDialogueState == AngleDialogueState::Confirming ||
		angleDialogueState == AngleDialogueState::Completed) {
		// 대화 출력
		std::println("    ┌────────────────────────────────────────────────┐");
		std::println("    │ [당신]                                         │");
		std::println("    │ \"{}\"", PadToWidth(GetPlayerAngleDialogue(pendingAngle,
			world->GetAngleOrderCountToday()), 45));
		std::println("    └────────────────────────────────────────────────┘");
		std::println("");

		if (lowerDriveLeader) {
			std::string gender = lowerDriveLeader->IsMale() ? "♂" : "♀";
			std::println("    ┌────────────────────────────────────────────────┐");
			std::println("    │ [작업반장 {}{}]                              │",
				PadToWidth(lowerDriveLeader->GetName(), 6), gender);
			std::println("    │ \"{}\"", PadToWidth(GetWorkerResponse(lowerDriveLeader,
				pendingAngle, world->GetAngleOrderCountToday()), 45));
			std::println("    └────────────────────────────────────────────────┘");
		}

		std::println("");
		std::println("    >> 각도가 {}°로 설정되었습니다.", pendingAngle);

		if (world->GetAngleOrderCountToday() >= 3) {
			std::println("");
			std::println("    [!] 잦은 지시로 작업자들의 피로와 스트레스가 증가했습니다.");
		}

		std::println("");
		std::println("    [아무 키나 누르세요]");
	}

	std::println("");
	std::println("  ╚══════════════════════════════════════════════════════════╝");
}

void GameScene::HandleAngleInput(char input)
{
	if (!world || !world->GetNavigation()) return;

	Navigation* nav = world->GetNavigation();

	// 대화 완료 상태에서 아무 키나 누르면 Idle로
	if (angleDialogueState == AngleDialogueState::Completed) {
		angleDialogueState = AngleDialogueState::Idle;
		angleInputBuffer = 0;
		DisplayAngleMenu();
		return;
	}

	// 대화 확인 중 상태에서 아무 키나 누르면 완료로
	if (angleDialogueState == AngleDialogueState::Confirming) {
		angleDialogueState = AngleDialogueState::Completed;
		DisplayAngleMenu();
		return;
	}

	// Idle 상태
	if (input == 27) {  // ESC
		showingAngleMenu = false;
		angleInputBuffer = 0;
		angleDialogueState = AngleDialogueState::Idle;
		lowerDriveLeader = nullptr;
		system("cls");
		DisplayDayStart();
		for (const auto& d : dayLog) {
			std::println("    \"{}\"", d);
		}
		return;
	}

	// C로 입력 초기화
	if (input == 'c' || input == 'C') {
		angleInputBuffer = 0;
		DisplayAngleMenu();
		return;
	}

	// 숫자 입력
	if (input >= '0' && input <= '9') {
		angleInputBuffer = angleInputBuffer * 10 + (input - '0');
		if (angleInputBuffer > 359) {
			angleInputBuffer = angleInputBuffer % 1000;
			if (angleInputBuffer > 359) {
				angleInputBuffer = angleInputBuffer % 100;
			}
		}
		DisplayAngleMenu();
		return;
	}

	// Enter로 지시 확정
	if (input == '\r' && angleInputBuffer >= 0) {
		pendingAngle = angleInputBuffer;

		// 실제 각도 설정
		nav->SetMovementAngle(pendingAngle);

		// 지시 횟수 증가 및 스트레스/피로 적용
		world->IncrementAngleOrderCount();

		// 작업반장 상태 업데이트
		if (lowerDriveLeader) {
			lowerDriveLeader->UpdateMentalState();
		}

		// 대화 상태로 전환
		angleDialogueState = AngleDialogueState::Confirming;
		DisplayAngleMenu();
		return;
	}
}

// 방향키 처리를 위한 sHandleInput에서 각도 조정
// Scene.cpp의 sHandleInput 함수 내에서 처리

// ============================================================
// 힌트 표시 함수
// ============================================================
void GameScene::CheckAndDisplayHints() {
	if (!world) return;

	Navigation* nav = world->GetNavigation();
	if (!nav) return;

	// 대기 중인 힌트가 있는지 확인
	if (!nav->HasPendingHintNotification()) return;

	const auto& hints = nav->GetPendingHintNotifications();

	for (const auto& hint : hints) {
		std::println("");
		std::println("    ========================================");

		// 쪽지 발견 대사
		if (!hint.discoveryMessage.empty()) {
			std::println("    {}", hint.discoveryMessage);
		}
		else {
			std::println("    낡은 쪽지를 발견했다.");
		}

		std::println("");

		// 힌트 내용
		std::println("    {}", hint.description);

		std::println("    ========================================");
		std::println("");

		// dayLog에 기록
		std::string logEntry;
		if (hint.isSanctuary) {
			logEntry = "[힌트] ★ 안정지대 정보 획득";
		}
		else {
			switch (hint.hintType) {
			case HintType::Coordinate:
				logEntry = std::format("[힌트] {} - 좌표 정보 획득", hint.regionName);
				break;
			case HintType::Direction:
				logEntry = std::format("[힌트] {} - 방향 정보 획득 ({}도)", hint.regionName, hint.directionAngle);
				break;
			case HintType::Characteristic:
				logEntry = std::format("[힌트] {} 지형의 장소 정보 획득", hint.terrainName);
				break;
			}
		}
		dayLog.push_back(logEntry);
	}

	// 힌트 알림 클리어
	nav->ClearPendingHintNotifications();
}

// ==================== EndingScene ====================

void EndingScene::Enter(std::unique_ptr<World>& w)
{
	system("cls");
	world = w.get();
	textLoaded = false;
	displayComplete = false;
	LoadEndingText();
}

void EndingScene::Update(float deltaTime)
{
	// 특별한 업데이트 없음
}

void EndingScene::Display()
{
	if (!displayComplete) {
		gotoxy(0, 0);
		std::cout << endingText << std::endl;
		std::cout << "\n\n    [아무 키나 누르면 타이틀로 돌아갑니다]" << std::endl;
		displayComplete = true;
	}
}

void EndingScene::Exit()
{
	sceneChangeRequested = false;
	oldScene = "ending";
}

void EndingScene::HandleInput(char input)
{
	// 아무 키나 누르면 타이틀로
	RequestSceneChange("start");
}

void EndingScene::sHandleInput(char input)
{
	// 방향키도 타이틀로
	RequestSceneChange("start");
}

void EndingScene::LoadEndingText()
{
	if (textLoaded) return;

	std::string filepath = "data/ending_sanctuary.txt";
	std::ifstream file(GetFullPath(filepath));

	if (!file.is_open()) {
		endingText = "[엔딩 텍스트를 불러올 수 없습니다]";
		textLoaded = true;
		return;
	}

	// 엔딩 타입에 따른 섹션 선택
	std::string sectionStart, sectionEnd;
	switch (endingType) {
	case GameEndState::Victory_Good:
		sectionStart = "[ENDING_GOOD]";
		sectionEnd = "[ENDING_GOOD_END]";
		break;
	case GameEndState::Victory_Normal:
		sectionStart = "[ENDING_NORMAL]";
		sectionEnd = "[ENDING_NORMAL_END]";
		break;
	case GameEndState::Victory_Bad:
	default:
		sectionStart = "[ENDING_BAD]";
		sectionEnd = "[ENDING_BAD_END]";
		break;
	}

	std::string line;
	bool inSection = false;
	std::ostringstream content;

	while (std::getline(file, line)) {
		if (line.find(sectionStart) != std::string::npos) {
			inSection = true;
			continue;
		}
		if (line.find(sectionEnd) != std::string::npos) {
			break;
		}
		if (inSection) {
			content << line << "\n";
		}
	}

	endingText = FormatEndingText(content.str());
	textLoaded = true;

	// 타자기 효과로 출력
	typewriter_print(endingText, 30);
}

std::string EndingScene::FormatEndingText(const std::string& text)
{
	std::string result = text;

	// %DATE% 치환
	if (world) {
		std::string date = std::format("2156-{:02d}-{:02d}", world->GetMonth(), world->GetDay());
		size_t pos;
		while ((pos = result.find("%DATE%")) != std::string::npos) {
			result.replace(pos, 6, date);
		}

		// %POP% 치환
		std::string pop = std::to_string(world->GetHumansSize());
		while ((pos = result.find("%POP%")) != std::string::npos) {
			result.replace(pos, 5, pop);
		}

		// %DAYS% 치환
		std::string days = std::to_string(world->GetCurrentDay());
		while ((pos = result.find("%DAYS%")) != std::string::npos) {
			result.replace(pos, 6, days);
		}
	}

	return result;
}

// ==================== GameOverScene ====================

void GameOverScene::Enter(std::unique_ptr<World>& w)
{
	system("cls");
	world = w.get();
	textLoaded = false;
	displayComplete = false;
	LoadGameOverText();
}

void GameOverScene::Update(float deltaTime)
{
	// 특별한 업데이트 없음
}

void GameOverScene::Display()
{
	if (!displayComplete) {
		gotoxy(0, 0);
		std::cout << gameOverText << std::endl;
		std::cout << "\n\n    [아무 키나 누르면 타이틀로 돌아갑니다]" << std::endl;
		displayComplete = true;
	}
}

void GameOverScene::Exit()
{
	sceneChangeRequested = false;
	oldScene = "gameover";
}

void GameOverScene::HandleInput(char input)
{
	// 아무 키나 누르면 타이틀로
	RequestSceneChange("start");
}

void GameOverScene::sHandleInput(char input)
{
	// 방향키도 타이틀로
	RequestSceneChange("start");
}

void GameOverScene::LoadGameOverText()
{
	if (textLoaded) return;

	std::string filepath;
	switch (gameOverType) {
	case GameEndState::GameOver_Coup:
		filepath = "data/gameover_coup.txt";
		break;
	case GameEndState::GameOver_Collapse:
		filepath = "data/gameover_collapse.txt";
		break;
	case GameEndState::GameOver_Exodus:
		filepath = "data/gameover_exodus.txt";
		break;
	case GameEndState::GameOver_Starvation:
		filepath = "data/gameover_starvation.txt";
		break;
	default:
		filepath = "data/gameover_collapse.txt";
		break;
	}

	std::ifstream file(GetFullPath(filepath));

	if (!file.is_open()) {
		gameOverText = "[게임오버 텍스트를 불러올 수 없습니다]";
		textLoaded = true;
		return;
	}

	std::ostringstream content;
	std::string line;

	while (std::getline(file, line)) {
		// 주석 라인 스킵
		if (line.empty() || line[0] == '#') continue;
		content << line << "\n";
	}

	gameOverText = FormatGameOverText(content.str());
	textLoaded = true;

	// 타자기 효과로 출력
	typewriter_print(gameOverText, 30);
}

std::string GameOverScene::FormatGameOverText(const std::string& text)
{
	std::string result = text;

	// %DATE% 치환
	if (world) {
		std::string date = std::format("2156-{:02d}-{:02d}", world->GetMonth(), world->GetDay());
		size_t pos;
		while ((pos = result.find("%DATE%")) != std::string::npos) {
			result.replace(pos, 6, date);
		}

		// %COLLAPSE_DAYS% 치환 (회생불가 일수)
		std::string collapseDays = std::to_string(world->GetCriticalDaysCount());
		while ((pos = result.find("%COLLAPSE_DAYS%")) != std::string::npos) {
			result.replace(pos, 15, collapseDays);
		}

		// %STARVE_DAYS% 치환 (기아 일수)
		std::string starveDays = std::to_string(world->GetStarvationDaysCount());
		while ((pos = result.find("%STARVE_DAYS%")) != std::string::npos) {
			result.replace(pos, 13, starveDays);
		}

		// %REMAIN% 치환 (남은 인원)
		std::string remain = std::to_string(world->GetHumansSize() / 2);  // 절반 이탈 가정
		while ((pos = result.find("%REMAIN%")) != std::string::npos) {
			result.replace(pos, 8, remain);
		}
	}

	return result;
}
