# CMakeList.txt: проект CMake для ccomp; включите исходный код и определения,
# укажите здесь логику для конкретного проекта.
#

# Добавьте источник в исполняемый файл этого проекта.
add_executable (ccomp 
	"Main.cpp" "Main.h"
	"Lexer.cpp" "Lexer.h"
	"Parser.cpp" "Parser.h"
	"TVar.cpp" "TVar.h"
	"Token.cpp" "Token.h"
	"Gen.cpp" "Gen.h"
		Nodes.cpp
		Nodes.h
)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Set your project compile flags.
# E.g. if using the C++ header files
# you will need to enable C++11 support
# for your compiler.

include_directories(${LLVM_INCLUDE_DIRS})

separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

# Find the libraries that correspond to the LLVM components
# that we wish to use
llvm_map_components_to_libnames(llvm_libs
  BitWriter
  Core
  ExecutionEngine
  MC
  MCJIT
  Support
  nativecodegen)

# Link against LLVM libraries
target_link_libraries(ccomp ${llvm_libs})