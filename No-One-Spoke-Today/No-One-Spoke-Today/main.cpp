#include "framework.h"
#include "Toolkit.h"
#include <print>

int main() {
	SetConsoleOutputCP(CP_UTF8);
	/*std::string ASCII_image;
	LoadText(ASCII_image, "titleASCIIart.txt");
	std::cout << ASCII_image << std::endl;
	Framework game;
	game.Init();
	game.Loop();
	game.Destroy();*/
	std::cout << "-------------------------------------" << std::endl;
	std::cout << "        세이브 파일이 없습니다.        " << std::endl;
	std::cout << "-------------------------------------" << std::endl;
	std::println("        |{:>2} {:<15}|", 15, "이름");
	std::println("        |{:16}일|", 17);
	std::cout << std::endl;
	std::cout << "<이전    S:저장      L:로드     다음>" << std::endl;

}