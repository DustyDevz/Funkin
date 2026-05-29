// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#include "launcher.hpp"
#include "project.hpp"
#include "shared/log.hpp"
#include <imgui.h>
#include <filesystem>
#include <string>
#include <vector>

namespace Funkin::App {
    static std::string OpenFolderDialog() {
        std::string resultPath = "";
        #ifdef _WIN32
            HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
            if (SUCCEEDED(hr)) {
                IFileOpenDialog* pFileOpen = nullptr;
                hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));
                if (SUCCEEDED(hr)) {
                    DWORD dwOptions;
                    if (SUCCEEDED(pFileOpen->GetOptions(&dwOptions))) {
                        pFileOpen->SetOptions(dwOptions | FOS_PICKFOLDERS);
                    }
                    if (SUCCEEDED(pFileOpen->Show(NULL))) {
                        IShellItem* pItem = nullptr;
                        if (SUCCEEDED(pFileOpen->GetResult(&pItem))) {
                            PWSTR pszFilePath = nullptr;
                            if (SUCCEEDED(pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath))) {
                                std::wstring ws(pszFilePath);
                                resultPath = std::string(ws.begin(), ws.end());
                                CoTaskMemFree(pszFilePath);
                            }
                            pItem->Release();
                        }
                    }
                    pFileOpen->Release();
                }
                CoUninitialize();
            }
        #else
            FILE* pipe = popen("zenity --file-selection --directory 2>/dev/null", "r");
            if (!pipe) pipe = popen("kdialog --getexistingdirectory 2>/dev/null", "r");
            if (pipe) {
                char buffer[1024];
                if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                    resultPath = buffer;
                    if (!resultPath.empty() && resultPath.back() == '\n') {
                        resultPath.pop_back();
                    }
                }
                pclose(pipe);
            }
#       endif
        return resultPath;
    }

    bool RunLauncher() {
        static std::vector<RecentProject> s_recents    = Project::loadRecent();
        static char s_newName[128]                     = "";
        static char s_newFolder[512]                   = "";
        static char s_openPath[512]                    = "";
        static std::string s_error                     = "";

        ImGuiIO& io = ImGui::GetIO();
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f),
            ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_Always);
        ImGui::Begin("Funkin Engine", nullptr,
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoCollapse);

        if (!s_error.empty()) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0.3f, 0.3f, 1));
            ImGui::TextWrapped("%s", s_error.c_str());
            ImGui::PopStyleColor();
            ImGui::Separator();
        }

        bool shouldClose = false;

        if (ImGui::BeginTabBar("launcher_tabs")) {

            if (ImGui::BeginTabItem("Recent Projects")) {
                if (s_recents.empty()) {
                    ImGui::TextDisabled("No recent projects.");
                } else {
                    for (auto& rp : s_recents) {
                        ImGui::PushID(rp.path.c_str());
                        ImGui::Text("%s", rp.name.c_str());
                        ImGui::SameLine();
                        ImGui::TextDisabled("(%s)", rp.path.c_str());
                        ImGui::SameLine();
                        if (ImGui::SmallButton("Open")) {
                            auto projectFile = std::filesystem::path(rp.path) / "funkin.project";
                            if (Project::get().load(projectFile)) {
                                shouldClose = true;
                            } else {
                                s_error = "Failed to load project: " + rp.path;
                            }
                        }
                        ImGui::PopID();
                    }
                }

                ImGui::Separator();
                ImGui::Text("Open existing project:");
                ImGui::InputText("##openpath", s_openPath, sizeof(s_openPath));
                ImGui::SameLine();
                if (ImGui::Button("Browse##open")) {
                    std::string picked = OpenFolderDialog();
                    if (!picked.empty()) {
                        snprintf(s_openPath, sizeof(s_openPath), "%s", picked.c_str());
                    }
                }
                ImGui::SameLine();
                if (ImGui::Button("Open##btn")) {
                    auto projectFile = std::filesystem::path(s_openPath) / "funkin.project";
                    if (Project::get().load(projectFile)) {
                        s_recents = Project::loadRecent();
                        shouldClose = true;
                    } else {
                        s_error = "funkin.project not found in: " + std::string(s_openPath);
                    }
                }
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("New Project")) {
                ImGui::InputText("Name",    s_newName,    sizeof(s_newName));
                ImGui::InputText("Folder",  s_newFolder,  sizeof(s_newFolder));
                ImGui::SameLine();
                if (ImGui::Button("Browse##new")) {
                    std::string picked = OpenFolderDialog();
                    if (!picked.empty()) {
                        snprintf(s_newFolder, sizeof(s_newFolder), "%s", picked.c_str());
                    }
                }

                ImGui::Spacing();
                if (ImGui::Button("Create Project", ImVec2(-1, 0))) {
                    if (strlen(s_newName) == 0) {
                        s_error = "Name cannot be empty.";
                    } else if (strlen(s_newFolder) == 0) {
                        s_error = "Folder cannot be empty.";
                    } else if (!std::filesystem::exists(s_newFolder)) {
                        s_error = "Folder does not exist.";
                    } else {
                        s_error = "";
                        if (Project::get().create(s_newFolder, s_newName)) {
                            shouldClose = true;
                        } else {
                            s_error = "Failed to create project.";
                        }
                    }
                }
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::End();
        return shouldClose;
    }
}