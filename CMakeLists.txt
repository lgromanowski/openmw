project(OpenMW)

if (APPLE)
    set(APP_BUNDLE_NAME "${CMAKE_PROJECT_NAME}.app")

    set(APP_BUNDLE_DIR "${OpenMW_BINARY_DIR}/${APP_BUNDLE_NAME}")
endif (APPLE)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/)

# Version
message(STATUS "Configuring OpenMW...")

set(OPENMW_VERSION_MAJOR 0)
set(OPENMW_VERSION_MINOR 32)
set(OPENMW_VERSION_RELEASE 0)

set(OPENMW_VERSION_COMMITHASH "")
set(OPENMW_VERSION_TAGHASH "")

set(OPENMW_VERSION "${OPENMW_VERSION_MAJOR}.${OPENMW_VERSION_MINOR}.${OPENMW_VERSION_RELEASE}")

if(EXISTS ${PROJECT_SOURCE_DIR}/.git)
    if(NOT EXISTS ${PROJECT_SOURCE_DIR}/.git/shallow)
        find_package(Git)

        if(GIT_FOUND)
            include(GetGitRevisionDescription)
            get_git_tag_revision(TAGHASH --tags --max-count=1)
            get_git_head_revision(REFSPEC COMMITHASH)
            git_describe(VERSION --tags ${TAGHASH})

            string(REGEX MATCH "^openmw-[^0-9]*[0-9]+\\.[0-9]+\\.[0-9]+.*" MATCH "${VERSION}")
            if(MATCH)
                string(REGEX REPLACE "^openmw-([0-9]+)\\..*" "\\1" GIT_VERSION_MAJOR "${VERSION}")
                string(REGEX REPLACE "^openmw-[0-9]+\\.([0-9]+).*" "\\1" GIT_VERSION_MINOR "${VERSION}")
                string(REGEX REPLACE "^openmw-[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" GIT_VERSION_RELEASE "${VERSION}")

                set(OPENMW_VERSION_COMMITHASH "${COMMITHASH}")
                set(OPENMW_VERSION_TAGHASH "${TAGHASH}")

                message(STATUS "OpenMW version ${OPENMW_VERSION}")
            else(MATCH)
                message(WARNING "Failed to get valid version information from Git")
            endif(MATCH)
        else(GIT_FOUND)
            message(WARNING "Git executable not found")
        endif(GIT_FOUND)
    else(NOT EXISTS ${PROJECT_SOURCE_DIR}/.git/shallow)
        message(STATUS "Shallow Git clone detected, not attempting to retrieve version info")
    endif(NOT EXISTS ${PROJECT_SOURCE_DIR}/.git/shallow)
endif(EXISTS ${PROJECT_SOURCE_DIR}/.git)

# Macros
include(OpenMWMacros)

if (ANDROID)
    set(CMAKE_FIND_ROOT_PATH ${OPENMW_DEPENDENCIES_DIR} "${CMAKE_FIND_ROOT_PATH}")
endif (ANDROID)

# doxygen main page

configure_file ("${OpenMW_SOURCE_DIR}/docs/mainpage.hpp.cmake" "${OpenMW_SOURCE_DIR}/docs/mainpage.hpp")

option(MYGUI_STATIC "Link static build of Mygui into the binaries" FALSE)
option(OGRE_STATIC  "Link static build of Ogre and Ogre Plugins into the binaries" FALSE)
option(BOOST_STATIC "Link static build of Boost into the binaries" FALSE)
option(SDL2_STATIC "Link static build of SDL into the binaries" FALSE)

option(OPENMW_UNITY_BUILD "Use fewer compilation units to speed up compile time" FALSE)

# Apps and tools
option(BUILD_BSATOOL "build BSA extractor" ON)
option(BUILD_ESMTOOL "build ESM inspector" ON)
option(BUILD_LAUNCHER "build Launcher" ON)
option(BUILD_MWINIIMPORTER "build MWiniImporter" ON)
option(BUILD_OPENCS "build OpenMW Construction Set" ON)
option(BUILD_WITH_CODE_COVERAGE "Enable code coverage with gconv" OFF)
option(BUILD_UNITTESTS "Enable Unittests with Google C++ Unittest and GMock frameworks" OFF)
option(BUILD_NIFTEST "build nif file tester" OFF)
option(BUILD_MYGUI_PLUGIN "build MyGUI plugin for OpenMW resources, to use with MyGUI tools" ON)

# OS X deployment
option(OPENMW_OSX_DEPLOYMENT OFF)

# Location of morrowind data files
if (APPLE)
    set(MORROWIND_DATA_FILES "./data" CACHE PATH "location of Morrowind data files")
    set(OPENMW_RESOURCE_FILES "./resources" CACHE PATH "location of OpenMW resources files")
elseif(UNIX)
    set(MORROWIND_DATA_FILES "${CMAKE_INSTALL_PREFIX}/share/games/openmw/data/" CACHE PATH "location of Morrowind data files")
    set(OPENMW_RESOURCE_FILES "${CMAKE_INSTALL_PREFIX}/share/games/openmw/resources/" CACHE PATH "location of OpenMW resources files")
else()
    set(MORROWIND_DATA_FILES "data" CACHE PATH "location of Morrowind data files")
    set(OPENMW_RESOURCE_FILES "resources" CACHE PATH "location of OpenMW resources files")
endif(APPLE)

if (WIN32)
    option(USE_DEBUG_CONSOLE "whether a debug console should be enabled for debug builds, if false debug output is redirected to Visual Studio output" ON)
endif()

# We probably support older versions than this.
cmake_minimum_required(VERSION 2.6)

# source directory: libs

set(LIBS_DIR ${CMAKE_SOURCE_DIR}/libs)

set(OENGINE_OGRE
  ${LIBS_DIR}/openengine/ogre/renderer.cpp
  ${LIBS_DIR}/openengine/ogre/lights.cpp
  ${LIBS_DIR}/openengine/ogre/selectionbuffer.cpp
  ${LIBS_DIR}/openengine/ogre/imagerotate.cpp
)

set(OENGINE_GUI
  ${LIBS_DIR}/openengine/gui/loglistener.cpp
  ${LIBS_DIR}/openengine/gui/manager.cpp
  ${LIBS_DIR}/openengine/gui/layout.hpp
)

set(OENGINE_BULLET
    ${LIBS_DIR}/openengine/bullet/BtOgre.cpp
    ${LIBS_DIR}/openengine/bullet/BtOgreExtras.h
    ${LIBS_DIR}/openengine/bullet/BtOgreGP.h
    ${LIBS_DIR}/openengine/bullet/BtOgrePG.h
    ${LIBS_DIR}/openengine/bullet/physic.cpp
    ${LIBS_DIR}/openengine/bullet/physic.hpp
    ${LIBS_DIR}/openengine/bullet/BulletShapeLoader.cpp
    ${LIBS_DIR}/openengine/bullet/BulletShapeLoader.h
    ${LIBS_DIR}/openengine/bullet/trace.cpp
    ${LIBS_DIR}/openengine/bullet/trace.h

)

set(OENGINE_ALL ${OENGINE_OGRE} ${OENGINE_GUI} ${OENGINE_BULLET})
source_group(libs\\openengine FILES ${OENGINE_ALL})

set(OPENMW_LIBS ${OENGINE_ALL})
set(OPENMW_LIBS_HEADER)

# Sound setup
set(FFmpeg_FIND_COMPONENTS AVCODEC AVFORMAT AVUTIL SWSCALE SWRESAMPLE AVRESAMPLE)
unset(FFMPEG_LIBRARIES CACHE)
find_package(FFmpeg)
if ( NOT AVCODEC_FOUND OR NOT AVFORMAT_FOUND OR NOT AVUTIL_FOUND OR NOT SWSCALE_FOUND )
    message(FATAL_ERROR "FFmpeg component required, but not found!")
endif()
set(SOUND_INPUT_INCLUDES ${FFMPEG_INCLUDE_DIRS})
set(SOUND_INPUT_LIBRARY ${FFMPEG_LIBRARIES} ${SWSCALE_LIBRARIES})
if( SWRESAMPLE_FOUND )
    add_definitions(-DHAVE_LIBSWRESAMPLE)
    set(SOUND_INPUT_LIBRARY ${FFMPEG_LIBRARIES} ${SWRESAMPLE_LIBRARIES})
else()
    if( AVRESAMPLE_FOUND )
        set(SOUND_INPUT_LIBRARY ${FFMPEG_LIBRARIES} ${AVRESAMPLE_LIBRARIES})
    else()
        message(FATAL_ERROR "Install either libswresample (FFmpeg) or libavresample (Libav).")
    endif()
endif()

# TinyXML
option(USE_SYSTEM_TINYXML "Use system TinyXML library instead of internal." OFF)
if(USE_SYSTEM_TINYXML)
    find_library(TINYXML_LIBRARIES tinyxml)
    find_path(TINYXML_INCLUDE_DIR tinyxml.h)
    message(STATUS "Found TinyXML: ${TINYXML_LIBRARIES} ${TINYXML_INCLUDE_DIR}")
    add_definitions (-DTIXML_USE_STL)
    if(TINYXML_LIBRARIES AND TINYXML_INCLUDE_DIR)
        include_directories(${TINYXML_INCLUDE_DIR})
        message(STATUS "Using system TinyXML library.")
    else()
        message(FATAL_ERROR "Detection of system TinyXML incomplete.")
    endif()
endif()

# Platform specific
if (WIN32)
    set(Boost_USE_STATIC_LIBS   ON)
    set(PLATFORM_INCLUDE_DIR "platform")
    add_definitions(-DBOOST_ALL_NO_LIB)

    # Suppress WinMain(), provided by SDL
    add_definitions(-DSDL_MAIN_HANDLED)
else (WIN32)
    set(PLATFORM_INCLUDE_DIR "")
endif (WIN32)
if (MSVC10)
    set(PLATFORM_INCLUDE_DIR "")
endif()

# Dependencies

# Fix for not visible pthreads functions for linker with glibc 2.15
if (UNIX AND NOT APPLE)
    find_package (Threads)
endif()

# Look for stdint.h
include(CheckIncludeFile)
check_include_file(stdint.h HAVE_STDINT_H)
if(NOT HAVE_STDINT_H)
    unset(HAVE_STDINT_H CACHE)
    message(FATAL_ERROR "stdint.h was not found" )
endif()

include (CheckIncludeFileCXX)
check_include_file_cxx(unordered_map HAVE_UNORDERED_MAP)
if (HAVE_UNORDERED_MAP)
    add_definitions(-DHAVE_UNORDERED_MAP)
endif ()


set(BOOST_COMPONENTS system filesystem program_options)
if(WIN32)
    set(BOOST_COMPONENTS ${BOOST_COMPONENTS} locale)
endif(WIN32)

IF(BOOST_STATIC)
    set(Boost_USE_STATIC_LIBS   ON)
endif()

find_package(OGRE REQUIRED)
if (${OGRE_VERSION} VERSION_LESS "1.9")
    message(FATAL_ERROR "OpenMW requires Ogre 1.9 or later, please install the latest stable version from http://ogre3d.org")
endif()

find_package(MyGUI REQUIRED)
if (${MYGUI_VERSION} VERSION_LESS "3.2.1")
    message(FATAL_ERROR "OpenMW requires MyGUI 3.2.1 or later, please install the latest version from http://mygui.info")
endif()

find_package(Boost REQUIRED COMPONENTS ${BOOST_COMPONENTS})
find_package(SDL2 REQUIRED)
find_package(OpenAL REQUIRED)
find_package(Bullet REQUIRED)

set(OGRE_PLUGIN_INCLUDE_DIRS "")
set(OGRE_STATIC_PLUGINS "")

macro(add_static_ogre_plugin PLUGIN)
    if(OGRE_${PLUGIN}_FOUND)
        # strip RenderSystem_ or Plugin_ prefix from plugin name
        string(REPLACE "RenderSystem_" "" PLUGIN_TEMP ${PLUGIN})
        string(REPLACE "Plugin_" "" PLUGIN_NAME ${PLUGIN_TEMP})
        add_definitions(-DENABLE_PLUGIN_${PLUGIN_NAME})

        list(APPEND OGRE_PLUGIN_INCLUDE_DIRS ${OGRE_${PLUGIN}_INCLUDE_DIRS})
        list(APPEND OGRE_STATIC_PLUGINS ${OGRE_${PLUGIN}_LIBRARIES})
    endif(OGRE_${PLUGIN}_FOUND)
endmacro(add_static_ogre_plugin)

if(OGRE_STATIC)
    # set up OGRE_PLUGIN_INCLUDE_DIRS and OGRE_STATIC_PLUGINS
    add_static_ogre_plugin(Plugin_OctreeSceneManager)
    add_static_ogre_plugin(Plugin_ParticleFX)
    find_package(Cg)
    if(Cg_FOUND)
        add_static_ogre_plugin(Plugin_CgProgramManager)
        list(APPEND OGRE_STATIC_PLUGINS ${Cg_LIBRARIES})
    endif(Cg_FOUND)

if (ANDROID)
    add_static_ogre_plugin(RenderSystem_GLES2)
else ()
    add_static_ogre_plugin(RenderSystem_GL)
endif ()

    if(WIN32)
        add_static_ogre_plugin(RenderSystem_Direct3D9)
    endif(WIN32)
endif(OGRE_STATIC)

include_directories("."
    ${OGRE_INCLUDE_DIR} ${OGRE_INCLUDE_DIR}/Ogre ${OGRE_INCLUDE_DIR}/OGRE ${OGRE_INCLUDE_DIRS} ${OGRE_PLUGIN_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIR}
    ${Boost_INCLUDE_DIR}
    ${PLATFORM_INCLUDE_DIR}
    ${MYGUI_INCLUDE_DIRS}
    ${MYGUI_PLATFORM_INCLUDE_DIRS}
    ${OPENAL_INCLUDE_DIR}
    ${LIBS_DIR}
)

link_directories(${SDL2_LIBRARY_DIRS} ${Boost_LIBRARY_DIRS} ${OGRE_LIB_DIR} ${MYGUI_LIB_DIR})

if(MYGUI_STATIC)
   add_definitions(-DMYGUI_STATIC)
endif (MYGUI_STATIC)

if (APPLE)
    # List used Ogre plugins
    SET(USED_OGRE_PLUGINS ${OGRE_RenderSystem_GL_LIBRARY_REL}
                          ${OGRE_Plugin_ParticleFX_LIBRARY_REL})

    # Actually we must use OGRE_Plugin_CgProgramManager_FOUND but it's
    # not reliable and equals TRUE even if there's no Ogre Cg plugin
    if (Cg_FOUND)
        set(USED_OGRE_PLUGINS ${USED_OGRE_PLUGINS}
                              ${OGRE_Plugin_CgProgramManager_LIBRARY_REL})
    endif ()

    if (${OGRE_PLUGIN_DIR_REL}})
        set(OGRE_PLUGINS_REL_FOUND TRUE)
    endif ()

    if (${OGRE_PLUGIN_DIR_DBG})
        set(OGRE_PLUGINS_DBG_FOUND TRUE)
    endif ()

    if (${OGRE_PLUGINS_REL_FOUND})
        set(OGRE_PLUGIN_DIR ${OGRE_PLUGIN_DIR_REL})
    else ()
        set(OGRE_PLUGIN_DIR ${OGRE_PLUGIN_DIR_DBG})
    endif ()

    configure_file(${OpenMW_SOURCE_DIR}/files/mac/Info.plist
        "${APP_BUNDLE_DIR}/Contents/Info.plist")

    configure_file(${OpenMW_SOURCE_DIR}/files/mac/openmw.icns
        "${APP_BUNDLE_DIR}/Contents/Resources/OpenMW.icns" COPYONLY)
endif (APPLE)

# Set up DEBUG define
set_directory_properties(PROPERTIES COMPILE_DEFINITIONS_DEBUG DEBUG=1)

# Set up Ogre plugin folder & debug suffix
if (APPLE)
    # Ogre on OS X doesn't use "_d" suffix (see Ogre's CMakeLists.txt)
    add_definitions(-DOGRE_PLUGIN_DEBUG_SUFFIX="")
else ()
    add_definitions(-DOGRE_PLUGIN_DEBUG_SUFFIX="_d")
endif()

if (APPLE AND OPENMW_OSX_DEPLOYMENT)
    # make it empty so plugin loading code can check this and try to find plugins inside app bundle
    add_definitions(-DOGRE_PLUGIN_DIR="")
else()
    if (NOT DEFINED ${OGRE_PLUGIN_DIR})
        set(OGRE_PLUGIN_DIR ${OGRE_PLUGIN_DIR_REL})
    endif()

    add_definitions(-DOGRE_PLUGIN_DIR="${OGRE_PLUGIN_DIR}")
endif()


add_subdirectory(files/)
add_subdirectory(files/mygui)

# Specify build paths

if (APPLE)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${APP_BUNDLE_DIR}/Contents/MacOS")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${APP_BUNDLE_DIR}/Contents/MacOS")
else (APPLE)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${OpenMW_BINARY_DIR}")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${OpenMW_BINARY_DIR}")
endif (APPLE)

# Other files

configure_file(${OpenMW_SOURCE_DIR}/files/settings-default.cfg
    "${OpenMW_BINARY_DIR}/settings-default.cfg")

configure_file(${OpenMW_SOURCE_DIR}/files/transparency-overrides.cfg
    "${OpenMW_BINARY_DIR}/transparency-overrides.cfg")

configure_file(${OpenMW_SOURCE_DIR}/files/openmw.cfg.local
    "${OpenMW_BINARY_DIR}/openmw.cfg")

configure_file(${OpenMW_SOURCE_DIR}/files/openmw.cfg
    "${OpenMW_BINARY_DIR}/openmw.cfg.install")

configure_file(${OpenMW_SOURCE_DIR}/files/opencs.ini
    "${OpenMW_BINARY_DIR}/opencs.ini")

configure_file(${OpenMW_SOURCE_DIR}/files/opencs/defaultfilters
    "${OpenMW_BINARY_DIR}/resources/defaultfilters" COPYONLY)

if (NOT WIN32 AND NOT APPLE)
    configure_file(${OpenMW_SOURCE_DIR}/files/openmw.desktop
        "${OpenMW_BINARY_DIR}/openmw.desktop")
    configure_file(${OpenMW_SOURCE_DIR}/files/opencs.desktop
        "${OpenMW_BINARY_DIR}/opencs.desktop")
endif()

# Compiler settings
if (CMAKE_COMPILER_IS_GNUCC)
    SET(CMAKE_CXX_FLAGS "-Wall -Wextra -Wno-unused-parameter -Wno-reorder -std=c++98 -pedantic -Wno-long-long ${CMAKE_CXX_FLAGS}")

    execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpversion
                OUTPUT_VARIABLE GCC_VERSION)
    if ("${GCC_VERSION}" VERSION_GREATER 4.6 OR "${GCC_VERSION}" VERSION_EQUAL 4.6)
        SET(CMAKE_CXX_FLAGS "-Wno-unused-but-set-parameter ${CMAKE_CXX_FLAGS}")
    endif("${GCC_VERSION}" VERSION_GREATER 4.6 OR "${GCC_VERSION}" VERSION_EQUAL 4.6)
endif (CMAKE_COMPILER_IS_GNUCC)

IF(NOT WIN32 AND NOT APPLE)
    # Linux building
    # Paths
    SET(BINDIR "${CMAKE_INSTALL_PREFIX}/bin" CACHE PATH "Where to install binaries")
    SET(LIBDIR "${CMAKE_INSTALL_PREFIX}/lib" CACHE PATH "Where to install libraries")
    SET(DATAROOTDIR "${CMAKE_INSTALL_PREFIX}/share" CACHE PATH "Sets the root of data directories to a non-default location")
    SET(GLOBAL_DATA_PATH "${DATAROOTDIR}/games/" CACHE PATH "Set data path prefix")
    SET(DATADIR "${GLOBAL_DATA_PATH}/openmw" CACHE PATH "Sets the openmw data directories to a non-default location")
    SET(ICONDIR "${DATAROOTDIR}/pixmaps" CACHE PATH "Set icon dir")
    SET(LICDIR "${DATAROOTDIR}/licenses/openmw" CACHE PATH "Sets the openmw license directory to a non-default location.")
    IF("${CMAKE_INSTALL_PREFIX}" STREQUAL "/usr")
        SET(GLOBAL_CONFIG_PATH "/etc/" CACHE PATH "Set config dir prefix")
    ELSE()
        SET(GLOBAL_CONFIG_PATH "${CMAKE_INSTALL_PREFIX}/etc/" CACHE PATH "Set config dir prefix")
    ENDIF()
    SET(SYSCONFDIR "${GLOBAL_CONFIG_PATH}/openmw" CACHE PATH "Set config dir")

    # Install binaries
    INSTALL(PROGRAMS "${OpenMW_BINARY_DIR}/openmw" DESTINATION "${BINDIR}" )
    IF(BUILD_LAUNCHER)
        INSTALL(PROGRAMS "${OpenMW_BINARY_DIR}/omwlauncher" DESTINATION "${BINDIR}" )
    ENDIF(BUILD_LAUNCHER)
    IF(BUILD_BSATOOL)
        INSTALL(PROGRAMS "${OpenMW_BINARY_DIR}/bsatool" DESTINATION "${BINDIR}" )
    ENDIF(BUILD_BSATOOL)
    IF(BUILD_ESMTOOL)
        INSTALL(PROGRAMS "${OpenMW_BINARY_DIR}/esmtool" DESTINATION "${BINDIR}" )
    ENDIF(BUILD_ESMTOOL)
    IF(BUILD_MWINIIMPORTER)
        INSTALL(PROGRAMS "${OpenMW_BINARY_DIR}/mwiniimport" DESTINATION "${BINDIR}" )
    ENDIF(BUILD_MWINIIMPORTER)
    IF(BUILD_OPENCS)
        INSTALL(PROGRAMS "${OpenMW_BINARY_DIR}/opencs" DESTINATION "${BINDIR}" )
    ENDIF(BUILD_OPENCS)
    IF(BUILD_NIFTEST)
        INSTALL(PROGRAMS "${OpenMW_BINARY_DIR}/niftest" DESTINATION "${BINDIR}" )
    ENDIF(BUILD_NIFTEST)
    if(BUILD_MYGUI_PLUGIN)
        INSTALL(PROGRAMS "${OpenMW_BINARY_DIR}/Plugin_MyGUI_OpenMW_Resources.so" DESTINATION "${LIBDIR}" )
    ENDIF(BUILD_MYGUI_PLUGIN)

    # Install licenses
    INSTALL(FILES "docs/license/DejaVu Font License.txt" DESTINATION "${LICDIR}" )
    INSTALL(FILES "extern/shiny/License.txt" DESTINATION "${LICDIR}" RENAME "Shiny License.txt" )

    # Install icon and desktop file
    INSTALL(FILES "${OpenMW_BINARY_DIR}/openmw.desktop" DESTINATION "${DATAROOTDIR}/applications" PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ COMPONENT "openmw")
    INSTALL(FILES "${OpenMW_SOURCE_DIR}/files/launcher/images/openmw.png" DESTINATION "${ICONDIR}" PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ COMPONENT "openmw")
    IF(BUILD_OPENCS)
        INSTALL(FILES "${OpenMW_BINARY_DIR}/opencs.desktop" DESTINATION "${DATAROOTDIR}/applications" PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ COMPONENT "opencs")
        INSTALL(FILES "${OpenMW_SOURCE_DIR}/files/opencs/opencs.png" DESTINATION "${ICONDIR}" PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ COMPONENT "opencs")
    ENDIF(BUILD_OPENCS)

    # Install global configuration files
    INSTALL(FILES "${OpenMW_BINARY_DIR}/settings-default.cfg" DESTINATION "${SYSCONFDIR}" PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ COMPONENT "openmw")
    INSTALL(FILES "${OpenMW_BINARY_DIR}/transparency-overrides.cfg" DESTINATION "${SYSCONFDIR}" PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ COMPONENT "openmw")
    INSTALL(FILES "${OpenMW_BINARY_DIR}/openmw.cfg.install" DESTINATION "${SYSCONFDIR}" RENAME "openmw.cfg" PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ COMPONENT "openmw")
    IF(BUILD_OPENCS)
        INSTALL(FILES "${OpenMW_BINARY_DIR}/opencs.ini" DESTINATION "${SYSCONFDIR}" PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ COMPONENT "opencs")
    ENDIF(BUILD_OPENCS)

    # Install resources
    INSTALL(DIRECTORY "${OpenMW_BINARY_DIR}/resources" DESTINATION "${DATADIR}" FILE_PERMISSIONS OWNER_READ GROUP_READ WORLD_READ COMPONENT "Resources")
    INSTALL(DIRECTORY DESTINATION "${DATADIR}/data" COMPONENT "Resources")
ENDIF(NOT WIN32 AND NOT APPLE)

if(WIN32)
    FILE(GLOB dll_files "${OpenMW_BINARY_DIR}/Release/*.dll")
    INSTALL(FILES ${dll_files} DESTINATION ".")
    INSTALL(FILES "${OpenMW_BINARY_DIR}/openmw.cfg.install" DESTINATION "." RENAME "openmw.cfg")
    INSTALL(FILES
        "${OpenMW_SOURCE_DIR}/readme.txt"
        "${OpenMW_SOURCE_DIR}/Docs/license/GPL3.txt"
        "${OpenMW_SOURCE_DIR}/Docs/license/DejaVu Font License.txt"
        "${OpenMW_BINARY_DIR}/settings-default.cfg"
        "${OpenMW_BINARY_DIR}/transparency-overrides.cfg"
        "${OpenMW_BINARY_DIR}/Release/openmw.exe"
        DESTINATION ".")

    IF(BUILD_LAUNCHER)
        INSTALL(PROGRAMS "${OpenMW_BINARY_DIR}/Release/omwlauncher.exe" DESTINATION ".")
    ENDIF(BUILD_LAUNCHER)
    IF(BUILD_MWINIIMPORTER)
        INSTALL(PROGRAMS "${OpenMW_BINARY_DIR}/Release/mwiniimport.exe" DESTINATION ".")
    ENDIF(BUILD_MWINIIMPORTER)
    IF(BUILD_OPENCS)
        INSTALL(PROGRAMS "${OpenMW_BINARY_DIR}/Release/opencs.exe" DESTINATION ".")
        INSTALL(FILES "${OpenMW_BINARY_DIR}/opencs.ini" DESTINATION ".")
    ENDIF(BUILD_OPENCS)
    if(BUILD_MYGUI_PLUGIN)
        INSTALL(PROGRAMS "${OpenMW_BINARY_DIR}/Release/Plugin_MyGUI_OpenMW_Resources.dll" DESTINATION ".")
    ENDIF(BUILD_MYGUI_PLUGIN)

    INSTALL(DIRECTORY "${OpenMW_BINARY_DIR}/resources" DESTINATION ".")

    SET(CPACK_GENERATOR "NSIS")
    SET(CPACK_PACKAGE_NAME "OpenMW")
    SET(CPACK_PACKAGE_VENDOR "OpenMW.org")
    SET(CPACK_PACKAGE_VERSION ${OPENMW_VERSION})
    SET(CPACK_PACKAGE_VERSION_MAJOR ${OPENMW_VERSION_MAJOR})
    SET(CPACK_PACKAGE_VERSION_MINOR ${OPENMW_VERSION_MINOR})
    SET(CPACK_PACKAGE_VERSION_PATCH ${OPENMW_VERSION_RELEASE})
    SET(CPACK_PACKAGE_EXECUTABLES "openmw;OpenMW")
    IF(BUILD_LAUNCHER)
        SET(CPACK_PACKAGE_EXECUTABLES "${CPACK_PACKAGE_EXECUTABLES};omwlauncher;OpenMW Launcher")
    ENDIF(BUILD_LAUNCHER)
    IF(BUILD_OPENCS)
        SET(CPACK_PACKAGE_EXECUTABLES "${CPACK_PACKAGE_EXECUTABLES};opencs;OpenMW Construction Set")
    ENDIF(BUILD_OPENCS)
    SET(CPACK_NSIS_CREATE_ICONS_EXTRA "CreateShortCut '\$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Readme.lnk' '\$INSTDIR\\\\readme.txt'")
    SET(CPACK_NSIS_DELETE_ICONS_EXTRA "
        !insertmacro MUI_STARTMENU_GETFOLDER Application $MUI_TEMP
        Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Readme.lnk\\\"
        ")
    SET(CPACK_RESOURCE_FILE_README "${OpenMW_SOURCE_DIR}/readme.txt")
    SET(CPACK_PACKAGE_DESCRIPTION_FILE "${OpenMW_SOURCE_DIR}/readme.txt")
    SET(CPACK_NSIS_EXECUTABLES_DIRECTORY ".")
    SET(CPACK_NSIS_DISPLAY_NAME "OpenMW ${OPENMW_VERSION}")
    SET(CPACK_NSIS_HELP_LINK "http:\\\\\\\\www.openmw.org")
    SET(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\www.openmw.org")
    SET(CPACK_NSIS_INSTALLED_ICON_NAME "omwlauncher.exe")
    SET(CPACK_NSIS_MUI_ICON "${OpenMW_SOURCE_DIR}/files/windows/openmw.ico")
    SET(CPACK_NSIS_MUI_UNIICON "${OpenMW_SOURCE_DIR}/files/windows/openmw.ico")
    SET(CPACK_PACKAGE_ICON "${OpenMW_SOURCE_DIR}\\\\files\\\\openmw.bmp")

    SET(VCREDIST32 "${OpenMW_BINARY_DIR}/vcredist_x86.exe")
    if(EXISTS ${VCREDIST32})
        INSTALL(FILES ${VCREDIST32} DESTINATION "redist")
        SET(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "ExecWait '\\\"$INSTDIR\\\\redist\\\\vcredist_x86.exe\\\" /q'" )
    endif(EXISTS ${VCREDIST32})

    SET(VCREDIST64 "${OpenMW_BINARY_DIR}/vcredist_x64.exe")
    if(EXISTS ${VCREDIST64})
        INSTALL(FILES ${VCREDIST64} DESTINATION "redist")
        SET(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "ExecWait '\\\"$INSTDIR\\\\redist\\\\vcredist_x64.exe\\\" /q'" )
    endif(EXISTS ${VCREDIST64})

    SET(OALREDIST "${OpenMW_BINARY_DIR}/oalinst.exe")
    if(EXISTS ${OALREDIST})
        INSTALL(FILES ${OALREDIST} DESTINATION "redist")
        SET(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "${CPACK_NSIS_EXTRA_INSTALL_COMMANDS}
            ExecWait '\\\"$INSTDIR\\\\redist\\\\oalinst.exe\\\" /s'" )
    endif(EXISTS ${OALREDIST})

    if(CMAKE_CL_64)
        SET(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64")
    endif()

    include(CPack)
endif(WIN32)

# Extern
add_subdirectory (extern/shiny)
add_subdirectory (extern/oics)
add_subdirectory (extern/sdl4ogre)

# Components
add_subdirectory (components)

# Plugins
if (BUILD_MYGUI_PLUGIN)
    add_subdirectory(plugins/mygui_resource_plugin)
endif()

#Testing
if (BUILD_NIFTEST)
  add_subdirectory(components/nif/tests/)
endif(BUILD_NIFTEST)

# Apps and tools
add_subdirectory( apps/openmw )

if (BUILD_BSATOOL)
  add_subdirectory( apps/bsatool )
endif()

if (BUILD_ESMTOOL)
  add_subdirectory( apps/esmtool )
endif()

if (BUILD_LAUNCHER)
    if(NOT WIN32)
        find_package(LIBUNSHIELD REQUIRED)
        if(NOT LIBUNSHIELD_FOUND)
               message(SEND_ERROR "Failed to find libunshield")
        endif(NOT LIBUNSHIELD_FOUND)
    endif(NOT WIN32)

   add_subdirectory( apps/launcher )
endif()

if (BUILD_MWINIIMPORTER)
   add_subdirectory( apps/mwiniimporter )
endif()

if (BUILD_OPENCS)
   add_subdirectory (apps/opencs)
endif()

# UnitTests
if (BUILD_UNITTESTS)
  add_subdirectory( apps/openmw_test_suite )
endif()

if (WIN32)
  if (MSVC)
    if (MULTITHREADED_BUILD)
        set( MT_BUILD "/MP")
    endif (MULTITHREADED_BUILD)

    foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
        string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
        set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} "$(SolutionDir)$(Configuration)" )
        set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} "$(ProjectDir)$(Configuration)" )
    endforeach( OUTPUTCONFIG )

    if (USE_DEBUG_CONSOLE)
      set_target_properties(openmw PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
      set_target_properties(openmw PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:CONSOLE")
      set_target_properties(openmw PROPERTIES COMPILE_DEFINITIONS_DEBUG "_CONSOLE")
    else()
      # Turn off debug console, debug output will be written to visual studio output instead
      set_target_properties(openmw PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:WINDOWS")
      set_target_properties(openmw PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:WINDOWS")
    endif()

    # Release builds use the debug console
    set_target_properties(openmw PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:CONSOLE")
    set_target_properties(openmw PROPERTIES COMPILE_DEFINITIONS_RELEASE "_CONSOLE")
    set_target_properties(openmw PROPERTIES LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:CONSOLE")

    # Play a bit with the warning levels

    set(WARNINGS "/Wall") # Since windows can only disable specific warnings, not enable them

    set(WARNINGS_DISABLE
        # Warnings that aren't enabled normally and don't need to be enabled
        # They're unneeded and sometimes completely retarded warnings that /Wall enables
        # Not going to bother commenting them as they tend to warn on every standard library files
        4061 4263 4264 4266 4350 4371 4514 4548 4571 4610 4619 4623 4625 4626 4628 4640 4668 4710 4711 4820 4826 4917 4946

        # Warnings that are thrown on standard libraries and not OpenMW
        4347 # Non-template function with same name and parameter count as template function
        4365 # Variable signed/unsigned mismatch
        4510 4512 # Unable to generate copy constructor/assignment operator as it's not public in the base
        4706 # Assignment in conditional expression
        4738 # Storing 32-bit float result in memory, possible loss of performance
        4986 # Undocumented warning that occurs in the crtdbg.h file
        4987 # nonstandard extension used (triggered by setjmp.h)
        4996 # Function was declared deprecated

        # cause by ogre extensivly
        4193 # #pragma warning(pop) : no matching '#pragma warning(push)'
        4251 # class 'XXXX' needs to have dll-interface to be used by clients of class 'YYYY'
        4275 # non dll-interface struct 'XXXX' used as base for dll-interface class 'YYYY'

        # caused by boost
        4191 # 'type cast' : unsafe conversion (1.56, thread_primitives.hpp, normally off)

        # OpenMW specific warnings
        4099 # Type mismatch, declared class or struct is defined with other type
        4100 # Unreferenced formal parameter (-Wunused-parameter)
        4127 # Conditional expression is constant
        4242 # Storing value in a variable of a smaller type, possible loss of data
        4244 # Storing value of one type in variable of another (size_t in int, for example)
        4267 # Conversion from 'size_t' to 'int', possible loss of data
        4305 # Truncating value (double to float, for example)
        4309 # Variable overflow, trying to store 128 in a signed char for example
        4351 # New behavior: elements of array 'array' will be default initialized (desired behavior)
        4355 # Using 'this' in member initialization list
        4505 # Unreferenced local function has been removed
        4701 # Potentially uninitialized local variable used
        4702 # Unreachable code
        4800 # Boolean optimization warning, e.g. myBool = (myInt != 0) instead of myBool = myInt
        )

    foreach(d ${WARNINGS_DISABLE})
        set(WARNINGS "${WARNINGS} /wd${d}")
    endforeach(d)

    # boost::wave has a few issues with signed / unsigned conversions, so we suppress those here
    set(SHINY_WARNINGS "${WARNINGS} /wd4245")
    set_target_properties(shiny PROPERTIES COMPILE_FLAGS "${SHINY_WARNINGS} ${MT_BUILD}")
    # there's an unreferenced local variable in the ogre platform, suppress it
    set(SHINY_OGRE_WARNINGS "${WARNINGS} /wd4101")
    set_target_properties(shiny.OgrePlatform PROPERTIES COMPILE_FLAGS "${SHINY_OGRE_WARNINGS} ${MT_BUILD}")
    set_target_properties(sdl4ogre PROPERTIES COMPILE_FLAGS "${WARNINGS} ${MT_BUILD}")
    # oics uses tinyxml, which has an initialized but unused variable
    set(OICS_WARNINGS "${WARNINGS} /wd4189")
    set_target_properties(oics PROPERTIES COMPILE_FLAGS "${OICS_WARNINGS} ${MT_BUILD}")
    set_target_properties(components PROPERTIES COMPILE_FLAGS "${WARNINGS} ${MT_BUILD}")
    if (BUILD_LAUNCHER)
        set_target_properties(omwlauncher PROPERTIES COMPILE_FLAGS "${WARNINGS} ${MT_BUILD}")
    endif (BUILD_LAUNCHER)
    set_target_properties(openmw PROPERTIES COMPILE_FLAGS "${WARNINGS} ${MT_BUILD}")
    if (BUILD_BSATOOL)
        set_target_properties(bsatool PROPERTIES COMPILE_FLAGS "${WARNINGS} ${MT_BUILD}")
    endif (BUILD_BSATOOL)
    if (BUILD_ESMTOOL)
        set_target_properties(esmtool PROPERTIES COMPILE_FLAGS "${WARNINGS} ${MT_BUILD}")
    endif (BUILD_ESMTOOL)
    if (BUILD_OPENCS)
        # QT triggers an informational warning that the object layout may differ when compiled with /vd2
        set(OPENCS_WARNINGS "${WARNINGS} ${MT_BUILD} /wd4435")
        set_target_properties(opencs PROPERTIES COMPILE_FLAGS ${OPENCS_WARNINGS})
    endif (BUILD_OPENCS)
    if (BUILD_MWINIIMPORTER)
        set_target_properties(mwiniimport PROPERTIES COMPILE_FLAGS "${WARNINGS} ${MT_BUILD}")
    endif (BUILD_MWINIIMPORTER)
  endif(MSVC)

  # Same for MinGW
  if (MINGW)
    if (USE_DEBUG_CONSOLE)
      set_target_properties(openmw PROPERTIES LINK_FLAGS_DEBUG "-Wl,-subsystem,console")
      set_target_properties(openmw PROPERTIES LINK_FLAGS_RELWITHDEBINFO "-Wl,-subsystem,console")
      set_target_properties(openmw PROPERTIES COMPILE_DEFINITIONS_DEBUG "_CONSOLE")
    else(USE_DEBUG_CONSOLE)
      set_target_properties(openmw PROPERTIES LINK_FLAGS_DEBUG "-Wl,-subsystem,windows")
      set_target_properties(openmw PROPERTIES LINK_FLAGS_RELWITHDEBINFO "-Wl,-subsystem,windows")
    endif(USE_DEBUG_CONSOLE)

    set_target_properties(openmw PROPERTIES LINK_FLAGS_RELEASE "-Wl,-subsystem,console")
    set_target_properties(openmw PROPERTIES LINK_FLAGS_MINSIZEREL "-Wl,-subsystem,console")
    set_target_properties(openmw PROPERTIES COMPILE_DEFINITIONS_RELEASE "_CONSOLE")
  endif(MINGW)

  # TODO: At some point release builds should not use the console but rather write to a log file
  #set_target_properties(openmw PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
  #set_target_properties(openmw PROPERTIES LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:WINDOWS")
endif()

# Apple bundling
if (APPLE)
    set(INSTALL_SUBDIR OpenMW)

    install(DIRECTORY "${APP_BUNDLE_DIR}" USE_SOURCE_PERMISSIONS DESTINATION "${INSTALL_SUBDIR}" COMPONENT Runtime)
    install(DIRECTORY "${OpenMW_BINARY_DIR}/resources" DESTINATION "${INSTALL_SUBDIR}" COMPONENT Runtime)
    install(FILES "${OpenMW_BINARY_DIR}/openmw.cfg.install" RENAME "openmw.cfg" DESTINATION "${INSTALL_SUBDIR}" COMPONENT Runtime)
    install(FILES "${OpenMW_BINARY_DIR}/settings-default.cfg" DESTINATION "${INSTALL_SUBDIR}" COMPONENT Runtime)
    install(FILES "${OpenMW_BINARY_DIR}/transparency-overrides.cfg" DESTINATION "${INSTALL_SUBDIR}" COMPONENT Runtime)
    install(FILES "${OpenMW_BINARY_DIR}/opencs.ini" DESTINATION "${INSTALL_SUBDIR}" COMPONENT Runtime)

    set(CPACK_GENERATOR "DragNDrop")
    set(CPACK_PACKAGE_VERSION ${OPENMW_VERSION})
    set(CPACK_PACKAGE_VERSION_MAJOR ${OPENMW_VERSION_MAJOR})
    set(CPACK_PACKAGE_VERSION_MINOR ${OPENMW_VERSION_MINOR})
    set(CPACK_PACKAGE_VERSION_PATCH ${OPENMW_VERSION_RELEASE})

    set(OPENMW_APP "\${CMAKE_INSTALL_PREFIX}/${INSTALL_SUBDIR}/${APP_BUNDLE_NAME}")

    set(OPENCS_BUNDLE_NAME "OpenCS.app")
    set(OPENCS_APP "\${CMAKE_INSTALL_PREFIX}/${INSTALL_SUBDIR}/${OPENCS_BUNDLE_NAME}")

    set(ABSOLUTE_PLUGINS "")

    foreach (PLUGIN ${USED_OGRE_PLUGINS})
        get_filename_component(PLUGIN_ABS ${PLUGIN} REALPATH)
        set(ABSOLUTE_PLUGINS ${PLUGIN_ABS} ${ABSOLUTE_PLUGINS})
    endforeach ()

    install(CODE "
        set(BU_CHMOD_BUNDLE_ITEMS ON)
        include(BundleUtilities)
    " COMPONENT Runtime)

    # installs used plugins in bundle at given path (bundle_path must be relative to ${CMAKE_INSTALL_PREFIX})
    # and returns list of install paths for all installed plugins
    function (install_plugins_for_bundle bundle_path plugins_var)
        set(RELATIVE_PLUGIN_INSTALL_BASE "${bundle_path}/Contents/Frameworks")

        set(PLUGINS "")
        set(PLUGIN_INSTALL_BASE "\${CMAKE_INSTALL_PREFIX}/${RELATIVE_PLUGIN_INSTALL_BASE}")

        foreach (PLUGIN ${ABSOLUTE_PLUGINS})
            get_filename_component(PLUGIN_RELATIVE ${PLUGIN} NAME)
            get_filename_component(PLUGIN_RELATIVE_WE ${PLUGIN} NAME_WE)

            set(PLUGIN_DYLIB_IN_BUNDLE "${PLUGIN_INSTALL_BASE}/${PLUGIN_RELATIVE}/${PLUGIN_RELATIVE_WE}")
            set(PLUGINS ${PLUGINS} "${PLUGIN_DYLIB_IN_BUNDLE}")

            install(CODE "
                copy_resolved_framework_into_bundle(\"${PLUGIN}/${PLUGIN_RELATIVE_WE}\" \"${PLUGIN_DYLIB_IN_BUNDLE}\")
            " COMPONENT Runtime)
        endforeach ()

        set(${plugins_var} ${PLUGINS} PARENT_SCOPE)
    endfunction (install_plugins_for_bundle)

    install_plugins_for_bundle("${INSTALL_SUBDIR}/${APP_BUNDLE_NAME}" PLUGINS)
    install_plugins_for_bundle("${INSTALL_SUBDIR}/${OPENCS_BUNDLE_NAME}" OPENCS_PLUGINS)

    #For now, search unresolved dependencies only in default system paths, so if you put unresolveable (i.e. with @executable_path in id name) lib or framework somewhere else, it would fail
    set(DIRS "")

    # Overriding item resolving during installation, it needed if
    # some library already has been "fixed up", i.e. its id name contains @executable_path,
    # but library is not embedded in bundle. For example, it's Ogre.framework from Ogre SDK.
    # Current implementation of GetPrerequsities/BundleUtilities doesn't handle that case.
    #
    # Current limitations:
    #   1. Handles only frameworks, not simple libs
    INSTALL(CODE "
        cmake_policy(SET CMP0009 OLD)
        set(CMAKE_FIND_LIBRARY_PREFIXES ${CMAKE_FIND_LIBRARY_PREFIXES})
        set(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES})
        set(CMAKE_SYSTEM_FRAMEWORK_PATH ${CMAKE_SYSTEM_FRAMEWORK_PATH})

        set(OPENMW_RESOLVED_ITEMS \"\")

        function(gp_resolve_item_override context item exepath dirs resolved_item_var resolved_var)
            if(item MATCHES \"@executable_path\" AND NOT \${\${resolved_var}})
                if (item MATCHES \"Frameworks\") # if it is a framework
                    # get last segment of path
                    get_filename_component(fname \"\${item}\" NAME_WE)
                    find_library(ri NAMES \${fname} PATHS \${exepath} \${dirs} \${CMAKE_SYSTEM_FRAMEWORK_PATH})
                    if (ri)
                        string(REGEX REPLACE \"^.*/Frameworks/.*\\\\.framework\" \"\" item_part \${item})
                        set(ri \"\${ri}\${item_part}\")
                        set(\${resolved_item_var} \${ri} PARENT_SCOPE)
                        set(\${resolved_var} 1 PARENT_SCOPE)
                    endif()
                else()
                    # code path for standard (non-framework) libs (ogre & qt pugins)
                    get_filename_component(fname \"\${item}\" NAME_WE)
                    string(REGEX REPLACE \"^lib\" \"\" fname \${fname})
                    find_library(ri NAMES \${fname} PATHS \${exepath} \${dirs} /usr/lib /usr/local/lib)
                    if (ri)
                        set(\${resolved_item_var} \${ri} PARENT_SCOPE)
                        set(\${resolved_var} 1 PARENT_SCOPE)
                    endif ()
                endif()
            endif()
        endfunction(gp_resolve_item_override)

        fixup_bundle(\"${OPENMW_APP}\" \"${PLUGINS}\" \"${DIRS}\")
        fixup_bundle(\"${OPENCS_APP}\" \"${OPENCS_PLUGINS}\" \"${DIRS}\")
        " COMPONENT Runtime)
        include(CPack)
endif (APPLE)

