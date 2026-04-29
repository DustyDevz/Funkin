// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once

namespace Funkin::Scene {
    class Scene {
    public:
        virtual ~Scene() = default;

        virtual void init()     = 0;
        virtual void update()   = 0;
        virtual void draw()     = 0;
        virtual void shutdown() = 0;
    };
}