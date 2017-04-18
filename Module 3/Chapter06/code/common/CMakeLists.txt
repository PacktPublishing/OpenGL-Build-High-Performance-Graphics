# Create a library called "Hello" which includes the source file "hello.cxx".
# The extension is already found. Any number of sources could be listed here.
add_library (shader shader.cpp shader.hpp common.h)
add_library (controls controls.cpp controls.hpp common.h)
add_library (texture texture.cpp texture.hpp common.h)
add_library (ObjLoader ObjLoader.cpp ObjLoader.hpp common.h)

# Make sure the compiler can find include files for our Hello library
# when other libraries or executables link to Hello
target_include_directories (shader PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories (controls PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories (texture PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories (ObjLoader PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
