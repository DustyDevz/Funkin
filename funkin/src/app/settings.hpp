#pragma once

namespace Funkin {
    enum class VSyncMode {
        Off,
        On,
    };

    struct Settings {
        VSyncMode vsync = VSyncMode::On;
        int fpsCap      = 60;
    };

    inline Settings g_Settings;
}