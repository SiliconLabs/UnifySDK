####################################################################
# Automatically-generated file. Do not edit!                       #
# Makefile Version 10                                              #
####################################################################

BASE_SDK_PATH = $(GSDK_LOCATION)
UNAME:=$(shell uname -s | sed -e 's/^\(CYGWIN\).*/\1/' | sed -e 's/^\(MINGW\).*/\1/')
ifeq ($(UNAME),MINGW)
# Translate "C:/super" into "/C/super" for MinGW make.
SDK_PATH := /$(shell echo $(BASE_SDK_PATH) | sed s/://)
endif
SDK_PATH ?= $(BASE_SDK_PATH)
COPIED_SDK_PATH ?= gecko_sdk_4.0.2

# This uses the explicit build rules below
PROJECT_SOURCE_FILES =

C_SOURCE_FILES   += $(filter %.c, $(PROJECT_SOURCE_FILES))
CXX_SOURCE_FILES += $(filter %.cpp, $(PROJECT_SOURCE_FILES))
CXX_SOURCE_FILES += $(filter %.cc, $(PROJECT_SOURCE_FILES))
ASM_SOURCE_FILES += $(filter %.s, $(PROJECT_SOURCE_FILES))
ASM_SOURCE_FILES += $(filter %.S, $(PROJECT_SOURCE_FILES))
LIB_FILES        += $(filter %.a, $(PROJECT_SOURCE_FILES))

C_DEFS += \
 '-DEMBER_AF_PLUGIN_GATEWAY_SUPPORT_MAX_WAIT_FOR_EVENTS_TIMEOUT_MS=10' \
 '-DEMBER_AF_PLUGIN_POLL_CONTROL_CLIENT_DEFAULT_FAST_POLL_TIMEOUT=32' \
 '-DSL_COMPONENT_CATALOG_PRESENT=1' \
 '-DPLATFORM_HEADER="platform-header.h"' \
 '-DEZSP_HOST=1' \
 '-DGATEWAY_APP=1' \
 '-DUC_BUILD=1' \
 '-DEZSP_ASH=1' \
 '-DEZSP_UART=1' \
 '-DUSE_ZAP_CONFIG=1' \
 '-DCONFIGURATION_HEADER="app/framework/util/config.h"'

ASM_DEFS += \
 '-DEMBER_AF_PLUGIN_GATEWAY_SUPPORT_MAX_WAIT_FOR_EVENTS_TIMEOUT_MS=10' \
 '-DEMBER_AF_PLUGIN_POLL_CONTROL_CLIENT_DEFAULT_FAST_POLL_TIMEOUT=32' \
 '-DSL_COMPONENT_CATALOG_PRESENT=1' \
 '-DPLATFORM_HEADER="platform-header.h"' \
 '-DEZSP_HOST=1' \
 '-DGATEWAY_APP=1' \
 '-DUC_BUILD=1' \
 '-DEZSP_ASH=1' \
 '-DEZSP_UART=1' \
 '-DUSE_ZAP_CONFIG=1' \
 '-DCONFIGURATION_HEADER="app/framework/util/config.h"'

INCLUDES += \
 -Iconfig \
 -Iautogen \
 -I../src \
 -I../include \
 -I$(SDK_PATH)/platform/CMSIS/Core/Include \
 -I$(SDK_PATH)/platform/emlib/inc \
 -I$(SDK_PATH)/platform/emlib/host/inc \
 -I$(SDK_PATH)/platform/common/inc \
 -I$(SDK_PATH)/platform/service/iostream/inc \
 -I$(SDK_PATH)/platform/service/legacy_common_ash/inc \
 -I$(SDK_PATH)/platform/service/legacy_hal/inc \
 -I$(SDK_PATH)/util/third_party/printf \
 -I$(SDK_PATH)/util/third_party/printf/inc \
 -I$(SDK_PATH)/util/silicon_labs/silabs_core \
 -I$(SDK_PATH)/util/silicon_labs/silabs_core/event_queue \
 -I$(SDK_PATH)/util/silicon_labs/silabs_core/memory_manager \
 -I$(SDK_PATH)/platform/service/system/inc \
 -I$(SDK_PATH)/protocol/zigbee/app/framework/plugin/address-table \
 -I$(SDK_PATH)/protocol/zigbee/app/framework/plugin/basic \
 -I$(SDK_PATH)/protocol/zigbee/app/framework/service-function \
 -I$(SDK_PATH)/protocol/zigbee/app/framework/plugin/concentrator \
 -I$(SDK_PATH)/protocol/zigbee/app/framework/plugin/counters \
 -I$(SDK_PATH)/protocol/zigbee/app/framework/plugin/debug-print \
 -I$(SDK_PATH)/. \
 -I$(SDK_PATH)/protocol/zigbee \
 -I$(SDK_PATH)/protocol/zigbee/app/ezsp-host \
 -I$(SDK_PATH)/protocol/zigbee/app/util/gateway \
 -I$(SDK_PATH)/protocol/zigbee/app/util/ezsp \
 -I$(SDK_PATH)/protocol/zigbee/app/util/zigbee-framework \
 -I$(SDK_PATH)/protocol/zigbee/app/util/serial \
 -I$(SDK_PATH)/protocol/zigbee/stack \
 -I$(SDK_PATH)/protocol/zigbee/stack/platform/host \
 -I$(SDK_PATH)/protocol/zigbee/stack/include \
 -I$(SDK_PATH)/platform/radio/mac \
 -I$(SDK_PATH)/platform/service/token_manager/inc \
 -I$(SDK_PATH)/protocol/zigbee/app/ezsp-host/ash \
 -I$(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/file-descriptor-dispatch \
 -I$(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/gateway \
 -I$(SDK_PATH)/protocol/zigbee/app/framework/common \
 -I$(SDK_PATH)/platform/service/legacy_host/inc \
 -I$(SDK_PATH)/protocol/zigbee/stack/config \
 -I$(SDK_PATH)/protocol/zigbee/stack/zll \
 -I$(SDK_PATH)/protocol/zigbee/stack/core \
 -I$(SDK_PATH)/platform/service/legacy_printf/inc \
 -I$(SDK_PATH)/protocol/zigbee/app/framework/plugin/network-creator \
 -I$(SDK_PATH)/protocol/zigbee/app/framework/plugin/network-creator-security \
 -I$(SDK_PATH)/protocol/zigbee/app/framework/plugin/network-steering \
 -I$(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-common \
 -I$(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-server-policy \
 -I$(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-server \
 -I$(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-storage-simple \
 -I$(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-storage-common \
 -I$(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-storage-posix-filesystem \
 -I$(SDK_PATH)/protocol/zigbee/app/framework/plugin/reporting \
 -I$(SDK_PATH)/protocol/zigbee/app/framework/plugin/scan-dispatch \
 -I$(SDK_PATH)/protocol/zigbee/app/framework/signature-decode \
 -I$(SDK_PATH)/protocol/zigbee/app/framework/plugin/trust-center-nwk-key-update-unicast \
 -I$(SDK_PATH)/protocol/zigbee/app/framework/plugin/update-tc-link-key \
 -I$(SDK_PATH)/protocol/zigbee/app/framework/include \
 -I$(SDK_PATH)/protocol/zigbee/app/framework/util \
 -I$(SDK_PATH)/protocol/zigbee/app/framework/security \
 -I$(SDK_PATH)/protocol/zigbee/app/util/counters \
 -I$(SDK_PATH)/protocol/zigbee/app/framework/cli \
 -I$(SDK_PATH)/protocol/zigbee/app/util/common \
 -I$(SDK_PATH)/protocol/zigbee/app/util/security \
 -I$(SDK_PATH)/protocol/zigbee/app/framework/plugin/zcl-framework-core

GROUP_START =
GROUP_END =

PROJECT_LIBS = \
 -lc \
 -lm

LIBS += $(GROUP_START) $(PROJECT_LIBS) $(GROUP_END)

LIB_FILES += $(filter %.a, $(PROJECT_LIBS))

C_FLAGS += \
 -std=c99 \
 -Wall \
 -Wextra \
 -Os \
 -std=c11 \
 -Wno-unused-parameter \
 -Wno-missing-field-initializers \
 -Wno-missing-braces \
 -Wno-format

CXX_FLAGS += \
 -std=c++11 \
 -Wall \
 -Wextra \
 -Os \
 -std=c++11 \
 -Wno-unused-parameter \
 -Wno-missing-field-initializers \
 -Wno-missing-braces \
 -Wno-format

ASM_FLAGS += \

LD_FLAGS += \


####################################################################
# SDK Build Rules                                                  #
####################################################################
$(OUTPUT_DIR)/sdk/platform/common/src/sl_assert.o: $(SDK_PATH)/platform/common/src/sl_assert.c
	@echo 'Building $(SDK_PATH)/platform/common/src/sl_assert.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/common/src/sl_assert.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/common/src/sl_assert.d
OBJS += $(OUTPUT_DIR)/sdk/platform/common/src/sl_assert.o

$(OUTPUT_DIR)/sdk/platform/service/iostream/src/sl_iostream.o: $(SDK_PATH)/platform/service/iostream/src/sl_iostream.c
	@echo 'Building $(SDK_PATH)/platform/service/iostream/src/sl_iostream.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/iostream/src/sl_iostream.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/iostream/src/sl_iostream.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/iostream/src/sl_iostream.o

$(OUTPUT_DIR)/sdk/platform/service/iostream/src/sl_iostream_stdio.o: $(SDK_PATH)/platform/service/iostream/src/sl_iostream_stdio.c
	@echo 'Building $(SDK_PATH)/platform/service/iostream/src/sl_iostream_stdio.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/iostream/src/sl_iostream_stdio.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/iostream/src/sl_iostream_stdio.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/iostream/src/sl_iostream_stdio.o

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

$(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/ember-printf-convert.o: $(SDK_PATH)/platform/service/legacy_hal/src/ember-printf-convert.c
	@echo 'Building $(SDK_PATH)/platform/service/legacy_hal/src/ember-printf-convert.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/legacy_hal/src/ember-printf-convert.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/ember-printf-convert.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/ember-printf-convert.o

$(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/micro_host.o: $(SDK_PATH)/platform/service/legacy_hal/src/micro_host.c
	@echo 'Building $(SDK_PATH)/platform/service/legacy_hal/src/micro_host.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/legacy_hal/src/micro_host.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/micro_host.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/micro_host.o

$(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/random.o: $(SDK_PATH)/platform/service/legacy_hal/src/random.c
	@echo 'Building $(SDK_PATH)/platform/service/legacy_hal/src/random.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/legacy_hal/src/random.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/random.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/random.o

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

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/ezsp-host/ash/ash-host-ui.o: $(SDK_PATH)/protocol/zigbee/app/ezsp-host/ash/ash-host-ui.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/ezsp-host/ash/ash-host-ui.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/ezsp-host/ash/ash-host-ui.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/ezsp-host/ash/ash-host-ui.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/ezsp-host/ash/ash-host-ui.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/ezsp-host/ash/ash-host.o: $(SDK_PATH)/protocol/zigbee/app/ezsp-host/ash/ash-host.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/ezsp-host/ash/ash-host.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/ezsp-host/ash/ash-host.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/ezsp-host/ash/ash-host.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/ezsp-host/ash/ash-host.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/ezsp-host/ezsp-host-io.o: $(SDK_PATH)/protocol/zigbee/app/ezsp-host/ezsp-host-io.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/ezsp-host/ezsp-host-io.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/ezsp-host/ezsp-host-io.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/ezsp-host/ezsp-host-io.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/ezsp-host/ezsp-host-io.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/ezsp-host/ezsp-host-queues.o: $(SDK_PATH)/protocol/zigbee/app/ezsp-host/ezsp-host-queues.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/ezsp-host/ezsp-host-queues.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/ezsp-host/ezsp-host-queues.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/ezsp-host/ezsp-host-queues.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/ezsp-host/ezsp-host-queues.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/ezsp-host/ezsp-host-ui.o: $(SDK_PATH)/protocol/zigbee/app/ezsp-host/ezsp-host-ui.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/ezsp-host/ezsp-host-ui.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/ezsp-host/ezsp-host-ui.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/ezsp-host/ezsp-host-ui.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/ezsp-host/ezsp-host-ui.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/common/zigbee_app_framework_common.o: $(SDK_PATH)/protocol/zigbee/app/framework/common/zigbee_app_framework_common.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/common/zigbee_app_framework_common.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/common/zigbee_app_framework_common.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/common/zigbee_app_framework_common.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/common/zigbee_app_framework_common.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/common/zigbee_app_framework_host_cb.o: $(SDK_PATH)/protocol/zigbee/app/framework/common/zigbee_app_framework_host_cb.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/common/zigbee_app_framework_host_cb.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/common/zigbee_app_framework_host_cb.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/common/zigbee_app_framework_host_cb.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/common/zigbee_app_framework_host_cb.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin-host/ezsp/ezsp-cb.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/ezsp/ezsp-cb.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/ezsp/ezsp-cb.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/ezsp/ezsp-cb.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin-host/ezsp/ezsp-cb.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin-host/ezsp/ezsp-cb.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin-host/file-descriptor-dispatch/file-descriptor-dispatch-cb.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/file-descriptor-dispatch/file-descriptor-dispatch-cb.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/file-descriptor-dispatch/file-descriptor-dispatch-cb.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/file-descriptor-dispatch/file-descriptor-dispatch-cb.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin-host/file-descriptor-dispatch/file-descriptor-dispatch-cb.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin-host/file-descriptor-dispatch/file-descriptor-dispatch-cb.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin-host/file-descriptor-dispatch/file-descriptor-dispatch.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/file-descriptor-dispatch/file-descriptor-dispatch.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/file-descriptor-dispatch/file-descriptor-dispatch.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/file-descriptor-dispatch/file-descriptor-dispatch.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin-host/file-descriptor-dispatch/file-descriptor-dispatch.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin-host/file-descriptor-dispatch/file-descriptor-dispatch.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin-host/gateway/backchannel-support.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/gateway/backchannel-support.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/gateway/backchannel-support.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/gateway/backchannel-support.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin-host/gateway/backchannel-support.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin-host/gateway/backchannel-support.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin-host/gateway/gateway-cb.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/gateway/gateway-cb.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/gateway/gateway-cb.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/gateway/gateway-cb.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin-host/gateway/gateway-cb.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin-host/gateway/gateway-cb.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin-host/gateway/gateway-support.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/gateway/gateway-support.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/gateway/gateway-support.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/gateway/gateway-support.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin-host/gateway/gateway-support.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin-host/gateway/gateway-support.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin-host/ncp-configuration/ncp-configuration.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/ncp-configuration/ncp-configuration.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/ncp-configuration/ncp-configuration.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/ncp-configuration/ncp-configuration.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin-host/ncp-configuration/ncp-configuration.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin-host/ncp-configuration/ncp-configuration.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/address-table/address-table.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/address-table/address-table.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/address-table/address-table.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/address-table/address-table.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/address-table/address-table.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/address-table/address-table.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/basic/basic-cb.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/basic/basic-cb.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/basic/basic-cb.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/basic/basic-cb.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/basic/basic-cb.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/basic/basic-cb.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/basic/basic.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/basic/basic.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/basic/basic.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/basic/basic.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/basic/basic.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/basic/basic.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/concentrator/concentrator-cb.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/concentrator/concentrator-cb.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/concentrator/concentrator-cb.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/concentrator/concentrator-cb.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/concentrator/concentrator-cb.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/concentrator/concentrator-cb.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/concentrator/concentrator-support.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/concentrator/concentrator-support.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/concentrator/concentrator-support.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/concentrator/concentrator-support.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/concentrator/concentrator-support.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/concentrator/concentrator-support.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/counters/counters-cb.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/counters/counters-cb.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/counters/counters-cb.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/counters/counters-cb.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/counters/counters-cb.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/counters/counters-cb.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/counters/counters-host.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/counters/counters-host.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/counters/counters-host.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/counters/counters-host.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/counters/counters-host.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/counters/counters-host.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/counters/counters-ota-host.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/counters/counters-ota-host.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/counters/counters-ota-host.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/counters/counters-ota-host.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/counters/counters-ota-host.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/counters/counters-ota-host.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/debug-print/sl_zigbee_debug_print.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/debug-print/sl_zigbee_debug_print.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/debug-print/sl_zigbee_debug_print.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/debug-print/sl_zigbee_debug_print.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/debug-print/sl_zigbee_debug_print.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/debug-print/sl_zigbee_debug_print.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/network-creator-security/network-creator-security.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/network-creator-security/network-creator-security.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/network-creator-security/network-creator-security.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/network-creator-security/network-creator-security.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/network-creator-security/network-creator-security.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/network-creator-security/network-creator-security.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/network-creator/network-creator-cb.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/network-creator/network-creator-cb.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/network-creator/network-creator-cb.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/network-creator/network-creator-cb.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/network-creator/network-creator-cb.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/network-creator/network-creator-cb.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/network-creator/network-creator.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/network-creator/network-creator.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/network-creator/network-creator.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/network-creator/network-creator.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/network-creator/network-creator.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/network-creator/network-creator.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/network-steering/network-steering-cb.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/network-steering/network-steering-cb.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/network-steering/network-steering-cb.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/network-steering/network-steering-cb.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/network-steering/network-steering-cb.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/network-steering/network-steering-cb.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/network-steering/network-steering-host.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/network-steering/network-steering-host.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/network-steering/network-steering-host.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/network-steering/network-steering-host.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/network-steering/network-steering-host.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/network-steering/network-steering-host.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/network-steering/network-steering-v2.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/network-steering/network-steering-v2.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/network-steering/network-steering-v2.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/network-steering/network-steering-v2.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/network-steering/network-steering-v2.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/network-steering/network-steering-v2.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/network-steering/network-steering.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/network-steering/network-steering.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/network-steering/network-steering.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/network-steering/network-steering.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/network-steering/network-steering.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/network-steering/network-steering.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/ota-common/ota-common.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-common/ota-common.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-common/ota-common.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-common/ota-common.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/ota-common/ota-common.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/ota-common/ota-common.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/ota-server-policy/ota-server-policy-cb.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-server-policy/ota-server-policy-cb.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-server-policy/ota-server-policy-cb.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-server-policy/ota-server-policy-cb.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/ota-server-policy/ota-server-policy-cb.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/ota-server-policy/ota-server-policy-cb.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/ota-server-policy/ota-server-policy.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-server-policy/ota-server-policy.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-server-policy/ota-server-policy.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-server-policy/ota-server-policy.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/ota-server-policy/ota-server-policy.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/ota-server-policy/ota-server-policy.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/ota-server/ota-server-cb.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-server/ota-server-cb.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-server/ota-server-cb.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-server/ota-server-cb.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/ota-server/ota-server-cb.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/ota-server/ota-server-cb.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/ota-server/ota-server-dynamic-block-period.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-server/ota-server-dynamic-block-period.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-server/ota-server-dynamic-block-period.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-server/ota-server-dynamic-block-period.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/ota-server/ota-server-dynamic-block-period.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/ota-server/ota-server-dynamic-block-period.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/ota-server/ota-server-page-request.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-server/ota-server-page-request.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-server/ota-server-page-request.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-server/ota-server-page-request.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/ota-server/ota-server-page-request.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/ota-server/ota-server-page-request.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/ota-server/ota-server.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-server/ota-server.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-server/ota-server.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-server/ota-server.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/ota-server/ota-server.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/ota-server/ota-server.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/ota-storage-common/ota-storage-common.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-storage-common/ota-storage-common.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-storage-common/ota-storage-common.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-storage-common/ota-storage-common.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/ota-storage-common/ota-storage-common.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/ota-storage-common/ota-storage-common.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/ota-storage-posix-filesystem/ota-storage-linux.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-storage-posix-filesystem/ota-storage-linux.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-storage-posix-filesystem/ota-storage-linux.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-storage-posix-filesystem/ota-storage-linux.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/ota-storage-posix-filesystem/ota-storage-linux.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/ota-storage-posix-filesystem/ota-storage-linux.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/reporting/reporting-cb.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/reporting/reporting-cb.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/reporting/reporting-cb.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/reporting/reporting-cb.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/reporting/reporting-cb.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/reporting/reporting-cb.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/reporting/reporting-default-configuration.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/reporting/reporting-default-configuration.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/reporting/reporting-default-configuration.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/reporting/reporting-default-configuration.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/reporting/reporting-default-configuration.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/reporting/reporting-default-configuration.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/reporting/reporting.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/reporting/reporting.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/reporting/reporting.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/reporting/reporting.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/reporting/reporting.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/reporting/reporting.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/scan-dispatch/scan-dispatch.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/scan-dispatch/scan-dispatch.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/scan-dispatch/scan-dispatch.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/scan-dispatch/scan-dispatch.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/scan-dispatch/scan-dispatch.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/scan-dispatch/scan-dispatch.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/trust-center-nwk-key-update-periodic/trust-center-nwk-key-update-periodic.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/trust-center-nwk-key-update-periodic/trust-center-nwk-key-update-periodic.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/trust-center-nwk-key-update-periodic/trust-center-nwk-key-update-periodic.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/trust-center-nwk-key-update-periodic/trust-center-nwk-key-update-periodic.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/trust-center-nwk-key-update-periodic/trust-center-nwk-key-update-periodic.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/trust-center-nwk-key-update-periodic/trust-center-nwk-key-update-periodic.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/trust-center-nwk-key-update-unicast/trust-center-nwk-key-update-unicast.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/trust-center-nwk-key-update-unicast/trust-center-nwk-key-update-unicast.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/trust-center-nwk-key-update-unicast/trust-center-nwk-key-update-unicast.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/trust-center-nwk-key-update-unicast/trust-center-nwk-key-update-unicast.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/trust-center-nwk-key-update-unicast/trust-center-nwk-key-update-unicast.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/trust-center-nwk-key-update-unicast/trust-center-nwk-key-update-unicast.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/update-tc-link-key/update-tc-link-key-cb.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/update-tc-link-key/update-tc-link-key-cb.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/update-tc-link-key/update-tc-link-key-cb.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/update-tc-link-key/update-tc-link-key-cb.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/update-tc-link-key/update-tc-link-key-cb.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/update-tc-link-key/update-tc-link-key-cb.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/update-tc-link-key/update-tc-link-key.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/update-tc-link-key/update-tc-link-key.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/update-tc-link-key/update-tc-link-key.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/update-tc-link-key/update-tc-link-key.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/update-tc-link-key/update-tc-link-key.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/update-tc-link-key/update-tc-link-key.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/zcl-framework-core/zcl-framework-core-cb.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/zcl-framework-core/zcl-framework-core-cb.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/zcl-framework-core/zcl-framework-core-cb.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/zcl-framework-core/zcl-framework-core-cb.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/zcl-framework-core/zcl-framework-core-cb.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/zcl-framework-core/zcl-framework-core-cb.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/security/af-node.o: $(SDK_PATH)/protocol/zigbee/app/framework/security/af-node.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/security/af-node.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/security/af-node.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/security/af-node.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/security/af-node.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/security/af-security-common.o: $(SDK_PATH)/protocol/zigbee/app/framework/security/af-security-common.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/security/af-security-common.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/security/af-security-common.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/security/af-security-common.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/security/af-security-common.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/security/af-trust-center.o: $(SDK_PATH)/protocol/zigbee/app/framework/security/af-trust-center.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/security/af-trust-center.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/security/af-trust-center.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/security/af-trust-center.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/security/af-trust-center.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/security/crypto-state.o: $(SDK_PATH)/protocol/zigbee/app/framework/security/crypto-state.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/security/crypto-state.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/security/crypto-state.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/security/crypto-state.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/security/crypto-state.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/service-function/sl_service_function.o: $(SDK_PATH)/protocol/zigbee/app/framework/service-function/sl_service_function.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/service-function/sl_service_function.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/service-function/sl_service_function.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/service-function/sl_service_function.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/service-function/sl_service_function.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/signature-decode/sl_signature_decode.o: $(SDK_PATH)/protocol/zigbee/app/framework/signature-decode/sl_signature_decode.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/signature-decode/sl_signature_decode.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/signature-decode/sl_signature_decode.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/signature-decode/sl_signature_decode.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/signature-decode/sl_signature_decode.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/af-common.o: $(SDK_PATH)/protocol/zigbee/app/framework/util/af-common.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/util/af-common.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/util/af-common.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/af-common.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/af-common.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/af-event.o: $(SDK_PATH)/protocol/zigbee/app/framework/util/af-event.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/util/af-event.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/util/af-event.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/af-event.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/af-event.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/af-host.o: $(SDK_PATH)/protocol/zigbee/app/framework/util/af-host.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/util/af-host.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/util/af-host.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/af-host.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/af-host.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/attribute-size.o: $(SDK_PATH)/protocol/zigbee/app/framework/util/attribute-size.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/util/attribute-size.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/util/attribute-size.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/attribute-size.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/attribute-size.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/attribute-storage.o: $(SDK_PATH)/protocol/zigbee/app/framework/util/attribute-storage.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/util/attribute-storage.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/util/attribute-storage.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/attribute-storage.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/attribute-storage.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/attribute-table.o: $(SDK_PATH)/protocol/zigbee/app/framework/util/attribute-table.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/util/attribute-table.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/util/attribute-table.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/attribute-table.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/attribute-table.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/client-api.o: $(SDK_PATH)/protocol/zigbee/app/framework/util/client-api.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/util/client-api.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/util/client-api.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/client-api.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/client-api.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/global-callback.o: $(SDK_PATH)/protocol/zigbee/app/framework/util/global-callback.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/util/global-callback.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/util/global-callback.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/global-callback.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/global-callback.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/global-other-callback.o: $(SDK_PATH)/protocol/zigbee/app/framework/util/global-other-callback.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/util/global-other-callback.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/util/global-other-callback.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/global-other-callback.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/global-other-callback.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/message.o: $(SDK_PATH)/protocol/zigbee/app/framework/util/message.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/util/message.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/util/message.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/message.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/message.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/multi-network.o: $(SDK_PATH)/protocol/zigbee/app/framework/util/multi-network.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/util/multi-network.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/util/multi-network.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/multi-network.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/multi-network.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/print-formatter.o: $(SDK_PATH)/protocol/zigbee/app/framework/util/print-formatter.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/util/print-formatter.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/util/print-formatter.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/print-formatter.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/print-formatter.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/print.o: $(SDK_PATH)/protocol/zigbee/app/framework/util/print.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/util/print.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/util/print.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/print.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/print.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/process-cluster-message.o: $(SDK_PATH)/protocol/zigbee/app/framework/util/process-cluster-message.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/util/process-cluster-message.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/util/process-cluster-message.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/process-cluster-message.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/process-cluster-message.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/process-global-message.o: $(SDK_PATH)/protocol/zigbee/app/framework/util/process-global-message.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/util/process-global-message.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/util/process-global-message.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/process-global-message.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/process-global-message.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/service-discovery-common.o: $(SDK_PATH)/protocol/zigbee/app/framework/util/service-discovery-common.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/util/service-discovery-common.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/util/service-discovery-common.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/service-discovery-common.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/service-discovery-common.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/service-discovery-host.o: $(SDK_PATH)/protocol/zigbee/app/framework/util/service-discovery-host.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/util/service-discovery-host.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/util/service-discovery-host.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/service-discovery-host.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/service-discovery-host.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/time-util.o: $(SDK_PATH)/protocol/zigbee/app/framework/util/time-util.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/util/time-util.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/util/time-util.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/time-util.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/time-util.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/util.o: $(SDK_PATH)/protocol/zigbee/app/framework/util/util.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/util/util.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/util/util.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/util.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/util.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/common/library.o: $(SDK_PATH)/protocol/zigbee/app/util/common/library.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/util/common/library.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/util/common/library.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/common/library.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/common/library.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/ezsp/ezsp-callbacks.o: $(SDK_PATH)/protocol/zigbee/app/util/ezsp/ezsp-callbacks.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/util/ezsp/ezsp-callbacks.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/util/ezsp/ezsp-callbacks.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/ezsp/ezsp-callbacks.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/ezsp/ezsp-callbacks.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/ezsp/ezsp-enum-decode.o: $(SDK_PATH)/protocol/zigbee/app/util/ezsp/ezsp-enum-decode.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/util/ezsp/ezsp-enum-decode.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/util/ezsp/ezsp-enum-decode.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/ezsp/ezsp-enum-decode.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/ezsp/ezsp-enum-decode.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/ezsp/ezsp-frame-utilities.o: $(SDK_PATH)/protocol/zigbee/app/util/ezsp/ezsp-frame-utilities.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/util/ezsp/ezsp-frame-utilities.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/util/ezsp/ezsp-frame-utilities.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/ezsp/ezsp-frame-utilities.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/ezsp/ezsp-frame-utilities.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/ezsp/ezsp.o: $(SDK_PATH)/protocol/zigbee/app/util/ezsp/ezsp.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/util/ezsp/ezsp.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/util/ezsp/ezsp.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/ezsp/ezsp.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/ezsp/ezsp.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/ezsp/secure-ezsp-stub.o: $(SDK_PATH)/protocol/zigbee/app/util/ezsp/secure-ezsp-stub.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/util/ezsp/secure-ezsp-stub.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/util/ezsp/secure-ezsp-stub.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/ezsp/secure-ezsp-stub.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/ezsp/secure-ezsp-stub.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/ezsp/serial-interface-uart.o: $(SDK_PATH)/protocol/zigbee/app/util/ezsp/serial-interface-uart.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/util/ezsp/serial-interface-uart.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/util/ezsp/serial-interface-uart.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/ezsp/serial-interface-uart.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/ezsp/serial-interface-uart.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/security/security-address-cache.o: $(SDK_PATH)/protocol/zigbee/app/util/security/security-address-cache.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/util/security/security-address-cache.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/util/security/security-address-cache.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/security/security-address-cache.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/security/security-address-cache.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/serial/linux-serial.o: $(SDK_PATH)/protocol/zigbee/app/util/serial/linux-serial.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/util/serial/linux-serial.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/util/serial/linux-serial.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/serial/linux-serial.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/serial/linux-serial.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/zigbee-framework/zigbee-device-common.o: $(SDK_PATH)/protocol/zigbee/app/util/zigbee-framework/zigbee-device-common.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/util/zigbee-framework/zigbee-device-common.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/util/zigbee-framework/zigbee-device-common.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/zigbee-framework/zigbee-device-common.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/zigbee-framework/zigbee-device-common.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/zigbee-framework/zigbee-device-host.o: $(SDK_PATH)/protocol/zigbee/app/util/zigbee-framework/zigbee-device-host.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/util/zigbee-framework/zigbee-device-host.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/util/zigbee-framework/zigbee-device-host.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/zigbee-framework/zigbee-device-host.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/zigbee-framework/zigbee-device-host.o

$(OUTPUT_DIR)/sdk/util/silicon_labs/silabs_core/event_queue/event-queue.o: $(SDK_PATH)/util/silicon_labs/silabs_core/event_queue/event-queue.c
	@echo 'Building $(SDK_PATH)/util/silicon_labs/silabs_core/event_queue/event-queue.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/silicon_labs/silabs_core/event_queue/event-queue.c
CDEPS += $(OUTPUT_DIR)/sdk/util/silicon_labs/silabs_core/event_queue/event-queue.d
OBJS += $(OUTPUT_DIR)/sdk/util/silicon_labs/silabs_core/event_queue/event-queue.o

$(OUTPUT_DIR)/sdk/util/silicon_labs/silabs_core/memory_manager/sl_malloc.o: $(SDK_PATH)/util/silicon_labs/silabs_core/memory_manager/sl_malloc.c
	@echo 'Building $(SDK_PATH)/util/silicon_labs/silabs_core/memory_manager/sl_malloc.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/silicon_labs/silabs_core/memory_manager/sl_malloc.c
CDEPS += $(OUTPUT_DIR)/sdk/util/silicon_labs/silabs_core/memory_manager/sl_malloc.d
OBJS += $(OUTPUT_DIR)/sdk/util/silicon_labs/silabs_core/memory_manager/sl_malloc.o

$(OUTPUT_DIR)/sdk/util/third_party/printf/printf.o: $(SDK_PATH)/util/third_party/printf/printf.c
	@echo 'Building $(SDK_PATH)/util/third_party/printf/printf.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/printf/printf.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/printf/printf.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/printf/printf.o

$(OUTPUT_DIR)/sdk/util/third_party/printf/src/iostream_printf.o: $(SDK_PATH)/util/third_party/printf/src/iostream_printf.c
	@echo 'Building $(SDK_PATH)/util/third_party/printf/src/iostream_printf.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/printf/src/iostream_printf.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/printf/src/iostream_printf.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/printf/src/iostream_printf.o

$(OUTPUT_DIR)/project/_/src/zigbee_host.o: ../src/zigbee_host.c
	@echo 'Building ../src/zigbee_host.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../src/zigbee_host.c
CDEPS += $(OUTPUT_DIR)/project/_/src/zigbee_host.d
OBJS += $(OUTPUT_DIR)/project/_/src/zigbee_host.o

$(OUTPUT_DIR)/project/_/src/zigbee_host_address_table.o: ../src/zigbee_host_address_table.c
	@echo 'Building ../src/zigbee_host_address_table.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../src/zigbee_host_address_table.c
CDEPS += $(OUTPUT_DIR)/project/_/src/zigbee_host_address_table.d
OBJS += $(OUTPUT_DIR)/project/_/src/zigbee_host_address_table.o

$(OUTPUT_DIR)/project/_/src/zigbee_host_attribute_mgmt.o: ../src/zigbee_host_attribute_mgmt.c
	@echo 'Building ../src/zigbee_host_attribute_mgmt.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../src/zigbee_host_attribute_mgmt.c
CDEPS += $(OUTPUT_DIR)/project/_/src/zigbee_host_attribute_mgmt.d
OBJS += $(OUTPUT_DIR)/project/_/src/zigbee_host_attribute_mgmt.o

$(OUTPUT_DIR)/project/_/src/zigbee_host_cli.o: ../src/zigbee_host_cli.c
	@echo 'Building ../src/zigbee_host_cli.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../src/zigbee_host_cli.c
CDEPS += $(OUTPUT_DIR)/project/_/src/zigbee_host_cli.d
OBJS += $(OUTPUT_DIR)/project/_/src/zigbee_host_cli.o

$(OUTPUT_DIR)/project/_/src/zigbee_host_cluster_command_parse.o: ../src/zigbee_host_cluster_command_parse.c
	@echo 'Building ../src/zigbee_host_cluster_command_parse.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../src/zigbee_host_cluster_command_parse.c
CDEPS += $(OUTPUT_DIR)/project/_/src/zigbee_host_cluster_command_parse.d
OBJS += $(OUTPUT_DIR)/project/_/src/zigbee_host_cluster_command_parse.o

$(OUTPUT_DIR)/project/_/src/zigbee_host_counters.o: ../src/zigbee_host_counters.c
	@echo 'Building ../src/zigbee_host_counters.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../src/zigbee_host_counters.c
CDEPS += $(OUTPUT_DIR)/project/_/src/zigbee_host_counters.d
OBJS += $(OUTPUT_DIR)/project/_/src/zigbee_host_counters.o

$(OUTPUT_DIR)/project/_/src/zigbee_host_discovery.o: ../src/zigbee_host_discovery.c
	@echo 'Building ../src/zigbee_host_discovery.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../src/zigbee_host_discovery.c
CDEPS += $(OUTPUT_DIR)/project/_/src/zigbee_host_discovery.d
OBJS += $(OUTPUT_DIR)/project/_/src/zigbee_host_discovery.o

$(OUTPUT_DIR)/project/_/src/zigbee_host_ota_server.o: ../src/zigbee_host_ota_server.c
	@echo 'Building ../src/zigbee_host_ota_server.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../src/zigbee_host_ota_server.c
CDEPS += $(OUTPUT_DIR)/project/_/src/zigbee_host_ota_server.d
OBJS += $(OUTPUT_DIR)/project/_/src/zigbee_host_ota_server.o

$(OUTPUT_DIR)/project/_/src/zigbee_host_poll_checkin.o: ../src/zigbee_host_poll_checkin.c
	@echo 'Building ../src/zigbee_host_poll_checkin.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../src/zigbee_host_poll_checkin.c
CDEPS += $(OUTPUT_DIR)/project/_/src/zigbee_host_poll_checkin.d
OBJS += $(OUTPUT_DIR)/project/_/src/zigbee_host_poll_checkin.o

$(OUTPUT_DIR)/project/_/src/zigbee_host_trust_center.o: ../src/zigbee_host_trust_center.c
	@echo 'Building ../src/zigbee_host_trust_center.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../src/zigbee_host_trust_center.c
CDEPS += $(OUTPUT_DIR)/project/_/src/zigbee_host_trust_center.d
OBJS += $(OUTPUT_DIR)/project/_/src/zigbee_host_trust_center.o

$(OUTPUT_DIR)/project/_/src/zigbee_host_util.o: ../src/zigbee_host_util.c
	@echo 'Building ../src/zigbee_host_util.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../src/zigbee_host_util.c
CDEPS += $(OUTPUT_DIR)/project/_/src/zigbee_host_util.d
OBJS += $(OUTPUT_DIR)/project/_/src/zigbee_host_util.o

$(OUTPUT_DIR)/project/_/src/zigbee_host_zcl_frame.o: ../src/zigbee_host_zcl_frame.c
	@echo 'Building ../src/zigbee_host_zcl_frame.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../src/zigbee_host_zcl_frame.c
CDEPS += $(OUTPUT_DIR)/project/_/src/zigbee_host_zcl_frame.d
OBJS += $(OUTPUT_DIR)/project/_/src/zigbee_host_zcl_frame.o

$(OUTPUT_DIR)/project/autogen/sl_cluster_service_gen.o: autogen/sl_cluster_service_gen.c
	@echo 'Building autogen/sl_cluster_service_gen.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ autogen/sl_cluster_service_gen.c
CDEPS += $(OUTPUT_DIR)/project/autogen/sl_cluster_service_gen.d
OBJS += $(OUTPUT_DIR)/project/autogen/sl_cluster_service_gen.o

$(OUTPUT_DIR)/project/autogen/sl_event_handler.o: autogen/sl_event_handler.c
	@echo 'Building autogen/sl_event_handler.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ autogen/sl_event_handler.c
CDEPS += $(OUTPUT_DIR)/project/autogen/sl_event_handler.d
OBJS += $(OUTPUT_DIR)/project/autogen/sl_event_handler.o

$(OUTPUT_DIR)/project/autogen/sl_iostream_handles.o: autogen/sl_iostream_handles.c
	@echo 'Building autogen/sl_iostream_handles.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ autogen/sl_iostream_handles.c
CDEPS += $(OUTPUT_DIR)/project/autogen/sl_iostream_handles.d
OBJS += $(OUTPUT_DIR)/project/autogen/sl_iostream_handles.o

$(OUTPUT_DIR)/project/autogen/zap-cli.o: autogen/zap-cli.c
	@echo 'Building autogen/zap-cli.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ autogen/zap-cli.c
CDEPS += $(OUTPUT_DIR)/project/autogen/zap-cli.d
OBJS += $(OUTPUT_DIR)/project/autogen/zap-cli.o

$(OUTPUT_DIR)/project/autogen/zap-cluster-command-parser.o: autogen/zap-cluster-command-parser.c
	@echo 'Building autogen/zap-cluster-command-parser.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ autogen/zap-cluster-command-parser.c
CDEPS += $(OUTPUT_DIR)/project/autogen/zap-cluster-command-parser.d
OBJS += $(OUTPUT_DIR)/project/autogen/zap-cluster-command-parser.o

$(OUTPUT_DIR)/project/autogen/zap-event.o: autogen/zap-event.c
	@echo 'Building autogen/zap-event.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ autogen/zap-event.c
CDEPS += $(OUTPUT_DIR)/project/autogen/zap-event.d
OBJS += $(OUTPUT_DIR)/project/autogen/zap-event.o

$(OUTPUT_DIR)/project/autogen/zigbee_common_callback_dispatcher.o: autogen/zigbee_common_callback_dispatcher.c
	@echo 'Building autogen/zigbee_common_callback_dispatcher.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ autogen/zigbee_common_callback_dispatcher.c
CDEPS += $(OUTPUT_DIR)/project/autogen/zigbee_common_callback_dispatcher.d
OBJS += $(OUTPUT_DIR)/project/autogen/zigbee_common_callback_dispatcher.o

$(OUTPUT_DIR)/project/autogen/zigbee_host_callback_dispatcher.o: autogen/zigbee_host_callback_dispatcher.c
	@echo 'Building autogen/zigbee_host_callback_dispatcher.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ autogen/zigbee_host_callback_dispatcher.c
CDEPS += $(OUTPUT_DIR)/project/autogen/zigbee_host_callback_dispatcher.d
OBJS += $(OUTPUT_DIR)/project/autogen/zigbee_host_callback_dispatcher.o

$(OUTPUT_DIR)/project/autogen/zigbee_stack_callback_dispatcher.o: autogen/zigbee_stack_callback_dispatcher.c
	@echo 'Building autogen/zigbee_stack_callback_dispatcher.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ autogen/zigbee_stack_callback_dispatcher.c
CDEPS += $(OUTPUT_DIR)/project/autogen/zigbee_stack_callback_dispatcher.d
OBJS += $(OUTPUT_DIR)/project/autogen/zigbee_stack_callback_dispatcher.o

$(OUTPUT_DIR)/project/autogen/zigbee_zcl_callback_dispatcher.o: autogen/zigbee_zcl_callback_dispatcher.c
	@echo 'Building autogen/zigbee_zcl_callback_dispatcher.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ autogen/zigbee_zcl_callback_dispatcher.c
CDEPS += $(OUTPUT_DIR)/project/autogen/zigbee_zcl_callback_dispatcher.d
OBJS += $(OUTPUT_DIR)/project/autogen/zigbee_zcl_callback_dispatcher.o

# Automatically-generated Simplicity Studio Metadata
# Please do not edit or delete these lines!
# SIMPLICITY_STUDIO_METADATA=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END_SIMPLICITY_STUDIO_METADATA
# END OF METADATA
