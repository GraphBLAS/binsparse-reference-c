function(add_cpp_example example_name)
  add_executable(${example_name} ${example_name}.cpp)
  target_link_libraries(${example_name} binsparse-rc)
endfunction()

set(CMAKE_CXX_STANDARD 20)
add_cpp_example(simple)
