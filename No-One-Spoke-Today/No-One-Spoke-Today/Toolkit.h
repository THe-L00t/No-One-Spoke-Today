#pragma once
#include "pch.h"

size_t utf8_char_length(unsigned char c);
void typewriter_print(const std::string& text, int delay_ms = 40);
void LoadText(std::string&,const std::string&);
inline void gotoxy(int x, int y)
{
    COORD Cur;
    Cur.X = x;
    Cur.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), Cur);
}