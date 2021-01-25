t = meson.current_source_dir() + '/'
protos = [
	t + 'tileGrid.proto'
]

src += protoc_gen.process(protos, extra_args: ['--proto_path=' + meson.current_source_dir()])