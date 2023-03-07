/**
 * @file ui.hpp
 * @brief 
 * @date 2023-01-12
 * 
 */
#ifndef EV2_UI_H
#define EV2_UI_H

#include <scene/scene.h>

namespace ev2 {

void show_settings_window(bool* p_open);

class SceneEditor {
public:
    void editor(Scene* scene);
    void show_scene_explorer(Scene* scene, bool* p_open);
    void show_node_editor_widget(Node* node);
private:
    void show_scene_tree_widget(int id, Node* node);
    void show_transform_editor(Transform* tr);

    Node* selected_node;

    bool m_scene_editor_open = false;
    bool m_show_settings = false;
    bool m_material_editor_open = false;
    bool m_demo_open = false;
};

}

#endif // EV2_UI_H