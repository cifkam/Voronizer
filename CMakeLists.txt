cmake_minimum_required(VERSION 3.0)
set (CMAKE_CXX_STANDARD 17)
project( Voronizer )
find_package( OpenCV REQUIRED )
include_directories( ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/hpp ${OpenCV_INCLUDE_DIRS} )
file(GLOB cpp_files
     "cpp/*.cpp"
)
add_executable( Voronizer ${cpp_files})
target_link_libraries( Voronizer ${OpenCV_LIBS} )
