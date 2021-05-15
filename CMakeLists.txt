cmake_minimum_required(VERSION 3.1)
project(Interpreter_C++)

set(CMAKE_CXX_STANDARD 20)

add_executable(Compiler_C++
  scanner-regexp
  scanner
  scannerDemo
  parser
  AST
  ContextInfo
  generateHERA
  main
)

include_directories(../HaverfordCS/include /home/courses/include)
