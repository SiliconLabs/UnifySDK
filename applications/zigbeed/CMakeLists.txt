# ##############################################################################
# Zigbeed Generate, Build and Packaging
# ##############################################################################


if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "armhf")
  set(ZIGBEED_ARCHITECTURE arm32v7)
elseif(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "arm64")
  set(ZIGBEED_ARCHITECTURE arm64v8)
else()
  set(ZIGBEED_ARCHITECTURE arch_x86_64)
endif()

    find_package(GeckoSDK 4.2.2 EXACT REQUIRED)
    find_package(CPCD 4.2.2 EXACT REQUIRED)
    find_program(MAKE_EXECUTABLE NAMES make REQUIRED)

    set(OUTPUT_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}/build)
    set(SLC_PROJECT_DIR ${CMAKE_CURRENT_BINARY_DIR}/gen)


    add_custom_target(
        zigbeed-generated ALL
        COMMAND cp -r ${CMAKE_CURRENT_SOURCE_DIR}/project_files/ ${SLC_PROJECT_DIR})

    add_custom_target(
        zigbeed ALL
        WORKING_DIRECTORY ${SLC_PROJECT_DIR}
        DEPENDS zigbeed-generated cpc cpcd
    COMMAND
        ${MAKE_EXECUTABLE} -f zigbeed.Makefile
        LD_FLAGS=-L${cpcd_BINARY_DIR}
        BASE_SDK_PATH=${GeckoSDK_ROOT_DIR}
        ARCHITECTURE=${ZIGBEED_ARCHITECTURE}
        CPC_DAEMON_LIB=${cpcd_SOURCE_DIR}/lib
        BUILD_DIR=${OUTPUT_BUILD_DIR} OUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}
        CC=${CMAKE_C_COMPILER} CXX=${CMAKE_CXX_COMPILER} LD=${CMAKE_C_COMPILER}
        AR=${CMAKE_C_COMPILER_AR} C_FLAGS_DEBUG=${CMAKE_C_FLAGS_DEBUG}
        C_FLAGS_RELEASE=${CMAKE_C_FLAGS_RELEASE} AR_FLAGS=rs)

    # Package as .debian
    add_component_to_uic(
        uic-zigbeed # Package Name
        "Universal IOT controller - zigbeed"
        "${CMAKE_PROJECT_NAME}-zigbeed" # Package Debian filename
        "uic-cpcd,socat" # Package Depends on
        ""
        "${CMAKE_BINARY_DIR}/applications/zigbeed/debconf/postinst;\
        ${CMAKE_BINARY_DIR}/applications/zigbeed/debconf/prerm;\
        ${CMAKE_BINARY_DIR}/applications/zigbeed/debconf/postrm")

    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/debconf/postinst.in
               ${CMAKE_CURRENT_BINARY_DIR}/debconf/postinst)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/debconf/prerm.in
               ${CMAKE_CURRENT_BINARY_DIR}/debconf/prerm)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/debconf/postrm.in
               ${CMAKE_CURRENT_BINARY_DIR}/debconf/postrm)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/debconf/services/uic-zigbeed.service.in
               ${CMAKE_CURRENT_BINARY_DIR}/debconf/services/uic-zigbeed.service)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/debconf/services/uic-zigbeed-socat.service.in
               ${CMAKE_CURRENT_BINARY_DIR}/debconf/services/uic-zigbeed-socat.service)

    install(
        FILES ${CMAKE_CURRENT_SOURCE_DIR}/z3gateway/Z3Gateway
        DESTINATION "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}"
        PERMISSIONS
            OWNER_READ
            OWNER_WRITE
            OWNER_EXECUTE
            GROUP_EXECUTE
            GROUP_READ
            WORLD_READ
            WORLD_EXECUTE
    COMPONENT uic-zigbeed)

    install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/debconf/services/uic-zigbeed.service
        DESTINATION /lib/systemd/system/
        COMPONENT uic-zigbeed)

    install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/debconf/services/uic-zigbeed-socat.service
        DESTINATION /lib/systemd/system/
        COMPONENT uic-zigbeed)

    install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/zigbeed
        DESTINATION "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}"
        PERMISSIONS
            OWNER_READ
            OWNER_WRITE
            OWNER_EXECUTE
            GROUP_EXECUTE
            GROUP_READ
            WORLD_READ
            WORLD_EXECUTE
        COMPONENT uic-zigbeed)

    install(
        FILES ${CMAKE_CURRENT_SOURCE_DIR}/zigbeed.conf
        DESTINATION /usr/local/etc/
        COMPONENT uic-zigbeed)


