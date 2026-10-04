cmake_minimum_required(VERSION 3.16)
project(photoblack)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Chỉ định đường dẫn tới thư viện wxWidgets
set(wxWidgets_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/wxWidgets")
set(wxWidgets_LIB_DIR "${wxWidgets_ROOT_DIR}/lib/gcc_lib")

# Thêm Include Path
include_directories(
    ${wxWidgets_ROOT_DIR}/include
    ${wxWidgets_LIB_DIR}/mswud # Hoặc mswu nếu build Release
)

# Thêm Link Path
link_directories(${wxWidgets_LIB_DIR})

# Khai báo file nguồn chính
add_executable(photoblack src/main.cpp)

# Link các thư viện tĩnh của wxWidgets (đối với GCC)
target_link_libraries(photoblack PRIVATE
    wxmsw32ud_core
    wxbase32u
    wxtiff
    wxjpeg
    wxpng
    wxzlib
    kernel32
    user32
    gdi32
    comdlg32
    winspool
    winmm
    shell32
    comctl32
    ole32
    oleaut32
    uuid
    rpcrt4
    advapi32
    version
    ws2_32
)