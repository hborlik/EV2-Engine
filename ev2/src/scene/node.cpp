#include "scene/node.hpp"
#include "scene/scene_tree.hpp"

namespace ev2 {

void Node::add_child(Ref<Node> node) {
    if (!node)
        return;
    
    // if the node already has a parent, remove the child from that parent
    if (node->parent)
        node->parent->remove_child(node);
    
    int ind = node->add_as_child(this);

    on_child_added(node, ind);
}

void Node::remove_child(Ref<Node> node) {
    auto itr = std::find(children.begin(), children.end(), node);
    if (itr != children.end()) {
        (*itr)->remove_from_parent();
        children.erase(itr);
        on_child_removed(node);
    } else {
        throw engine_exception{"Node: " + name + " does not have child " + node->name};
    }
}

void Node::destroy() {
    if (m_is_destroyed_queued)
        return;

    m_is_destroyed_queued = true;

    if (scene_tree)
        scene_tree->queue_destroy(this->get_ref<Node>());
}

void Node::internal_destroy() {
    if (m_is_destroyed)
        return;

    increment(); // ensure the node is not deconstructed while we are removing it from the scene

    // fast remove and destroy all children
    for (auto& c : children) {
        c->remove_from_parent(); // prevent child from calling remove_child and invalidating children list
        c->destroy();
    }
    children = {};

    if (parent)
        parent->remove_child(this->get_ref().ref_cast<Node>()); // has the potential to call deconstructor without increment() above

    if (scene_tree)
        node_propagate_exit_tree();

    parent = nullptr;
    scene_tree = nullptr;
    
    on_destroy();

    m_is_destroyed = true;

    decrement(); // possibly deconstruct
}

void Node::node_propagate_update(float dt) {
    on_process(dt);

    for (auto& c : children) {
        c->node_propagate_update(dt);
    }
}

void Node::node_propagate_ready() {
    m_is_ready = true;
    on_ready();

    for (auto& c : children) {
        c->node_propagate_ready();
    }
}

void Node::node_propagate_enter_tree(SceneTree* scene_tree) {
    if (!scene_tree) // use exit tree to set scene_tree null
        return;

    this->scene_tree = scene_tree;
    this->scene_tree->node_added(this);

    for (auto& c : children) {
        c->node_propagate_enter_tree(scene_tree);
    }
}

void Node::node_propagate_exit_tree() {
    if (!scene_tree)
        return;

    for (auto& c : children) {
        c->node_propagate_exit_tree();
    }

    if (scene_tree)
        scene_tree->node_removed(this);

    scene_tree = nullptr;
}

void Node::node_propagate_transform_changed(Node* p_origin) {
    if (!is_inside_tree())
        return;

    b_world_transform_dirty = true;

    for (auto& c : children) {
        c->node_propagate_transform_changed(p_origin);
    }

    on_transform_changed(p_origin->get_ref().ref_cast<Node>());
}

void Node::node_propagate_pre_render() {
    pre_render();

    for (auto& c : children) {
        c->node_propagate_pre_render();
    }
}

// add this as a child to p_node
int Node::add_as_child(Node* p_node) {
    assert(parent == nullptr);
    assert(p_node != this);

    int ind = p_node->children.size();
    
    p_node->children.push_back(get_ref<Node>());
    parent = p_node;

    // first enter scene tree for bookkeeping
    node_propagate_enter_tree(p_node->scene_tree);

    // changing parent pointer invalidates transforms
    node_propagate_transform_changed(this);

    return ind;
}

void Node::remove_from_parent() {
    parent = nullptr;

    // changing parent pointer invalidates transforms
    node_propagate_transform_changed(this);

    node_propagate_exit_tree();
}

} // namespace ev2