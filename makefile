cpp_compiler?=gcc
linker?=gcc
config?=debug
build_dir:=build/${config}

client_compile_flags:= -Isrc -Ifuse-3/include -std=c++17 -Wall -Werror -MMD -MP -D_FILE_OFFSET_BITS=64 -Winvalid-pch -D_REENTRANT -Wextra -Wno-sign-compare -fno-strict-aliasing -fpermissive -Wno-unused-result -Wno-missing-field-initializers
client_link_flags:= -lstdc++ -pthread -Lfuse-3/build/lib -lfuse3 '-Wl,-rpath,$$ORIGIN/lib'

gtest_dir:= googletest/googletest
gtest_compile_flags:= -isystem ${gtest_dir}/include -I${gtest_dir}

utest_compile_flags:=${gtest_compile_flags} ${client_compile_flags}
utest_link_flags:= -lstdc++ -pthread

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


.PHONY: all client utest
all: client utest

utest: ${build_dir}/utest

client: ${build_dir}/client 


clean:
	rm -f ${build_dir}/client
	rm -f ${build_dir}/utest
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
${build_dir}/googletest/googletest/src/gtest-all.o: googletest/googletest/src/gtest-all.cc | ${build_dir}/googletest/googletest/src 
	${cpp_compiler} ${gtest_compile_flags} -c googletest/googletest/src/gtest-all.cc -o ${build_dir}/googletest/googletest/src/gtest-all.o
${build_dir}/googletest/googletest/src:
	mkdir -p $@
${build_dir}/utest_src/main.o: utest_src/main.cpp | ${build_dir}/utest_src 
	${cpp_compiler} ${utest_compile_flags} -c utest_src/main.cpp -o ${build_dir}/utest_src/main.o
${build_dir}/utest_src:
	mkdir -p $@
${build_dir}/utest_src/example.o: utest_src/example.cpp | ${build_dir}/utest_src 
	${cpp_compiler} ${utest_compile_flags} -c utest_src/example.cpp -o ${build_dir}/utest_src/example.o
${build_dir}/utest: ${build_dir}/googletest/googletest/src/gtest-all.o ${build_dir}/utest_src/main.o ${build_dir}/utest_src/example.o  | ${build_dir}
	${linker} ${build_dir}/googletest/googletest/src/gtest-all.o ${build_dir}/utest_src/main.o ${build_dir}/utest_src/example.o  ${utest_link_flags} -o ${build_dir}/utest
deps:=${build_dir}/client_src/main.d ${build_dir}/googletest/googletest/src/gtest-all.d ${build_dir}/utest_src/main.d ${build_dir}/utest_src/example.d 

-include ${deps}


