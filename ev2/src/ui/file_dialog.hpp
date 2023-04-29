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

#include <string>
#include <filesystem>

namespace ev2::ui {

class FileDialogWindow {
public:

    enum class FileDialogType {
        OpenFile,
        SelectFolder
    };

public:

    bool show_file_dialog(bool* p_open, std::string* select_path);

private:
    std::filesystem::path current_path;
    FileDialogType type = FileDialogType::OpenFile;
};

}

#endif // 