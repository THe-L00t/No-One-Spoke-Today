#include "Toolkit.h"

size_t utf8_char_length(unsigned char c)
{
    if ((c & 0x80) == 0) return 1;
    else if ((c & 0xE0) == 0xC0) return 2;
    else if ((c & 0xF0) == 0xE0) return 3;
    else if ((c & 0xF8) == 0xF0) return 4;
    return 1;
}

void typewriter_print(const std::string& text, int delay_ms) {
    for (size_t i = 0; i < text.size();) {
        size_t len = utf8_char_length((unsigned char)text[i]);

        std::cout << text.substr(i, len) << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));

        i += len;
    }
    std::cout << std::endl;
}