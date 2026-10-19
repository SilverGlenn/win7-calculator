set(CMAKE_AUTOMOC ON)
set(CORE "${PROJECT_SOURCE_DIR}/src/CalcManager")
set(CORE_SOURCES
  CEngine/Number.cpp CEngine/scicomm.cpp CEngine/calc.cpp CEngine/RationalMath.cpp
  CEngine/scifunc.cpp CEngine/sciset.cpp CEngine/Rational.cpp CEngine/scidisp.cpp
  CEngine/History.cpp CEngine/CalcUtils.cpp CEngine/scioper.cpp CEngine/CalcInput.cpp
  Ratpack/num.cpp Ratpack/basex.cpp Ratpack/fact.cpp Ratpack/trans.cpp
  Ratpack/logic.cpp Ratpack/exp.cpp Ratpack/conv.cpp Ratpack/support.cpp
  Ratpack/itrans.cpp Ratpack/transh.cpp Ratpack/itransh.cpp Ratpack/rat.cpp
  CalculatorManager.cpp CalculatorHistory.cpp ExpressionCommand.cpp)
list(TRANSFORM CORE_SOURCES PREPEND "${CORE}/")
set(STRINGS_HEADER "${PROJECT_BINARY_DIR}/generated/EngineStringsData.h")
add_custom_command(OUTPUT "${STRINGS_HEADER}" "${PROJECT_BINARY_DIR}/generated/engine-en-US.json"
  COMMAND Python3::Interpreter "${PROJECT_SOURCE_DIR}/tools/linux/generate-engine-strings.py"
    --source "${PROJECT_SOURCE_DIR}/src/Calculator/Resources/en-US/CEngineStrings.resw"
    --ids "${CORE}/Header Files/EngineStrings.h" --header "${STRINGS_HEADER}"
    --metadata "${PROJECT_BINARY_DIR}/generated/engine-en-US.json"
  DEPENDS "${PROJECT_SOURCE_DIR}/tools/linux/generate-engine-strings.py"
    "${PROJECT_SOURCE_DIR}/src/Calculator/Resources/en-US/CEngineStrings.resw"
    "${CORE}/Header Files/EngineStrings.h" VERBATIM)
add_library(win7calc_engine STATIC ${CORE_SOURCES} "${STRINGS_HEADER}" engine/Engine.cpp engine/Numeric.cpp engine/Resources.cpp)
target_include_directories(win7calc_engine PUBLIC "${CORE}" "${CMAKE_CURRENT_SOURCE_DIR}" PRIVATE "${PROJECT_BINARY_DIR}/generated")
target_compile_options(win7calc_engine PRIVATE -Wall -Wextra -Wno-unused-parameter -Wno-unknown-pragmas)
# Preserve upstream aggregate-initialization and signed/unsigned conventions;
# retain full diagnostics for the new adapter code.
set_source_files_properties(${CORE_SOURCES} PROPERTIES
  COMPILE_OPTIONS "-Wno-missing-field-initializers;-Wno-sign-compare")
add_library(win7calc_model STATIC model/CalculatorController.cpp model/CalculatorController.h)
target_link_libraries(win7calc_model PUBLIC win7calc_engine Qt6::Widgets)
