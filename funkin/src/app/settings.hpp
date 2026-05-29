// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#pragma once

namespace Funkin {
    struct Settings {
        enum class VSyncMode { Off, On };
        VSyncMode vsync   = VSyncMode::On;
        int fpsCap        = 60;
        int windowWidth   = 1280;
        int windowHeight = 720;
    };
}