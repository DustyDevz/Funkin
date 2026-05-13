// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once
#include <math/math_color.hpp>
#include <string>

namespace Funkin::UI {
    struct Theme {
        // backgrounds
        Math::Color bgBase       = { 0.08f, 0.08f, 0.08f, 1.0f };
        Math::Color bgPanel      = { 0.06f, 0.06f, 0.06f, 1.0f };
        Math::Color bgHeader     = { 0.06f, 0.06f, 0.06f, 1.0f };
        Math::Color bgItem       = { 0.10f, 0.10f, 0.10f, 1.0f };
        Math::Color bgItemHover  = { 0.14f, 0.14f, 0.14f, 1.0f };
        Math::Color bgItemSelect = { 0.10f, 0.10f, 0.10f, 1.0f };
        Math::Color bgInput      = { 0.11f, 0.11f, 0.11f, 1.0f };

        // borders
        Math::Color border       = { 0.15f, 0.15f, 0.15f, 1.0f };
        Math::Color borderFocus  = { 0.25f, 0.55f, 1.00f, 1.0f };
        Math::Color borderCard   = { 0.15f, 0.15f, 0.15f, 1.0f };

        // text
        Math::Color textPrimary   = { 0.95f, 0.95f, 0.95f, 1.0f };
        Math::Color textSecondary = { 0.42f, 0.42f, 0.42f, 1.0f };
        Math::Color textDisabled  = { 0.26f, 0.26f, 0.26f, 1.0f };
        Math::Color textAccent    = { 0.25f, 0.55f, 1.00f, 1.0f };
        Math::Color textMuted     = { 0.32f, 0.32f, 0.32f, 1.0f };

        // buttons
        Math::Color btnBg         = { 0.13f, 0.13f, 0.13f, 1.0f };
        Math::Color btnBgHover    = { 0.18f, 0.18f, 0.18f, 1.0f };
        Math::Color btnBgPress    = { 0.08f, 0.08f, 0.08f, 1.0f };
        Math::Color btnBgPrimary  = { 0.22f, 0.48f, 0.85f, 1.0f };
        Math::Color btnBgPrimaryH = { 0.28f, 0.55f, 0.95f, 1.0f };
        Math::Color btnBorder     = { 0.22f, 0.22f, 0.22f, 1.0f };

        // accent
        Math::Color accent        = { 0.22f, 0.48f, 0.85f, 1.0f };
        Math::Color accentHover   = { 0.28f, 0.55f, 0.95f, 1.0f };

        // nav
        Math::Color navActiveBg   = { 0.13f, 0.13f, 0.13f, 1.0f };
        Math::Color navActiveBar  = { 0.22f, 0.48f, 0.85f, 1.0f };

        Math::Color focusRing     = { 0.22f, 0.48f, 0.85f, 0.8f };
        float focusWidth    = 2.0f;

        // spacing
        float paddingXS = 4.0f;
        float paddingSM = 8.0f;
        float paddingMD = 16.0f;
        float paddingLG = 24.0f;
        float paddingXL = 32.0f;

        // font sizes
        float fontSizeSM = 11.0f;
        float fontSizeMD = 13.0f;
        float fontSizeLG = 16.0f;
        float fontSizeXL = 24.0f;

        // font
        std::string fontRegular = "fonts/reg.ttf";
        std::string fontBold    = "fonts/bold.ttf";

        // radi
        float radiusSM = 2.0f;
        float radiusMD = 3.0f;
        float radiusLG = 4.0f;

        static Theme& get() {
            static Theme s;
            return s;
        }
    };
}