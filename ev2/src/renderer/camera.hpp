/**
 * @file camera.h
 * @brief 
 * @date 2022-04-18
 * 
 */
#ifndef EV2_CAMERA_H
#define EV2_CAMERA_H

#include <array>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <geometry.hpp>


namespace ev2 {

enum class CameraProjection {
    ORTHOGRAPHIC, // no perspective
    RECTILINEAR,  // preserves straight lines (OpenGL / DirectX)
    FISHEYE       // conformal (stereographic projection)
};

class Camera {
public:
    /**
     * @brief Get the world space frustums of this camera
     * 
     * @return Frustum 
     */
    Frustum extract_frustum() const noexcept {
        using namespace glm;
        if (dirty)
            force_update_internal();
        return ev2::extract_frustum(p_v);
    }
    
    /**
     * @brief get the world space frustum corners. 
     * 
     * @param frustum_extent_far percentage of the frustum far extent
     * @return std::array<glm::vec3, 8> 
     */
    std::array<glm::vec3, 8> extract_frustum_corners(float frustum_extent_far = 1.0f) const noexcept {
        using namespace glm;
        if (dirty)
            force_update_internal();
        const float f_n = m_far - m_near;
        const float f_view = f_n * frustum_extent_far + m_near;
        const float ndc_far = (m_far + m_near) / f_n + 2 * m_far * m_near / (f_n * -f_view);
        std::array<glm::vec3, 8> ndcPoints = {
            glm::vec3(-1, -1,  ndc_far),
            glm::vec3( 1, -1,  ndc_far),
            glm::vec3(-1,  1,  ndc_far),
            glm::vec3( 1,  1,  ndc_far),
            glm::vec3(-1, -1, -1),
            glm::vec3( 1, -1, -1),
            glm::vec3(-1,  1, -1),
            glm::vec3( 1,  1, -1)
        };    
        std::array<glm::vec3, 8> worldPoints;
        for (int i = 0; i < 8; i++)
        {
            glm::vec4 pos = inv_pv() * vec4(ndcPoints[i], 1.0);
            pos = pos / pos.w;
            worldPoints[i] = glm::vec3(pos);
        }
        return worldPoints;
    }

    std::array<glm::vec3, 8> extract_frustum_corners_world(float frustum_extent_far_world = 10.0f) const noexcept {
        using namespace glm;
        if (dirty)
            force_update_internal();
        const float f_n = m_far - m_near;
        const float f_view = frustum_extent_far_world + m_near;
        const float ndc_far = (m_far + m_near) / f_n + 2 * m_far * m_near / (f_n * -f_view);
        std::array<glm::vec3, 8> ndcPoints = {
            glm::vec3(-1, -1,  ndc_far),
            glm::vec3( 1, -1,  ndc_far),
            glm::vec3(-1,  1,  ndc_far),
            glm::vec3( 1,  1,  ndc_far),
            glm::vec3(-1, -1, -1),
            glm::vec3( 1, -1, -1),
            glm::vec3(-1,  1, -1),
            glm::vec3( 1,  1, -1)
        };    
        std::array<glm::vec3, 8> worldPoints;
        for (int i = 0; i < 8; i++)
        {
            glm::vec4 pos = inv_pv() * vec4(ndcPoints[i], 1.0);
            pos = pos / pos.w;
            worldPoints[i] = glm::vec3(pos);
        }
        return worldPoints;
    }

    /**
     * @brief Force internal View and Projection * View matrices to be updated.
     * 
     */
    void force_update_internal() const noexcept {
        using namespace glm;
        viewInv = translate(identity<glm::mat4>(), position) * mat4_cast(rotation);
        view = inverse(viewInv);
        p_v = projection * view;
        dirty = false;
    }

    /**
     * @brief Get the View Matrix for Camera
     * 
     * @return glm::mat4 
     */
    glm::mat4 get_view() const noexcept {
        if (dirty)
            force_update_internal();
        return view;
    }

    /**
     * @brief Get the Inverse View Matrix for Camera
     * 
     * @return glm::mat4 
     */
    glm::mat4 get_view_inv() const noexcept {
        if (dirty)
            force_update_internal();
        return viewInv;
    }

    /**
     * @brief get inverse Projection * View matrix
     * 
     * @return glm::mat4 
     */
    glm::mat4 inv_pv() const {
        if (dirty)
            force_update_internal();
        return glm::inverse(p_v);
    }

    glm::mat4 get_projection() const {return projection;}
    glm::vec3 get_position() const {return position;}
    glm::quat get_rotation() const {return rotation;}

    void set_perspective(float _fov, float _aspect, float _near, float _far) {
        fov = _fov;
        aspect = _aspect;
        m_near = _near;
        m_far = _far;
        projection = glm::perspective(glm::radians(fov), aspect, m_near, m_far);
        dirty = true;
        projection_mode = CameraProjection::RECTILINEAR;
    }

    void set_position(const glm::vec3& p) {
        position = p;
        dirty = true;
    }

    void set_rotation(const glm::quat& q) {
        rotation = q;
        dirty = true;
    }

    /**
     * @brief Move camera in a local direction
     * 
     * @param direction 
     */
    void move(const glm::vec3 &direction) {
        using namespace glm;
        vec3 dir = glm::mat3(rotation) * direction;
        position += dir;
    }

    glm::vec3 get_forward() const {
        return -glm::mat4_cast(rotation)[2];
    }

    glm::vec3 get_up() const {
        return glm::mat4_cast(rotation)[1];
    }

    glm::vec3 get_right() const {
        return glm::mat4_cast(rotation)[0];
    }

    Ray screen_pos_to_ray(glm::vec2 s_pos) const {
        glm::vec2 s_pos_ndc = 2.f * (s_pos) - glm::vec2{1, 1};
        s_pos_ndc.y *= -1;
        glm::mat4 pv_inv = inv_pv();
        glm::vec4 pos = pv_inv * glm::vec4{s_pos_ndc, -1, 1.0};

        pos.x /= pos.w;
        pos.y /= pos.w;
        pos.z /= pos.w;

        glm::vec3 c_pos = get_position();
        return Ray{c_pos, glm::vec3(pos) - c_pos};
    }

    CameraProjection get_projection_mode() const noexcept {return projection_mode;}

    float get_fov() const noexcept {return fov;}

private:
    glm::vec3 position{};
    glm::quat rotation = glm::identity<glm::quat>();

    glm::mat4 projection = glm::identity<glm::mat4>();
    CameraProjection projection_mode = CameraProjection::RECTILINEAR;

    mutable glm::mat4 view = glm::identity<glm::mat4>();
    mutable glm::mat4 viewInv = glm::identity<glm::mat4>();
    mutable glm::mat4 p_v = glm::identity<glm::mat4>();
    mutable bool dirty = true;

    float fov = 60.0f;
    float m_near = 1.f, m_far = 64000.0f;
    float aspect = 1.0f;
};

} // namespace ev

#endif // EV2_CAMERA_H