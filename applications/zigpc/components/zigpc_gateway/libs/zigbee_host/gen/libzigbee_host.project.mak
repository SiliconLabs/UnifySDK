####################################################################
# Automatically-generated file. Do not edit!                       #
# Makefile Version 10                                              #
####################################################################

BASE_SDK_PATH =
UNAME:=$(shell uname -s | sed -e 's/^\(CYGWIN\).*/\1/' | sed -e 's/^\(MINGW\).*/\1/')
ifeq ($(UNAME),MINGW)
# Translate "C:/super" into "/C/super" for MinGW make.
SDK_PATH := /$(shell echo $(BASE_SDK_PATH) | sed s/://)
endif
SDK_PATH ?= $(BASE_SDK_PATH)
COPIED_SDK_PATH ?= gecko_sdk_4.2.0

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

$(OUTPUT_DIR)/project/_/src/zigbee_host_neighbor_table.o: ../src/zigbee_host_neighbor_table.c
	@echo 'Building ../src/zigbee_host_neighbor_table.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../src/zigbee_host_neighbor_table.c
CDEPS += $(OUTPUT_DIR)/project/_/src/zigbee_host_neighbor_table.d
OBJS += $(OUTPUT_DIR)/project/_/src/zigbee_host_neighbor_table.o

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

$(OUTPUT_DIR)/project/_/src/zigbee_host_sl_cli.o: ../src/zigbee_host_sl_cli.c
	@echo 'Building ../src/zigbee_host_sl_cli.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../src/zigbee_host_sl_cli.c
CDEPS += $(OUTPUT_DIR)/project/_/src/zigbee_host_sl_cli.d
OBJS += $(OUTPUT_DIR)/project/_/src/zigbee_host_sl_cli.o

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

$(OUTPUT_DIR)/project/autogen/zap-event.o: autogen/zap-event.c
	@echo 'Building autogen/zap-event.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ autogen/zap-event.c
CDEPS += $(OUTPUT_DIR)/project/autogen/zap-event.d
OBJS += $(OUTPUT_DIR)/project/autogen/zap-event.o

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
