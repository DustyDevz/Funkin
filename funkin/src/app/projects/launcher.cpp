// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#include "launcher.hpp"
#include "project.hpp"
#include "shared/log.hpp"
#include <imgui.h>
#include <filesystem>
#include <string>
#include <vector>

namespace Funkin::App {
    bool RunLauncher() {
        static std::vector<RecentProject> s_recents    = Project::loadRecent();
        static char s_newName[128]                     = "";
        static char s_newAuthor[128]                   = "";
        static char s_newVersion[32]                   = "";
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
                ImGui::InputText("Author",  s_newAuthor,  sizeof(s_newAuthor));
                ImGui::InputText("Version", s_newVersion, sizeof(s_newVersion));
                ImGui::InputText("Folder",  s_newFolder,  sizeof(s_newFolder));

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
                        if (Project::get().create(s_newFolder, s_newName, s_newAuthor, s_newVersion)) {
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