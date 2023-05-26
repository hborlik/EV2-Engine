/**
 * @file scene.h
 * @brief 
 * @date 2022-04-21
 * 
 */
#ifndef EV2_SCENE_H
#define EV2_SCENE_H

#include "evpch.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "reference_counted.hpp"
#include "scene/node.hpp"
#include "scene/visual_nodes.hpp"

namespace ev2 {

class Node;

class SceneTree {
public:
    SceneTree() = default;
    ~SceneTree();

    void update(float dt);
    void update_pre_render();

    void change_scene(Ref<Node> p_scene);

    Ref<Node> get_node(std::size_t uuid_hash);

private:
    friend class Node;

    void tree_changed();
    void node_added(Node *p_node);
    void node_removed(Node *p_node);
    void node_renamed(Node *p_node);

    void queue_destroy(Ref<Node> node);

private:
    Ref<Node> current_scene = nullptr;
    int node_count = 0;

    std::queue<Ref<Node>> m_destroy_queue{};
    std::unordered_map<std::size_t, Node*> m_object_map;
};

}

#endif // EV2_SCENE_H