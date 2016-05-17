file(GLOB_RECURSE sources_localdir *.c *.h)
SET(sv_Files ${sv_Files} ${sources_localdir})

# Some include directories
include_directories(
    /usr/local/include
    /usr/include
)

# Creates the executable
add_executable(client ${sv_Files})