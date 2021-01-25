project('game', 'cpp', 'c',
	version: '0.1.0',
	default_options: ['cpp_std=c++17'],
)

cc = meson.get_compiler('c')
include = include_directories('include')
src_dir = 'src'

lib_dl = cc.find_library('dl')

jsoncpp_proj = subproject('jsoncpp')
jsoncpp_dep = jsoncpp_proj.get_variable('jsoncpp_dep')

protobuf_proj = subproject('protobuf')
protobuf_dep = protobuf_proj.get_variable('protobuf_lite_dep') # Remove _lite if you need more features
protoc_dep = protobuf_proj.get_variable('protoc_dep') # Remove _lite if you need more features
protoc_gen = protobuf_proj.get_variable('protoc_gen')

# imgui_proj = subproject('imgui')
# imgui_dep = imgui_proj.get_variable('imgui_dep')

core_dep = [
	dependency('GL', method: 'auto'),
	dependency('threads'),
	dependency('glfw3'),
	dependency('glm'),
	lib_dl,
	jsoncpp_dep,
	protobuf_dep,
	protoc_dep
	# imgui_dep
]

subdir(src_dir)

test = executable('demo',
	src,
	include_directories: include,
	dependencies: core_dep
)