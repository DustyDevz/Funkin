// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#include "registry.hpp"

namespace Funkin::App {
    void ApplyFileAssociation() {
        wchar_t exePath[MAX_PATH];
        GetModuleFileNameW(nullptr, exePath, MAX_PATH);
        std::wstring wExePath(exePath);
        std::wstring classesPath = L"Software\\Classes\\";
        
        std::wstring progIdPath = classesPath + L"Funkin.ProjectFile";
        HKEY hKey;

        if (RegCreateKeyExW(HKEY_CURRENT_USER, progIdPath.c_str(), 0, NULL, 
                            REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
                
            std::wstring friendlyDocName = L"Funkin Engine Project";
            RegSetValueExW(hKey, NULL, 0, REG_SZ, 
                        (const BYTE*)friendlyDocName.c_str(), (friendlyDocName.length() + 1) * sizeof(wchar_t));
            RegCloseKey(hKey);
        }

        std::wstring commandKeyPath = progIdPath + L"\\shell\\open\\command";
        if (RegCreateKeyExW(HKEY_CURRENT_USER, commandKeyPath.c_str(), 0, NULL, 
                            REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
            
            std::wstring commandStr = L"\"" + wExePath + L"\" \"%1\"";
            RegSetValueExW(hKey, NULL, 0, REG_SZ, 
                        (const BYTE*)commandStr.c_str(), (commandStr.length() + 1) * sizeof(wchar_t));
            RegCloseKey(hKey);
        }

        std::wstring iconKeyPath = progIdPath + L"\\DefaultIcon";
        if (RegCreateKeyExW(HKEY_CURRENT_USER, iconKeyPath.c_str(), 0, NULL, 
                            REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
            
            std::wstring iconStr = L"\"" + wExePath + L"\",0";
            RegSetValueExW(hKey, NULL, 0, REG_SZ, 
                        (const BYTE*)iconStr.c_str(), (iconStr.length() + 1) * sizeof(wchar_t));
            RegCloseKey(hKey);
        }

        std::wstring assocKeyPath = classesPath + L".funkin\\OpenWithProgids";
        if (RegCreateKeyExW(HKEY_CURRENT_USER, assocKeyPath.c_str(), 0, NULL, 
                            REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
            
            RegSetValueExW(hKey, L"Funkin.ProjectFile", 0, REG_NONE, NULL, 0);
            RegCloseKey(hKey);
        }

        SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
    }
}