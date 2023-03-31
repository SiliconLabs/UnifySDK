# Fetch dependencies if forced
if(FETCH_GSDK MATCHES ON)
  find_package(GeckoSDK 4.2.2 REQUIRED)
endif()

# Build options will determine if the applications will be included and build

## Protocol Controllers
if(BUILD_ZPC MATCHES ON)
  add_subdirectory(zpc)
  message(STATUS "ZPC will build")
else()
  message(STATUS "ZPC will NOT build")
endif()

## Example applications/components
if((BUILD_UIC_DEMO MATCHES ON) OR (BUILD_EPC MATCHES ON))
  add_subdirectory(examples)
  message(STATUS "UIC Demo and/or EPC will build")
else()
  message(STATUS "UIC Demo and/or EPC will NOT build")
endif()

## IoT services
if(BUILD_DEV_GUI MATCHES ON)
  add_subdirectory(dev_ui/dev_gui)
  message(STATUS "DevGui will build")
else()
  message(STATUS "DevGui will NOT build")
endif()

if(BUILD_UPVL MATCHES ON)
  message(STATUS "UPVL will build")
  add_subdirectory(upvl)
else()
  message(STATUS "UPVL will NOT build")
endif()

if(BUILD_GMS MATCHES ON)
  add_subdirectory(gms)
  message(STATUS "GMS will build")
else()
  message(STATUS "GMS will NOT build")
endif()

if(BUILD_IMAGE_PROVIDER MATCHES ON)
  add_subdirectory(image_provider)
  message(STATUS "Image provider will build")
else()
  message(STATUS "Image provider will NOT build")
endif()


if(BUILD_NAL MATCHES ON)
  add_subdirectory(nal)
  message(STATUS "NAL will build")
else()
  message(STATUS "NAL will NOT build")
endif()

if(BUILD_UPTI_CAP MATCHES ON)
  add_subdirectory(upti_cap)
  message(STATUS "UPTI Cap will build")
else()
  message(STATUS "UPTI Cap will NOT build")
endif()

if(BUILD_UPTI_WRITER MATCHES ON)
  add_subdirectory(upti_writer)
  message(STATUS "UPTI Writer will build")
else()
  message(STATUS "UPTI Writer will NOT build")
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
    add_subdirectory(bt_host_empty)
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
    message(STATUS "Zigbeed will build")
  else()
    message(STATUS "Zigbeed will NOT build")
  endif()

  if(BUILD_ZIGPC MATCHES ON)
    add_subdirectory(zigpc)
    message(STATUS "ZigPC will build")
  else()
    message(STATUS "ZigPC will NOT build")
  endif()
endif()
