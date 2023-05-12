/**
 * @file sc_wfc.hpp
 * @brief 
 * @date 2023-04-17
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef EV2_SC_WHC_HPP
#define EV2_SC_WHC_HPP

#include "evpch.hpp"

#include "scene/node.hpp"
#include "scene/visual_nodes.hpp"
#include "geometry.hpp"

#include "wfc.hpp"

namespace ev2::pcg {

class SCWFCGraphNode;

class SCWFC : public Node {
public:
    explicit SCWFC(std::string name);

    void reset();

    void on_init() override;

    void on_child_removed(Ref<Node> child) override;

    void on_child_added(Ref<Node> child, int index) override;

    void update_all_adjacencies(Ref<SCWFCGraphNode> n, float radius);

    glm::vec3 sphere_repulsion(const Sphere& sph) const;

    /**
     * @brief Check if a node intersects any of its adjacent nodes in the scene.
     *  Note that this does not check for intersections among all children, only those nodes
     *  added by update_all_adjacencies()
     * 
     * @param n 
     * @return true 
     * @return false 
     */
    bool intersects_any_solved_neighbor(const Ref<SCWFCGraphNode>& n);

    wfc::SparseGraph<wfc::DGraphNode>* get_graph();

private:
    friend class SCWFCEditor;
    struct Data;
    std::shared_ptr<Data> m_data{};
};

class SCWFCGraphNode : public VisualInstance, public wfc::DGraphNode {
public:
    explicit SCWFCGraphNode(const std::string &name) : VisualInstance{name}, wfc::DGraphNode{name, (int)uuid_hash} {}

    void on_init() override {
        VisualInstance::on_init();

        m_bounding_sphere = Sphere{get_world_position(), 1.f};
        m_neighborhood_r = m_bounding_sphere.radius * 4.f;
    }

    void on_transform_changed(Ref<ev2::Node> origin) override {
        VisualInstance::on_transform_changed(origin);

        m_bounding_sphere.center = get_world_position();

        if (auto parent = get_parent().ref_cast<SCWFC>(); parent) {
            parent->update_all_adjacencies(this->get_ref<SCWFCGraphNode>(), m_neighborhood_r);
        }
    }

    const Sphere& get_bounding_sphere() const noexcept {return m_bounding_sphere;}

    void set_radius(float r) noexcept {m_bounding_sphere.radius = r;}
    void set_neighborhood_radius(float r) noexcept {m_neighborhood_r = r;}

    float get_radius() noexcept {return m_bounding_sphere.radius;}
    float get_neighborhood_radius() noexcept {return m_neighborhood_r;}

    bool is_finalized() const noexcept {return m_is_finalized;}
    void set_finalized() noexcept {m_is_finalized = true;}

private:
    Sphere m_bounding_sphere{};
    float m_neighborhood_r{};
    bool m_is_finalized = false;
};

class SCWFCAttractorNode : public VisualInstance {
public:
    explicit SCWFCAttractorNode(const std::string &name) : VisualInstance{name} {}

};


}

#endif // EV2_SC_WHC_HPP