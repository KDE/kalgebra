# Find libanalitza
# Once done this will define
#
#  ANALITZA_FOUND    - system has Analitza Library
#  ANALITZA_INCLUDES - the Analitza include directory
#  ANALITZA_LIBS     - link these to use Analitza
#  ANALITZA_VERSION  - the version of the Analitza Library

# Copyright (c) 2011, Aleix Pol Gonzalez <aleixpol@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

find_library(ANALITZA_LIBRARY NAMES analitza HINTS ${KDE4_LIB_INSTALL_DIR} ${QT_LIBRARY_DIR})
find_library(ANALITZAGUI_LIBRARY NAMES analitzagui HINTS ${KDE4_LIB_INSTALL_DIR} ${QT_LIBRARY_DIR})
find_library(ANALITZAPLOT_LIBRARY NAMES analitzaplot HINTS ${KDE4_LIB_INSTALL_DIR} ${QT_LIBRARY_DIR})

find_path(ANALITZA_INCLUDE_DIR NAMES analitza/analitzaexport.h HINTS ${KDE4_INCLUDE_INSTALL_DIR} ${QT_INCLUDE_DIR} ${INCLUDE_INSTALL_DIR})

if(ANALITZA_INCLUDE_DIR AND ANALITZA_LIBRARY)
   set(ANALITZA_LIBS ${analitza_LIB_DEPENDS} ${ANALITZA_LIBRARY} ${ANALITZAGUI_LIBRARY})
   set(ANALITZA_INCLUDES ${ANALITZA_INCLUDE_DIR}/KDE ${ANALITZA_INCLUDE_DIR})
endif(ANALITZA_INCLUDE_DIR AND ANALITZA_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Analitza  DEFAULT_MSG  ANALITZA_INCLUDE_DIR ANALITZA_LIBRARY)

mark_as_advanced(ANALITZA_INCLUDE_DIR ANALITZA_LIBRARY)
