cmake_minimum_required( VERSION 3.4 )
project( cvexprocedural )

list( APPEND CMAKE_PREFIX_PATH "$ENV{HT}/cmake")
find_package( Houdini REQUIRED )

set( library_name RAY_CvexProcedural )
add_library( ${library_name} SHARED
    src/RAY_CvexProcedural.cpp
    src/RAY_CvexProcedural.hpp
)

# Link against the Houdini libraries, and add required include directories and compile definitions.
target_link_libraries( ${library_name} Houdini )
# Configure several common target properties, such as its output directory.
houdini_configure_target( ${library_name} )