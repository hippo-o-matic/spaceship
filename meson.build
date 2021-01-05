project('game', 'cpp', 'c',
	version: '0.1.0',
	default_options: ['cpp_std=c++17'],
)

cc = meson.get_compiler('c')
include = include_directories('include')

lib_dl = cc.find_library('dl')

jsoncpp_proj = subproject('jsoncpp')
jsoncpp_dep = jsoncpp_proj.get_variable('jsoncpp_dep')

core_dep = [
	dependency('GL', method: 'auto'),
	dependency('threads'),
	dependency('glfw3'),
	dependency('glm'),
	lib_dl,
	jsoncpp_dep
]

subdir('src')

test = executable('demo',
	src,
	include_directories: include,
	dependencies: core_dep
)