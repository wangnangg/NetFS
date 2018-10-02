include makefile.in 

all: ${build_dir}/client ${build_dir}/utest 
.PHONY: all

${build_dir}/client_src/main.o: client_src/main.cpp | ${build_dir}/client_src
	${cpp_compiler} ${client_compile_flags} -MMD -MP -c client_src/main.cpp -o ${build_dir}/client_src/main.o

-include ${build_dir}/client_src/main.d 

${build_dir}/client_src:
	mkdir -p ${build_dir}/client_src

${build_dir}/client: ${build_dir}/client_src/main.o  | ${build_dir} 
	${linker} ${build_dir}/client_src/main.o  ${client_link_flags} -o ${build_dir}/client

${build_dir}:
	mkdir -p ${build_dir}

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

${build_dir}/utest_src/example.o: utest_src/example.cpp | ${build_dir}/utest_src
	${cpp_compiler} ${utest_compile_flags} -MMD -MP -c utest_src/example.cpp -o ${build_dir}/utest_src/example.o

-include ${build_dir}/utest_src/example.d 

${build_dir}/utest: ${build_dir}/googletest/googletest/src/gtest-all.o ${build_dir}/utest_src/main.o ${build_dir}/utest_src/example.o  | ${build_dir} 
	${linker} ${build_dir}/googletest/googletest/src/gtest-all.o ${build_dir}/utest_src/main.o ${build_dir}/utest_src/example.o  ${utest_link_flags} -o ${build_dir}/utest

clean:
	rm -f ${build_dir}/googletest/googletest/src/gtest-all.o ${build_dir}/client ${build_dir}/utest_src/main.o ${build_dir}/utest_src/example.o ${build_dir}/utest ${build_dir}/client_src/main.o 
	rm -f ${build_dir}/googletest/googletest/src/gtest-all.d ${build_dir}/utest_src/main.d ${build_dir}/client_src/main.d ${build_dir}/utest_src/example.d 
.PHONY: clean

