file(GLOB LIB_SRC  event_engine/thread_pool/*.cc  *.cc */*.cc */*/*.cc)
message("srclist ${LIB_SRC}")
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/src)
add_library(core_lib  ${LIB_SRC}
        ../../../include/crpc/impl/any_invocable_internal.h
        event_engine/posix_event_engine/timer.h)