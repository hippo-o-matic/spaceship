#include "render.h"

std::multimap<int, Renderable*> Renderable::draw_order;
std::vector<Shader*> Renderable::shaders;

Renderable::Renderable(Shader* shader, int layer) : 
	render_shader(shader)
{
	draw_order_it = draw_order.emplace(layer, this);
}

Renderable::~Renderable(){
	draw_order.erase(draw_order_it);
};

int Renderable::getLayer() {
	return draw_order_it->first;
}

int Renderable::changeLayer(int layer) {
	draw_order.erase(draw_order_it);
	draw_order_it = draw_order.emplace(layer, this);
	return layer;
}

Shader* Renderable::register_shader(Shader* shader) {
	if(std::find(shaders.begin(), shaders.end(), shader) == shaders.end())
		shaders.push_back(shader);
	return shader;
}

void Renderable::draw(Shader& shader) {}

void Renderable::draw_all() {
	// for(Camera2d* cam : cameras) {
		for(auto& shader : Renderable::shaders) {
			shader->set("view", Camera2d::main_camera->getViewMatrix());
			shader->set("projection", Camera2d::main_camera->getProjectionMatrix());
		}

		for(auto it = draw_order.begin(); it != draw_order.end(); it++) {
			Renderable& obj = *(it->second);
			if(obj.visible) {
				obj.draw(*obj.render_shader);
			}
		}
	// }
}