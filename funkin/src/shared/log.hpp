#pragma once

#include <iostream>
#include <string_view>
#include <format>
#include <chrono>

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <Windows.h>
#endif

namespace Funkin::Shared {
    enum class LogLevel {
        Info,
        Warning,
        Error,
        Renderer
    };

    namespace Detail {
        constexpr std::string_view Reset   = "\x1b[0m";
        constexpr std::string_view Dim     = "\x1b[2m";
        constexpr std::string_view Cyan    = "\x1b[96m";
        constexpr std::string_view Yellow  = "\x1b[93m";
        constexpr std::string_view Red     = "\x1b[91m";
        constexpr std::string_view Magenta = "\x1b[95m";

        inline void EnableAnsi() {
            #ifdef _WIN32
                static bool enabled = false;
                if (enabled) return;
                HANDLE hOut  = GetStdHandle(STD_OUTPUT_HANDLE);
                HANDLE hErr  = GetStdHandle(STD_ERROR_HANDLE);
                DWORD mode = 0;
                if (GetConsoleMode(hOut, &mode)) SetConsoleMode(hOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
                if (GetConsoleMode(hErr, &mode)) SetConsoleMode(hErr, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
                enabled = true;
            #endif
        }
    }

    constexpr std::string_view StripPath(std::string_view path) {
        size_t pos = path.find_last_of("/\\");
        return pos == std::string_view::npos ? path : path.substr(pos + 1);
    }

    template<typename... Args>
    void LogMessage(LogLevel level, std::string_view file, int line, std::string_view fmt, Args&&... args) {
        Detail::EnableAnsi();

        auto now = std::chrono::floor<std::chrono::milliseconds>(std::chrono::system_clock::now());

        std::string_view levelStr;
        std::string_view levelColor;
        std::ostream*    out = &std::cout;

        switch (level) {
            case LogLevel::LOG:      levelStr = "LOG"; levelColor = Detail::Cyan;    break;
            case LogLevel::Warning:  levelStr = "WARN"; levelColor = Detail::Yellow;  break;
            case LogLevel::Error:    levelStr = "ERROR"; levelColor = Detail::Red;     out = &std::cerr; break;
        }

        auto message = std::vformat(fmt, std::make_format_args(args...));

        *out << std::format("{}[{:%H:%M:%S}]{} {}[{}]{} {}[{}:{}]{} {}\n",
            Detail::Dim,    now,             Detail::Reset,
            levelColor,     levelStr,        Detail::Reset,
            Detail::Dim,    StripPath(file), line, Detail::Reset,
            message
        );
    }

}

#if defined(FUNKIN_DEBUG) || defined(FORCE_PRINT_ON_RELEASE)
    #define LOG(fmt, ...)      ::Funkin::Shared::LogMessage(::Funkin::Shared::LogLevel::Log,      __FILE__, __LINE__, fmt, ##__VA_ARGS__)
    #define LOG_WARN(fmt, ...) ::Funkin::Shared::LogMessage(::Funkin::Shared::LogLevel::Warning,  __FILE__, __LINE__, fmt, ##__VA_ARGS__)
    #define LOG_ERR(fmt, ...)  ::Funkin::Shared::LogMessage(::Funkin::Shared::LogLevel::Error,    __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#else
    #define LOG(fmt, ...)
    #define LOG_WARN(fmt, ...)
    #define LOG_ERR(fmt, ...)
#endif