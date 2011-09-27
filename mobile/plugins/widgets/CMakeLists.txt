option(MOBILE_BACKEND "backend to install, currently desktop and harmattan (directories in this folder)" "desktop")
if(NOT MOBILE_BACKEND)
    set(MOBILE_BACKEND "desktop")
endif(NOT MOBILE_BACKEND)

add_subdirectory(${MOBILE_BACKEND})