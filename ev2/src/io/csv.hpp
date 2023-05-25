/**
 * @file csv.hpp
 * @author Hunter Borlik ()
 * @brief 
 * @date 2023-05-24
 * 
 * 
 */
#ifndef CSV_HPP
#define CSV_HPP

#include <filesystem>
#include <fstream>

class CSVWriter {
public:
    CSVWriter(const std::filesystem::path& filename) : m_output_stream{filename} {
        if (!m_output_stream.is_open())
            throw std::invalid_argument{filename.string() + " could not be opened for writing"};
    }

    void write_header(const std::vector<std::string>& column_names) {
        for (std::size_t i = 0; i < column_names.size(); ++i) {
            m_output_stream << column_names[i];
            if (i < column_names.size()-1)
                m_output_stream << ", ";
        }
    }

    template<typename T>
    void write_value(const T& v) noexcept {
        using std::to_string;
        if (m_first_line_value_written){
            m_output_stream << ", ";
        }
        m_output_stream << v;
        m_first_line_value_written = true;
    }

    void write_line(const std::initializer_list<std::string>& values) {
        m_first_line_value_written = false;
        for (auto& v : values) {
            write_value(v);
        }
        m_output_stream << "\n";
    }

    template<typename... Args>
    void write_line(Args&&... args) {
        m_first_line_value_written = false;
        (void)std::initializer_list<int>{((void)write_value(std::forward<Args>(args)), 0)...};
        m_output_stream << "\n";
    }

private:
    std::ofstream m_output_stream{};
    bool m_first_line_value_written = false;
};

#endif // CSV_HPP