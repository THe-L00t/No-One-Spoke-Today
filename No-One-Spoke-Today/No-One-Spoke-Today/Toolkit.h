#pragma once
#include "pch.h"

size_t utf8_char_length(unsigned char c);
void typewriter_print(const std::string& text, int delay_ms = 40);