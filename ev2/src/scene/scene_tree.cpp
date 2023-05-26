
#include "scene/scene_tree.hpp"
#include "scene/node.hpp"

namespace ev2 {

SceneTree::~SceneTree() {
    current_scene->destroy();

    while(!m_destroy_queue.empty()) {
        m_destroy_queue.front()->internal_destroy();
        m_destroy_queue.pop();
    }
}

void SceneTree::node_added(Node* p_node) {
    assert(p_node);
    p_node->scene_tree = this;
    node_count++;
    m_object_map.insert({p_node->uuid_hash, p_node});
}

void SceneTree::node_removed(Node *p_node) {
    assert(p_node);
    node_count--;
    m_object_map.erase(p_node->uuid_hash);
}

void SceneTree::node_renamed(Node *p_node) {
    assert(p_node);
}

void SceneTree::queue_destroy(Ref<Node> node) {
    m_destroy_queue.push(node);
}

Ref<Node> SceneTree::get_node(std::size_t uuid_hash) {
    Ref<Node> ref{};
    auto itr = m_object_map.find(uuid_hash);
    if (itr != m_object_map.end()) {
        ref = itr->second->get_ref<Node>();
    }
    return ref;
}

void SceneTree::update(float dt) {
    if (!current_scene->m_is_ready) {
        current_scene->node_propagate_ready();
    }
    current_scene->node_propagate_update(dt);

    while(!m_destroy_queue.empty()) {
        m_destroy_queue.front()->internal_destroy();
        m_destroy_queue.pop();
    }
}

void SceneTree::update_pre_render() {
    current_scene->node_propagate_pre_render();
}

void SceneTree::change_scene(Ref<Node> p_scene) {
    if (current_scene)
        current_scene->node_propagate_exit_tree();

    current_scene = p_scene;

    if (current_scene)
        current_scene->node_propagate_enter_tree(this);
}

} // namespace ev2