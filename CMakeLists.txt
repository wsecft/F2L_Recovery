cmake_minimum_required (VERSION 3.20)
project("F2L_Recovery" VERSION 1.0 LANGUAGES CXX)
# Compile dynamiquement. 
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
set(SFML_STATIC_LIBRARIES FALSE)

# Spécifiez la version C++ standard.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Activez Rechargement à chaud pour les compilateurs MSVC si cela est pris en charge.
if(POLICY CMP0141)
    cmake_policy(SET CMP0141 NEW)
    if(MSVC)
        set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
    endif()
endif()

# Ajout de la bibliothèque SFML.
include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG master)
FetchContent_MakeAvailable(SFML)

FetchContent_Declare(frozen
    GIT_REPOSITORY https://github.com/serge-sans-paille/frozen.git
    GIT_TAG master)
FetchContent_MakeAvailable(frozen)

# Ajout des fichiers cpp et hpp.
file(GLOB_RECURSE source_files
    "src/*.cpp"
    "src/*.hpp"
    "res/*.json"
)

add_executable(${PROJECT_NAME} ${source_files})
target_link_libraries(${PROJECT_NAME} PRIVATE SFML::System SFML::Window SFML::Graphics frozen::frozen)

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