# Build options will determine if the applications will be included and build

## Example applications/components
if((BUILD_UIC_DEMO MATCHES ON) OR (BUILD_EPC MATCHES ON) OR (BUILD_EED MATCHES ON))
  add_subdirectory(examples)
  message(STATUS "UIC Demo, Emulated End Device (EED) and/or EPC will build")
else()
  message(STATUS "UIC Demo, Emulated End Device (EED) and/or EPC will NOT build")
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


