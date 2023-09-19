/**
 * @file texture.hpp
 * @brief 
 * @date 2023-08-12
 * 
 */
#ifndef EV2_TEXTURE_HPP
#define EV2_TEXTURE_HPP

#include "evpch.hpp"

namespace ev2::renderer {

enum class TextureType {
    Tex_1D,
    Tex_2D
};

class Texture {
public:
    virtual ~Texture() = default;

    virtual TextureType get_type() const = 0;

    static std::unique_ptr<Texture> make_texture(TextureType type);
};

class Texture1D : public Texture {
public:
    TextureType get_type() const override {return TextureType::Tex_1D;}

    static std::unique_ptr<Texture1D> make_texture();
};

class Texture2D : public Texture {
public:
    TextureType get_type() const override {return TextureType::Tex_2D;}

    static std::unique_ptr<Texture2D> make_texture();
};

}

#endif // EV2_TEXTURE_HPP