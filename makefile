cpp_compiler?=gcc
linker?=gcc
config?=debug
build_dir:=build/${config}

client_compile_flags:= -Isrc -Ifuse-3/include -std=c++17 -Wall -Werror -MMD -MP -D_FILE_OFFSET_BITS=64 -Winvalid-pch -D_REENTRANT -Wextra -Wno-sign-compare -fno-strict-aliasing -fpermissive -Wno-unused-result -Wno-missing-field-initializers
client_link_flags:= -lstdc++ -pthread -Lfuse-3/build/lib -lfuse3 '-Wl,-rpath,$$ORIGIN/lib'

ifeq ($(config), release)
  client_compile_flags+= -g -O3
  client_link_flags+= -g -O3
else
	ifeq ($(config), debug)
	  client_compile_flags+= -g
	  client_link_flags+= -g
	else
		$(error Unknown config: $(config))
	endif
endif


.PHONY: all
all: ${build_dir}/client

clean:
	rm -f ${build_dir}/client
	rm -f $(shell find build -name "*.o")
	rm -f $(shell find build -name "*.d")

${build_dir}/client_src/main.o: client_src/main.cpp | ${build_dir}/client_src 
	${cpp_compiler} ${client_compile_flags} -c client_src/main.cpp -o ${build_dir}/client_src/main.o
${build_dir}/client_src:
	mkdir -p $@
${build_dir}/client: ${build_dir}/client_src/main.o  | ${build_dir}
	${linker} ${build_dir}/client_src/main.o  ${client_link_flags} -o ${build_dir}/client
${build_dir}:
	mkdir -p $@
deps:=${build_dir}/client_src/main.d 

-include ${deps}


