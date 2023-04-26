/**
 * @file ui.hpp
 * @brief 
 * @date 2023-01-12
 * 
 */
#ifndef EV2_UI_H
#define EV2_UI_H

#include <unordered_map>
#include <typeinfo>
#include <typeindex>

#include <scene/scene_tree.hpp>
#include <scene/node.hpp>

namespace ev2 {

void show_settings_window(bool* p_open);

class NodeEditor {
public:
    virtual void show_editor(Node* node) = 0;
    virtual std::type_index get_edited_type() const = 0;

protected:
    friend class SceneEditor;
    class SceneEditor* m_editor = nullptr;
};

template<typename T>
class NodeEditorT : public NodeEditor {
public:
    std::type_index get_edited_type() const override {
        return std::type_index(typeid(T));
    }
};

class EditorTool {
public:
    virtual void show_editor_tool() = 0;
    virtual std::string get_name() const = 0;

    virtual void on_selected_node(Node* node) {}

protected:
    friend class SceneEditor;
    class SceneEditor* m_editor = nullptr;
    bool m_is_open = false;
};

class SceneEditor {
public:
    void editor(Node* scene);
    void show_scene_explorer(Node* scene, bool* p_open);
    void show_node_editor_widget(Node* node);

    void set_selected_node(Node* node) noexcept {select_node(Ref<Node>{node});}
    Node* get_selected_node() const noexcept {return Ref<Node>{m_selected_node}.get();}

    void add_custom_node_editor(std::shared_ptr<NodeEditor> editor);

    void add_custom_editor_tool(std::shared_ptr<EditorTool> editor_tool);
    std::shared_ptr<EditorTool> get_editor_tool(std::string_view name);

private:
    void show_scene_tree_widget(int id, Node* node);
    void show_transform_editor(Node* node);

    void select_node(const Ref<Node>& n);

private:
    Ref<Node> m_selected_node;

    bool m_scene_editor_open = false;
    bool m_show_settings = false;
    bool m_material_editor_open = false;
    bool m_demo_open = false;

    std::unordered_map<std::type_index, std::shared_ptr<NodeEditor>> m_editor_types;
    std::unordered_map<std::string, std::shared_ptr<EditorTool>> m_editor_tools;
};

}

#endif // EV2_UI_H