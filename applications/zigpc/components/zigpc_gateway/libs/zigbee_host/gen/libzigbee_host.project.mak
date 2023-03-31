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
COPIED_SDK_PATH ?= gecko_sdk_4.2.2

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
