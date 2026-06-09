// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#pragma once
#include <string>
#include <vector>
#include <functional>
#include <mutex>

namespace Funkin::UI {
    enum class ConsoleLevel { Info, Warn, Error };

    struct ConsoleEntry {
        ConsoleLevel level;
        std::string  message;
    };

    class ConsoleLog {
    public:
        using Sink = std::function<void(const ConsoleEntry&)>;

        static ConsoleLog& get() {
            static ConsoleLog s;
            return s;
        }

        void info (const std::string& msg) { push({ ConsoleLevel::Info,  msg }); }
        void warn (const std::string& msg) { push({ ConsoleLevel::Warn,  msg }); }
        void error(const std::string& msg) { push({ ConsoleLevel::Error, msg }); }

        void addSink(Sink s) {
            std::lock_guard lock(m_mutex);
            m_sinks.push_back(s);
        }

        const std::vector<ConsoleEntry>& entries() const { return m_entries; }
        void clear() {
            std::lock_guard lock(m_mutex);
            m_entries.clear();
        }

    private:
        void push(ConsoleEntry e) {
            std::lock_guard lock(m_mutex);
            m_entries.push_back(e);
            for (auto& s : m_sinks) s(e);
        }

        std::vector<ConsoleEntry> m_entries;
        std::vector<Sink>         m_sinks;
        std::mutex                m_mutex;
    };
}

// do NOT log debug shit, plsss
#define UI_LOG_INFO(msg)  Funkin::UI::ConsoleLog::get().info(msg)
#define UI_LOG_WARN(msg)  Funkin::UI::ConsoleLog::get().warn(msg)
#define UI_LOG_ERROR(msg) Funkin::UI::ConsoleLog::get().error(msg)