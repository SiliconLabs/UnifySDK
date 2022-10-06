# Fetch dependencies if forced
if(FETCH_GSDK MATCHES ON)
  find_package(GeckoSDK 4.1.1 REQUIRED)
endif()

# Build options will determine if the applications will be included and build

## Protocol Controllers
if(BUILD_ZPC MATCHES ON)
  add_subdirectory(zpc)
endif()

## Example applications/components
if((BUILD_UIC_DEMO MATCHES ON) OR (BUILD_EPC MATCHES ON))
  add_subdirectory(examples)
endif()

## IoT services
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


if(BUILD_NAL MATCHES ON)
  add_subdirectory(nal)
endif()

if(BUILD_UPTI_CAP MATCHES ON)
  add_subdirectory(upti_cap)
endif()

if(BUILD_UPTI_WRITER MATCHES ON)
  add_subdirectory(upti_writer)
endif()

if(NOT APPLE )
if((BUILD_AOXPC MATCHES ON) OR (BUILD_POSITIONING MATCHES ON))
  add_subdirectory(aox)
  message(STATUS "AoX applications will build")
else()
  message(STATUS "AoX applications will NOT build")
endif()


if(BUILD_CPCD MATCHES ON)
  add_subdirectory(cpcd)
  add_subdirectory(cpc-hci-bridge)
  message(STATUS "CPCd daemon and applications will build")
else()
  message(STATUS "CPCd daemon and applications will NOT build")
endif()

IF (BUILD_OTBR MATCHES ON)
  add_subdirectory(openthread_border_router)
  message(STATUS "OTBR application will build")
else()
  message(STATUS "OTBR application will NOT build")
endif()

if(BUILD_ZIGBEED MATCHES ON)
  add_subdirectory(zigbeed)
endif()

if(BUILD_ZIGPC MATCHES ON)
  add_subdirectory(zigpc)
  message(STATUS "Zigbee Protocol Controller will build")
else()
  message(STATUS "Zigbee Protocol Controller will NOT build")
endif()

endif()


if (BUILD_MATTER_BRIDGE MATCHES ON AND NOT CMAKE_GCOV)
  add_subdirectory(matter_bridge)
endif()
