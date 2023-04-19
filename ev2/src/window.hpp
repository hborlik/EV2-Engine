/**
 * @file window.h
 * @author Hunter Borlik
 * @brief Window manager. Manages a single window resource.
 * @version 0.1
 * @date 2019-09-04
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#ifndef EV2_WINDOW_H
#define EV2_WINDOW_H

#include <string>
#include <cstdint>

#include <ev.hpp>
#include <renderer/ev_gl.hpp>
#include <delegate.hpp>
#include <singleton.hpp>
#include <application.hpp>

namespace ev2::window {

void init(const Args& args);
void setWindowTitle(std::string_view title);
void setApplication(Application* app);
GLFWwindow* getContextWindow();
void setMouseCaptured(bool captured);
bool getMouseCaptured();

glm::vec2 getWindowSize();

glm::vec2 getCursorPosition();
double getFrameTime();
bool frame();


} // ev2

#endif // EV2_WINDOW_H