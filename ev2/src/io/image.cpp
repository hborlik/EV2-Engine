#include "io/image.hpp"

#include <stb_image.h>
#include <stb_image_write.h>

namespace ev2 {

Image::Image(int width, int height, int comp, int pf, uint8_t* data) : 
    m_texels{},
    m_height(height),
    m_width(width),
    m_comp(comp),
    m_pf(pf) {

    assert(width > 0);
    assert(height > 0);
    assert(comp > 0);
    assert(pf > 0);

    if (!data)
        m_texels = std::vector<uint8_t>(memory_requirement());
    else
        m_texels = std::vector<uint8_t>(data, data+memory_requirement());
}

Image::Image(int width, int height, int comp, int pf, std::vector<uint8_t> data) :
    m_texels{std::move(data)},
    m_height(height),
    m_width(width),
    m_comp(comp),
    m_pf(pf) {}

void Image::set_image(int width, int height, int n_comps, int pf, uint8_t* data) {
    m_width     = width;
    m_height    = height;
    m_comp      = n_comps;
    m_pf        = pf;
    m_texels    = std::vector<uint8_t>(data, data+memory_requirement());
}

void Image::write(const std::filesystem::path& path) {
    if (!path.has_filename())
        throw std::invalid_argument{"Failed to save " + path.string() + " : " +
                                    path.string() +
                                    " is not a valid filename."};
    std::string ext = path.extension();

    stbi_flip_vertically_on_write(1);
    if (ext == ".png") {
        if (!stbi_write_png(path.c_str(), m_width, m_height, m_comp, m_texels.data(), m_width*bytes_per_pixel()))
            throw std::runtime_error{"Failed to save " + path.string()};
    } else {
        throw std::invalid_argument {
            "Failed to save " + path.string() + " : " + ext +
                " is not a supported extension for images."
        };
    }
}

}