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
    void show_scene_explorer(Scene* scene);

private:
    void show_scene_node_widget(int id, Node* node);
    void show_transform_editor(Transform* tr);
};

}

#endif // EV2_UI_H