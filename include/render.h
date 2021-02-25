#pragma once

#include "shader.h"
#include "camera2d.h"

#include <functional>

class Renderable {
public:
	Renderable(Shader* shader, int layer = 1);
	virtual ~Renderable();

	Shader* render_shader;
	bool visible = true;	

	int getLayer();
	int changeLayer(int layer);

	virtual void draw(Shader& shader);

	static void draw_all();
    static Shader* register_shader(Shader* shader);

private:
	// std::function<void(Shader&)> draw_call;
	std::multimap<int, Renderable*>::iterator draw_order_it;

    static std::vector<Shader*> shaders;
	static std::multimap<int, Renderable*> draw_order;
	static std::vector<Camera2d*> cameras;
};