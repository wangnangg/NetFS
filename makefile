include makefile.in 

all: ${build_dir}/client ${build_dir}/server ${build_dir}/utest 
.PHONY: all

${build_dir}/common/msg_base.o: common/msg_base.cpp | ${build_dir}/common
	${cpp_compiler} ${common_compile_flags} -MMD -MP -c common/msg_base.cpp -o ${build_dir}/common/msg_base.o

-include ${build_dir}/common/msg_base.d 

${build_dir}/common:
	mkdir -p ${build_dir}/common

${build_dir}/common/msg.o: common/msg.cpp | ${build_dir}/common
	${cpp_compiler} ${common_compile_flags} -MMD -MP -c common/msg.cpp -o ${build_dir}/common/msg.o

-include ${build_dir}/common/msg.d 

${build_dir}/common/msg_statfs.o: common/msg_statfs.cpp | ${build_dir}/common
	${cpp_compiler} ${common_compile_flags} -MMD -MP -c common/msg_statfs.cpp -o ${build_dir}/common/msg_statfs.o

-include ${build_dir}/common/msg_statfs.d 

${build_dir}/common/time.o: common/time.cpp | ${build_dir}/common
	${cpp_compiler} ${common_compile_flags} -MMD -MP -c common/time.cpp -o ${build_dir}/common/time.o

-include ${build_dir}/common/time.d 

${build_dir}/common/serial.o: common/serial.cpp | ${build_dir}/common
	${cpp_compiler} ${common_compile_flags} -MMD -MP -c common/serial.cpp -o ${build_dir}/common/serial.o

-include ${build_dir}/common/serial.d 

${build_dir}/client_src/cache.o: client_src/cache.cpp | ${build_dir}/client_src
	${cpp_compiler} ${client_compile_flags} -MMD -MP -c client_src/cache.cpp -o ${build_dir}/client_src/cache.o

-include ${build_dir}/client_src/cache.d 

${build_dir}/client_src:
	mkdir -p ${build_dir}/client_src

${build_dir}/client_src/main.o: client_src/main.cpp | ${build_dir}/client_src
	${cpp_compiler} ${client_compile_flags} -MMD -MP -c client_src/main.cpp -o ${build_dir}/client_src/main.o

-include ${build_dir}/client_src/main.d 

${build_dir}/client_src/range.o: client_src/range.cpp | ${build_dir}/client_src
	${cpp_compiler} ${client_compile_flags} -MMD -MP -c client_src/range.cpp -o ${build_dir}/client_src/range.o

-include ${build_dir}/client_src/range.d 

${build_dir}/client_src/netfs.o: client_src/netfs.cpp | ${build_dir}/client_src
	${cpp_compiler} ${client_compile_flags} -MMD -MP -c client_src/netfs.cpp -o ${build_dir}/client_src/netfs.o

-include ${build_dir}/client_src/netfs.d 

${build_dir}/client_src/stream.o: client_src/stream.cpp | ${build_dir}/client_src
	${cpp_compiler} ${client_compile_flags} -MMD -MP -c client_src/stream.cpp -o ${build_dir}/client_src/stream.o

-include ${build_dir}/client_src/stream.d 

${build_dir}/client: ${build_dir}/client_src/cache.o ${build_dir}/client_src/main.o ${build_dir}/client_src/netfs.o ${build_dir}/client_src/range.o ${build_dir}/client_src/stream.o ${build_dir}/common/msg.o ${build_dir}/common/msg_base.o ${build_dir}/common/msg_statfs.o ${build_dir}/common/serial.o ${build_dir}/common/time.o  | ${build_dir} 
	${linker} ${build_dir}/client_src/cache.o ${build_dir}/client_src/main.o ${build_dir}/client_src/netfs.o ${build_dir}/client_src/range.o ${build_dir}/client_src/stream.o ${build_dir}/common/msg.o ${build_dir}/common/msg_base.o ${build_dir}/common/msg_statfs.o ${build_dir}/common/serial.o ${build_dir}/common/time.o  ${client_link_flags} -o ${build_dir}/client

${build_dir}:
	mkdir -p ${build_dir}

${build_dir}/server_src/fileop.o: server_src/fileop.cpp | ${build_dir}/server_src
	${cpp_compiler} ${server_compile_flags} -MMD -MP -c server_src/fileop.cpp -o ${build_dir}/server_src/fileop.o

-include ${build_dir}/server_src/fileop.d 

${build_dir}/server_src:
	mkdir -p ${build_dir}/server_src

${build_dir}/server_src/StorageServer.o: server_src/StorageServer.cpp | ${build_dir}/server_src
	${cpp_compiler} ${server_compile_flags} -MMD -MP -c server_src/StorageServer.cpp -o ${build_dir}/server_src/StorageServer.o

-include ${build_dir}/server_src/StorageServer.d 

${build_dir}/server_src/msg_response.o: server_src/msg_response.cpp | ${build_dir}/server_src
	${cpp_compiler} ${server_compile_flags} -MMD -MP -c server_src/msg_response.cpp -o ${build_dir}/server_src/msg_response.o

-include ${build_dir}/server_src/msg_response.d 

${build_dir}/server_src/StorageServerConnection.o: server_src/StorageServerConnection.cpp | ${build_dir}/server_src
	${cpp_compiler} ${server_compile_flags} -MMD -MP -c server_src/StorageServerConnection.cpp -o ${build_dir}/server_src/StorageServerConnection.o

-include ${build_dir}/server_src/StorageServerConnection.d 

${build_dir}/server_src/StorageServerConnectionFactory.o: server_src/StorageServerConnectionFactory.cpp | ${build_dir}/server_src
	${cpp_compiler} ${server_compile_flags} -MMD -MP -c server_src/StorageServerConnectionFactory.cpp -o ${build_dir}/server_src/StorageServerConnectionFactory.o

-include ${build_dir}/server_src/StorageServerConnectionFactory.d 

${build_dir}/server_src/StorageServerParams.o: server_src/StorageServerParams.cpp | ${build_dir}/server_src
	${cpp_compiler} ${server_compile_flags} -MMD -MP -c server_src/StorageServerParams.cpp -o ${build_dir}/server_src/StorageServerParams.o

-include ${build_dir}/server_src/StorageServerParams.d 

${build_dir}/server_src/StorageInterface.o: server_src/StorageInterface.cpp | ${build_dir}/server_src
	${cpp_compiler} ${server_compile_flags} -MMD -MP -c server_src/StorageInterface.cpp -o ${build_dir}/server_src/StorageInterface.o

-include ${build_dir}/server_src/StorageInterface.d 

${build_dir}/server: ${build_dir}/common/msg.o ${build_dir}/common/msg_base.o ${build_dir}/common/msg_statfs.o ${build_dir}/common/serial.o ${build_dir}/common/time.o ${build_dir}/server_src/StorageInterface.o ${build_dir}/server_src/StorageServer.o ${build_dir}/server_src/StorageServerConnection.o ${build_dir}/server_src/StorageServerConnectionFactory.o ${build_dir}/server_src/StorageServerParams.o ${build_dir}/server_src/fileop.o ${build_dir}/server_src/msg_response.o  | ${build_dir} 
	${linker} ${build_dir}/common/msg.o ${build_dir}/common/msg_base.o ${build_dir}/common/msg_statfs.o ${build_dir}/common/serial.o ${build_dir}/common/time.o ${build_dir}/server_src/StorageInterface.o ${build_dir}/server_src/StorageServer.o ${build_dir}/server_src/StorageServerConnection.o ${build_dir}/server_src/StorageServerConnectionFactory.o ${build_dir}/server_src/StorageServerParams.o ${build_dir}/server_src/fileop.o ${build_dir}/server_src/msg_response.o  ${server_link_flags} -o ${build_dir}/server

${build_dir}/googletest/googletest/src/gtest-all.o: googletest/googletest/src/gtest-all.cc | ${build_dir}/googletest/googletest/src
	${cpp_compiler} ${gtest_compile_flags} -MMD -MP -c googletest/googletest/src/gtest-all.cc -o ${build_dir}/googletest/googletest/src/gtest-all.o

-include ${build_dir}/googletest/googletest/src/gtest-all.d 

${build_dir}/googletest/googletest/src:
	mkdir -p ${build_dir}/googletest/googletest/src

${build_dir}/utest_src/cache.o: utest_src/cache.cpp | ${build_dir}/utest_src
	${cpp_compiler} ${utest_compile_flags} -MMD -MP -c utest_src/cache.cpp -o ${build_dir}/utest_src/cache.o

-include ${build_dir}/utest_src/cache.d 

${build_dir}/utest_src:
	mkdir -p ${build_dir}/utest_src

${build_dir}/utest_src/main.o: utest_src/main.cpp | ${build_dir}/utest_src
	${cpp_compiler} ${utest_compile_flags} -MMD -MP -c utest_src/main.cpp -o ${build_dir}/utest_src/main.o

-include ${build_dir}/utest_src/main.d 

${build_dir}/utest_src/range.o: utest_src/range.cpp | ${build_dir}/utest_src
	${cpp_compiler} ${utest_compile_flags} -MMD -MP -c utest_src/range.cpp -o ${build_dir}/utest_src/range.o

-include ${build_dir}/utest_src/range.d 

${build_dir}/utest_src/example.o: utest_src/example.cpp | ${build_dir}/utest_src
	${cpp_compiler} ${utest_compile_flags} -MMD -MP -c utest_src/example.cpp -o ${build_dir}/utest_src/example.o

-include ${build_dir}/utest_src/example.d 

${build_dir}/utest_src/stream.o: utest_src/stream.cpp | ${build_dir}/utest_src
	${cpp_compiler} ${utest_compile_flags} -MMD -MP -c utest_src/stream.cpp -o ${build_dir}/utest_src/stream.o

-include ${build_dir}/utest_src/stream.d 

${build_dir}/utest_src/msg.o: utest_src/msg.cpp | ${build_dir}/utest_src
	${cpp_compiler} ${utest_compile_flags} -MMD -MP -c utest_src/msg.cpp -o ${build_dir}/utest_src/msg.o

-include ${build_dir}/utest_src/msg.d 

${build_dir}/utest_src/serial.o: utest_src/serial.cpp | ${build_dir}/utest_src
	${cpp_compiler} ${utest_compile_flags} -MMD -MP -c utest_src/serial.cpp -o ${build_dir}/utest_src/serial.o

-include ${build_dir}/utest_src/serial.d 

${build_dir}/utest: ${build_dir}/client_src/cache.o ${build_dir}/client_src/netfs.o ${build_dir}/client_src/range.o ${build_dir}/client_src/stream.o ${build_dir}/common/msg.o ${build_dir}/common/msg_base.o ${build_dir}/common/msg_statfs.o ${build_dir}/common/serial.o ${build_dir}/common/time.o ${build_dir}/googletest/googletest/src/gtest-all.o ${build_dir}/server_src/StorageInterface.o ${build_dir}/server_src/StorageServerConnection.o ${build_dir}/server_src/StorageServerConnectionFactory.o ${build_dir}/server_src/StorageServerParams.o ${build_dir}/server_src/fileop.o ${build_dir}/server_src/msg_response.o ${build_dir}/utest_src/cache.o ${build_dir}/utest_src/example.o ${build_dir}/utest_src/main.o ${build_dir}/utest_src/msg.o ${build_dir}/utest_src/range.o ${build_dir}/utest_src/serial.o ${build_dir}/utest_src/stream.o  | ${build_dir} 
	${linker} ${build_dir}/client_src/cache.o ${build_dir}/client_src/netfs.o ${build_dir}/client_src/range.o ${build_dir}/client_src/stream.o ${build_dir}/common/msg.o ${build_dir}/common/msg_base.o ${build_dir}/common/msg_statfs.o ${build_dir}/common/serial.o ${build_dir}/common/time.o ${build_dir}/googletest/googletest/src/gtest-all.o ${build_dir}/server_src/StorageInterface.o ${build_dir}/server_src/StorageServerConnection.o ${build_dir}/server_src/StorageServerConnectionFactory.o ${build_dir}/server_src/StorageServerParams.o ${build_dir}/server_src/fileop.o ${build_dir}/server_src/msg_response.o ${build_dir}/utest_src/cache.o ${build_dir}/utest_src/example.o ${build_dir}/utest_src/main.o ${build_dir}/utest_src/msg.o ${build_dir}/utest_src/range.o ${build_dir}/utest_src/serial.o ${build_dir}/utest_src/stream.o  ${utest_link_flags} -o ${build_dir}/utest

clean:
	rm -f ${build_dir}/client ${build_dir}/client_src/cache.o ${build_dir}/client_src/main.o ${build_dir}/client_src/netfs.o ${build_dir}/client_src/range.o ${build_dir}/client_src/stream.o ${build_dir}/common/msg.o ${build_dir}/common/msg_base.o ${build_dir}/common/msg_statfs.o ${build_dir}/common/serial.o ${build_dir}/common/time.o ${build_dir}/googletest/googletest/src/gtest-all.o ${build_dir}/server ${build_dir}/server_src/StorageInterface.o ${build_dir}/server_src/StorageServer.o ${build_dir}/server_src/StorageServerConnection.o ${build_dir}/server_src/StorageServerConnectionFactory.o ${build_dir}/server_src/StorageServerParams.o ${build_dir}/server_src/fileop.o ${build_dir}/server_src/msg_response.o ${build_dir}/utest ${build_dir}/utest_src/cache.o ${build_dir}/utest_src/example.o ${build_dir}/utest_src/main.o ${build_dir}/utest_src/msg.o ${build_dir}/utest_src/range.o ${build_dir}/utest_src/serial.o ${build_dir}/utest_src/stream.o 
	rm -f ${build_dir}/client_src/cache.d ${build_dir}/client_src/main.d ${build_dir}/client_src/netfs.d ${build_dir}/client_src/range.d ${build_dir}/client_src/stream.d ${build_dir}/common/msg.d ${build_dir}/common/msg_base.d ${build_dir}/common/msg_statfs.d ${build_dir}/common/serial.d ${build_dir}/common/time.d ${build_dir}/googletest/googletest/src/gtest-all.d ${build_dir}/server_src/StorageInterface.d ${build_dir}/server_src/StorageServer.d ${build_dir}/server_src/StorageServerConnection.d ${build_dir}/server_src/StorageServerConnectionFactory.d ${build_dir}/server_src/StorageServerParams.d ${build_dir}/server_src/fileop.d ${build_dir}/server_src/msg_response.d ${build_dir}/utest_src/cache.d ${build_dir}/utest_src/example.d ${build_dir}/utest_src/main.d ${build_dir}/utest_src/msg.d ${build_dir}/utest_src/range.d ${build_dir}/utest_src/serial.d ${build_dir}/utest_src/stream.d 
.PHONY: clean

