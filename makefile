include makefile.in 

all: ${build_dir}/client ${build_dir}/server ${build_dir}/utest 
.PHONY: all

${build_dir}/common/stream.o: common/stream.cpp | ${build_dir}/common
	${cpp_compiler} ${common_compile_flags} -MMD -MP -c common/stream.cpp -o ${build_dir}/common/stream.o

-include ${build_dir}/common/stream.d 

${build_dir}/common:
	mkdir -p ${build_dir}/common

${build_dir}/common/msg.o: common/msg.cpp | ${build_dir}/common
	${cpp_compiler} ${common_compile_flags} -MMD -MP -c common/msg.cpp -o ${build_dir}/common/msg.o

-include ${build_dir}/common/msg.d 

${build_dir}/client_src/main.o: client_src/main.cpp | ${build_dir}/client_src
	${cpp_compiler} ${client_compile_flags} -MMD -MP -c client_src/main.cpp -o ${build_dir}/client_src/main.o

-include ${build_dir}/client_src/main.d 

${build_dir}/client_src:
	mkdir -p ${build_dir}/client_src

${build_dir}/client_src/net.o: client_src/net.cpp | ${build_dir}/client_src
	${cpp_compiler} ${client_compile_flags} -MMD -MP -c client_src/net.cpp -o ${build_dir}/client_src/net.o

-include ${build_dir}/client_src/net.d 

${build_dir}/client_src/netfs.o: client_src/netfs.cpp | ${build_dir}/client_src
	${cpp_compiler} ${client_compile_flags} -MMD -MP -c client_src/netfs.cpp -o ${build_dir}/client_src/netfs.o

-include ${build_dir}/client_src/netfs.d 

${build_dir}/client: ${build_dir}/client_src/main.o ${build_dir}/client_src/net.o ${build_dir}/client_src/netfs.o ${build_dir}/common/msg.o ${build_dir}/common/stream.o  | ${build_dir} 
	${linker} ${build_dir}/client_src/main.o ${build_dir}/client_src/net.o ${build_dir}/client_src/netfs.o ${build_dir}/common/msg.o ${build_dir}/common/stream.o  ${client_link_flags} -o ${build_dir}/client

${build_dir}:
	mkdir -p ${build_dir}

${build_dir}/server_src/main.o: server_src/main.cpp | ${build_dir}/server_src
	${cpp_compiler} ${server_compile_flags} -MMD -MP -c server_src/main.cpp -o ${build_dir}/server_src/main.o

-include ${build_dir}/server_src/main.d 

${build_dir}/server_src:
	mkdir -p ${build_dir}/server_src

${build_dir}/server: ${build_dir}/common/msg.o ${build_dir}/common/stream.o ${build_dir}/server_src/main.o  | ${build_dir} 
	${linker} ${build_dir}/common/msg.o ${build_dir}/common/stream.o ${build_dir}/server_src/main.o  ${server_link_flags} -o ${build_dir}/server

${build_dir}/googletest/googletest/src/gtest-all.o: googletest/googletest/src/gtest-all.cc | ${build_dir}/googletest/googletest/src
	${cpp_compiler} ${gtest_compile_flags} -MMD -MP -c googletest/googletest/src/gtest-all.cc -o ${build_dir}/googletest/googletest/src/gtest-all.o

-include ${build_dir}/googletest/googletest/src/gtest-all.d 

${build_dir}/googletest/googletest/src:
	mkdir -p ${build_dir}/googletest/googletest/src

${build_dir}/utest_src/main.o: utest_src/main.cpp | ${build_dir}/utest_src
	${cpp_compiler} ${utest_compile_flags} -MMD -MP -c utest_src/main.cpp -o ${build_dir}/utest_src/main.o

-include ${build_dir}/utest_src/main.d 

${build_dir}/utest_src:
	mkdir -p ${build_dir}/utest_src

${build_dir}/utest_src/net.o: utest_src/net.cpp | ${build_dir}/utest_src
	${cpp_compiler} ${utest_compile_flags} -MMD -MP -c utest_src/net.cpp -o ${build_dir}/utest_src/net.o

-include ${build_dir}/utest_src/net.d 

${build_dir}/utest_src/example.o: utest_src/example.cpp | ${build_dir}/utest_src
	${cpp_compiler} ${utest_compile_flags} -MMD -MP -c utest_src/example.cpp -o ${build_dir}/utest_src/example.o

-include ${build_dir}/utest_src/example.d 

${build_dir}/utest_src/stream.o: utest_src/stream.cpp | ${build_dir}/utest_src
	${cpp_compiler} ${utest_compile_flags} -MMD -MP -c utest_src/stream.cpp -o ${build_dir}/utest_src/stream.o

-include ${build_dir}/utest_src/stream.d 

${build_dir}/utest_src/msg.o: utest_src/msg.cpp | ${build_dir}/utest_src
	${cpp_compiler} ${utest_compile_flags} -MMD -MP -c utest_src/msg.cpp -o ${build_dir}/utest_src/msg.o

-include ${build_dir}/utest_src/msg.d 

${build_dir}/utest: ${build_dir}/client_src/net.o ${build_dir}/client_src/netfs.o ${build_dir}/common/msg.o ${build_dir}/common/stream.o ${build_dir}/googletest/googletest/src/gtest-all.o ${build_dir}/utest_src/example.o ${build_dir}/utest_src/main.o ${build_dir}/utest_src/msg.o ${build_dir}/utest_src/net.o ${build_dir}/utest_src/stream.o  | ${build_dir} 
	${linker} ${build_dir}/client_src/net.o ${build_dir}/client_src/netfs.o ${build_dir}/common/msg.o ${build_dir}/common/stream.o ${build_dir}/googletest/googletest/src/gtest-all.o ${build_dir}/utest_src/example.o ${build_dir}/utest_src/main.o ${build_dir}/utest_src/msg.o ${build_dir}/utest_src/net.o ${build_dir}/utest_src/stream.o  ${utest_link_flags} -o ${build_dir}/utest

clean:
	rm -f ${build_dir}/client ${build_dir}/client_src/main.o ${build_dir}/client_src/net.o ${build_dir}/client_src/netfs.o ${build_dir}/common/msg.o ${build_dir}/common/stream.o ${build_dir}/googletest/googletest/src/gtest-all.o ${build_dir}/server ${build_dir}/server_src/main.o ${build_dir}/utest ${build_dir}/utest_src/example.o ${build_dir}/utest_src/main.o ${build_dir}/utest_src/msg.o ${build_dir}/utest_src/net.o ${build_dir}/utest_src/stream.o 
	rm -f ${build_dir}/client_src/main.d ${build_dir}/client_src/net.d ${build_dir}/client_src/netfs.d ${build_dir}/common/msg.d ${build_dir}/common/stream.d ${build_dir}/googletest/googletest/src/gtest-all.d ${build_dir}/server_src/main.d ${build_dir}/utest_src/example.d ${build_dir}/utest_src/main.d ${build_dir}/utest_src/msg.d ${build_dir}/utest_src/net.d ${build_dir}/utest_src/stream.d 
.PHONY: clean

