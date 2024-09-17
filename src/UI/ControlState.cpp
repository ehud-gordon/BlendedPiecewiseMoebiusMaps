#include "UI/ControlState.h"

ControlState::ControlState(float initial_width, float initial_height)
    : last_x(initial_width / 2.0f),
    last_y(initial_height / 2.0f),
    first_mouse(true),
    delta_time(0.0f),
    last_frame(0.0f),
    left_mouse_pressed(false),
    right_mouse_pressed(false),
    ctrl_pressed(false) ,
    shift_pressed(false)
{}

void ControlState::UpdateDeltaTime(float current_frame) {
    delta_time = current_frame - last_frame;
    last_frame = current_frame;
}