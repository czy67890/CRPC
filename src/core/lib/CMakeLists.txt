file(GLOB LIB_SRC *.cc */*.cc */*/*.cc)
#设置库的输出路径
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/src)
add_library(core_lib SHARED ${LIB_SRC})