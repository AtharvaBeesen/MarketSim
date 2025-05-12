cmake_minimum_required(VERSION 3.10)
project(OrderBook CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set compiler flags
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Include directories
include_directories(include)

# Find Boost
find_package(Boost REQUIRED COMPONENTS system json)

# Source files
set(SOURCES
    src/OrderBook.cpp
    src/OrderBookManager.cpp
    src/main.cpp
)

# Add header files
set(HEADERS
    include/OrderBook.hpp
    include/OrderBookManager.hpp
    include/Order.hpp
    include/Trade.hpp
)

# Create executable
add_executable(orderbook ${SOURCES} ${HEADERS})

# Install target
install(TARGETS orderbook
    RUNTIME DESTINATION bin
)

# Add tests if needed
option(BUILD_TESTS "Build tests" OFF)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Include directories
target_include_directories(orderbook PRIVATE include ${Boost_INCLUDE_DIRS})

# Link libraries
target_link_libraries(orderbook PRIVATE ${Boost_LIBRARIES}) 