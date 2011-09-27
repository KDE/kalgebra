set(MOBILE_BACKEND "desktop" CACHE STRING "backend to install, currently desktop and harmattan (directories in this folder)")

add_subdirectory(${MOBILE_BACKEND})