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
COPIED_SDK_PATH ?= gecko_sdk_4.3.0

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
 -I$(SDK_PATH)/protocol/zigbee/stack/security \
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

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/zcl-util.o: $(SDK_PATH)/protocol/zigbee/app/framework/util/zcl-util.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/util/zcl-util.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/util/zcl-util.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/zcl-util.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/zcl-util.o

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

$(OUTPUT_DIR)/sdk/protocol/zigbee/stack/config/ember-configuration-host-access.o: $(SDK_PATH)/protocol/zigbee/stack/config/ember-configuration-host-access.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/stack/config/ember-configuration-host-access.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/stack/config/ember-configuration-host-access.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/config/ember-configuration-host-access.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/config/ember-configuration-host-access.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/stack/security/zigbee-security-manager-host.o: $(SDK_PATH)/protocol/zigbee/stack/security/zigbee-security-manager-host.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/stack/security/zigbee-security-manager-host.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/stack/security/zigbee-security-manager-host.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/security/zigbee-security-manager-host.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/security/zigbee-security-manager-host.o

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
# SIMPLICITY_STUDIO_METADATA=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=END_SIMPLICITY_STUDIO_METADATA
# END OF METADATA