# Build options will determine if the applications will be included and build
if(BUILD_ZPC MATCHES ON)
  add_subdirectory(zpc)
endif()

if(BUILD_ZIGPC MATCHES ON)
  find_package(GeckoSDK)
  if (GeckoSDK_FOUND)
    add_subdirectory(zigpc)
    message(STATUS "Zigbee Protocol Controller will build")
  else()
    message(STATUS "Zigbee Protocol Controller will NOT build")
  endif()
endif()

if(BUILD_UIC_DEMO MATCHES ON)
  add_subdirectory(demo_uic_lib)
endif()

if(BUILD_DEV_CLI MATCHES ON)
  add_subdirectory(dev_ui/dev_cli)
endif()

if(BUILD_DEV_GUI MATCHES ON)
  add_subdirectory(dev_ui/dev_gui)
endif()

if (BUILD_UPVL MATCHES ON)
  add_subdirectory(upvl)
endif()

if (BUILD_ANGEL MATCHES ON)
  add_subdirectory(angel)
endif()

if (BUILD_IMAGE_PROVIDER MATCHES ON)
  add_subdirectory(image_provider)
endif()
