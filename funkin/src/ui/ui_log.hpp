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
        std::string  timestamp;
        std::string  engineId = ""; // ? used only in engine logs, not player logs
    };

    class ConsoleLog {
    public:
        using Sink = std::function<void(const ConsoleEntry&)>;

        static ConsoleLog& get() {
            static ConsoleLog s;
            return s;
        }

        void info (const std::string& msg, const std::string& id = "") { push({ ConsoleLevel::Info,  msg, now(), id }); }
        void warn (const std::string& msg, const std::string& id = "") { push({ ConsoleLevel::Warn,  msg, now(), id }); }
        void error(const std::string& msg, const std::string& id = "") { push({ ConsoleLevel::Error, msg, now(), id }); }

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

        static std::string now() {
            auto t  = std::chrono::system_clock::now();
            auto tt = std::chrono::system_clock::to_time_t(t);
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                t.time_since_epoch()) % 1000;
            std::tm tm{};
            localtime_s(&tm, &tt);
            char buf[32];
            snprintf(buf, sizeof(buf), "%02d:%02d:%02d.%03d",
                tm.tm_hour, tm.tm_min, tm.tm_sec, (int)ms.count());
            return buf;
        }

        std::vector<ConsoleEntry> m_entries;
        std::vector<Sink>         m_sinks;
        std::mutex                m_mutex;
    };
}

// ! do NOT log debug shit, plsss
#define UI_LOG_INFO(msg)  Funkin::UI::ConsoleLog::get().info(msg)
#define UI_LOG_WARN(msg)  Funkin::UI::ConsoleLog::get().warn(msg)
#define UI_LOG_ERROR(msg) Funkin::UI::ConsoleLog::get().error(msg)

// ! ONLY use for internal debugs you WANT the user to see
#define UI_ENGINE_INFO(msg)  Funkin::UI::ConsoleLog::get().info(msg,  __FILE__ ":" + std::to_string(__LINE__))
#define UI_ENGINE_WARN(msg)  Funkin::UI::ConsoleLog::get().warn(msg,  __FILE__ ":" + std::to_string(__LINE__))
#define UI_ENGINE_ERROR(msg) Funkin::UI::ConsoleLog::get().error(msg, __FILE__ ":" + std::to_string(__LINE__))