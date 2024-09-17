#pragma once
struct ControlState {
	float last_x;
	float last_y;
	bool first_mouse;
	float delta_time;
	float last_frame;

	bool left_mouse_pressed;
	bool right_mouse_pressed;
	bool ctrl_pressed;
	bool shift_pressed;
	bool alt_pressed;

	ControlState(float initial_width, float initial_height);
	void UpdateDeltaTime(float current_frame);

};