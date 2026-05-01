// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once
#include <math/color.hpp>
#include <string>

namespace Funkin::UI {
    struct Theme {
        // backgrounds
        Color bgBase        = { 0.10f, 0.10f, 0.11f, 1.0f };
        Color bgPanel       = { 0.14f, 0.14f, 0.16f, 1.0f };
        Color bgItem        = { 0.18f, 0.18f, 0.20f, 1.0f };
        Color bgItemHover   = { 0.24f, 0.24f, 0.27f, 1.0f };
        Color bgItemSelect  = { 0.27f, 0.42f, 0.65f, 1.0f };

        // borders
        Color border        = { 0.25f, 0.25f, 0.28f, 1.0f };
        Color borderFocus   = { 0.40f, 0.60f, 0.90f, 1.0f };

        // text
        Color textPrimary   = { 0.92f, 0.92f, 0.92f, 1.0f };
        Color textSecondary = { 0.55f, 0.55f, 0.60f, 1.0f };
        Color textDisabled  = { 0.35f, 0.35f, 0.38f, 1.0f };
        Color textAccent    = { 0.40f, 0.70f, 1.00f, 1.0f };

        // buttons
        Color btnBg         = { 0.20f, 0.20f, 0.23f, 1.0f };
        Color btnBgHover    = { 0.28f, 0.28f, 0.32f, 1.0f };
        Color btnBgPress    = { 0.15f, 0.15f, 0.17f, 1.0f };
        Color btnBgPrimary  = { 0.20f, 0.45f, 0.80f, 1.0f };
        Color btnBgPrimaryH = { 0.25f, 0.52f, 0.90f, 1.0f };

        // accent
        Color accent        = { 0.30f, 0.60f, 1.00f, 1.0f };

        // spacing
        float paddingXS  = 4.0f;
        float paddingSM  = 8.0f;
        float paddingMD  = 16.0f;
        float paddingLG  = 24.0f;

        // font sizes
        float fontSizeSM = 11.0f;
        float fontSizeMD = 13.0f;
        float fontSizeLG = 16.0f;
        float fontSizeXL = 22.0f;

        // font
        std::string fontPath = "fonts/Inter-Regular.ttf";
        std::string fontBold = "fonts/Inter-Bold.ttf";

        // radii
        float radiusSM = 3.0f;
        float radiusMD = 6.0f;
        float radiusLG = 10.0f;

        static Theme& get() {
            static Theme s;
            return s;
        }
    };
}