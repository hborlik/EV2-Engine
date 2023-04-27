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

}