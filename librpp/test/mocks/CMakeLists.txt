project(rppmocks)

#
set(SOURCES
        mock_linux.h
)

add_library(rppmocks SHARED ${SOURCES})
set_target_properties(rppmocks PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(rppmocks
        PUBLIC
            "${CMAKE_CURRENT_SOURCE_DIR}"
        )