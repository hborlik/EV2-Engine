#include <scene/node.hpp>
#include <scene/scene_tree.hpp>

#include <algorithm>

namespace ev2 {

void Node::add_child(Ref<Node> node) {
    if (node.get() == this)
        return;
    if (node->parent)
        node->parent->remove_child(node);
    int ind = children.size();
    children.push_back(node);
    node->parent = this;

    if (scene_tree)
        scene_tree->node_added(node.get());

    on_child_added(node, ind);
}

void Node::remove_child(Ref<Node> node) {
    auto itr = std::find(children.begin(), children.end(), node);
    if (itr != children.end()) {
        (*itr)->parent = nullptr;
        children.erase(itr);
    } else {
        throw engine_exception{"Node: " + name + " does not have child " + node->name};
    }

    if (scene_tree)
        scene_tree->node_removed(node.get());

    on_child_removed(node);
}

void Node::destroy() {
    // cleanup children first
    for (auto& c : children) {
        c->destroy();
    }

    if (scene_tree)
        scene_tree->node_removed(this);

    on_destroy();
}

void Node::_update(float dt) {
    on_process(dt);

    for (auto& c : children) {
        c->_update(dt);
    }
}

void Node::_ready() {
    is_ready = true;
    on_ready();

    for (auto& c : children) {
        c->_ready();
    }
}

void Node::_update_pre_render() {
    pre_render();

    for (auto& c : children) {
        c->_update_pre_render();
    }
}

} // namespace ev2