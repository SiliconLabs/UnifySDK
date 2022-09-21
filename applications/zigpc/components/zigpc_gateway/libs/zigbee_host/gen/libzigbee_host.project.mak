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
 -I$(SDK_PATH)/platform/service/cli/inc \
 -I$(SDK_PATH)/platform/service/cli/src \
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
 -I$(SDK_PATH)/protocol/zigbee/app/util/serial \
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
 -lpthread \
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

$(OUTPUT_DIR)/sdk/platform/common/src/sl_slist.o: $(SDK_PATH)/platform/common/src/sl_slist.c
	@echo 'Building $(SDK_PATH)/platform/common/src/sl_slist.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/common/src/sl_slist.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/common/src/sl_slist.d
OBJS += $(OUTPUT_DIR)/sdk/platform/common/src/sl_slist.o

$(OUTPUT_DIR)/sdk/platform/common/src/sl_string.o: $(SDK_PATH)/platform/common/src/sl_string.c
	@echo 'Building $(SDK_PATH)/platform/common/src/sl_string.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/common/src/sl_string.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/common/src/sl_string.d
OBJS += $(OUTPUT_DIR)/sdk/platform/common/src/sl_string.o

$(OUTPUT_DIR)/sdk/platform/service/cli/src/sl_cli.o: $(SDK_PATH)/platform/service/cli/src/sl_cli.c
	@echo 'Building $(SDK_PATH)/platform/service/cli/src/sl_cli.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/cli/src/sl_cli.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/cli/src/sl_cli.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/cli/src/sl_cli.o

$(OUTPUT_DIR)/sdk/platform/service/cli/src/sl_cli_arguments.o: $(SDK_PATH)/platform/service/cli/src/sl_cli_arguments.c
	@echo 'Building $(SDK_PATH)/platform/service/cli/src/sl_cli_arguments.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/cli/src/sl_cli_arguments.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/cli/src/sl_cli_arguments.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/cli/src/sl_cli_arguments.o

$(OUTPUT_DIR)/sdk/platform/service/cli/src/sl_cli_command.o: $(SDK_PATH)/platform/service/cli/src/sl_cli_command.c
	@echo 'Building $(SDK_PATH)/platform/service/cli/src/sl_cli_command.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/cli/src/sl_cli_command.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/cli/src/sl_cli_command.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/cli/src/sl_cli_command.o

$(OUTPUT_DIR)/sdk/platform/service/cli/src/sl_cli_input.o: $(SDK_PATH)/platform/service/cli/src/sl_cli_input.c
	@echo 'Building $(SDK_PATH)/platform/service/cli/src/sl_cli_input.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/cli/src/sl_cli_input.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/cli/src/sl_cli_input.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/cli/src/sl_cli_input.o

$(OUTPUT_DIR)/sdk/platform/service/cli/src/sl_cli_io.o: $(SDK_PATH)/platform/service/cli/src/sl_cli_io.c
	@echo 'Building $(SDK_PATH)/platform/service/cli/src/sl_cli_io.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/cli/src/sl_cli_io.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/cli/src/sl_cli_io.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/cli/src/sl_cli_io.o

$(OUTPUT_DIR)/sdk/platform/service/cli/src/sl_cli_threaded_host.o: $(SDK_PATH)/platform/service/cli/src/sl_cli_threaded_host.c
	@echo 'Building $(SDK_PATH)/platform/service/cli/src/sl_cli_threaded_host.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/cli/src/sl_cli_threaded_host.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/cli/src/sl_cli_threaded_host.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/cli/src/sl_cli_threaded_host.o

$(OUTPUT_DIR)/sdk/platform/service/cli/src/sl_cli_tokenize.o: $(SDK_PATH)/platform/service/cli/src/sl_cli_tokenize.c
	@echo 'Building $(SDK_PATH)/platform/service/cli/src/sl_cli_tokenize.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/cli/src/sl_cli_tokenize.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/cli/src/sl_cli_tokenize.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/cli/src/sl_cli_tokenize.o

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

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/cli/core-cli.o: $(SDK_PATH)/protocol/zigbee/app/framework/cli/core-cli.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/cli/core-cli.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/cli/core-cli.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/cli/core-cli.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/cli/core-cli.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/cli/network-cli.o: $(SDK_PATH)/protocol/zigbee/app/framework/cli/network-cli.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/cli/network-cli.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/cli/network-cli.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/cli/network-cli.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/cli/network-cli.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/cli/option-cli.o: $(SDK_PATH)/protocol/zigbee/app/framework/cli/option-cli.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/cli/option-cli.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/cli/option-cli.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/cli/option-cli.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/cli/option-cli.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/cli/security-cli.o: $(SDK_PATH)/protocol/zigbee/app/framework/cli/security-cli.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/cli/security-cli.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/cli/security-cli.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/cli/security-cli.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/cli/security-cli.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/cli/zcl-cli.o: $(SDK_PATH)/protocol/zigbee/app/framework/cli/zcl-cli.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/cli/zcl-cli.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/cli/zcl-cli.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/cli/zcl-cli.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/cli/zcl-cli.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/cli/zdo-cli.o: $(SDK_PATH)/protocol/zigbee/app/framework/cli/zdo-cli.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/cli/zdo-cli.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/cli/zdo-cli.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/cli/zdo-cli.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/cli/zdo-cli.o

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

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin-host/gateway/gateway-support-cli.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/gateway/gateway-support-cli.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/gateway/gateway-support-cli.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/gateway/gateway-support-cli.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin-host/gateway/gateway-support-cli.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin-host/gateway/gateway-support-cli.o

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

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/address-table/address-table-cli.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/address-table/address-table-cli.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/address-table/address-table-cli.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/address-table/address-table-cli.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/address-table/address-table-cli.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/address-table/address-table-cli.o

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

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/concentrator/concentrator-support-cli.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/concentrator/concentrator-support-cli.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/concentrator/concentrator-support-cli.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/concentrator/concentrator-support-cli.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/concentrator/concentrator-support-cli.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/concentrator/concentrator-support-cli.o

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

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/counters/counters-cli.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/counters/counters-cli.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/counters/counters-cli.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/counters/counters-cli.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/counters/counters-cli.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/counters/counters-cli.o

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

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/network-creator-security/network-creator-security-cli.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/network-creator-security/network-creator-security-cli.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/network-creator-security/network-creator-security-cli.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/network-creator-security/network-creator-security-cli.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/network-creator-security/network-creator-security-cli.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/network-creator-security/network-creator-security-cli.o

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

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/network-creator/network-creator-cli.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/network-creator/network-creator-cli.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/network-creator/network-creator-cli.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/network-creator/network-creator-cli.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/network-creator/network-creator-cli.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/network-creator/network-creator-cli.o

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

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/network-steering/network-steering-cli.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/network-steering/network-steering-cli.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/network-steering/network-steering-cli.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/network-steering/network-steering-cli.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/network-steering/network-steering-cli.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/network-steering/network-steering-cli.o

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

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/ota-server/ota-server-cli.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-server/ota-server-cli.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-server/ota-server-cli.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-server/ota-server-cli.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/ota-server/ota-server-cli.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/ota-server/ota-server-cli.o

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

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/ota-storage-common/ota-storage-common-cli.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-storage-common/ota-storage-common-cli.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-storage-common/ota-storage-common-cli.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-storage-common/ota-storage-common-cli.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/ota-storage-common/ota-storage-common-cli.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/ota-storage-common/ota-storage-common-cli.o

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

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/reporting/reporting-cli.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/reporting/reporting-cli.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/reporting/reporting-cli.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/reporting/reporting-cli.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/reporting/reporting-cli.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/reporting/reporting-cli.o

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

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/update-tc-link-key/update-tc-link-key-cli.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/update-tc-link-key/update-tc-link-key-cli.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/update-tc-link-key/update-tc-link-key-cli.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/update-tc-link-key/update-tc-link-key-cli.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/update-tc-link-key/update-tc-link-key-cli.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/update-tc-link-key/update-tc-link-key-cli.o

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

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/zcl_cli/zigbee-zcl-cli.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/zcl_cli/zigbee-zcl-cli.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/zcl_cli/zigbee-zcl-cli.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/zcl_cli/zigbee-zcl-cli.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/zcl_cli/zigbee-zcl-cli.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/zcl_cli/zigbee-zcl-cli.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/zcl_cli/zigbee-zcl-custom-cluster-cli.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/zcl_cli/zigbee-zcl-custom-cluster-cli.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/zcl_cli/zigbee-zcl-custom-cluster-cli.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/zcl_cli/zigbee-zcl-custom-cluster-cli.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/zcl_cli/zigbee-zcl-custom-cluster-cli.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/zcl_cli/zigbee-zcl-custom-cluster-cli.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/zcl_cli/zigbee-zcl-global-cli.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/zcl_cli/zigbee-zcl-global-cli.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/zcl_cli/zigbee-zcl-global-cli.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/zcl_cli/zigbee-zcl-global-cli.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/zcl_cli/zigbee-zcl-global-cli.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/zcl_cli/zigbee-zcl-global-cli.o

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

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/serial/sl_zigbee_command_interpreter.o: $(SDK_PATH)/protocol/zigbee/app/util/serial/sl_zigbee_command_interpreter.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/util/serial/sl_zigbee_command_interpreter.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/util/serial/sl_zigbee_command_interpreter.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/serial/sl_zigbee_command_interpreter.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/serial/sl_zigbee_command_interpreter.o

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

$(OUTPUT_DIR)/project/autogen/sl_cli_command_table.o: autogen/sl_cli_command_table.c
	@echo 'Building autogen/sl_cli_command_table.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ autogen/sl_cli_command_table.c
CDEPS += $(OUTPUT_DIR)/project/autogen/sl_cli_command_table.d
OBJS += $(OUTPUT_DIR)/project/autogen/sl_cli_command_table.o

$(OUTPUT_DIR)/project/autogen/sl_cli_instances.o: autogen/sl_cli_instances.c
	@echo 'Building autogen/sl_cli_instances.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ autogen/sl_cli_instances.c
CDEPS += $(OUTPUT_DIR)/project/autogen/sl_cli_instances.d
OBJS += $(OUTPUT_DIR)/project/autogen/sl_cli_instances.o

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

$(OUTPUT_DIR)/project/autogen/sli_cli_hooks.o: autogen/sli_cli_hooks.c
	@echo 'Building autogen/sli_cli_hooks.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ autogen/sli_cli_hooks.c
CDEPS += $(OUTPUT_DIR)/project/autogen/sli_cli_hooks.d
OBJS += $(OUTPUT_DIR)/project/autogen/sli_cli_hooks.o

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
