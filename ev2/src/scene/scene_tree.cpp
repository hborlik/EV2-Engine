
#include <scene/scene_tree.hpp>
#include <scene/node.hpp>

#include <algorithm>

namespace ev2 {

void SceneTree::node_added(Node* p_node) {
    assert(p_node);
    p_node->scene_tree = this;
}

void SceneTree::node_removed(Node *p_node) {
    assert(p_node);
}

void SceneTree::node_renamed(Node *p_node) {
    assert(p_node);
}

void SceneTree::update(float dt) {
    if (!current_scene->is_ready) {
        current_scene->_ready();
    }
    current_scene->_update(dt);
}

void SceneTree::update_pre_render() {
    current_scene->_update_pre_render();
}

void SceneTree::change_scene(Ref<Node> p_scene) {
    current_scene = p_scene;
}

} // namespace ev2