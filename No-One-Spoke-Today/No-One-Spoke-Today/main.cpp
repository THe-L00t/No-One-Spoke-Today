#include "framework.h"
#include "Toolkit.h"

int main() {
	std::string ASCII_image;
	LoadText(ASCII_image, "titleASCIIart.txt");
	std::cout << ASCII_image << std::endl;
	Framework game;
	game.Init();
	game.Loop();
	game.Destroy();

}