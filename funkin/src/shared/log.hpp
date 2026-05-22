// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

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
    // ! Critical will force a crash
    // ! If the game can keep running and wont explode, please only use Error
    enum class LogLevel {
        Print,
        Warning,
        Error,
        Critical
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

        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        std::tm tm_now;
        #ifdef _WIN32
            localtime_s(&tm_now, &time_t_now);
        #else
            localtime_r(&time_t_now, &tm_now);
        #endif

        std::string_view levelStr;
        std::string_view levelColor;
        std::ostream* out = &std::cout;

        switch (level) {
            case LogLevel::Print:    levelStr = "PRINT"; levelColor = Detail::Cyan;    break;
            case LogLevel::Warning:  levelStr = "WARN";  levelColor = Detail::Yellow;  break;
            case LogLevel::Error:    levelStr = "ERROR"; levelColor = Detail::Red;     out = &std::cerr; break;
            case LogLevel::Critical: levelStr = "CRIT";  levelColor = Detail::Magenta; out = &std::cerr; break;
        }

        std::string message;
        if constexpr (sizeof...(Args) > 0) {
            message = std::vformat(fmt, std::make_format_args(args...));
        } else {
            message = fmt;
        }

        *out << std::format("{}[{:02d}:{:02d}:{:02d}]{} {}[{}]{} ",
            Detail::Dim, tm_now.tm_hour, tm_now.tm_min, tm_now.tm_sec, Detail::Reset,
            levelColor, levelStr, Detail::Reset
        );

        if (!file.empty()) {
            *out << std::format("{}[{}:{}]{} ", Detail::Dim, StripPath(file), line, Detail::Reset);
        }

        *out << message << "\n";
    }
}

#if defined(FUNKIN_DEBUG) || defined(FORCE_PRINT_ON_RELEASE)
    #define LOG_PRINT(fmt, ...) ::Funkin::Shared::LogMessage(::Funkin::Shared::LogLevel::Print,    __FILE__, __LINE__, fmt, ##__VA_ARGS__)
    #define LOG_WARN(fmt, ...)  ::Funkin::Shared::LogMessage(::Funkin::Shared::LogLevel::Warning,  __FILE__, __LINE__, fmt, ##__VA_ARGS__)
    #define LOG_ERR(fmt, ...)   ::Funkin::Shared::LogMessage(::Funkin::Shared::LogLevel::Error,    __FILE__, __LINE__, fmt, ##__VA_ARGS__)
    #define LOG_CRIT(fmt, ...)  ::Funkin::Shared::LogMessage(::Funkin::Shared::LogLevel::Critical, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#else
    #define LOG_PRINT(fmt, ...)
    #define LOG_WARN(fmt, ...)
    #define LOG_ERR(fmt, ...)
    #define LOG_CRIT(fmt, ...)
#endif