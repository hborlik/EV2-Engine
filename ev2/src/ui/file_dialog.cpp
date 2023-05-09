#include "file_dialog.hpp"

#include <cstring>
#include <vector>
#include <algorithm>
#include <filesystem>

#include "imgui.hpp"
#include "imgui_internal.hpp"

namespace fs = std::filesystem;

namespace ev2::ui {

enum class FileDialogColumnID {
    Name,
    Size,
    Type,
    Modified
};

bool FileDialogWindow::show_file_dialog_modal(std::string_view name, std::string* select_path, bool relative_path) {
    using fs::path;

    bool did_select = false;

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (!ImGui::BeginPopupModal(name.data(), nullptr,
                                ImGuiWindowFlags_AlwaysAutoResize |
                                    ImGuiWindowFlags_NoCollapse |
                                    ImGuiWindowFlags_NoSavedSettings)) {
        return did_select;
    }

    static ImGuiTableFlags flags =
        ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable
        | ImGuiTableFlags_Sortable | ImGuiTableFlags_SortMulti
        | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_NoBordersInBody
        | ImGuiTableFlags_ScrollY
        | ImGuiTableFlags_SizingFixedFit;

    static int file_select_idx = 0;
    static int dir_select_idx = 0;
    static std::string dialog_current_file = "";
    static std::string dialog_current_dir = "";

    static std::string fileDialogError = "";
    static bool items_need_sort = false;
    static bool need_directory_refresh = true;

    if (current_path.empty())
        current_path = fs::current_path();

    static std::vector<fs::directory_entry> files{};
    static std::vector<fs::directory_entry> folders{};
    try {
        if (need_directory_refresh) {
            files = {};
            folders = {};
            for (auto& p : fs::directory_iterator(current_path)) {
                if (p.is_directory())
                    folders.push_back(p);
                if (p.is_regular_file())
                    files.push_back(p);
            }
            items_need_sort = true;
            need_directory_refresh = false;
        }
    }
    catch (...) {
        fileDialogError = "Error: could not open current_path";
        current_path = fs::current_path();
    }

    ImGui::BeginChild("Directories##1", ImVec2(200, 300), true, ImGuiWindowFlags_HorizontalScrollbar);

    if (ImGui::Selectable("..", false, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(ImGui::GetWindowContentRegionWidth(), 0))) {
        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
            current_path = fs::path(current_path).parent_path();
            dir_select_idx = 0;
            file_select_idx = 0;
            need_directory_refresh = true;
            dialog_current_dir = "";
            need_directory_refresh = true;
        }
    }
    for (int i = 0; i < folders.size(); ++i) {
        if (ImGui::Selectable(folders[i].path().stem().string().c_str(), i == dir_select_idx, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(ImGui::GetWindowContentRegionWidth(), 0))) {
            dialog_current_file = "";
            if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                ImGui::SetScrollHereY(0.0f);
                current_path = folders[i].path();
                dir_select_idx = 0;
                file_select_idx = 0;
                dialog_current_dir = "";
                need_directory_refresh = true;
            }
            else {
                dir_select_idx = i;
                dialog_current_dir = folders[i].path().stem().string();
            }
        }
    }
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild("Files##1", ImVec2(516 , 300 ), true, ImGuiWindowFlags_HorizontalScrollbar);
    ImGui::TextWrapped("%s", current_path.c_str());

    if (ImGui::BeginTable("table_advanced", 4, flags,  ImVec2(0, 0), 0.f)) {
        ImGui::TableSetupColumn("File", ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_WidthStretch, 0.0f, (int)FileDialogColumnID::Name);
        ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed, 0.f, (int)FileDialogColumnID::Size);
        ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 0.f, (int)FileDialogColumnID::Type);
        ImGui::TableSetupColumn("Date", ImGuiTableColumnFlags_WidthFixed, 0.f, (int)FileDialogColumnID::Modified);

        // Sort our data if sort specs have been changed!
        ImGuiTableSortSpecs* sorts_specs = ImGui::TableGetSortSpecs();
        // Store in variable accessible by the sort function.
        auto dir_entry_compare = [sorts_specs](const fs::directory_entry& a, const fs::directory_entry& b) -> bool {

            for (int n = 0; n < sorts_specs->SpecsCount; n++)
            {
                // Identify columns using the ColumnUserID value that was passed to TableSetupColumn()
                // 
                const ImGuiTableColumnSortSpecs* sort_spec = &sorts_specs->Specs[n];
                const bool ascending = (sort_spec->SortDirection == ImGuiSortDirection_Ascending);
                const auto sort_mode = (FileDialogColumnID)sort_spec->ColumnUserID;
                
                int delta = 0;
                switch(sort_mode) {
                    case FileDialogColumnID::Name:      
                        delta = strcmp(a.path().c_str(), b.path().c_str()); 
                        break;
                    case FileDialogColumnID::Size:
                        delta = a.file_size() - b.file_size();
                        break;
                    case FileDialogColumnID::Type:
                        delta = strcmp(a.path().extension().c_str(), b.path().extension().c_str()); 
                        break;
                    case FileDialogColumnID::Modified:
                        delta = a.last_write_time() > b.last_write_time(); 
                        break;
                    default:
                        break;
                }
                if (delta > 0)
                    return ascending;
                if (delta < 0)
                    return !ascending;
            }

            return strcmp(a.path().c_str(), b.path().c_str()) > 0;
        };
        if (sorts_specs && sorts_specs->SpecsDirty)
            items_need_sort = true;
        if (sorts_specs && items_need_sort && files.size() > 1)
        {
            std::sort(files.begin(), files.end(), dir_entry_compare);
            sorts_specs->SpecsDirty = false;
        }
        items_need_sort = false;

        ImGui::TableHeadersRow();

        for (int i = 0; i < files.size(); ++i) {
            const bool item_is_selected = i == file_select_idx;
            ImGui::PushID(i);
            ImGui::TableNextRow(ImGuiTableRowFlags_None);
            if (ImGui::TableSetColumnIndex(0)) {
                if (ImGui::Selectable(files[i].path().filename().string().c_str(), item_is_selected, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(0, 0))) {
                    file_select_idx = i;
                    dialog_current_file = files[i].path().filename().string();
                    dialog_current_dir = "";
                }
            }

            if (files[i].is_regular_file()) {
                if (ImGui::TableSetColumnIndex(1)) {
                    ImGui::TextUnformatted(std::to_string(files[i].file_size()).c_str());
                }
                
                if (ImGui::TableSetColumnIndex(2)) {
                    ImGui::TextUnformatted(files[i].path().extension().string().c_str());
                }

                if (ImGui::TableSetColumnIndex(3)) {
                    auto ftime = files[i].last_write_time();
                    auto st = std::chrono::time_point_cast<std::chrono::system_clock::duration>(ftime - decltype(ftime)::clock::now() + std::chrono::system_clock::now());
                    std::time_t tt = std::chrono::system_clock::to_time_t(st);
                    std::tm* mt = std::localtime(&tt);
                    std::stringstream ss;
                    ss << std::put_time(mt, "%F %R");
                    ImGui::TextUnformatted(ss.str().c_str());
                }
            }
            
            ImGui::PopID();
        }

        ImGui::EndTable();
    }
    
    ImGui::EndChild();

    std::string selectedFilePath = path{dialog_current_dir.size() > 0 ? dialog_current_dir : dialog_current_file};
    selectedFilePath.resize(500);
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
    if (ImGui::InputText("##input path", selectedFilePath.data(), selectedFilePath.capacity())) {
        dialog_current_file = selectedFilePath;
    }

    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 6);

    if (ImGui::Button("New folder")) {
        ImGui::OpenPopup("NewFolderPopup");
    }
    ImGui::SameLine();

    static bool disableDeleteButton = false;
    disableDeleteButton = (dialog_current_dir == "");
    if (disableDeleteButton) {
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
    }
    if (ImGui::Button("Delete folder")) {
        ImGui::OpenPopup("DeleteFolderPopup");
    }
    if (disableDeleteButton) {
        ImGui::PopStyleVar();
        ImGui::PopItemFlag();
    }

    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopup("NewFolderPopup", ImGuiWindowFlags_Modal)) {
        ImGui::Text("Enter a name for the new folder");
        static char newFolderName[500] = "";
        static char newFolderError[500] = "";
        ImGui::InputText("##input folder name", newFolderName, sizeof(newFolderName));
        if (ImGui::Button("Create##1")) {
            if (strlen(newFolderName) <= 0) {
                strcpy(newFolderError, "Folder name can't be empty");
            }
            else {
                std::string newFilePath = current_path / path{newFolderName};
                fs::create_directory(newFilePath);
                need_directory_refresh = true;
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel##1")) {
            strcpy(newFolderName, "");
            strcpy(newFolderError, "");
            ImGui::CloseCurrentPopup();
        }
        ImGui::TextColored(ImColor(1.0f, 0.0f, 0.2f, 1.0f), newFolderError);
        ImGui::EndPopup();
    }

    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopup("DeleteFolderPopup", ImGuiWindowFlags_Modal)) {
        ImGui::TextColored(ImColor(1.0f, 0.0f, 0.2f, 1.0f), "Are you sure you want to delete this folder?");
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 6);
        ImGui::TextUnformatted(dialog_current_dir.c_str());
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 6);
        if (ImGui::Button("Yes")) {
            fs::remove(current_path / path{dialog_current_dir});
            need_directory_refresh = true;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("No")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 120 );

    if (ImGui::Button("Cancel")) {
        file_select_idx = 0;
        dir_select_idx = 0;
        dialog_current_file = "";
        ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button("Choose")) {
        need_directory_refresh = true;
        path output_path{};
        if (type == FileDialogType::SelectFolder) {
            if (dialog_current_dir == "") {
                fileDialogError = "Error: You must select a directory!";
            }
            else {
                output_path = path{current_path} / path{dialog_current_dir};
                file_select_idx = 0;
                dir_select_idx = 0;
                dialog_current_file = "";
                ImGui::CloseCurrentPopup();
                did_select = true;
            }
        } else if (type == FileDialogType::OpenFile) {
            if (dialog_current_file == "") {
                fileDialogError = "Error: You must select a file!";
            }
            else {
                output_path = current_path / path{dialog_current_file};
                file_select_idx = 0;
                dir_select_idx = 0;
                dialog_current_file = "";
                ImGui::CloseCurrentPopup();
                did_select = true;
            }
        }
        if (relative_path)
            output_path = std::filesystem::relative(output_path);
        *select_path = output_path;
    }

    if (!fileDialogError.empty()) {
        ImGui::TextColored(ImColor(1.0f, 0.0f, 0.2f, 1.0f), "%s", fileDialogError.c_str());
    }

    ImGui::EndPopup();
    return did_select;
}

}