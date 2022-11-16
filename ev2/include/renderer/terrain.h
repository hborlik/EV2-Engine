/**
 * @file terrain.h
 * @brief 
 * @date 2022-11-08
 * 
 */
#ifndef EV2_RENDERED_TERRAIN_H
#define EV2_RENDERED_TERRAIN_H

namespace ev2::renderer {

class Terrain {
public:
    // Terrain();

    void init();

    bool load_textures();

    bool load_buffers();

    bool load_vaos();

    bool load_programs();

    bool load_queries();
};

}

#endif // EV2_RENDERED_TERRAIN_H