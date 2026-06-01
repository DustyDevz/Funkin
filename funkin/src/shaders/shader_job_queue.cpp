// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#include "shader_job_queue.hpp"
#include "shader_program.hpp"
#include "shader_compiler.hpp"
#include "shader_cache.hpp"

namespace Funkin::Shader {
    struct PendingResult {
        std::filesystem::path vsCachePath;
        std::filesystem::path fsCachePath;
        std::function<void(bgfx::ProgramHandle)> onComplete;
        bool success;
    };

    static std::queue<PendingResult> s_resultQueue;
    static std::mutex                s_resultMutex;
    static std::atomic<int>          s_activeJobs{ 0 };
    static std::vector<Program*>     s_programs;
    static std::mutex                s_programMutex;

    void registerProgram(Program* p) {
        std::lock_guard lock(s_programMutex);
        s_programs.push_back(p);
    }

    void unregisterProgram(Program* p) {
        std::lock_guard lock(s_programMutex);
        s_programs.erase(std::remove(s_programs.begin(), s_programs.end(), p), s_programs.end());
    }

    void submitShaderJob(ShaderJob job) {
        s_activeJobs++;

        std::thread([job = std::move(job)]() mutable {
            Cache::ensureCacheDir();

            auto suffix  = rendererSuffix();
            auto vsCache = Cache::resolveCachePath(job.name, "vs", suffix);
            auto fsCache = Cache::resolveCachePath(job.name, "fs", suffix);

            auto vsFuture = std::async(std::launch::async, [&]() -> bool {
                if (Cache::isCached(vsCache)) return true;
                return compileShaderToDisk(job.shader.vs.c_str(), "vertex", vsCache.string());
            });

            auto fsFuture = std::async(std::launch::async, [&]() -> bool {
                if (Cache::isCached(fsCache)) return true;
                return compileShaderToDisk(job.shader.fs.c_str(), "fragment", fsCache.string());
            });

            bool ok = vsFuture.get() && fsFuture.get();

            {
                std::lock_guard lock(s_resultMutex);
                s_resultQueue.push({ vsCache, fsCache, std::move(job.onComplete), ok });
            }

            s_activeJobs--;
        }).detach();
    }

    void tickShaderJobs() {
        {
            std::lock_guard lock(s_resultMutex);
            while (!s_resultQueue.empty()) {
                auto& pending = s_resultQueue.front();

                if (!pending.success) {
                    LOG_ERR("Shader job failed, keeping existing program");
                    pending.onComplete(BGFX_INVALID_HANDLE);
                    s_resultQueue.pop();
                    continue;
                }

                bgfx::ShaderHandle vs = Cache::loadFromCache(pending.vsCachePath);
                bgfx::ShaderHandle fs = Cache::loadFromCache(pending.fsCachePath);

                bgfx::ProgramHandle prog = BGFX_INVALID_HANDLE;
                if (bgfx::isValid(vs) && bgfx::isValid(fs)) {
                    prog = bgfx::createProgram(vs, fs, true);
                    LOG_PRINT("Shader job complete, program created");
                } else {
                    LOG_ERR("Shader job produced invalid handles");
                }

                pending.onComplete(prog);
                s_resultQueue.pop();
            }
        }

        {
            std::lock_guard lock(s_programMutex);
            for (auto* p : s_programs) {
                if (bgfx::isValid(p->m_pending)) {
                    if (bgfx::isValid(p->m_handle))
                        bgfx::destroy(p->m_handle);
                    p->m_handle  = p->m_pending;
                    p->m_pending = BGFX_INVALID_HANDLE;
                    LOG_PRINT("Shader program hot-swapped");
                }
            }
        }
    }

    void shutdownShaderJobs() {
        while (s_activeJobs > 0)
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}