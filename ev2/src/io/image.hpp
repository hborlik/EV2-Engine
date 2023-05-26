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

#include "evpch.hpp"

#include "util.hpp"

namespace ev2 {

inline int mip_count(int width, int height, int depth = 1) noexcept {
    return 1 + std::floor(std::log2(util::vmax(width, height, depth)));
}

class Image {
public:
    Image() = default;
    Image(int width, int height, int comp, int pf, uint8_t* data = nullptr);
    Image(int width, int height, int comp, int pf, std::vector<uint8_t> data);

    Image(const Image& o) noexcept {*this = o;}
    Image(Image&& o) noexcept {swap(*this, o);}

    Image& operator=(const Image& o) noexcept {
        Image img{o};
        swap(*this, img);
        return *this;
    }

    Image& operator=(Image&& __r) noexcept {
        swap(*this, __r);
        return *this;
    }

    inline int width() const noexcept {return m_width;}
    inline int height() const noexcept {return m_height;}

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

    uint8_t* data() noexcept {return m_texels.data();}

    const uint8_t* data() const noexcept {return m_texels.data();}

    void write(const std::filesystem::path& path);

    /**
     * @brief Set the image object, copies memory_requirement bytes from data
     * 
     * @param width 
     * @param height 
     * @param n_comps   number of channels in the image
     * @param pf        bytes per pixel color component
     * @param data 
     */
    void set_image(int width, int height, int n_comps, int pf, uint8_t* data);

    friend void swap(Image& rhs, Image& lhs) noexcept {
        std::swap(rhs.m_texels, lhs.m_texels);
        std::swap(rhs.m_height, lhs.m_height);
        std::swap(rhs.m_width, lhs.m_width);
        std::swap(rhs.m_comp, lhs.m_comp);
        std::swap(rhs.m_pf, lhs.m_pf);
        std::swap(rhs.next, lhs.next);
    }

private:
    std::vector<uint8_t> m_texels{};
    // height and width
    int m_height = -1, m_width = -1;
    // number of components per pixel and bytes per pixel color component
    // so stride is m_pf * m_comp
    int m_comp = -1, m_pf = -1;

    Image* next = nullptr;
};

}

#endif // EV2_IMAGE_HPP