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

#include <stdint.h>

namespace ev2 {

class Image {
public:
    Image() = default;

    Image(const Image& o) noexcept {*this = o;}
    Image(Image&& o) noexcept {*this = std::move(o);}

    Image& operator=(const Image& o) noexcept;
    Image& operator=(Image&& o) noexcept;

    int get_width()  const noexcept {return width;}

    int get_height()  const noexcept {return height;}

private:
    uint8_t* texels = nullptr;
    int height = -1, width = -1;

    Image* next = nullptr;
};

}

#endif // EV2_IMAGE_HPP