/**
 * @file debug.h
 * @brief 
 * @date 2022-05-21
 * 
 */
#ifndef PLANT_DEBUG_H
#define PLANT_DEBUG_H

#include "game.h"

void show_settings_editor_window(GameState* game);
void show_tree_window(GameState* game, ev2::Ref<TreeNode> selected_tree);
#endif // PLANT_DEBUG_H