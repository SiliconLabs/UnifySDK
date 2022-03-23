# Build options will determine if the applications will be included and build
if(BUILD_ZPC MATCHES ON)
  add_subdirectory(zpc)
endif()

if(BUILD_ZIGPC MATCHES ON)
  find_package(GeckoSDK)
  if(GeckoSDK_FOUND)
    add_subdirectory(zigpc)
    message(STATUS "Zigbee Protocol Controller will build")
  else()
    message(STATUS "Zigbee Protocol Controller will NOT build")
  endif()
endif()

if(BUILD_UIC_DEMO MATCHES ON)
  add_subdirectory(demo_uic_lib)
endif()

if(BUILD_DEV_GUI MATCHES ON)
  add_subdirectory(dev_ui/dev_gui)
endif()

if(BUILD_UPVL MATCHES ON)
  add_subdirectory(upvl)
endif()

if(BUILD_GMS MATCHES ON)
  add_subdirectory(gms)
endif()

if(BUILD_IMAGE_PROVIDER MATCHES ON)
  add_subdirectory(image_provider)
endif()

if((BUILD_AOXPC MATCHES ON) OR (BUILD_POSITIONING MATCHES ON))
  find_package(GeckoSDK)
  if(GeckoSDK_FOUND)
    add_subdirectory(aox)
    message(STATUS "AoX applications will build")
  else()
    message(STATUS "AoX applications will NOT build")
  endif()
endif()

if(BUILD_NAL MATCHES ON)
  add_subdirectory(nal)
endif()

if (BUILD_UPTI_CAP MATCHES ON)
  add_subdirectory(upti_cap)
endif()

if (BUILD_UPTI_WRITER MATCHES ON)
  add_subdirectory(upti_writer)
endif()
