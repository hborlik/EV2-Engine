/**
 * @file scene.h
 * @brief 
 * @date 2022-04-21
 * 
 */
#ifndef EV2_SCENE_H
#define EV2_SCENE_H

#include <string>
#include <vector>
#include <list>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <reference_counted.hpp>
#include <scene/node.hpp>
#include <scene/visual_nodes.hpp>

namespace ev2 {

class Node;

class SceneTree {
public:
    SceneTree() = default;

    void update(float dt);
    void update_pre_render();

    void change_scene(Ref<Node> p_scene);

private:
    friend class Node;

    void tree_changed();
    void node_added(Node *p_node);
    void node_removed(Node *p_node);
    void node_renamed(Node *p_node);

    Ref<Node> current_scene = nullptr;
};

}

#endif // EV2_SCENE_H