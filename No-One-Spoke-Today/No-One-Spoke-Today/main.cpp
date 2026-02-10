#include "framework.h"
#include "Toolkit.h"
#include <print>

// 번들 폰트 등록 및 콘솔 폰트 설정
bool InstallBundledFont(const std::wstring& fontPath, const wchar_t* fontName) {
	// 폰트 파일을 세션에 임시 등록 (프로세스 종료 시 자동 해제)
	int result = AddFontResourceExW(fontPath.c_str(), FR_PRIVATE, 0);
	if (result == 0) return false;

	// 콘솔 폰트 설정
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_FONT_INFOEX cfi = {};
	cfi.cbSize = sizeof(cfi);
	cfi.nFont = 0;
	cfi.dwFontSize.X = 0;
	cfi.dwFontSize.Y = 18;
	cfi.FontFamily = FF_DONTCARE;
	cfi.FontWeight = FW_NORMAL;
	wcscpy_s(cfi.FaceName, fontName);

	return SetCurrentConsoleFontEx(hOut, FALSE, &cfi) != 0;
}

// 콘솔 초기화 함수
void InitConsole() {
	// 콘솔 제목 설정
	SetConsoleTitleW(L"No One Spoke Today");

	// 콘솔창 아이콘 설정
	HWND consoleWnd = GetConsoleWindow();
	if (consoleWnd) {
		// EXE 기준 경로로 아이콘 로드
		std::string iconPath = GetFullPath("resource\\Icon.ico");
		std::wstring wIconPath(iconPath.begin(), iconPath.end());

		HICON hIcon = (HICON)LoadImageW(
			NULL,
			wIconPath.c_str(),
			IMAGE_ICON,
			0, 0,
			LR_LOADFROMFILE | LR_DEFAULTSIZE
		);
		if (hIcon) {
			SendMessage(consoleWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
			SendMessage(consoleWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
		}
	}

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

	// 번들 폰트 로드 (data/fonts/D2Coding.ttf)
	std::string fontPathA = GetFullPath("data\\fonts\\D2Coding.ttf");
	std::wstring fontPathW(fontPathA.begin(), fontPathA.end());

	if (!InstallBundledFont(fontPathW, L"D2Coding")) {
		// 번들 폰트 실패 시 시스템 폰트로 폴백
		CONSOLE_FONT_INFOEX cfi = {};
		cfi.cbSize = sizeof(cfi);
		cfi.dwFontSize.Y = 18;
		cfi.FontFamily = FF_DONTCARE;
		cfi.FontWeight = FW_NORMAL;
		wcscpy_s(cfi.FaceName, L"NSimSun");
		if (!SetCurrentConsoleFontEx(hOut, FALSE, &cfi)) {
			// NSimSun도 없으면 Consolas로 최종 폴백
			wcscpy_s(cfi.FaceName, L"Consolas");
			SetCurrentConsoleFontEx(hOut, FALSE, &cfi);
		}
	}
}

int main() {
	InitConsole();

	std::string ASCII_image;
	LoadText(ASCII_image, "data/titleASCIIart.txt");
	std::cout << ASCII_image << std::endl;

	Framework game;
	game.Init();
	game.Loop();
	game.Destroy();

	return 0;
}