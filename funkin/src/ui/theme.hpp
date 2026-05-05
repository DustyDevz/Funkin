// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once
#include <math/color.hpp>
#include <string>

namespace Funkin::UI {
    struct Theme {
        // backgrounds
        Color bgBase       = { 0.08f, 0.08f, 0.08f, 1.0f };
        Color bgPanel      = { 0.06f, 0.06f, 0.06f, 1.0f };
        Color bgHeader     = { 0.06f, 0.06f, 0.06f, 1.0f };
        Color bgItem       = { 0.10f, 0.10f, 0.10f, 1.0f };
        Color bgItemHover  = { 0.14f, 0.14f, 0.14f, 1.0f };
        Color bgItemSelect = { 0.10f, 0.10f, 0.10f, 1.0f };
        Color bgInput      = { 0.11f, 0.11f, 0.11f, 1.0f };

        // borders
        Color border       = { 0.15f, 0.15f, 0.15f, 1.0f };
        Color borderFocus  = { 0.25f, 0.55f, 1.00f, 1.0f };
        Color borderCard   = { 0.15f, 0.15f, 0.15f, 1.0f };

        // text
        Color textPrimary   = { 0.95f, 0.95f, 0.95f, 1.0f };
        Color textSecondary = { 0.42f, 0.42f, 0.42f, 1.0f };
        Color textDisabled  = { 0.26f, 0.26f, 0.26f, 1.0f };
        Color textAccent    = { 0.25f, 0.55f, 1.00f, 1.0f };
        Color textMuted     = { 0.32f, 0.32f, 0.32f, 1.0f };

        // buttons
        Color btnBg         = { 0.13f, 0.13f, 0.13f, 1.0f };
        Color btnBgHover    = { 0.18f, 0.18f, 0.18f, 1.0f };
        Color btnBgPress    = { 0.08f, 0.08f, 0.08f, 1.0f };
        Color btnBgPrimary  = { 0.22f, 0.48f, 0.85f, 1.0f };
        Color btnBgPrimaryH = { 0.28f, 0.55f, 0.95f, 1.0f };
        Color btnBorder     = { 0.22f, 0.22f, 0.22f, 1.0f };

        // accent
        Color accent        = { 0.22f, 0.48f, 0.85f, 1.0f };
        Color accentHover   = { 0.28f, 0.55f, 0.95f, 1.0f };

        // nav
        Color navActiveBg   = { 0.13f, 0.13f, 0.13f, 1.0f };
        Color navActiveBar  = { 0.22f, 0.48f, 0.85f, 1.0f };

        Color focusRing     = { 0.22f, 0.48f, 0.85f, 0.8f };
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