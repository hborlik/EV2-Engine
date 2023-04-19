#include <stdexcept>

#include <image.hpp>
#include <renderer/ev_gl.hpp>
#include <stb_image.h>

namespace ev2 {

Image::~Image() {
    if (m_texels)
        delete m_texels;
}

void Image::set_image(int width, int height, int n_comps, int pf, uint8_t* data) {
    if (m_texels)
        delete m_texels;
    m_width     = width;
    m_height    = height;
    m_comp      = n_comps;
    m_pf        = pf;
    m_texels    = data;
}

std::unique_ptr<Image> load_image(const std::string& path) {
    int w, h, ncomps;
    stbi_set_flip_vertically_on_load(false);
    uint8_t* data = stbi_load(path.c_str(), &w, &h, &ncomps, 0);

    if (data) {
        if (ncomps > 4) {
            std::cerr << "Failed to load texture " + path << "invalid ncomps " << ncomps << std::endl;
            stbi_image_free(data);
            throw std::runtime_error{"Failed to load texture " + path};
        }
        std::unique_ptr<Image> image = std::make_unique<Image>();

        // allow image to keep the memory (will free)
        image->set_image(w, h, ncomps, 1, data);

        std::cout << "Loaded texture " << path << std::endl;
        // stbi_image_free(data);
        return image;
    } else {
        std::cerr << "Failed to load texture " << path << std::endl;
        throw std::runtime_error{"Failed to load texture " + path};
    }
}

std::unique_ptr<Image> load_image_16(const std::string& path) {
    int w, h, ncomps;
    stbi_set_flip_vertically_on_load(false);
    uint16_t* data = stbi_load_16(path.c_str(), &w, &h, &ncomps, 0);

    if (data) {
        if (ncomps > 4) {
            std::cerr << "Failed to load texture " << path << "invalid ncomps " << ncomps << std::endl;
            stbi_image_free(data);
            throw std::runtime_error{"Failed to load texture " + path};
        }
        std::unique_ptr<Image> image = std::make_unique<Image>();

        // allow image to keep the memory (will free)
        image->set_image(w, h, ncomps, 2, (uint8_t*)data);

        std::cout << "Loaded texture " << path << std::endl;
        // stbi_image_free(data);
        return image;
    } else {
        std::cerr << "Failed to load texture " << path << std::endl;
        throw std::runtime_error{"Failed to load texture " + path};
    }
}

}