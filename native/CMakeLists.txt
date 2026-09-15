cmake_minimum_required(VERSION 3.20)

project(mcdose_particle_dmlc_native VERSION 0.1.0 LANGUAGES CXX)

option(
    MCDOSE_PARTICLE_DMLC_BUILD_EGSNRC_MORTRAN_BRIDGE
    "Build the host-specific EGSnrc Mortran callback bridge"
    OFF
)
option(
    MCDOSE_PARTICLE_DMLC_BUILD_STARTUP_LOADER
    "Build the strict SQLite producer startup-artifact loader"
    OFF
)
set(
    MCDOSE_PARTICLE_DMLC_EGSNRC_CONFIG_INCLUDE_DIR
    ""
    CACHE PATH
    "Directory containing the selected EGSnrc egs_config1.h"
)
set(
    MCDOSE_PARTICLE_DMLC_TEST_SOURCE_STARTUP_ARTIFACT
    ""
    CACHE FILEPATH
    "Synthetic startup SQLite artifact used by configured source tests"
)
set(
    MCDOSE_PARTICLE_DMLC_TEST_CORRECTED_SOURCE_STARTUP_ARTIFACT
    ""
    CACHE FILEPATH
    "Synthetic startup artifact bound to the output-correction test"
)
set(
    MCDOSE_PARTICLE_DMLC_TEST_SOURCE_OUTPUT_CORRECTION_ARTIFACT
    ""
    CACHE FILEPATH
    "Python-created source-output correction used by configured tests"
)

add_library(
    mcdose_particle_dmlc_native
    SHARED
    src/attenuation_v1.cpp
    src/delivery_v1.cpp
    src/interaction_v1.cpp
    src/machine_v1.cpp
    src/transport_v1.cpp
)
target_compile_features(mcdose_particle_dmlc_native PUBLIC cxx_std_17)
target_compile_definitions(mcdose_particle_dmlc_native PRIVATE MCDOSE_PARTICLE_DMLC_BUILD)
target_include_directories(
    mcdose_particle_dmlc_native
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
set_target_properties(
    mcdose_particle_dmlc_native
    PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION 1
)

if(MCDOSE_PARTICLE_DMLC_BUILD_EGSNRC_MORTRAN_BRIDGE)
    if(
        NOT EXISTS
        "${MCDOSE_PARTICLE_DMLC_EGSNRC_CONFIG_INCLUDE_DIR}/egs_config1.h"
    )
        message(
            FATAL_ERROR
            "MCDOSE_PARTICLE_DMLC_EGSNRC_CONFIG_INCLUDE_DIR must contain egs_config1.h"
        )
    endif()
    add_library(
        mcdose_particle_dmlc_egsnrc_mortran_bridge
        STATIC
        integration/egsnrc_mortran_bridge_v1.cpp
    )
    target_compile_features(
        mcdose_particle_dmlc_egsnrc_mortran_bridge PUBLIC cxx_std_17
    )
    target_compile_definitions(
        mcdose_particle_dmlc_egsnrc_mortran_bridge
        PRIVATE MCDOSE_PARTICLE_DMLC_BUILD
    )
    target_include_directories(
        mcdose_particle_dmlc_egsnrc_mortran_bridge
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
        PRIVATE ${MCDOSE_PARTICLE_DMLC_EGSNRC_CONFIG_INCLUDE_DIR}
    )
    set_target_properties(
        mcdose_particle_dmlc_egsnrc_mortran_bridge
        PROPERTIES POSITION_INDEPENDENT_CODE ON
    )
endif()

add_library(
    mcdose_particle_dmlc_host_adapter
    SHARED
    integration/host_adapter_v1.cpp
    integration/producer_v1.cpp
)
target_compile_features(mcdose_particle_dmlc_host_adapter PUBLIC cxx_std_17)
target_compile_definitions(
    mcdose_particle_dmlc_host_adapter PRIVATE MCDOSE_PARTICLE_DMLC_BUILD
)
target_include_directories(
    mcdose_particle_dmlc_host_adapter
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
target_link_libraries(
    mcdose_particle_dmlc_host_adapter PRIVATE mcdose_particle_dmlc_native
)
set_target_properties(
    mcdose_particle_dmlc_host_adapter
    PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION 1
)

add_library(
    mcdose_particle_dmlc_source_session
    SHARED
    integration/source_session_v1.cpp
)
target_compile_features(mcdose_particle_dmlc_source_session PUBLIC cxx_std_17)
target_compile_definitions(
    mcdose_particle_dmlc_source_session PRIVATE MCDOSE_PARTICLE_DMLC_BUILD
)
target_include_directories(
    mcdose_particle_dmlc_source_session
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
target_link_libraries(
    mcdose_particle_dmlc_source_session
    PRIVATE mcdose_particle_dmlc_host_adapter
)
set_target_properties(
    mcdose_particle_dmlc_source_session
    PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION 1
)

add_library(
    mcdose_particle_dmlc_source_output_correction
    SHARED
    integration/source_output_correction_v1.cpp
)
target_compile_features(
    mcdose_particle_dmlc_source_output_correction PUBLIC cxx_std_17
)
target_compile_definitions(
    mcdose_particle_dmlc_source_output_correction
    PRIVATE MCDOSE_PARTICLE_DMLC_BUILD
)
target_include_directories(
    mcdose_particle_dmlc_source_output_correction
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
set_target_properties(
    mcdose_particle_dmlc_source_output_correction
    PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION 1
)

if(MCDOSE_PARTICLE_DMLC_BUILD_STARTUP_LOADER)
    find_package(SQLite3 3.37 REQUIRED)
    if(TARGET SQLite3::SQLite3)
        set(MCDOSE_PARTICLE_DMLC_SQLITE_TARGET SQLite3::SQLite3)
    else()
        set(MCDOSE_PARTICLE_DMLC_SQLITE_TARGET SQLite::SQLite3)
    endif()
    add_library(
        mcdose_particle_dmlc_startup_loader
        SHARED
        integration/startup_loader_v1.cpp
    )
    target_compile_features(
        mcdose_particle_dmlc_startup_loader PUBLIC cxx_std_17
    )
    target_compile_definitions(
        mcdose_particle_dmlc_startup_loader PRIVATE MCDOSE_PARTICLE_DMLC_BUILD
    )
    target_include_directories(
        mcdose_particle_dmlc_startup_loader
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
    )
    target_link_libraries(
        mcdose_particle_dmlc_startup_loader
        PRIVATE
            mcdose_particle_dmlc_host_adapter
            ${MCDOSE_PARTICLE_DMLC_SQLITE_TARGET}
    )
    set_target_properties(
        mcdose_particle_dmlc_startup_loader
        PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION 1
    )
endif()

add_library(
    mcdose_particle_dmlc_egsnrc_adapter
    SHARED
    integration/egsnrc_adapter_v1.cpp
)
target_compile_features(mcdose_particle_dmlc_egsnrc_adapter PUBLIC cxx_std_17)
target_compile_definitions(
    mcdose_particle_dmlc_egsnrc_adapter PRIVATE MCDOSE_PARTICLE_DMLC_BUILD
)
target_include_directories(
    mcdose_particle_dmlc_egsnrc_adapter
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
set_target_properties(
    mcdose_particle_dmlc_egsnrc_adapter
    PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION 1
)

if(MCDOSE_PARTICLE_DMLC_BUILD_STARTUP_LOADER)
    add_library(
        mcdose_particle_dmlc_egsnrc_source
        SHARED
        integration/egsnrc_source_v1.cpp
    )
    target_compile_features(
        mcdose_particle_dmlc_egsnrc_source PUBLIC cxx_std_17
    )
    target_compile_definitions(
        mcdose_particle_dmlc_egsnrc_source PRIVATE MCDOSE_PARTICLE_DMLC_BUILD
    )
    target_include_directories(
        mcdose_particle_dmlc_egsnrc_source
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
    )
    target_link_libraries(
        mcdose_particle_dmlc_egsnrc_source
        PRIVATE
            mcdose_particle_dmlc_startup_loader
            mcdose_particle_dmlc_host_adapter
            mcdose_particle_dmlc_egsnrc_adapter
            mcdose_particle_dmlc_source_output_correction
    )
    set_target_properties(
        mcdose_particle_dmlc_egsnrc_source
        PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION 1
    )
endif()

if(
    MCDOSE_PARTICLE_DMLC_BUILD_STARTUP_LOADER
    AND MCDOSE_PARTICLE_DMLC_BUILD_EGSNRC_MORTRAN_BRIDGE
)
    add_library(
        mcdose_particle_dmlc_egsnrc_source_mortran_bridge
        STATIC
        integration/egsnrc_source_mortran_bridge_v1.cpp
    )
    target_compile_features(
        mcdose_particle_dmlc_egsnrc_source_mortran_bridge PUBLIC cxx_std_17
    )
    target_compile_definitions(
        mcdose_particle_dmlc_egsnrc_source_mortran_bridge
        PRIVATE MCDOSE_PARTICLE_DMLC_BUILD
    )
    target_include_directories(
        mcdose_particle_dmlc_egsnrc_source_mortran_bridge
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
        PRIVATE ${MCDOSE_PARTICLE_DMLC_EGSNRC_CONFIG_INCLUDE_DIR}
    )
    target_link_libraries(
        mcdose_particle_dmlc_egsnrc_source_mortran_bridge
        PRIVATE mcdose_particle_dmlc_egsnrc_source
    )
    set_target_properties(
        mcdose_particle_dmlc_egsnrc_source_mortran_bridge
        PROPERTIES POSITION_INDEPENDENT_CODE ON
    )
endif()

if(MSVC)
    target_compile_options(mcdose_particle_dmlc_native PRIVATE /W4 /WX)
    target_compile_options(mcdose_particle_dmlc_host_adapter PRIVATE /W4 /WX)
    target_compile_options(mcdose_particle_dmlc_source_session PRIVATE /W4 /WX)
    target_compile_options(mcdose_particle_dmlc_egsnrc_adapter PRIVATE /W4 /WX)
    target_compile_options(
        mcdose_particle_dmlc_source_output_correction PRIVATE /W4 /WX
    )
    if(TARGET mcdose_particle_dmlc_startup_loader)
        target_compile_options(
            mcdose_particle_dmlc_startup_loader PRIVATE /W4 /WX
        )
    endif()
    if(TARGET mcdose_particle_dmlc_egsnrc_source)
        target_compile_options(
            mcdose_particle_dmlc_egsnrc_source PRIVATE /W4 /WX
        )
    endif()
else()
    target_compile_options(
        mcdose_particle_dmlc_native PRIVATE -Wall -Wextra -Wpedantic -Werror
    )
    target_compile_options(
        mcdose_particle_dmlc_host_adapter PRIVATE -Wall -Wextra -Wpedantic -Werror
    )
    target_compile_options(
        mcdose_particle_dmlc_source_session
        PRIVATE -Wall -Wextra -Wpedantic -Werror
    )
    target_compile_options(
        mcdose_particle_dmlc_egsnrc_adapter PRIVATE -Wall -Wextra -Wpedantic -Werror
    )
    target_compile_options(
        mcdose_particle_dmlc_source_output_correction
        PRIVATE -Wall -Wextra -Wpedantic -Werror
    )
    if(TARGET mcdose_particle_dmlc_startup_loader)
        target_compile_options(
            mcdose_particle_dmlc_startup_loader
            PRIVATE -Wall -Wextra -Wpedantic -Werror
        )
    endif()
    if(TARGET mcdose_particle_dmlc_egsnrc_source)
        target_compile_options(
            mcdose_particle_dmlc_egsnrc_source
            PRIVATE -Wall -Wextra -Wpedantic -Werror
        )
    endif()
endif()

if(TARGET mcdose_particle_dmlc_egsnrc_mortran_bridge)
    if(MSVC)
        target_compile_options(
            mcdose_particle_dmlc_egsnrc_mortran_bridge PRIVATE /W4 /WX
        )
    else()
        target_compile_options(
            mcdose_particle_dmlc_egsnrc_mortran_bridge
            PRIVATE -Wall -Wextra -Wpedantic -Werror
        )
    endif()
endif()
if(TARGET mcdose_particle_dmlc_egsnrc_source_mortran_bridge)
    if(MSVC)
        target_compile_options(
            mcdose_particle_dmlc_egsnrc_source_mortran_bridge PRIVATE /W4 /WX
        )
    else()
        target_compile_options(
            mcdose_particle_dmlc_egsnrc_source_mortran_bridge
            PRIVATE -Wall -Wextra -Wpedantic -Werror
        )
    endif()
endif()

include(CTest)
if(BUILD_TESTING)
    find_package(Threads REQUIRED)
    add_executable(test_particle_dmlc_delivery_v1 tests/test_delivery_v1.cpp)
    target_compile_features(test_particle_dmlc_delivery_v1 PRIVATE cxx_std_17)
    target_link_libraries(test_particle_dmlc_delivery_v1 PRIVATE mcdose_particle_dmlc_native)
    add_test(NAME particle_dmlc_delivery_v1 COMMAND test_particle_dmlc_delivery_v1)
    add_executable(test_particle_dmlc_machine_v1 tests/test_machine_v1.cpp)
    target_compile_features(test_particle_dmlc_machine_v1 PRIVATE cxx_std_17)
    target_link_libraries(test_particle_dmlc_machine_v1 PRIVATE mcdose_particle_dmlc_native)
    add_test(NAME particle_dmlc_machine_v1 COMMAND test_particle_dmlc_machine_v1)
    add_executable(test_particle_dmlc_transport_v1 tests/test_transport_v1.cpp)
    target_compile_features(test_particle_dmlc_transport_v1 PRIVATE cxx_std_17)
    target_link_libraries(test_particle_dmlc_transport_v1 PRIVATE mcdose_particle_dmlc_native)
    add_test(NAME particle_dmlc_transport_v1 COMMAND test_particle_dmlc_transport_v1)
    add_executable(test_particle_dmlc_interaction_v1 tests/test_interaction_v1.cpp)
    target_compile_features(test_particle_dmlc_interaction_v1 PRIVATE cxx_std_17)
    target_link_libraries(test_particle_dmlc_interaction_v1 PRIVATE mcdose_particle_dmlc_native)
    add_test(NAME particle_dmlc_interaction_v1 COMMAND test_particle_dmlc_interaction_v1)
    add_executable(test_particle_dmlc_host_adapter_v1 tests/test_host_adapter_v1.cpp)
    target_compile_features(test_particle_dmlc_host_adapter_v1 PRIVATE cxx_std_17)
    target_link_libraries(
        test_particle_dmlc_host_adapter_v1 PRIVATE mcdose_particle_dmlc_host_adapter
    )
    add_test(NAME particle_dmlc_host_adapter_v1 COMMAND test_particle_dmlc_host_adapter_v1)
    add_executable(test_particle_dmlc_producer_v1 tests/test_producer_v1.cpp)
    target_compile_features(test_particle_dmlc_producer_v1 PRIVATE cxx_std_17)
    target_link_libraries(
        test_particle_dmlc_producer_v1
        PRIVATE mcdose_particle_dmlc_host_adapter Threads::Threads
    )
    add_test(NAME particle_dmlc_producer_v1 COMMAND test_particle_dmlc_producer_v1)
    add_executable(
        test_particle_dmlc_source_session_v1 tests/test_source_session_v1.cpp
    )
    target_compile_features(
        test_particle_dmlc_source_session_v1 PRIVATE cxx_std_17
    )
    target_link_libraries(
        test_particle_dmlc_source_session_v1
        PRIVATE
            mcdose_particle_dmlc_source_session
            mcdose_particle_dmlc_host_adapter
    )
    add_test(
        NAME particle_dmlc_source_session_v1
        COMMAND test_particle_dmlc_source_session_v1
    )
    add_executable(test_particle_dmlc_egsnrc_adapter_v1 tests/test_egsnrc_adapter_v1.cpp)
    target_compile_features(test_particle_dmlc_egsnrc_adapter_v1 PRIVATE cxx_std_17)
    target_link_libraries(
        test_particle_dmlc_egsnrc_adapter_v1 PRIVATE mcdose_particle_dmlc_egsnrc_adapter
    )
    add_test(NAME particle_dmlc_egsnrc_adapter_v1 COMMAND test_particle_dmlc_egsnrc_adapter_v1)
    add_executable(
        test_particle_dmlc_source_output_correction_v1
        tests/test_source_output_correction_v1.cpp
    )
    target_compile_features(
        test_particle_dmlc_source_output_correction_v1 PRIVATE cxx_std_17
    )
    target_link_libraries(
        test_particle_dmlc_source_output_correction_v1
        PRIVATE mcdose_particle_dmlc_source_output_correction
    )
    add_test(
        NAME particle_dmlc_source_output_correction_v1
        COMMAND test_particle_dmlc_source_output_correction_v1
    )
    if(EXISTS "${MCDOSE_PARTICLE_DMLC_TEST_SOURCE_OUTPUT_CORRECTION_ARTIFACT}")
        add_test(
            NAME particle_dmlc_source_output_correction_artifact_v1
            COMMAND
                test_particle_dmlc_source_output_correction_v1
                "${MCDOSE_PARTICLE_DMLC_TEST_SOURCE_OUTPUT_CORRECTION_ARTIFACT}"
        )
    endif()
    if(TARGET mcdose_particle_dmlc_startup_loader)
        add_executable(
            test_particle_dmlc_startup_loader_v1
            tests/test_startup_loader_v1.cpp
        )
        target_compile_features(
            test_particle_dmlc_startup_loader_v1 PRIVATE cxx_std_17
        )
        target_link_libraries(
            test_particle_dmlc_startup_loader_v1
            PRIVATE
                mcdose_particle_dmlc_startup_loader
                mcdose_particle_dmlc_host_adapter
        )
        add_executable(
            test_particle_dmlc_egsnrc_source_v1
            tests/test_egsnrc_source_v1.cpp
        )
        target_compile_features(
            test_particle_dmlc_egsnrc_source_v1 PRIVATE cxx_std_17
        )
        target_link_libraries(
            test_particle_dmlc_egsnrc_source_v1
            PRIVATE mcdose_particle_dmlc_egsnrc_source
        )
        if(EXISTS "${MCDOSE_PARTICLE_DMLC_TEST_SOURCE_STARTUP_ARTIFACT}")
            add_test(
                NAME particle_dmlc_egsnrc_source_v1
                COMMAND
                    test_particle_dmlc_egsnrc_source_v1
                    "${MCDOSE_PARTICLE_DMLC_TEST_SOURCE_STARTUP_ARTIFACT}"
            )
        else()
            message(
                STATUS
                "Configured EGSnrc source test is not registered: "
                "MCDOSE_PARTICLE_DMLC_TEST_SOURCE_STARTUP_ARTIFACT is absent"
            )
        endif()
        if(
            EXISTS "${MCDOSE_PARTICLE_DMLC_TEST_CORRECTED_SOURCE_STARTUP_ARTIFACT}"
            AND EXISTS "${MCDOSE_PARTICLE_DMLC_TEST_SOURCE_OUTPUT_CORRECTION_ARTIFACT}"
        )
            add_executable(
                test_particle_dmlc_egsnrc_source_output_correction_v1
                tests/test_egsnrc_source_output_correction_v1.cpp
            )
            target_compile_features(
                test_particle_dmlc_egsnrc_source_output_correction_v1
                PRIVATE cxx_std_17
            )
            target_link_libraries(
                test_particle_dmlc_egsnrc_source_output_correction_v1
                PRIVATE mcdose_particle_dmlc_egsnrc_source
            )
            add_test(
                NAME particle_dmlc_egsnrc_source_output_correction_v1
                COMMAND
                    test_particle_dmlc_egsnrc_source_output_correction_v1
                    "${MCDOSE_PARTICLE_DMLC_TEST_CORRECTED_SOURCE_STARTUP_ARTIFACT}"
                    "${MCDOSE_PARTICLE_DMLC_TEST_SOURCE_OUTPUT_CORRECTION_ARTIFACT}"
            )
        endif()
    endif()
    if(TARGET mcdose_particle_dmlc_egsnrc_mortran_bridge)
        add_executable(
            test_particle_dmlc_egsnrc_mortran_bridge_v1
            tests/test_egsnrc_mortran_bridge_v1.cpp
        )
        target_compile_features(
            test_particle_dmlc_egsnrc_mortran_bridge_v1 PRIVATE cxx_std_17
        )
        target_include_directories(
            test_particle_dmlc_egsnrc_mortran_bridge_v1
            PRIVATE ${MCDOSE_PARTICLE_DMLC_EGSNRC_CONFIG_INCLUDE_DIR}
        )
        target_link_libraries(
            test_particle_dmlc_egsnrc_mortran_bridge_v1
            PRIVATE
                mcdose_particle_dmlc_egsnrc_adapter
                mcdose_particle_dmlc_egsnrc_mortran_bridge
        )
        add_test(
            NAME particle_dmlc_egsnrc_mortran_bridge_v1
            COMMAND test_particle_dmlc_egsnrc_mortran_bridge_v1
        )
    endif()
    if(TARGET mcdose_particle_dmlc_egsnrc_source_mortran_bridge)
        add_executable(
            test_particle_dmlc_egsnrc_source_mortran_bridge_v1
            tests/test_egsnrc_source_mortran_bridge_v1.cpp
        )
        target_compile_features(
            test_particle_dmlc_egsnrc_source_mortran_bridge_v1
            PRIVATE cxx_std_17
        )
        target_include_directories(
            test_particle_dmlc_egsnrc_source_mortran_bridge_v1
            PRIVATE ${MCDOSE_PARTICLE_DMLC_EGSNRC_CONFIG_INCLUDE_DIR}
        )
        target_link_libraries(
            test_particle_dmlc_egsnrc_source_mortran_bridge_v1
            PRIVATE
                mcdose_particle_dmlc_egsnrc_source_mortran_bridge
                mcdose_particle_dmlc_egsnrc_source
        )
        if(EXISTS "${MCDOSE_PARTICLE_DMLC_TEST_SOURCE_STARTUP_ARTIFACT}")
            add_test(
                NAME particle_dmlc_egsnrc_source_mortran_bridge_v1
                COMMAND
                    test_particle_dmlc_egsnrc_source_mortran_bridge_v1
                    "${MCDOSE_PARTICLE_DMLC_TEST_SOURCE_STARTUP_ARTIFACT}"
            )
        endif()
    endif()
endif()

install(
    TARGETS
        mcdose_particle_dmlc_native
        mcdose_particle_dmlc_host_adapter
        mcdose_particle_dmlc_source_session
        mcdose_particle_dmlc_egsnrc_adapter
        mcdose_particle_dmlc_source_output_correction
)
if(TARGET mcdose_particle_dmlc_egsnrc_mortran_bridge)
    install(TARGETS mcdose_particle_dmlc_egsnrc_mortran_bridge)
endif()
if(TARGET mcdose_particle_dmlc_egsnrc_source_mortran_bridge)
    install(TARGETS mcdose_particle_dmlc_egsnrc_source_mortran_bridge)
endif()
if(TARGET mcdose_particle_dmlc_startup_loader)
    install(TARGETS mcdose_particle_dmlc_startup_loader)
endif()
if(TARGET mcdose_particle_dmlc_egsnrc_source)
    install(TARGETS mcdose_particle_dmlc_egsnrc_source)
endif()
install(
    FILES
        include/mcdose_particle_dmlc_delivery_v1.h
        include/mcdose_particle_dmlc_machine_v1.h
        include/mcdose_particle_dmlc_transport_v1.h
        include/mcdose_particle_dmlc_interaction_v1.h
        include/mcdose_particle_dmlc_host_adapter_v1.h
        include/mcdose_particle_dmlc_producer_v1.h
        include/mcdose_particle_dmlc_source_session_v1.h
        include/mcdose_particle_dmlc_egsnrc_adapter_v1.h
        include/mcdose_particle_dmlc_egsnrc_mortran_bridge_v1.h
        include/mcdose_particle_dmlc_startup_v1.h
        include/mcdose_particle_dmlc_egsnrc_source_v1.h
        include/mcdose_particle_dmlc_source_output_correction_v1.h
    DESTINATION include
)
install(
    FILES integration/mcdose_particle_dmlc_shower_v1.mortran
          integration/mcdose_particle_dmlc_source_v1.mortran
          integration/egsnrc_dosxyznrc_source21_v1.patch
    DESTINATION share/mcdose_particle_dmlc
)
