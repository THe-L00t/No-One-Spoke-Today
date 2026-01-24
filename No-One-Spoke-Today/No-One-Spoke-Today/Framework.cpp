#include "Framework.h"

Framework::Framework()
{
	timer = std::make_unique<Time>();
}

Framework::~Framework()
{
	std::cout << " 게임을 종료합니다... " << std::endl;
}

void Framework::Init()
{
}

void Framework::Loop()
{
    while (true) {
        if (_kbhit()) {
            char input = _getch();

        }

    }
}

void Framework::Destroy()
{
}
