cmake_minimum_required(VERSION 3.16)

# KDE Application Version, managed by release script
set(RELEASE_SERVICE_VERSION_MAJOR "24")
set(RELEASE_SERVICE_VERSION_MINOR "08")
set(RELEASE_SERVICE_VERSION_MICRO "3")
set(RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")

project(kalgebra VERSION ${RELEASE_SERVICE_VERSION})
set(KF_MIN_VERSION "6.3.0")
set(KDE_COMPILERSETTINGS_LEVEL "6.3.0")
find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)
include(ECMInstallIcons)
include(ECMSetupVersion)
include(FeatureSummary)
include(ECMAddAppIcon)
include(ECMDeprecationSettings)
if (NOT ANDROID)
    include(KDEClangFormat)
    include(KDEGitCommitHooks)
endif()
set(QT_REQUIRED_VERSION "6.5.0")

find_package(Qt6 ${QT_REQUIRED_VERSION} REQUIRED NO_MODULE COMPONENTS Qml Quick Xml Svg PrintSupport Test OpenGLWidgets)
find_package(Analitza6 REQUIRED)

ecm_set_disabled_deprecation_versions(QT 6.7
    KF 6.4.0
)


set(MOBILE_BACKEND "kde" CACHE STRING "Backend to install, currently. Check /mobile/plugins/widgets/*")

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})

set(CURSES_NEED_NCURSES TRUE)
find_package(Curses)
find_package(Readline)

ecm_setup_version(${RELEASE_SERVICE_VERSION} VARIABLE_PREFIX KALGEBRA VERSION_HEADER kalgebra_version.h)

set_package_properties(Readline PROPERTIES TYPE OPTIONAL
                                           PURPOSE "Allows KAlgebra to provide a console interface."
                                           URL "https://tiswww.case.edu/php/chet/readline/rltop.html")
set_package_properties(Curses PROPERTIES TYPE OPTIONAL
                                         PURPOSE "Allows KAlgebra to provide a console interface."
                                         URL "https://www.gnu.org/software/ncurses/")

add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
add_definitions(-DQT_NO_URL_CAST_FROM_STRING)
add_definitions(-DQT_NO_CAST_TO_ASCII)

find_package(KF6 ${KF_MIN_VERSION} REQUIRED COMPONENTS I18n CoreAddons)
find_package(KF6 ${KF_MIN_VERSION} OPTIONAL_COMPONENTS ConfigWidgets WidgetsAddons KIO DocTools XmlGui)
find_package(Plasma)
find_package(Qt6WebEngineWidgets)

if(KF6DocTools_FOUND AND Qt6WebEngineWidgets_FOUND AND KF6ConfigWidgets_FOUND AND KF6WidgetsAddons_FOUND AND KF6KIO_FOUND AND NOT CMAKE_SYSTEM MATCHES Android*)
    add_subdirectory(src)
endif()

if (Plasma_FOUND AND NOT CMAKE_SYSTEM MATCHES Android*)
    add_subdirectory(plasmoids)
endif()

add_subdirectory(icons)
add_subdirectory(mobile)

if(READLINE_FOUND AND CURSES_FOUND)
    add_subdirectory(calgebra)
endif()

if(KF6DocTools_FOUND)
  add_subdirectory(utils)

  add_custom_target(commandsdoc
    ${CMAKE_CURRENT_BINARY_DIR}/utils/docbook_analitzacommands commands.docbook
    DEPENDS docbook_analitzacommands
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/doc/
    COMMENT "Generating commands docbook information"
  )

  add_subdirectory(doc)
endif()

ki18n_install(po)
if (KF6DocTools_FOUND)
    kdoctools_install(po)
endif()
feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)

if (NOT ANDROID)
    file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES src/*.cpp src/*.h mobile/*.cpp mobile/*.h utils/*.cpp utils/*.h)
    kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})

    kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
endif()
