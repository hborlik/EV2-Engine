/**
 * @file image.hpp
 * @author Hunter Borlik
 * @brief 
 * @date 2022-11-29
 * 
 * 
 */
#ifndef EV2_IMAGE_HPP
#define EV2_IMAGE_HPP

#include <utility>
#include <string>
#include <memory>
#include <cmath>
#include <stdint.h>

#include <util.hpp>

namespace ev2 {

inline int mip_count(int width, int height, int depth = 1) noexcept {
    return 1 + std::floor(std::log2(util::vmax(width, height, depth)));
}

class Image {
public:
    Image() = default;

    ~Image();

    Image(const Image& o) noexcept {*this = o;}
    Image(Image&& o) noexcept {*this = std::move(o);}

    Image& operator=(const Image& o) noexcept;
    Image& operator=(Image&& o) noexcept;

    int width() const noexcept {return m_width;}

    int height() const noexcept {return m_height;}

    /**
     * @brief The number of color channels
     * 
     * @return int 
     */
    int comp() const noexcept {return m_comp;}

    /**
     * @brief bytes per color channel
     * 
     * @return int 
     */
    int pf() const noexcept {return m_pf;}

    /**
     * @brief Image Pixel stride. Bytes per pixel
     * 
     * @return int 
     */
    int bytes_per_pixel() const noexcept {return m_pf * m_comp;}

    /**
     * @brief 
     * 
     * @return std::size_t 
     */
    std::size_t memory_requirement() const noexcept {return m_height*m_width*m_comp*m_pf;}

    uint8_t* data() noexcept {return m_texels;}

    const uint8_t* data() const noexcept {return m_texels;}

    /**
     * @brief Set the image object, taking ownership of the allocated memory
     * 
     * @param width 
     * @param height 
     * @param n_comps   number of channels in the image
     * @param pf        bytes per pixel color component
     * @param data 
     */
    void set_image(int width, int height, int n_comps, int pf, uint8_t* data);

private:
    uint8_t* m_texels = nullptr;
    // height and width
    int m_height = -1, m_width = -1;
    // number of components per pixel and bytes per pixel color component
    int m_comp = -1, m_pf = -1;

    Image* next = nullptr;
};

}

#endif // EV2_IMAGE_HPP