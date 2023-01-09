cmake_minimum_required(VERSION 3.23)
project(SOL C)

set(CMAKE_C_STANDARD 99)

add_executable(SOL main.c collector.h collector.c cmake-build-debug/masterWorker.c parser.c cmake-build-debug/parser.c parser.h auxiliaryMW.c auxiliaryMW.h workers.h workers.c masterWorker.c collector.h collector.c masterWorker.c collector.c masterWorker.c masterWorker.c masterWorker.c masterWorker.c list.h list.c masterWorker.c collector.c)
