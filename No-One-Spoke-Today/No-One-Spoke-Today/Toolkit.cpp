#include "Toolkit.h"

// EXE 파일이 있는 디렉토리 경로 반환
std::string GetExeDirectory() {
    static std::string exeDir;
    if (exeDir.empty()) {
        wchar_t path[MAX_PATH];
        GetModuleFileNameW(NULL, path, MAX_PATH);

        // 파일명 제거하고 디렉토리만 남기기
        std::wstring wpath(path);
        size_t pos = wpath.find_last_of(L"\\/");
        if (pos != std::wstring::npos) {
            wpath = wpath.substr(0, pos + 1);
        }

        // wstring -> string 변환
        int size = WideCharToMultiByte(CP_UTF8, 0, wpath.c_str(), -1, nullptr, 0, nullptr, nullptr);
        exeDir.resize(size - 1);
        WideCharToMultiByte(CP_UTF8, 0, wpath.c_str(), -1, &exeDir[0], size, nullptr, nullptr);
    }
    return exeDir;
}

// data 폴더가 현재 작업 디렉토리에 있는지 확인 (디버그 모드 판별)
static bool IsDebugMode() {
    static int cached = -1;
    if (cached < 0) {
        DWORD attr = GetFileAttributesA("data");
        cached = (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY)) ? 1 : 0;
    }
    return cached == 1;
}

// 상대 경로를 절대 경로로 변환 (디버그/릴리즈 모두 지원)
std::string GetFullPath(const std::string& relativePath) {
    // 디버그 모드: 현재 작업 디렉토리 사용 (Visual Studio)
    if (IsDebugMode()) {
        return relativePath;
    }
    // 릴리즈 모드: EXE 디렉토리 기준
    return GetExeDirectory() + relativePath;
}

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

void LoadText(std::string& intro, const std::string& path)
{
    std::string fullPath = GetFullPath(path);
    std::ifstream in{ fullPath };
    if (not in) return ;

    std::ostringstream ss;
    ss << in.rdbuf();   // 핵심 한 줄
    intro = ss.str();
}

std::vector<std::string> loadSentences(const std::string& filename) {
    std::vector<std::string> sentences;
    std::string fullPath = GetFullPath(filename);
    std::ifstream file(fullPath);

    if (!file.is_open()) {
        std::cerr << "파일을 열 수 없습니다: " << fullPath << "\n";
        return sentences;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty()) {          // 빈 줄 무시
            sentences.push_back(line);
        }
    }

    file.close();
    return sentences;
}