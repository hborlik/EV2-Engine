#include <scene/node.hpp>
#include <scene/scene_tree.hpp>

#include <algorithm>

namespace ev2 {

void Node::add_child(Ref<Node> node) {
    if (node->parent)
        node->parent->remove_child(node);
    int ind = children.size();
    children.push_back(node);
    node->_add_as_child(this);

    on_child_added(node, ind);
}

void Node::remove_child(Ref<Node> node) {
    auto itr = std::find(children.begin(), children.end(), node);
    if (itr != children.end()) {
        (*itr)->_remove_from_parent(this);
        children.erase(itr);
    } else {
        throw engine_exception{"Node: " + name + " does not have child " + node->name};
    }

    if (scene_tree)
        scene_tree->node_removed(node.get());

    on_child_removed(node);
}

void Node::destroy() {
    // cleanup children
    for (auto& c : children) {
        c->_remove_from_parent(this); // prevent child from calling remove_child and invalidating children list
        c->destroy();
    }

    if (parent)
        parent->remove_child(this->get_ref().ref_cast<Node>());

    on_destroy();
}

void Node::_propagate_update(float dt) {
    on_process(dt);

    for (auto& c : children) {
        c->_propagate_update(dt);
    }
}

void Node::_propagate_ready() {
    is_ready = true;
    on_ready();

    for (auto& c : children) {
        c->_propagate_ready();
    }
}

void Node::_propagate_enter_tree() {
    if (parent) {
        scene_tree = parent->scene_tree;
    }

    scene_tree->node_added(this);

    for (auto& c : children) {
        c->_propagate_enter_tree();
    }
}

void Node::_propagate_exit_tree() {
    for (auto& c : children) {
        c->_propagate_exit_tree();
    }

    if (scene_tree)
        scene_tree->node_removed(this);

    scene_tree = nullptr;
}

void Node::_propagate_transform_changed(Node* p_origin) {
    if (!is_inside_tree())
        return;

    for (auto& c : children) {
        c->_propagate_transform_changed(p_origin);
    }

    b_world_transform_dirty = true;

    on_transform_changed(p_origin->get_ref().ref_cast<Node>());
}

void Node::_update_pre_render() {
    pre_render();

    for (auto& c : children) {
        c->_update_pre_render();
    }
}

void Node::_add_as_child(Node* p_node) {
    assert(parent == nullptr);
    assert(p_node != this);
    parent = p_node;

    if (parent->scene_tree)
        _propagate_enter_tree();
    
}

void Node::_remove_from_parent(Node* p_node) {
    assert(p_node != this);
    assert(parent == p_node);

    if (scene_tree)
        _propagate_exit_tree();
    
    parent = nullptr;
}

} // namespace ev2