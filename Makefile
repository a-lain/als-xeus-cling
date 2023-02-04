# First variables
PROJECT_NAME = als-xeus-cling
BUILD_DIR = /tmp/build_tmp/${PROJECT_NAME}
INCLUDE_DIR = /usr/include/${PROJECT_NAME}
LIB_DIR = /usr/lib
BIN_DIR = /usr/bin
CXX = g++
CXXFLAGS = -Wall -Wextra -Wpedantic -fPIC -O3 -I /opt/cling/include
LIBRARY_DEPENDENCIES = -l xeus -l xeus-zmq -l zmq -L /opt/cling/lib -l cling -l als-basic-utilities

all: ${BUILD_DIR}/als-xeus-cling-kernel

${BUILD_DIR}/als-xeus-cling-kernel: main.cpp xinterpreter.cpp xparser.cpp
	mkdir -p ${BUILD_DIR}
	${CXX} ${CXXFLAGS} ${LIBRARY_DEPENDENCIES} -o ${BUILD_DIR}/als-xeus-cling-kernel\
		main.cpp\
		xinterpreter.cpp\
		xparser.cpp

install: ${BUILD_DIR}/als-xeus-cling-kernel
	cp ${BUILD_DIR}/als-xeus-cling-kernel ${BIN_DIR}
	cp -r als-xeus-cling-kernel /usr/share/jupyter/kernels
	chmod -R 755 /usr/share/jupyter/kernels/als-xeus-cling-kernel
	mkdir -p ${INCLUDE_DIR}
	install -T als-xeus-cling-config.hpp ${INCLUDE_DIR}/als-xeus-cling-config.hpp
	install -T xdisplay.hpp ${INCLUDE_DIR}/xdisplay.hpp
	install -T xinterpreter.hpp ${INCLUDE_DIR}/xinterpreter.hpp
	rm -r ${BUILD_DIR}

${BUILD_DIR}/%.o: %.cpp %.hpp
	mkdir -p $(@D)
	${CXX} ${CXXFLAGS} -o $@ -c $<

    
