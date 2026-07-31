

add_executable(revres
    main.cpp
    server/tcp_server.cpp
)

target_include_directories(revres PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}   # lets us #include "server/tcp_server.h" etc.
)


if(WIN32)
    target_link_libraries(revres PRIVATE ws2_32)
endif()

# ── Compiler warnings ─────────────────────────────────────────────────────────
if(MSVC)
    target_compile_options(revres PRIVATE /W4)
else()
    target_compile_options(revres PRIVATE -Wall -Wextra -Wpedantic)
endif()
