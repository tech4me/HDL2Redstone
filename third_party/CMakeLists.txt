add_subdirectory(libblifparse)

set(NBT_BUILD_SHARED ON CACHE BOOL "" FORCE)
set(NBT_USE_ZLIB ON CACHE BOOL "" FORCE)
set(NBT_BUILD_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(libnbtplusplus)

set(JSON_BuildTests OFF CACHE INTERNAL "")
set(JSON_Install OFF CACHE INTERNAL "")
add_subdirectory(json)
