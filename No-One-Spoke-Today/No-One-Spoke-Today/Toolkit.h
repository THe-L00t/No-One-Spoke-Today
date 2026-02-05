#pragma once
#include "pch.h"

// EXE 파일 기준 경로 반환
std::string GetExeDirectory();
std::string GetFullPath(const std::string& relativePath);

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

std::vector<std::string> loadSentences(const std::string& filename);