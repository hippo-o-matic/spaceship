#pragma once

#include "shader.h"
#include "camera2d.h"

#include <functional>

class Renderable {
public:
	Renderable(std::function<void(Shader&)> draw_func, Shader* shader, int layer = 1);
	~Renderable();

	Shader* render_shader;
	bool visible = true;	

	int getLayer();
	int changeLayer(int layer);

	static Shader* register_shader(Shader* shader);
	static void draw_all();

private:
	std::function<void(Shader&)> draw_call;
	std::multimap<int, Renderable*>::iterator draw_call_it;

	static std::multimap<int, Renderable*> draw_calls;
	static std::vector<Shader*> shaders;
	static std::vector<Camera2d*> cameras;
};