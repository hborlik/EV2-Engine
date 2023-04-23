/**
 * @file node.h
 * @brief 
 * @date 2022-05-04
 * 
 */
#ifndef EV2_NODE_H
#define EV2_NODE_H

#include <string>
#include <vector>
#include <list>
#include <memory>

#include <transform.hpp>
#include <ev.hpp>
#include <reference_counted.hpp>

namespace ev2 {

class SceneTree;

class Node : public ObjectT<Node> {
public:
    Node() = default;
    explicit Node(const std::string& name) : name{name} {}
    virtual ~Node() = default;

    template<typename T, typename... Args>
    Ref<T> create_child_node(Args&&... args) {
        Ref<T> node{new T(std::forward<Args&&>(args)...)};
        node->on_init();
        add_child(node);
        return node;
    }

    template<typename T, typename... Args>
    static Ref<T> create_node(Args&&... args) {
        Ref<T> node{new T(std::forward<Args&&>(args)...)};
        node->on_init();
        return node;
    }

    /**
     * @brief node initialization
     * 
     */
    virtual void on_init() {}

    /**
     * @brief node has been added to scene
     * 
     */
    virtual void on_ready() {}

    /**
     * @brief called when the node has been flagged for destruction, and has been removed from the scene
     * 
     */
    virtual void on_destroy() {};

    /**
     * @brief per frame update function
     * 
     * @param delta 
     */
    virtual void on_process(float delta) {}

    virtual void on_child_added(Ref<Node> child, int index) {}

    virtual void on_child_removed(Ref<Node> child) {}

    virtual void on_transform_changed(Ref<Node> origin) {}

    /**
     * @brief call just before scene is rendered. Used to push changes to rendering server
     * 
     */
    virtual void pre_render() {};

    void add_child(Ref<Node> node);

    /**
     * @brief simply removes a child from this nodes children list. This does not destroy the child node.
     * 
     * @param node 
     */
    void remove_child(Ref<Node> node);

    /**
     * @brief Get the child at index
     * 
     * @param index 
     * @return Ref<Node> 
     */
    Ref<Node> get_child(int index) {
        assert(index >= 0);
        if (index > children.size())
            return {};
        int i = 0;
        auto itr = children.begin();
        for (; i < index; itr++, i++)
            ;
        return *itr;
    }

    size_t get_n_children() const noexcept {return children.size();}

    std::vector<Ref<Node>> get_children() {return {children.begin(), children.end()};}
    const auto& get_children() const {return children;}

    Ref<Node> get_parent() const {
        if (parent)
            return parent->get_ref<Node>();
        return{};
    }

    /**
     * @brief Trigger destruction events and remove node from scene
     * 
     */
    void destroy();

    std::string get_path() const {
        std::string path = "/" + name;
        if (parent)
            path = parent->get_path() + path;
        return path;
    }

    virtual glm::mat4 get_world_transform() const {
        glm::mat4 tr;
        if (b_world_transform_dirty) {
            tr = transform.get_transform();
            if (parent)
                tr = parent->get_world_transform() * tr;
            world_transform_cache = tr;
            b_world_transform_dirty = false;
        } else
            tr = world_transform_cache;
        return tr;
    }

    glm::vec3 get_world_position() const {return glm::vec3(get_world_transform()[3]);}

    void rotate(const glm::vec3& xyz) {transform.rotate(xyz);}

    glm::mat4 get_transform() const noexcept {return transform.get_transform();}
    glm::mat4 get_linear_transform() const noexcept {return transform.get_linear_transform();}
    inline glm::vec3 get_position() const noexcept {return transform.get_position();}
    inline glm::quat get_rotation() const noexcept {return transform.get_rotation();}
    inline glm::vec3 get_scale() const noexcept {return transform.get_scale();}

    inline void set_position(glm::vec3 pos) noexcept {transform.set_position(pos);node_propagate_transform_changed(this);}
    inline void set_rotation(glm::quat rot) noexcept {transform.set_rotation(rot);node_propagate_transform_changed(this);}
    inline void set_scale(glm::vec3 s) noexcept {transform.set_scale(s);node_propagate_transform_changed(this);}

    bool is_inside_tree() const noexcept {return scene_tree;}
    bool is_destroyed() const noexcept {return m_is_destroyed;}

public:
    std::string name = "Node";

private:
    friend class SceneTree;

    void node_propagate_update(float dt);
    void node_propagate_ready();
    void node_propagate_enter_tree();
    void node_propagate_exit_tree();
    void node_propagate_transform_changed(Node* p_origin);
    void node_propagate_pre_render();

    void add_as_child(Node* p_node);
    void remove_from_parent(Node* p_node);

    bool m_is_ready = false;
    bool m_is_destroyed = false;
    Transform transform{};
    std::list<Ref<Node>> children;
    Node* parent = nullptr;
    SceneTree* scene_tree = nullptr;

    mutable bool b_world_transform_dirty = true;
    mutable glm::mat4 world_transform_cache{};
};

} // namespace ev2

#endif // EV2_NODE_H