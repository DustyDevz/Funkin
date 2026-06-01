// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#pragma once
#include <string>
#include <windows.h>

namespace Funkin::Shared {
    std::string wideToUTF8(const std::wstring& ws);
}