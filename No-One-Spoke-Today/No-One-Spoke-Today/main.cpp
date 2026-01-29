#include "framework.h"
#include "Toolkit.h"
#include <print>

// 콘솔 초기화 함수
void InitConsole() {
	// UTF-8 입출력 설정
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);

	// 콘솔 모드 설정 (유니코드 지원 강화)
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD dwMode = 0;
	if (GetConsoleMode(hOut, &dwMode)) {
		// 가상 터미널 처리 활성화 (Windows 10+)
		dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
		SetConsoleMode(hOut, dwMode);
	}

	// 콘솔 폰트 확인 안내 (유니코드 박스 문자 지원 폰트 필요)
	CONSOLE_FONT_INFOEX cfi;
	cfi.cbSize = sizeof(cfi);
	if (GetCurrentConsoleFontEx(hOut, FALSE, &cfi)) {
		// 폰트가 래스터 폰트면 경고
		if (wcscmp(cfi.FaceName, L"Terminal") == 0 ||
			wcscmp(cfi.FaceName, L"") == 0) {
			std::cout << "[!] 콘솔 폰트를 'Consolas' 또는 'NSimSun'으로 변경하세요.\n";
			std::cout << "    (콘솔 제목 우클릭 > 속성 > 글꼴)\n\n";
		}
	}
}

int main() {
	InitConsole();

	std::string ASCII_image;
	LoadText(ASCII_image, "titleASCIIart.txt");
	std::cout << ASCII_image << std::endl;

	Framework game;
	game.Init();
	game.Loop();
	game.Destroy();

	return 0;
}