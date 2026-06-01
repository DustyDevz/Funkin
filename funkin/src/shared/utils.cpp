// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#include "utils.hpp"

namespace Funkin::Shared {
    std::string wideToUTF8(const std::wstring& ws) {
        int sizeNeeded = WideCharToMultiByte(
            CP_UTF8, 0,
            ws.c_str(), -1,
            nullptr, 0,
            nullptr, nullptr
        );

        std::string result(sizeNeeded - 1, 0);

        WideCharToMultiByte(
            CP_UTF8, 0,
            ws.c_str(), -1,
            result.data(), sizeNeeded,
            nullptr, nullptr
        );

        return result;
    }
}