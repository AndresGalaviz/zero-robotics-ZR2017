CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

project(AsteroSPHERES)
add_subdirectory("${PROJECT_SOURCE_DIR}/../" ${PROJECT_BINARY_DIR}/all_libs)

option(extradefs "Define extra definitions here")
add_all_zr_libs(${PROJECT_SOURCE_DIR} "${PROJECT_SOURCE_DIR}/../../../SpheresCore/Science/controllers/ctrl_avoid.c" ${extradefs})
	