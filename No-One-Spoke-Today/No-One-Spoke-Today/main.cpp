#include "framework.h"
#include "Toolkit.h"

int main() {
	/*Framework game;
	game.Init();
	game.Loop();
	game.Destroy();*/
	std::ifstream in{ "title.txt" };
	if (not in) return 0;
	std::string title;
	std::ostringstream ss;
	ss << in.rdbuf();   // ÇÙ½É ÇÑ ÁÙ
	title = ss.str();
	typewriter_print(title);

}