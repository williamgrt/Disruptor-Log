set(BASE_LIB
    Log.cc
    )

add_definitions(-std=c++11)
add_library(Gweb_base ${BASE_LIB})
target_link_libraries(Gweb_base pthread)