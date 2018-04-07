cmake_minimum_required(VERSION 2.8)

option(UPDATE_TRANSLATIONS "update Translation" OFF)
MESSAGE(STATUS "UPDATE TRANSLATIONS: ${UPDATE_TRANSLATIONS}")


project(dice)


# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

set(EXECUTABLE_OUTPUT_PATH bin/)


# Find the QtWidgets library
find_package(Qt5Core)
include_directories(${Qt5Core_INCLUDES} ../)
add_definitions(${Qt5Core_DEFINITIONS})

find_package(Qt5Gui)
include_directories(${Qt5Gui_INCLUDES} ../)
add_definitions(${Qt5Gui_DEFINITIONS})


set(MODE "cli")

ADD_DEFINITIONS(
    -std=c++11 # Or -std=c++0x
    # Other flags
)

set(dice_RESOURCES diceparser.qrc)
   

IF(UPDATE_TRANSLATIONS)
	MESSAGE( update Translation )
    FILE(GLOB_RECURSE translate_dice_SRCS ../*.cpp ../*.h)
    SET(translate_SRCS ${translate_dice_SRCS})
    SET(dice_TS "${CMAKE_CURRENT_SOURCE_DIR}/i18n/dice_en.ts" "${CMAKE_CURRENT_SOURCE_DIR}/i18n/dice_fr.ts")
ELSE()
	MESSAGE( NO updates for translations)
    FILE(GLOB dice_TS "${CMAKE_CURRENT_SOURCE_DIR}/i18n/*.ts")
ENDIF(UPDATE_TRANSLATIONS)

if(Qt5Core_FOUND)

    IF(UPDATE_TRANSLATIONS)
		MESSAGE(status  "find" ${dice_TS} ${translate_SRCS} )
        QT5_CREATE_TRANSLATION(dice_QM ${translate_SRCS} ${dice_TS})
    ELSE()
    ENDIF()

    QT5_ADD_RESOURCES(dice_RESOURCES_RCC ${dice_RESOURCES})

    # guess plugins and libraries directory
    set(QT_PLUGINS_DIR "${Qt5Core_DIR}/../../../plugins")
    get_target_property(QT_LIBRARY_DIR Qt5::Core LOCATION)
    get_filename_component(QT_LIBRARY_DIR ${QT_LIBRARY_DIR} PATH)
endif()

SET( dice_sources 
    ../diceparser.cpp
    ../range.cpp
    ../booleancondition.cpp
    ../validator.cpp
    ../compositevalidator.cpp
    ../operationcondition.cpp
    ../die.cpp
    ../parsingtoolbox.cpp
    ../dicealias.cpp
    ../result/result.cpp
    ../result/scalarresult.cpp
    ../result/stringresult.cpp
    ../result/diceresult.cpp
    ../node/countexecutenode.cpp
    ../node/dicerollernode.cpp
    ../node/executionnode.cpp
    ../node/explosedicenode.cpp
    ../node/helpnode.cpp
    ../node/mergenode.cpp
    ../node/jumpbackwardnode.cpp
    ../node/keepdiceexecnode.cpp
    ../node/listaliasnode.cpp
    ../node/listsetrollnode.cpp
    ../node/numbernode.cpp
    ../node/parenthesesnode.cpp
    ../node/paintnode.cpp
    ../node/rerolldicenode.cpp
    ../node/scalaroperatornode.cpp
    ../node/sortresult.cpp
    ../node/startingnode.cpp
    ../node/filternode.cpp
    ../node/stringnode.cpp
    ../node/ifnode.cpp
    ../node/splitnode.cpp
    ../node/groupnode.cpp
    main.cpp
    displaytoolbox.cpp
    ../highlightdice.cpp
    ../node/variablenode.cpp
)

add_executable( dice ${dice_sources} ${dice_QM}   )

target_link_libraries(dice ${Qt5Core_LIBRARIES} ${Qt5Gui_LIBRARIES})
INSTALL_TARGETS(/bin dice)

#qt5_use_modules()
