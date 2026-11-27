cmake_minimum_required (VERSION 3.20)
project("F2L_Recovery" VERSION 1.0 LANGUAGES CXX)
option(F2L_BUILD_APP "Build the SFML application" ON)
option(F2L_BUILD_TESTS "Build tests" ON)
option(F2L_BUILD_WEB "Build the WebAssembly browser engine" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(POLICY CMP0141)
    cmake_policy(SET CMP0141 NEW)
    if(MSVC)
        set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
    endif()
endif()

include(FetchContent)

FetchContent_Declare(frozen
    GIT_REPOSITORY https://github.com/serge-sans-paille/frozen.git
    GIT_TAG 1.2.0)
FetchContent_MakeAvailable(frozen)

add_library(f2l_core INTERFACE)
target_include_directories(f2l_core INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_features(f2l_core INTERFACE cxx_std_20)
target_link_libraries(f2l_core INTERFACE frozen::frozen)

if(F2L_BUILD_APP)
    option(BUILD_SHARED_LIBS "Build shared libraries for the application" ON)

    FetchContent_Declare(SFML
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 3.0.2)
    FetchContent_MakeAvailable(SFML)

    add_executable(${PROJECT_NAME}
        src/main.cpp
        src/draw.cpp
    )
    target_link_libraries(${PROJECT_NAME} PRIVATE f2l_core SFML::System SFML::Window SFML::Graphics)

    if(WIN32)
        # Windows: SFML DLLs are in bin/<CONFIG>
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${sfml_BINARY_DIR}/bin/$<CONFIG>
                $<TARGET_FILE_DIR:${PROJECT_NAME}>)
    endif()

    add_custom_target(run
        COMMAND $<TARGET_FILE:F2L_Recovery>
        DEPENDS F2L_Recovery
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Running F2L_Recovery..."
    )
endif()

if(F2L_BUILD_WEB)
    if(NOT EMSCRIPTEN)
        message(FATAL_ERROR "F2L_BUILD_WEB requires the Emscripten toolchain")
    endif()

    add_executable(f2l_web src/wasm_bindings.cpp)
    target_link_libraries(f2l_web PRIVATE f2l_core)
    target_compile_options(f2l_web PRIVATE -fexceptions)
    target_link_options(f2l_web PRIVATE
        --bind
        --no-entry
        -fexceptions
        -sDISABLE_EXCEPTION_CATCHING=0
        -sMODULARIZE=1
        -sEXPORT_ES6=1
        -sEXPORT_NAME=createF2LEngine
        -sENVIRONMENT=web,worker,node
        -sALLOW_MEMORY_GROWTH=1
        -sFILESYSTEM=0
        -sNO_EXIT_RUNTIME=1
    )
    set_target_properties(f2l_web PROPERTIES
        SUFFIX ".js"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/web"
    )
    add_custom_target(f2l_web_assets
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/web/"
            "${CMAKE_BINARY_DIR}/web"
        VERBATIM
    )
    add_dependencies(f2l_web f2l_web_assets)
endif()

if(F2L_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
