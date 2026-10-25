set(CMAKE_AUTOMOC ON)
foreach(name IN ITEMS engine numeric)
  add_executable(${name}_test ${name}_test.cpp)
  target_link_libraries(${name}_test PRIVATE win7calc_engine Qt6::Test)
  target_compile_definitions(${name}_test PRIVATE FIXTURE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/fixtures")
  add_test(NAME ${name} COMMAND ${name}_test)
endforeach()
add_test(NAME install_user COMMAND Python3::Interpreter -m unittest discover
  -s "${CMAKE_CURRENT_SOURCE_DIR}" -p install_test.py -v)
set_tests_properties(install_user PROPERTIES
  ENVIRONMENT "WIN7CALC_TEST_BUILD_DIR=${PROJECT_BINARY_DIR}"
  TIMEOUT 60)
add_test(NAME image_compare COMMAND Python3::Interpreter -m unittest discover
  -s "${CMAKE_CURRENT_SOURCE_DIR}" -p test_image_compare.py -v)
set_tests_properties(image_compare PROPERTIES
  ENVIRONMENT "PYTHONPYCACHEPREFIX=${PROJECT_BINARY_DIR}/python-cache"
  TIMEOUT 60)
add_test(NAME numeric_locale COMMAND numeric_test --comma)
add_executable(controller_test controller_test.cpp)
target_link_libraries(controller_test PRIVATE win7calc_model Qt6::Test)
target_compile_definitions(controller_test PRIVATE FIXTURE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/fixtures")
add_test(NAME controller COMMAND controller_test)
set_tests_properties(controller PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen;LC_ALL=C.UTF-8")
foreach(name IN ITEMS standard_ui layout scientific programmer statistics)
  add_executable(${name}_test ${name}_test.cpp)
  target_link_libraries(${name}_test PRIVATE win7calc_widgets Qt6::Test)
  add_test(NAME ${name} COMMAND ${name}_test)
  set_tests_properties(${name} PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen;LC_ALL=C.UTF-8")
endforeach()
