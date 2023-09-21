/**
 * @file render_api.hpp
 * @brief 
 * @date 2023-08-05
 * 
 */
#ifndef EV2_RENDER_API_HPP
#define EV2_RENDER_API_HPP

#include "core/singleton.hpp"

namespace ev2::renderer {

/**
 * @brief Generic functionality available from multiple rendering APIs
 * 
 */
class RenderAPI {
public:
    virtual ~RenderAPI() = default;

    enum class API {
        None = 0, OpenGL = 1
    };

    virtual void init() = 0;

    static API get_api() noexcept {return s_api;}
    static std::unique_ptr<RenderAPI> make_api();

private:
    static API s_api;
};
    
}

#endif // 