#pragma once

enum Anim_Easing {

};

template<typename T>
struct Keyframe {
	T value;
	Anim_Easing easing;
};

template<typename T>
struct Animation {
	std::map<unsigned long, Keyframe<T>> keyframes;

};

class Animator {

};