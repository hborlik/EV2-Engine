/**
 * @file file_dialog.hpp
 * @brief 
 * @date 2023-04-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef EV2_UI_FILE_DIALOG_HPP
#define EV2_UI_FILE_DIALOG_HPP

#include "evpch.hpp"

namespace ev2::ui {

class FileDialogWindow {
public:

    enum class FileDialogType {
        OpenFile,
        SelectFolder
    };

public:

    bool show_file_dialog_modal(std::string_view name, std::string* select_path, bool relative_path = true);

private:
    std::filesystem::path current_path;
    FileDialogType type = FileDialogType::OpenFile;
};

}

#endif // 