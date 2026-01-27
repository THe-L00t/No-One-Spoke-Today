#include "framework.h"
#include "Toolkit.h"

int main() {
	/*Framework game;
	game.Init();
	game.Loop();
	game.Destroy();*/
	std::string test;
	LoadText(test, "titleASCIIart.txt");
	std::cout << test << std::endl;
}