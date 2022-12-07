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

#include <util.h>

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

    int get_width()  const noexcept {return m_width;}

    int get_height()  const noexcept {return m_height;}

    /**
     * @brief The number of color channels
     * 
     * @return int 
     */
    int get_comp() const noexcept {return m_comp;}

    /**
     * @brief bytes per color channel
     * 
     * @return int 
     */
    int get_pf() const noexcept {return m_pf;}

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
     * @param pf        bytes per pixel
     * @param data 
     */
    void set_image(int width, int height, int n_comps, int pf, uint8_t* data);

private:
    uint8_t* m_texels = nullptr;
    int m_height = -1, m_width = -1;
    int m_comp = -1, m_pf = -1;

    Image* next = nullptr;
};

std::unique_ptr<Image> load_image(const std::string& path);

std::unique_ptr<Image> load_image_16(const std::string& path);

}

#endif // EV2_IMAGE_HPP