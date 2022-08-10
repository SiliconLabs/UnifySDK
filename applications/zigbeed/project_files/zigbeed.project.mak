####################################################################
# Automatically-generated file. Do not edit!                       #
# Makefile Version 10                                              #
####################################################################

BASE_SDK_PATH = 
CPC_DAEMON_LIB =

UNAME:=$(shell uname -s | sed -e 's/^\(CYGWIN\).*/\1/' | sed -e 's/^\(MINGW\).*/\1/')
ifeq ($(UNAME),MINGW)
# Translate "C:/super" into "/C/super" for MinGW make.
SDK_PATH := /$(shell echo $(BASE_SDK_PATH) | sed s/://)
endif
SDK_PATH ?= $(BASE_SDK_PATH)
COPIED_SDK_PATH ?= gecko_sdk_4.1.0

# This uses the explicit build rules below
PROJECT_SOURCE_FILES =

C_SOURCE_FILES   += $(filter %.c, $(PROJECT_SOURCE_FILES))
CXX_SOURCE_FILES += $(filter %.cpp, $(PROJECT_SOURCE_FILES))
CXX_SOURCE_FILES += $(filter %.cc, $(PROJECT_SOURCE_FILES))
ASM_SOURCE_FILES += $(filter %.s, $(PROJECT_SOURCE_FILES))
ASM_SOURCE_FILES += $(filter %.S, $(PROJECT_SOURCE_FILES))
LIB_FILES        += $(filter %.a, $(PROJECT_SOURCE_FILES))

C_DEFS += \
 '-DEMBER_AF_API_EMBER_TYPES="../../../stack/include/ember-types.h"' \
 '-DEMBER_AF_API_EZSP="../../em260/command-context.h"' \
 '-DEMBER_AF_API_EZSP_PROTOCOL="../../util/ezsp/ezsp-protocol.h"' \
 '-DEMBER_AF_API_EZSP_SECURE="../../util/ezsp/secure-ezsp-protocol.h"' \
 '-DEMBER_AF_API_STACK="../../../stack/include/ember.h"' \
 '-DEMBER_AF_ZC_AND_ZR_DEVICE_COUNT=1' \
 '-DEMBER_APPLICATION_HAS_TRUST_CENTER_JOIN_HANDLER=1' \
 '-DSL_COMPONENT_CATALOG_PRESENT=1' \
 '-DSPINEL_PLATFORM_HEADER="spinel_platform.h"' \
 '-DMBEDTLS_CONFIG_FILE=<mbedtls_config.h>' \
 '-DOPENTHREAD_POSIX_CONFIG_RCP_BUS=OT_POSIX_RCP_BUS_CPC' \
 '-DUC_BUILD=1' \
 '-DEMBER_AF_NCP=1' \
 '-DEMBER_SERIAL1_RTSCTS=1' \
 '-DEZSP_UART=1' \
 '-DNO_USB=1' \
 '-DEMBER_MULTI_NETWORK_STRIPPED=1' \
 '-DAPP_BAUD_RATE=BAUD_115200' \
 '-DAPP_SERIAL=1' \
 '-DBOARD_SIMULATION=1' \
 '-DCONFIGURATION_HEADER="../../../zigbeed/zigbeed_configuration.h"' \
 '-DEMBER_NO_IDLE_SUPPORT=1' \
 '-DEMBER_TEST=1' \
 '-DEZSP_NO_BOOTLOADER=1' \
 '-DPHY_SIMULATION=1' \
 '-DPLATFORM_HEADER="platform-header.h"' \
 '-DRESERVED_AVAILABLE_MEMORY=8000' \
 '-DTEST_SUITE_MEMORY_BUFFER_ALLOC=1' \
 '-DUNIX=1' \
 '-DUNIX_HOST=1' \
 '-DZIGBEE_STACK_ON_HOST=1' \
 '-D__INLINE=inline'

ASM_DEFS += \
 '-DEMBER_AF_API_EMBER_TYPES="../../../stack/include/ember-types.h"' \
 '-DEMBER_AF_API_EZSP="../../em260/command-context.h"' \
 '-DEMBER_AF_API_EZSP_PROTOCOL="../../util/ezsp/ezsp-protocol.h"' \
 '-DEMBER_AF_API_EZSP_SECURE="../../util/ezsp/secure-ezsp-protocol.h"' \
 '-DEMBER_AF_API_STACK="../../../stack/include/ember.h"' \
 '-DEMBER_AF_ZC_AND_ZR_DEVICE_COUNT=1' \
 '-DEMBER_APPLICATION_HAS_TRUST_CENTER_JOIN_HANDLER=1' \
 '-DSL_COMPONENT_CATALOG_PRESENT=1' \
 '-DSPINEL_PLATFORM_HEADER="spinel_platform.h"' \
 '-DMBEDTLS_CONFIG_FILE=<mbedtls_config.h>' \
 '-DOPENTHREAD_POSIX_CONFIG_RCP_BUS=OT_POSIX_RCP_BUS_CPC' \
 '-DUC_BUILD=1' \
 '-DEMBER_AF_NCP=1' \
 '-DEMBER_SERIAL1_RTSCTS=1' \
 '-DEZSP_UART=1' \
 '-DNO_USB=1' \
 '-DEMBER_MULTI_NETWORK_STRIPPED=1' \
 '-DAPP_BAUD_RATE=BAUD_115200' \
 '-DAPP_SERIAL=1' \
 '-DBOARD_SIMULATION=1' \
 '-DCONFIGURATION_HEADER="../../../zigbeed/zigbeed_configuration.h"' \
 '-DEMBER_NO_IDLE_SUPPORT=1' \
 '-DEMBER_TEST=1' \
 '-DEZSP_NO_BOOTLOADER=1' \
 '-DPHY_SIMULATION=1' \
 '-DPLATFORM_HEADER="platform-header.h"' \
 '-DRESERVED_AVAILABLE_MEMORY=8000' \
 '-DTEST_SUITE_MEMORY_BUFFER_ALLOC=1' \
 '-DUNIX=1' \
 '-DUNIX_HOST=1' \
 '-DZIGBEE_STACK_ON_HOST=1' \
 '-D__INLINE=inline'

INCLUDES += \
 -Iconfig \
 -Iautogen \
 -I. \
 -I$(SDK_PATH)/platform/common/inc \
 -I$(SDK_PATH)/platform/service/legacy_common_ash/inc \
 -I$(SDK_PATH)/platform/service/legacy_ncp_ash/inc \
 -I$(SDK_PATH)/util/third_party/openthread/src/core \
 -I$(SDK_PATH)/util/third_party/openthread/include \
 -I$(SDK_PATH)/util/third_party/openthread/include/openthread \
 -I$(SDK_PATH)/util/third_party/openthread/third_party/tcplp \
 -I$(SDK_PATH)/util/third_party/openthread/src \
 -I$(SDK_PATH)/util/third_party/openthread/src/lib/spinel \
 -I$(SDK_PATH)/util/third_party/openthread/src/posix/platform/include \
 -I$(SDK_PATH)/util/third_party/openthread/src/posix/platform \
 -I$(SDK_PATH)/util/third_party/crypto/mbedtls/include \
 -I$(SDK_PATH)/platform/radio/mac \
 -I$(SDK_PATH)/util/third_party/crypto/mbedtls/library \
 -I$(SDK_PATH)/util/third_party/crypto/sl_component/sl_mbedtls_support/config \
 -I$(SDK_PATH)/util/silicon_labs/silabs_core/memory_manager \
 -I$(SDK_PATH)/platform/service/system/inc \
 -I$(CPC_DAEMON_LIB) \
 -I$(SDK_PATH)/protocol/zigbee/stack/platform/micro \
 -I$(SDK_PATH)/protocol/zigbee/stack/include \
 -I$(SDK_PATH)/protocol/zigbee/app/framework/common \
 -I$(SDK_PATH)/protocol/zigbee/app/em260 \
 -I$(SDK_PATH)/protocol/zigbee/stack/gp \
 -I$(SDK_PATH)/protocol/zigbee/app/xncp \
 -I$(SDK_PATH)/protocol/zigbee/app/util/ezsp \
 -I$(SDK_PATH)/protocol/zigbee/app/framework/util \
 -I$(SDK_PATH)/protocol/zigbee/app/util/security \
 -I$(SDK_PATH)/platform/radio/rail_lib/plugin \
 -I$(SDK_PATH)/protocol/zigbee \
 -I$(SDK_PATH)/protocol/zigbee/stack \
 -I$(SDK_PATH)/protocol/zigbee/stack/zigbee \
 -I$(SDK_PATH)/platform/radio/mac/rail_mux \
 -I$(SDK_PATH)/util/silicon_labs/silabs_core \
 -I$(SDK_PATH)/protocol/zigbee/stack/core \
 -I$(SDK_PATH)/protocol/zigbee/stack/mac \
 -I$(SDK_PATH)/. \
 -I$(SDK_PATH)/platform/Device/SiliconLabs/simulation/Include \
 -I$(SDK_PATH)/protocol/zigbee/stack/framework \
 -I$(SDK_PATH)/platform/service/legacy_host/inc \
 -I$(SDK_PATH)/platform/service/token_manager/inc \
 -I$(SDK_PATH)/platform/emlib/host/inc \
 -I$(SDK_PATH)/platform/service/legacy_hal/inc \
 -I$(SDK_PATH)/platform/radio/rail_lib/plugin/coexistence/protocol/ieee802154 \
 -I$(SDK_PATH)/platform/service/iostream/inc \
 -I$(SDK_PATH)/protocol/zigbee/app/zigbeed \
 -I$(SDK_PATH)/protocol/zigbee/app/ncp/plugin/xncp \
 -I$(SDK_PATH)/protocol/zigbee/stack/zll

GROUP_START =
GROUP_END =

PROJECT_LIBS = \
 -lcpc \
 -lc \
 -lm \
 -lutil \
 -lstdc++ \
 $(SDK_PATH)/protocol/zigbee/build/binding-table-library-unix-gcc-simulation-null-arm32v7-ember_multi_network_stripped/binding-table-library.a \
 $(SDK_PATH)/protocol/zigbee/build/ncp-binding-library-unix-gcc-simulation-null-arm32v7-ember_multi_network_stripped/ncp-binding-library.a \
 $(SDK_PATH)/protocol/zigbee/build/end-device-bind-library-unix-gcc-simulation-null-arm32v7-ember_multi_network_stripped/end-device-bind-library.a \
 $(SDK_PATH)/protocol/zigbee/build/gp-library-unix-gcc-simulation-null-arm32v7-ember_multi_network_stripped/gp-library.a \
 $(SDK_PATH)/protocol/zigbee/build/ncp-gp-library-unix-gcc-simulation-null-arm32v7/ncp-gp-library.a \
 $(SDK_PATH)/protocol/zigbee/build/install-code-library-unix-gcc-simulation-null-arm32v7-ember_multi_network_stripped/install-code-library.a \
 $(SDK_PATH)/protocol/zigbee/build/em260-uart-util-library-unix-gcc-simulation-null-arm32v7-ember_multi_network_stripped/em260-uart-util-library.a \
 $(SDK_PATH)/protocol/zigbee/build/packet-validate-library-unix-gcc-simulation-null-arm32v7/packet-validate-library.a \
 $(SDK_PATH)/protocol/zigbee/build/zigbee-pro-stack-unix-gcc-simulation-null-arm32v7-ember_multi_network_stripped/zigbee-pro-stack.a \
 $(SDK_PATH)/protocol/zigbee/build/ncp-pro-library-unix-gcc-simulation-null-arm32v7-ember_multi_network_stripped/ncp-pro-library.a \
 $(SDK_PATH)/protocol/zigbee/build/zigbee-r22-support-library-unix-gcc-simulation-null-arm32v7-ember_multi_network_stripped/zigbee-r22-support-library.a \
 $(SDK_PATH)/protocol/zigbee/build/security-library-link-keys-unix-gcc-simulation-null-arm32v7-ember_multi_network_stripped/security-library-link-keys.a \
 $(SDK_PATH)/protocol/zigbee/build/source-route-library-unix-gcc-simulation-null-arm32v7-ember_multi_network_stripped/source-route-library.a \
 $(SDK_PATH)/protocol/zigbee/build/ncp-source-route-library-unix-gcc-simulation-null-arm32v7/ncp-source-route-library.a \
 $(SDK_PATH)/protocol/zigbee/build/em260-xncp-library-unix-gcc-simulation-null-arm32v7/em260-xncp-library.a \
 $(SDK_PATH)/protocol/zigbee/build/zll-library-unix-gcc-simulation-null-arm32v7-ember_multi_network_stripped/zll-library.a \
 $(SDK_PATH)/protocol/zigbee/build/ncp-zll-library-unix-gcc-simulation-null-arm32v7/ncp-zll-library.a

LIBS += $(GROUP_START) $(PROJECT_LIBS) $(GROUP_END)

LIB_FILES += $(filter %.a, $(PROJECT_LIBS))

C_FLAGS += \
 -std=c99 \
 -Wall \
 -Wextra \
 -O0 \
 -imacros lower-mac-spinel-config.h \
 -Wno-unused-parameter \
 -Wno-missing-field-initializers \
 -Wno-missing-braces \
 -Wno-format \
 -Werror \
 -Wno-error=implicit-function-declaration

CXX_FLAGS += \
 -std=c++11 \
 -Wall \
 -Wextra \
 -O0 \
 -imacros lower-mac-spinel-config.h \
 -Wno-unused-parameter \
 -Wno-missing-field-initializers \
 -Wno-missing-braces \
 -Wno-format \
 -Werror \
 -Wno-error=implicit-function-declaration

ASM_FLAGS += \
 -imacros lower-mac-spinel-config.h

LD_FLAGS += \


####################################################################
# SDK Build Rules                                                  #
####################################################################
$(OUTPUT_DIR)/sdk/platform/service/legacy_common_ash/src/ash-common.o: $(SDK_PATH)/platform/service/legacy_common_ash/src/ash-common.c
	@echo 'Building $(SDK_PATH)/platform/service/legacy_common_ash/src/ash-common.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/legacy_common_ash/src/ash-common.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/legacy_common_ash/src/ash-common.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/legacy_common_ash/src/ash-common.o

$(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/crc.o: $(SDK_PATH)/platform/service/legacy_hal/src/crc.c
	@echo 'Building $(SDK_PATH)/platform/service/legacy_hal/src/crc.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/legacy_hal/src/crc.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/crc.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/crc.o

$(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/random.o: $(SDK_PATH)/platform/service/legacy_hal/src/random.c
	@echo 'Building $(SDK_PATH)/platform/service/legacy_hal/src/random.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/legacy_hal/src/random.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/random.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/random.o

$(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/rijndael-alg-fst.o: $(SDK_PATH)/platform/service/legacy_hal/src/rijndael-alg-fst.c
	@echo 'Building $(SDK_PATH)/platform/service/legacy_hal/src/rijndael-alg-fst.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/legacy_hal/src/rijndael-alg-fst.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/rijndael-alg-fst.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/rijndael-alg-fst.o

$(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/rijndael-api-fst.o: $(SDK_PATH)/platform/service/legacy_hal/src/rijndael-api-fst.c
	@echo 'Building $(SDK_PATH)/platform/service/legacy_hal/src/rijndael-api-fst.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/legacy_hal/src/rijndael-api-fst.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/rijndael-api-fst.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/rijndael-api-fst.o

$(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/system-timer.o: $(SDK_PATH)/platform/service/legacy_hal/src/system-timer.c
	@echo 'Building $(SDK_PATH)/platform/service/legacy_hal/src/system-timer.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/legacy_hal/src/system-timer.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/system-timer.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/system-timer.o

$(OUTPUT_DIR)/sdk/platform/service/legacy_host/src/token.o: $(SDK_PATH)/platform/service/legacy_host/src/token.c
	@echo 'Building $(SDK_PATH)/platform/service/legacy_host/src/token.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/legacy_host/src/token.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/legacy_host/src/token.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/legacy_host/src/token.o

$(OUTPUT_DIR)/sdk/platform/service/legacy_ncp_ash/src/ash-ncp.o: $(SDK_PATH)/platform/service/legacy_ncp_ash/src/ash-ncp.c
	@echo 'Building $(SDK_PATH)/platform/service/legacy_ncp_ash/src/ash-ncp.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/legacy_ncp_ash/src/ash-ncp.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/legacy_ncp_ash/src/ash-ncp.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/legacy_ncp_ash/src/ash-ncp.o

$(OUTPUT_DIR)/sdk/platform/service/system/src/sl_system_init.o: $(SDK_PATH)/platform/service/system/src/sl_system_init.c
	@echo 'Building $(SDK_PATH)/platform/service/system/src/sl_system_init.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/system/src/sl_system_init.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/system/src/sl_system_init.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/system/src/sl_system_init.o

$(OUTPUT_DIR)/sdk/platform/service/system/src/sl_system_process_action.o: $(SDK_PATH)/platform/service/system/src/sl_system_process_action.c
	@echo 'Building $(SDK_PATH)/platform/service/system/src/sl_system_process_action.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/system/src/sl_system_process_action.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/system/src/sl_system_process_action.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/system/src/sl_system_process_action.o

$(OUTPUT_DIR)/sdk/platform/service/token_manager/src/sl_token_def.o: $(SDK_PATH)/platform/service/token_manager/src/sl_token_def.c
	@echo 'Building $(SDK_PATH)/platform/service/token_manager/src/sl_token_def.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/token_manager/src/sl_token_def.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/token_manager/src/sl_token_def.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/token_manager/src/sl_token_def.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/command-handlers-binding-generated.o: $(SDK_PATH)/protocol/zigbee/app/em260/command-handlers-binding-generated.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/em260/command-handlers-binding-generated.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/em260/command-handlers-binding-generated.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/command-handlers-binding-generated.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/command-handlers-binding-generated.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/command-handlers-green-power-generated.o: $(SDK_PATH)/protocol/zigbee/app/em260/command-handlers-green-power-generated.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/em260/command-handlers-green-power-generated.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/em260/command-handlers-green-power-generated.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/command-handlers-green-power-generated.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/command-handlers-green-power-generated.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/command-handlers-messaging-generated.o: $(SDK_PATH)/protocol/zigbee/app/em260/command-handlers-messaging-generated.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/em260/command-handlers-messaging-generated.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/em260/command-handlers-messaging-generated.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/command-handlers-messaging-generated.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/command-handlers-messaging-generated.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/command-handlers-networking-generated.o: $(SDK_PATH)/protocol/zigbee/app/em260/command-handlers-networking-generated.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/em260/command-handlers-networking-generated.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/em260/command-handlers-networking-generated.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/command-handlers-networking-generated.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/command-handlers-networking-generated.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/command-handlers-secure-ezsp-stub.o: $(SDK_PATH)/protocol/zigbee/app/em260/command-handlers-secure-ezsp-stub.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/em260/command-handlers-secure-ezsp-stub.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/em260/command-handlers-secure-ezsp-stub.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/command-handlers-secure-ezsp-stub.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/command-handlers-secure-ezsp-stub.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/command-handlers-security-generated.o: $(SDK_PATH)/protocol/zigbee/app/em260/command-handlers-security-generated.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/em260/command-handlers-security-generated.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/em260/command-handlers-security-generated.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/command-handlers-security-generated.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/command-handlers-security-generated.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/command-handlers-token-interface-generated.o: $(SDK_PATH)/protocol/zigbee/app/em260/command-handlers-token-interface-generated.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/em260/command-handlers-token-interface-generated.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/em260/command-handlers-token-interface-generated.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/command-handlers-token-interface-generated.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/command-handlers-token-interface-generated.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/command-handlers-token-interface.o: $(SDK_PATH)/protocol/zigbee/app/em260/command-handlers-token-interface.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/em260/command-handlers-token-interface.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/em260/command-handlers-token-interface.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/command-handlers-token-interface.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/command-handlers-token-interface.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/command-handlers-trust-center-generated.o: $(SDK_PATH)/protocol/zigbee/app/em260/command-handlers-trust-center-generated.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/em260/command-handlers-trust-center-generated.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/em260/command-handlers-trust-center-generated.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/command-handlers-trust-center-generated.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/command-handlers-trust-center-generated.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/command-handlers-zll-generated.o: $(SDK_PATH)/protocol/zigbee/app/em260/command-handlers-zll-generated.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/em260/command-handlers-zll-generated.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/em260/command-handlers-zll-generated.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/command-handlers-zll-generated.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/command-handlers-zll-generated.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/em260-common.o: $(SDK_PATH)/protocol/zigbee/app/em260/em260-common.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/em260/em260-common.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/em260/em260-common.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/em260-common.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/em260-common.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/ncp-stack-stub.o: $(SDK_PATH)/protocol/zigbee/app/em260/ncp-stack-stub.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/em260/ncp-stack-stub.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/em260/ncp-stack-stub.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/ncp-stack-stub.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/ncp-stack-stub.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/serial-interface-uart.o: $(SDK_PATH)/protocol/zigbee/app/em260/serial-interface-uart.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/em260/serial-interface-uart.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/em260/serial-interface-uart.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/serial-interface-uart.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/serial-interface-uart.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/common/zigbee_app_framework_common.o: $(SDK_PATH)/protocol/zigbee/app/framework/common/zigbee_app_framework_common.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/common/zigbee_app_framework_common.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/common/zigbee_app_framework_common.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/common/zigbee_app_framework_common.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/common/zigbee_app_framework_common.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/common/zigbee_app_framework_sleep.o: $(SDK_PATH)/protocol/zigbee/app/framework/common/zigbee_app_framework_sleep.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/common/zigbee_app_framework_sleep.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/common/zigbee_app_framework_sleep.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/common/zigbee_app_framework_sleep.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/common/zigbee_app_framework_sleep.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/common/zigbee_app_framework_stack_cb.o: $(SDK_PATH)/protocol/zigbee/app/framework/common/zigbee_app_framework_stack_cb.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/common/zigbee_app_framework_stack_cb.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/common/zigbee_app_framework_stack_cb.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/common/zigbee_app_framework_stack_cb.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/common/zigbee_app_framework_stack_cb.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/common/zigbee_ncp_framework_cb.o: $(SDK_PATH)/protocol/zigbee/app/framework/common/zigbee_ncp_framework_cb.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/common/zigbee_ncp_framework_cb.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/common/zigbee_ncp_framework_cb.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/common/zigbee_ncp_framework_cb.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/common/zigbee_ncp_framework_cb.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/common/zigbee_token_interface.o: $(SDK_PATH)/protocol/zigbee/app/framework/common/zigbee_token_interface.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/common/zigbee_token_interface.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/common/zigbee_token_interface.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/common/zigbee_token_interface.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/common/zigbee_token_interface.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/common/zigbee_token_interface_stub.o: $(SDK_PATH)/protocol/zigbee/app/framework/common/zigbee_token_interface_stub.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/common/zigbee_token_interface_stub.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/common/zigbee_token_interface_stub.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/common/zigbee_token_interface_stub.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/common/zigbee_token_interface_stub.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/ezsp/secure-ezsp-stub.o: $(SDK_PATH)/protocol/zigbee/app/util/ezsp/secure-ezsp-stub.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/util/ezsp/secure-ezsp-stub.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/util/ezsp/secure-ezsp-stub.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/ezsp/secure-ezsp-stub.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/ezsp/secure-ezsp-stub.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/security/security-address-cache.o: $(SDK_PATH)/protocol/zigbee/app/util/security/security-address-cache.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/util/security/security-address-cache.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/util/security/security-address-cache.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/security/security-address-cache.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/security/security-address-cache.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/xncp/xncp_cb.o: $(SDK_PATH)/protocol/zigbee/app/xncp/xncp_cb.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/xncp/xncp_cb.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/xncp/xncp_cb.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/xncp/xncp_cb.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/xncp/xncp_cb.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/zigbeed/coexistence-802154-stubs.o: $(SDK_PATH)/protocol/zigbee/app/zigbeed/coexistence-802154-stubs.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/zigbeed/coexistence-802154-stubs.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/zigbeed/coexistence-802154-stubs.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/zigbeed/coexistence-802154-stubs.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/zigbeed/coexistence-802154-stubs.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/zigbeed/micro-stubs.o: $(SDK_PATH)/protocol/zigbee/app/zigbeed/micro-stubs.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/zigbeed/micro-stubs.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/zigbeed/micro-stubs.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/zigbeed/micro-stubs.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/zigbeed/micro-stubs.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/zigbeed/serial_adapter.o: $(SDK_PATH)/protocol/zigbee/app/zigbeed/serial_adapter.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/zigbeed/serial_adapter.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/zigbeed/serial_adapter.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/zigbeed/serial_adapter.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/zigbeed/serial_adapter.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/zigbeed/simulator_stubs.o: $(SDK_PATH)/protocol/zigbee/app/zigbeed/simulator_stubs.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/zigbeed/simulator_stubs.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/zigbeed/simulator_stubs.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/zigbeed/simulator_stubs.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/zigbeed/simulator_stubs.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/stack/config/ember-configuration.o: $(SDK_PATH)/protocol/zigbee/stack/config/ember-configuration.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/stack/config/ember-configuration.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/stack/config/ember-configuration.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/config/ember-configuration.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/config/ember-configuration.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/stack/core/ember-multi-network-stub.o: $(SDK_PATH)/protocol/zigbee/stack/core/ember-multi-network-stub.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/stack/core/ember-multi-network-stub.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/stack/core/ember-multi-network-stub.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/core/ember-multi-network-stub.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/core/ember-multi-network-stub.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/stack/core/multi-pan-common.o: $(SDK_PATH)/protocol/zigbee/stack/core/multi-pan-common.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/stack/core/multi-pan-common.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/stack/core/multi-pan-common.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/core/multi-pan-common.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/core/multi-pan-common.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/stack/core/multi-pan-stub.o: $(SDK_PATH)/protocol/zigbee/stack/core/multi-pan-stub.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/stack/core/multi-pan-stub.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/stack/core/multi-pan-stub.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/core/multi-pan-stub.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/core/multi-pan-stub.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/stack/framework/aes-ecb.o: $(SDK_PATH)/protocol/zigbee/stack/framework/aes-ecb.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/stack/framework/aes-ecb.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/stack/framework/aes-ecb.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/framework/aes-ecb.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/framework/aes-ecb.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/stack/framework/ccm-star.o: $(SDK_PATH)/protocol/zigbee/stack/framework/ccm-star.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/stack/framework/ccm-star.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/stack/framework/ccm-star.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/framework/ccm-star.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/framework/ccm-star.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/stack/framework/debug-extended-stub.o: $(SDK_PATH)/protocol/zigbee/stack/framework/debug-extended-stub.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/stack/framework/debug-extended-stub.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/stack/framework/debug-extended-stub.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/framework/debug-extended-stub.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/framework/debug-extended-stub.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/stack/framework/debug-stub.o: $(SDK_PATH)/protocol/zigbee/stack/framework/debug-stub.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/stack/framework/debug-stub.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/stack/framework/debug-stub.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/framework/debug-stub.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/framework/debug-stub.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/stack/framework/strong-random-api.o: $(SDK_PATH)/protocol/zigbee/stack/framework/strong-random-api.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/stack/framework/strong-random-api.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/stack/framework/strong-random-api.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/framework/strong-random-api.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/framework/strong-random-api.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/stack/framework/zigbee-event-logger-stub-gen.o: $(SDK_PATH)/protocol/zigbee/stack/framework/zigbee-event-logger-stub-gen.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/stack/framework/zigbee-event-logger-stub-gen.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/stack/framework/zigbee-event-logger-stub-gen.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/framework/zigbee-event-logger-stub-gen.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/framework/zigbee-event-logger-stub-gen.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/stack/security/cbke-crypto-engine-163k1-stub.o: $(SDK_PATH)/protocol/zigbee/stack/security/cbke-crypto-engine-163k1-stub.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/stack/security/cbke-crypto-engine-163k1-stub.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/stack/security/cbke-crypto-engine-163k1-stub.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/security/cbke-crypto-engine-163k1-stub.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/security/cbke-crypto-engine-163k1-stub.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/stack/security/cbke-crypto-engine-283k1-stub.o: $(SDK_PATH)/protocol/zigbee/stack/security/cbke-crypto-engine-283k1-stub.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/stack/security/cbke-crypto-engine-283k1-stub.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/stack/security/cbke-crypto-engine-283k1-stub.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/security/cbke-crypto-engine-283k1-stub.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/security/cbke-crypto-engine-283k1-stub.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/stack/security/cbke-crypto-engine-dsa-sign-stub.o: $(SDK_PATH)/protocol/zigbee/stack/security/cbke-crypto-engine-dsa-sign-stub.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/stack/security/cbke-crypto-engine-dsa-sign-stub.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/stack/security/cbke-crypto-engine-dsa-sign-stub.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/security/cbke-crypto-engine-dsa-sign-stub.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/security/cbke-crypto-engine-dsa-sign-stub.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/stack/security/cbke-crypto-engine-dsa-verify-283k1-stub.o: $(SDK_PATH)/protocol/zigbee/stack/security/cbke-crypto-engine-dsa-verify-283k1-stub.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/stack/security/cbke-crypto-engine-dsa-verify-283k1-stub.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/stack/security/cbke-crypto-engine-dsa-verify-283k1-stub.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/security/cbke-crypto-engine-dsa-verify-283k1-stub.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/security/cbke-crypto-engine-dsa-verify-283k1-stub.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/stack/security/cbke-crypto-engine-dsa-verify-stub.o: $(SDK_PATH)/protocol/zigbee/stack/security/cbke-crypto-engine-dsa-verify-stub.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/stack/security/cbke-crypto-engine-dsa-verify-stub.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/stack/security/cbke-crypto-engine-dsa-verify-stub.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/security/cbke-crypto-engine-dsa-verify-stub.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/security/cbke-crypto-engine-dsa-verify-stub.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/stack/security/cbke-crypto-engine-stub.o: $(SDK_PATH)/protocol/zigbee/stack/security/cbke-crypto-engine-stub.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/stack/security/cbke-crypto-engine-stub.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/stack/security/cbke-crypto-engine-stub.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/security/cbke-crypto-engine-stub.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/security/cbke-crypto-engine-stub.o

$(OUTPUT_DIR)/sdk/util/silicon_labs/silabs_core/memory_manager/sl_malloc.o: $(SDK_PATH)/util/silicon_labs/silabs_core/memory_manager/sl_malloc.c
	@echo 'Building $(SDK_PATH)/util/silicon_labs/silabs_core/memory_manager/sl_malloc.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/silicon_labs/silabs_core/memory_manager/sl_malloc.c
CDEPS += $(OUTPUT_DIR)/sdk/util/silicon_labs/silabs_core/memory_manager/sl_malloc.d
OBJS += $(OUTPUT_DIR)/sdk/util/silicon_labs/silabs_core/memory_manager/sl_malloc.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/aes.o: $(SDK_PATH)/util/third_party/crypto/mbedtls/library/aes.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/mbedtls/library/aes.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/mbedtls/library/aes.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/aes.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/aes.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/constant_time.o: $(SDK_PATH)/util/third_party/crypto/mbedtls/library/constant_time.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/mbedtls/library/constant_time.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/mbedtls/library/constant_time.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/constant_time.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/constant_time.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/ctr_drbg.o: $(SDK_PATH)/util/third_party/crypto/mbedtls/library/ctr_drbg.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/mbedtls/library/ctr_drbg.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/mbedtls/library/ctr_drbg.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/ctr_drbg.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/ctr_drbg.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/entropy.o: $(SDK_PATH)/util/third_party/crypto/mbedtls/library/entropy.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/mbedtls/library/entropy.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/mbedtls/library/entropy.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/entropy.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/entropy.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/entropy_poll.o: $(SDK_PATH)/util/third_party/crypto/mbedtls/library/entropy_poll.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/mbedtls/library/entropy_poll.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/mbedtls/library/entropy_poll.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/entropy_poll.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/entropy_poll.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/hmac_drbg.o: $(SDK_PATH)/util/third_party/crypto/mbedtls/library/hmac_drbg.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/mbedtls/library/hmac_drbg.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/mbedtls/library/hmac_drbg.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/hmac_drbg.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/hmac_drbg.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/platform.o: $(SDK_PATH)/util/third_party/crypto/mbedtls/library/platform.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/mbedtls/library/platform.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/mbedtls/library/platform.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/platform.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/platform.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/platform_util.o: $(SDK_PATH)/util/third_party/crypto/mbedtls/library/platform_util.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/mbedtls/library/platform_util.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/mbedtls/library/platform_util.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/platform_util.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/platform_util.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/sha256.o: $(SDK_PATH)/util/third_party/crypto/mbedtls/library/sha256.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/mbedtls/library/sha256.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/mbedtls/library/sha256.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/sha256.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/sha256.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/threading.o: $(SDK_PATH)/util/third_party/crypto/mbedtls/library/threading.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/mbedtls/library/threading.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/mbedtls/library/threading.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/threading.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/threading.o

$(OUTPUT_DIR)/sdk/util/third_party/openthread/src/core/common/log.o: $(SDK_PATH)/util/third_party/openthread/src/core/common/log.cpp
	@echo 'Building $(SDK_PATH)/util/third_party/openthread/src/core/common/log.cpp'
	@mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/openthread/src/core/common/log.cpp
CXXDEPS += $(OUTPUT_DIR)/sdk/util/third_party/openthread/src/core/common/log.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/openthread/src/core/common/log.o

$(OUTPUT_DIR)/sdk/util/third_party/openthread/src/core/crypto/mbedtls.o: $(SDK_PATH)/util/third_party/openthread/src/core/crypto/mbedtls.cpp
	@echo 'Building $(SDK_PATH)/util/third_party/openthread/src/core/crypto/mbedtls.cpp'
	@mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/openthread/src/core/crypto/mbedtls.cpp
CXXDEPS += $(OUTPUT_DIR)/sdk/util/third_party/openthread/src/core/crypto/mbedtls.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/openthread/src/core/crypto/mbedtls.o

$(OUTPUT_DIR)/sdk/util/third_party/openthread/src/posix/platform/cpc_interface.o: $(SDK_PATH)/util/third_party/openthread/src/posix/platform/cpc_interface.cpp
	@echo 'Building $(SDK_PATH)/util/third_party/openthread/src/posix/platform/cpc_interface.cpp'
	@mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/openthread/src/posix/platform/cpc_interface.cpp
CXXDEPS += $(OUTPUT_DIR)/sdk/util/third_party/openthread/src/posix/platform/cpc_interface.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/openthread/src/posix/platform/cpc_interface.o

$(OUTPUT_DIR)/project/app.o: app.c
	@echo 'Building app.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ app.c
CDEPS += $(OUTPUT_DIR)/project/app.d
OBJS += $(OUTPUT_DIR)/project/app.o

$(OUTPUT_DIR)/project/autogen/sl_event_handler.o: autogen/sl_event_handler.c
	@echo 'Building autogen/sl_event_handler.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ autogen/sl_event_handler.c
CDEPS += $(OUTPUT_DIR)/project/autogen/sl_event_handler.d
OBJS += $(OUTPUT_DIR)/project/autogen/sl_event_handler.o

$(OUTPUT_DIR)/project/autogen/zigbee_common_callback_dispatcher.o: autogen/zigbee_common_callback_dispatcher.c
	@echo 'Building autogen/zigbee_common_callback_dispatcher.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ autogen/zigbee_common_callback_dispatcher.c
CDEPS += $(OUTPUT_DIR)/project/autogen/zigbee_common_callback_dispatcher.d
OBJS += $(OUTPUT_DIR)/project/autogen/zigbee_common_callback_dispatcher.o

$(OUTPUT_DIR)/project/autogen/zigbee_ncp_callback_dispatcher.o: autogen/zigbee_ncp_callback_dispatcher.c
	@echo 'Building autogen/zigbee_ncp_callback_dispatcher.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ autogen/zigbee_ncp_callback_dispatcher.c
CDEPS += $(OUTPUT_DIR)/project/autogen/zigbee_ncp_callback_dispatcher.d
OBJS += $(OUTPUT_DIR)/project/autogen/zigbee_ncp_callback_dispatcher.o

$(OUTPUT_DIR)/project/autogen/zigbee_stack_callback_dispatcher.o: autogen/zigbee_stack_callback_dispatcher.c
	@echo 'Building autogen/zigbee_stack_callback_dispatcher.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ autogen/zigbee_stack_callback_dispatcher.c
CDEPS += $(OUTPUT_DIR)/project/autogen/zigbee_stack_callback_dispatcher.d
OBJS += $(OUTPUT_DIR)/project/autogen/zigbee_stack_callback_dispatcher.o

$(OUTPUT_DIR)/project/main.o: main.c
	@echo 'Building main.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ main.c
CDEPS += $(OUTPUT_DIR)/project/main.d
OBJS += $(OUTPUT_DIR)/project/main.o

$(OUTPUT_DIR)/project/zigbeed_custom_ezsp_commands.o: zigbeed_custom_ezsp_commands.c
	@echo 'Building zigbeed_custom_ezsp_commands.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ zigbeed_custom_ezsp_commands.c
CDEPS += $(OUTPUT_DIR)/project/zigbeed_custom_ezsp_commands.d
OBJS += $(OUTPUT_DIR)/project/zigbeed_custom_ezsp_commands.o

# Automatically-generated Simplicity Studio Metadata
# Please do not edit or delete these lines!
# SIMPLICITY_STUDIO_METADATA=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=END_SIMPLICITY_STUDIO_METADATA
# END OF METADATA
