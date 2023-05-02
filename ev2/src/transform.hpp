/**
 * @file transform.h
 * @brief 
 * @date 2022-05-13
 * 
 * 
 */
#ifndef EV2_TRANSFORM_H
#define EV2_TRANSFORM_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>

namespace ev2 {

struct Transform {
    glm::mat4 get_transform() const noexcept { return m_transform; }

    glm::mat4 get_linear_transform() const noexcept {
        update_prs();
        glm::mat4 tr = glm::mat4_cast(m_prs.rotation);
        tr[3] = glm::vec4{m_prs.position, 1.0f};
        return tr;
    }

    inline glm::vec3 get_position() const noexcept {
        update_prs();
        return m_prs.position;
    }

    inline glm::quat get_rotation() const noexcept {
        update_prs();
        return m_prs.rotation;
    }

    inline glm::vec3 get_scale() const noexcept {
        update_prs();
        return m_prs.scale;
    }

    inline void set_position(glm::vec3 pos) noexcept {
        update_prs();
        m_prs.position = pos;
        update_transform_from_prs();
    }

    inline void set_rotation(glm::quat rot) noexcept {
        update_prs();
        m_prs.rotation = rot;
        update_transform_from_prs();
    }

    void set_matrix(const glm::mat4& mat) noexcept {
        m_transform = mat;
        m_prs_cache_valid = false;
    }

    /**
     * @brief apply euler rotations
     *
     * @param xyz in radians
     */
    void rotate(const glm::vec3& xyz) {
        update_prs();
        m_prs.rotation = glm::rotate(
            glm::rotate(glm::rotate(m_prs.rotation, xyz.x, {1, 0, 0}), xyz.y,
                        {0, 1, 0}),
            xyz.z, {0, 0, 1});
        update_transform_from_prs();
    }

    inline void set_scale(glm::vec3 s) noexcept {
        update_prs();
        m_prs.scale = s;
        update_transform_from_prs();
    }

   private:
    struct PRS {
        glm::vec3 position{};
        glm::quat rotation = glm::identity<glm::quat>();
        glm::vec3 scale{1, 1, 1};
    };

    inline void update_prs() const noexcept {
        if (m_prs_cache_valid) return;
        glm::vec3 skew;
        glm::vec4 perspective;
        glm::decompose(m_transform, m_prs.scale, m_prs.rotation, m_prs.position,
                       skew, perspective);
        m_prs_cache_valid = true;
    }

    void update_transform_from_prs() noexcept {
        m_transform = glm::mat4_cast(m_prs.rotation) *
                      glm::scale(glm::identity<glm::mat4>(), m_prs.scale);
        m_transform[3] = glm::vec4{m_prs.position, 1.0f};
    }

   private:
    mutable bool m_prs_cache_valid = false;
    mutable PRS m_prs{};

    glm::mat4 m_transform{1};
};
}

#endif // 
