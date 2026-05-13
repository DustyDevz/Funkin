// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once

namespace Funkin::Platform {
    class Window {
    public:
        virtual ~Window() = default;

        virtual bool init(const std::string& title, int w, int h) = 0;
        virtual bool pump()     = 0;
        virtual bool shutdown() = 0;
        
        virtual void setBorderless(bool enabled) = 0;

        static std::unique_ptr<Window> create();
    };
}