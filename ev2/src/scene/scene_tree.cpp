
#include <scene/scene_tree.hpp>
#include <scene/node.hpp>

#include <algorithm>

namespace ev2 {

void SceneTree::node_added(Node* p_node) {
    assert(p_node);
    p_node->scene_tree = this;
    node_count++;
}

void SceneTree::node_removed(Node *p_node) {
    assert(p_node);
    node_count--;
}

void SceneTree::node_renamed(Node *p_node) {
    assert(p_node);
}

void SceneTree::update(float dt) {
    if (!current_scene->m_is_ready) {
        current_scene->_propagate_ready();
    }
    current_scene->_propagate_update(dt);
}

void SceneTree::update_pre_render() {
    current_scene->_update_pre_render();
}

void SceneTree::change_scene(Ref<Node> p_scene) {
    current_scene = p_scene;
    if (current_scene) {
        current_scene->scene_tree = this;
        current_scene->_propagate_enter_tree();
    }
}

} // namespace ev2