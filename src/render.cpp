#include "render.h"

std::multimap<int, Renderable*> Renderable::draw_calls;
std::vector<Shader*> Renderable::shaders;

Renderable::Renderable(std::function<void(Shader&)> draw_func, Shader* shader, int layer) {
	this->render_shader = shader;
	draw_call = draw_func;
	draw_call_it = draw_calls.emplace(layer, this);
}

Renderable::~Renderable(){
	draw_calls.erase(draw_call_it);
};

int Renderable::getLayer() {
	return draw_call_it->first;
}

int Renderable::changeLayer(int layer) {
	draw_calls.erase(draw_call_it);
	draw_call_it = draw_calls.emplace(layer, this);
	return layer;
}

Shader* Renderable::register_shader(Shader* shader) {
	shaders.push_back(shader);
	return shader;
}

void Renderable::draw_all() {
	// for(Camera2d* cam : cameras) {
		for(Shader* shader : shaders) {
			shader->set("view", Camera2d::main_camera->getViewMatrix());
			shader->set("projection", Camera2d::main_camera->getProjectionMatrix());
		}

		for(auto it = draw_calls.begin(); it != draw_calls.end(); it++) {
			Renderable& obj = *(it->second);
			if(obj.visible) {
				obj.draw_call(*obj.render_shader);
			}
		}
	// }
}