/**
 * @file ui.hpp
 * @brief 
 * @date 2023-01-12
 * 
 */
#ifndef EV2_UI_H
#define EV2_UI_H

#include <scene.h>

namespace ev2 {

class SceneEditor {
public:
    void editor(Scene* scene);
    void show_scene_explorer(Scene* scene);
    void show_node_editor_widget(Node* node);
private:
    void show_scene_tree_widget(int id, Node* node);
    void show_transform_editor(Transform* tr);

    Node* selected_node;
};

}

#endif // EV2_UI_H