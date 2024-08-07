####################################################################
# Automatically-generated file. Do not edit!                       #
# Makefile Version 15                                              #
####################################################################

BASE_SDK_PATH = /home/sabollim/JIRA/utf/uic/externals/GeckoSDK
UNAME:=$(shell $(POSIX_TOOL_PATH)uname -s | $(POSIX_TOOL_PATH)sed -e 's/^\(CYGWIN\).*/\1/' | $(POSIX_TOOL_PATH)sed -e 's/^\(MINGW\).*/\1/')
ifeq ($(UNAME),MINGW)
# Translate "C:/super" into "/C/super" for MinGW make.
SDK_PATH := /$(shell $(POSIX_TOOL_PATH)echo $(BASE_SDK_PATH) | sed s/://)
endif
SDK_PATH ?= $(BASE_SDK_PATH)
COPIED_SDK_PATH ?= simplicity_sdk_2024.6.0

# This uses the explicit build rules below
PROJECT_SOURCE_FILES =

C_SOURCE_FILES   += $(filter %.c, $(PROJECT_SOURCE_FILES))
CXX_SOURCE_FILES += $(filter %.cpp, $(PROJECT_SOURCE_FILES))
CXX_SOURCE_FILES += $(filter %.cc, $(PROJECT_SOURCE_FILES))
ASM_SOURCE_FILES += $(filter %.s, $(PROJECT_SOURCE_FILES))
ASM_SOURCE_FILES += $(filter %.S, $(PROJECT_SOURCE_FILES))
LIB_FILES        += $(filter %.a, $(PROJECT_SOURCE_FILES))

C_DEFS += \
 '-DSL_ZIGBEE_AF_PLUGIN_GATEWAY_MAX_WAIT_FOR_EVENT_TIMEOUT_MS=10' \
 '-DSL_ZIGBEE_AF_PLUGIN_POLL_CONTROL_CLIENT_DEFAULT_FAST_POLL_TIMEOUT=32' \
 '-DSL_COMPONENT_CATALOG_PRESENT=1' \
 '-DPLATFORM_HEADER="platform-header.h"' \
 '-DSL_CODE_COMPONENT_CORE=core' \
 '-DEZSP_HOST=1' \
 '-DGATEWAY_APP=1' \
 '-DSTACK_CORE_HEADER="stack/core/sl_zigbee_stack.h"' \
 '-DSTACK_HEADER="stack/include/sl_zigbee.h"' \
 '-DSTACK_TYPES_HEADER="stack/include/sl_zigbee_types.h"' \
 '-DUC_BUILD=1' \
 '-DEZSP_ASH=1' \
 '-DEZSP_UART=1' \
 '-DUSE_ZAP_CONFIG=1' \
 '-DCONFIGURATION_HEADER="app/framework/util/config.h"'

ASM_DEFS += \
 '-DSL_ZIGBEE_AF_PLUGIN_GATEWAY_MAX_WAIT_FOR_EVENT_TIMEOUT_MS=10' \
 '-DSL_ZIGBEE_AF_PLUGIN_POLL_CONTROL_CLIENT_DEFAULT_FAST_POLL_TIMEOUT=32' \
 '-DSL_COMPONENT_CATALOG_PRESENT=1' \
 '-DPLATFORM_HEADER="platform-header.h"' \
 '-DSL_CODE_COMPONENT_CORE=core' \
 '-DEZSP_HOST=1' \
 '-DGATEWAY_APP=1' \
 '-DSTACK_CORE_HEADER="stack/core/sl_zigbee_stack.h"' \
 '-DSTACK_HEADER="stack/include/sl_zigbee.h"' \
 '-DSTACK_TYPES_HEADER="stack/include/sl_zigbee_types.h"' \
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
 -I$(SDK_PATH)/util/plugin/byte_utilities \
 -I$(SDK_PATH)/platform/service/cli/inc \
 -I$(SDK_PATH)/platform/service/cli/src \
 -I$(SDK_PATH)/platform/CMSIS/Core/Include \
 -I$(SDK_PATH)/platform/common/inc \
 -I$(SDK_PATH)/platform/emlib/host/inc \
 -I$(SDK_PATH)/platform/service/iostream/inc \
 -I$(SDK_PATH)/platform/service/legacy_common_ash/inc \
 -I$(SDK_PATH)/platform/service/legacy_hal/inc \
 -I$(SDK_PATH)/util/third_party/printf \
 -I$(SDK_PATH)/util/third_party/printf/inc \
 -I$(SDK_PATH)/util/silicon_labs/silabs_core \
 -I$(SDK_PATH)/util/silicon_labs/silabs_core/event_queue \
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
 -I$(SDK_PATH)/protocol/zigbee/app/util/ezsp \
 -I$(SDK_PATH)/protocol/zigbee/app/util/zigbee-framework \
 -I$(SDK_PATH)/protocol/zigbee/stack \
 -I$(SDK_PATH)/protocol/zigbee/stack/platform/host \
 -I$(SDK_PATH)/protocol/zigbee/stack/include \
 -I$(SDK_PATH)/platform/radio/mac \
 -I$(SDK_PATH)/platform/service/token_manager/inc \
 -I$(SDK_PATH)/platform/emlib/inc \
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
 -I$(SDK_PATH)/protocol/zigbee/stack/internal/inc \
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
 -std=c18 \
 -Wall \
 -Wextra \
 -Os \
 -std=c11 \
 -fPIC \
 -Wno-unused-parameter \
 -Wno-missing-field-initializers \
 -Wno-missing-braces

CXX_FLAGS += \
 -std=c++17 \
 -Wall \
 -Wextra \
 -Os \
 -std=c++11 \
 -fPIC \
 -Wno-unused-parameter \
 -Wno-missing-field-initializers \
 -Wno-missing-braces

ASM_FLAGS += \

LD_FLAGS += \


####################################################################
# Pre/Post Build Rules                                             #
####################################################################
pre-build:
	# No pre-build defined

post-build: $(OUTPUT_DIR)/$(PROJECTNAME).out
	# No post-build defined

####################################################################
# SDK Build Rules                                                  #
####################################################################
$(OUTPUT_DIR)/sdk/platform/common/src/sl_assert.o: $(SDK_PATH)/platform/common/src/sl_assert.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/common/src/sl_assert.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/common/src/sl_assert.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/common/src/sl_assert.d
OBJS += $(OUTPUT_DIR)/sdk/platform/common/src/sl_assert.o

$(OUTPUT_DIR)/sdk/platform/common/src/sl_core_mock.o: $(SDK_PATH)/platform/common/src/sl_core_mock.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/common/src/sl_core_mock.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/common/src/sl_core_mock.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/common/src/sl_core_mock.d
OBJS += $(OUTPUT_DIR)/sdk/platform/common/src/sl_core_mock.o

$(OUTPUT_DIR)/sdk/platform/common/src/sl_slist.o: $(SDK_PATH)/platform/common/src/sl_slist.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/common/src/sl_slist.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/common/src/sl_slist.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/common/src/sl_slist.d
OBJS += $(OUTPUT_DIR)/sdk/platform/common/src/sl_slist.o

$(OUTPUT_DIR)/sdk/platform/common/src/sl_string.o: $(SDK_PATH)/platform/common/src/sl_string.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/common/src/sl_string.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/common/src/sl_string.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/common/src/sl_string.d
OBJS += $(OUTPUT_DIR)/sdk/platform/common/src/sl_string.o

$(OUTPUT_DIR)/sdk/platform/service/cli/src/sl_cli.o: $(SDK_PATH)/platform/service/cli/src/sl_cli.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/service/cli/src/sl_cli.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/cli/src/sl_cli.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/cli/src/sl_cli.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/cli/src/sl_cli.o

$(OUTPUT_DIR)/sdk/platform/service/cli/src/sl_cli_arguments.o: $(SDK_PATH)/platform/service/cli/src/sl_cli_arguments.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/service/cli/src/sl_cli_arguments.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/cli/src/sl_cli_arguments.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/cli/src/sl_cli_arguments.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/cli/src/sl_cli_arguments.o

$(OUTPUT_DIR)/sdk/platform/service/cli/src/sl_cli_command.o: $(SDK_PATH)/platform/service/cli/src/sl_cli_command.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/service/cli/src/sl_cli_command.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/cli/src/sl_cli_command.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/cli/src/sl_cli_command.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/cli/src/sl_cli_command.o

$(OUTPUT_DIR)/sdk/platform/service/cli/src/sl_cli_input.o: $(SDK_PATH)/platform/service/cli/src/sl_cli_input.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/service/cli/src/sl_cli_input.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/cli/src/sl_cli_input.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/cli/src/sl_cli_input.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/cli/src/sl_cli_input.o

$(OUTPUT_DIR)/sdk/platform/service/cli/src/sl_cli_io.o: $(SDK_PATH)/platform/service/cli/src/sl_cli_io.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/service/cli/src/sl_cli_io.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/cli/src/sl_cli_io.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/cli/src/sl_cli_io.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/cli/src/sl_cli_io.o

$(OUTPUT_DIR)/sdk/platform/service/cli/src/sl_cli_threaded_host.o: $(SDK_PATH)/platform/service/cli/src/sl_cli_threaded_host.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/service/cli/src/sl_cli_threaded_host.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/cli/src/sl_cli_threaded_host.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/cli/src/sl_cli_threaded_host.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/cli/src/sl_cli_threaded_host.o

$(OUTPUT_DIR)/sdk/platform/service/cli/src/sl_cli_tokenize.o: $(SDK_PATH)/platform/service/cli/src/sl_cli_tokenize.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/service/cli/src/sl_cli_tokenize.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/cli/src/sl_cli_tokenize.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/cli/src/sl_cli_tokenize.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/cli/src/sl_cli_tokenize.o

$(OUTPUT_DIR)/sdk/platform/service/iostream/src/sl_iostream.o: $(SDK_PATH)/platform/service/iostream/src/sl_iostream.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/service/iostream/src/sl_iostream.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/iostream/src/sl_iostream.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/iostream/src/sl_iostream.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/iostream/src/sl_iostream.o

$(OUTPUT_DIR)/sdk/platform/service/iostream/src/sl_iostream_stdio.o: $(SDK_PATH)/platform/service/iostream/src/sl_iostream_stdio.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/service/iostream/src/sl_iostream_stdio.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/iostream/src/sl_iostream_stdio.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/iostream/src/sl_iostream_stdio.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/iostream/src/sl_iostream_stdio.o

$(OUTPUT_DIR)/sdk/platform/service/legacy_common_ash/src/ash-common.o: $(SDK_PATH)/platform/service/legacy_common_ash/src/ash-common.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/service/legacy_common_ash/src/ash-common.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/legacy_common_ash/src/ash-common.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/legacy_common_ash/src/ash-common.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/legacy_common_ash/src/ash-common.o

$(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/crc.o: $(SDK_PATH)/platform/service/legacy_hal/src/crc.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/service/legacy_hal/src/crc.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/legacy_hal/src/crc.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/crc.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/crc.o

$(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/ember-printf-convert.o: $(SDK_PATH)/platform/service/legacy_hal/src/ember-printf-convert.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/service/legacy_hal/src/ember-printf-convert.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/legacy_hal/src/ember-printf-convert.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/ember-printf-convert.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/ember-printf-convert.o

$(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/micro_host.o: $(SDK_PATH)/platform/service/legacy_hal/src/micro_host.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/service/legacy_hal/src/micro_host.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/legacy_hal/src/micro_host.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/micro_host.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/micro_host.o

$(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/random.o: $(SDK_PATH)/platform/service/legacy_hal/src/random.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/service/legacy_hal/src/random.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/legacy_hal/src/random.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/random.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/random.o

$(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/system-timer.o: $(SDK_PATH)/platform/service/legacy_hal/src/system-timer.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/service/legacy_hal/src/system-timer.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/legacy_hal/src/system-timer.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/system-timer.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/system-timer.o

$(OUTPUT_DIR)/sdk/platform/service/legacy_host/src/token.o: $(SDK_PATH)/platform/service/legacy_host/src/token.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/service/legacy_host/src/token.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/legacy_host/src/token.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/legacy_host/src/token.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/legacy_host/src/token.o

$(OUTPUT_DIR)/sdk/platform/service/system/src/sl_system_init.o: $(SDK_PATH)/platform/service/system/src/sl_system_init.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/service/system/src/sl_system_init.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/system/src/sl_system_init.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/system/src/sl_system_init.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/system/src/sl_system_init.o

$(OUTPUT_DIR)/sdk/platform/service/system/src/sl_system_process_action.o: $(SDK_PATH)/platform/service/system/src/sl_system_process_action.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/service/system/src/sl_system_process_action.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/system/src/sl_system_process_action.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/system/src/sl_system_process_action.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/system/src/sl_system_process_action.o

$(OUTPUT_DIR)/sdk/platform/service/token_manager/src/sl_token_def.o: $(SDK_PATH)/platform/service/token_manager/src/sl_token_def.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/service/token_manager/src/sl_token_def.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/token_manager/src/sl_token_def.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/token_manager/src/sl_token_def.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/token_manager/src/sl_token_def.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/ezsp-host/ash/ash-host-ui.o: $(SDK_PATH)/protocol/zigbee/app/ezsp-host/ash/ash-host-ui.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/ezsp-host/ash/ash-host-ui.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/ezsp-host/ash/ash-host-ui.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/ezsp-host/ash/ash-host-ui.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/ezsp-host/ash/ash-host-ui.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/ezsp-host/ash/ash-host.o: $(SDK_PATH)/protocol/zigbee/app/ezsp-host/ash/ash-host.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/ezsp-host/ash/ash-host.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/ezsp-host/ash/ash-host.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/ezsp-host/ash/ash-host.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/ezsp-host/ash/ash-host.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/ezsp-host/ezsp-host-io.o: $(SDK_PATH)/protocol/zigbee/app/ezsp-host/ezsp-host-io.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/ezsp-host/ezsp-host-io.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/ezsp-host/ezsp-host-io.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/ezsp-host/ezsp-host-io.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/ezsp-host/ezsp-host-io.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/ezsp-host/ezsp-host-queues.o: $(SDK_PATH)/protocol/zigbee/app/ezsp-host/ezsp-host-queues.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/ezsp-host/ezsp-host-queues.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/ezsp-host/ezsp-host-queues.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/ezsp-host/ezsp-host-queues.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/ezsp-host/ezsp-host-queues.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/ezsp-host/ezsp-host-ui.o: $(SDK_PATH)/protocol/zigbee/app/ezsp-host/ezsp-host-ui.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/ezsp-host/ezsp-host-ui.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/ezsp-host/ezsp-host-ui.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/ezsp-host/ezsp-host-ui.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/ezsp-host/ezsp-host-ui.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/cli/core-cli.o: $(SDK_PATH)/protocol/zigbee/app/framework/cli/core-cli.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/cli/core-cli.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/cli/core-cli.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/cli/core-cli.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/cli/core-cli.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/cli/network-cli.o: $(SDK_PATH)/protocol/zigbee/app/framework/cli/network-cli.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/cli/network-cli.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/cli/network-cli.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/cli/network-cli.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/cli/network-cli.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/cli/option-cli.o: $(SDK_PATH)/protocol/zigbee/app/framework/cli/option-cli.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/cli/option-cli.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/cli/option-cli.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/cli/option-cli.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/cli/option-cli.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/cli/security-cli.o: $(SDK_PATH)/protocol/zigbee/app/framework/cli/security-cli.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/cli/security-cli.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/cli/security-cli.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/cli/security-cli.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/cli/security-cli.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/cli/zcl-cli.o: $(SDK_PATH)/protocol/zigbee/app/framework/cli/zcl-cli.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/cli/zcl-cli.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/cli/zcl-cli.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/cli/zcl-cli.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/cli/zcl-cli.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/cli/zdo-cli.o: $(SDK_PATH)/protocol/zigbee/app/framework/cli/zdo-cli.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/cli/zdo-cli.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/cli/zdo-cli.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/cli/zdo-cli.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/cli/zdo-cli.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/common/sl_zigbee_system_common.o: $(SDK_PATH)/protocol/zigbee/app/framework/common/sl_zigbee_system_common.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/common/sl_zigbee_system_common.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/common/sl_zigbee_system_common.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/common/sl_zigbee_system_common.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/common/sl_zigbee_system_common.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/common/zigbee_app_framework_host_cb.o: $(SDK_PATH)/protocol/zigbee/app/framework/common/zigbee_app_framework_host_cb.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/common/zigbee_app_framework_host_cb.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/common/zigbee_app_framework_host_cb.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/common/zigbee_app_framework_host_cb.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/common/zigbee_app_framework_host_cb.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/common/zigbee_app_framework_sleep.o: $(SDK_PATH)/protocol/zigbee/app/framework/common/zigbee_app_framework_sleep.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/common/zigbee_app_framework_sleep.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/common/zigbee_app_framework_sleep.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/common/zigbee_app_framework_sleep.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/common/zigbee_app_framework_sleep.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/common/zigbee_app_framework_sleep_cli.o: $(SDK_PATH)/protocol/zigbee/app/framework/common/zigbee_app_framework_sleep_cli.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/common/zigbee_app_framework_sleep_cli.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/common/zigbee_app_framework_sleep_cli.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/common/zigbee_app_framework_sleep_cli.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/common/zigbee_app_framework_sleep_cli.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin-host/ezsp/ezsp-cb.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/ezsp/ezsp-cb.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/ezsp/ezsp-cb.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/ezsp/ezsp-cb.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin-host/ezsp/ezsp-cb.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin-host/ezsp/ezsp-cb.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin-host/file-descriptor-dispatch/file-descriptor-dispatch-cb.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/file-descriptor-dispatch/file-descriptor-dispatch-cb.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/file-descriptor-dispatch/file-descriptor-dispatch-cb.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/file-descriptor-dispatch/file-descriptor-dispatch-cb.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin-host/file-descriptor-dispatch/file-descriptor-dispatch-cb.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin-host/file-descriptor-dispatch/file-descriptor-dispatch-cb.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin-host/file-descriptor-dispatch/file-descriptor-dispatch.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/file-descriptor-dispatch/file-descriptor-dispatch.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/file-descriptor-dispatch/file-descriptor-dispatch.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/file-descriptor-dispatch/file-descriptor-dispatch.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin-host/file-descriptor-dispatch/file-descriptor-dispatch.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin-host/file-descriptor-dispatch/file-descriptor-dispatch.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin-host/gateway/gateway-cb.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/gateway/gateway-cb.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/gateway/gateway-cb.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/gateway/gateway-cb.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin-host/gateway/gateway-cb.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin-host/gateway/gateway-cb.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin-host/gateway/gateway-support-cli.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/gateway/gateway-support-cli.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/gateway/gateway-support-cli.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/gateway/gateway-support-cli.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin-host/gateway/gateway-support-cli.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin-host/gateway/gateway-support-cli.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin-host/gateway/gateway-support.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/gateway/gateway-support.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/gateway/gateway-support.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/gateway/gateway-support.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin-host/gateway/gateway-support.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin-host/gateway/gateway-support.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin-host/ncp-configuration/ncp-configuration.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/ncp-configuration/ncp-configuration.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/ncp-configuration/ncp-configuration.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/ncp-configuration/ncp-configuration.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin-host/ncp-configuration/ncp-configuration.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin-host/ncp-configuration/ncp-configuration.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/address-table/address-table-cli.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/address-table/address-table-cli.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/address-table/address-table-cli.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/address-table/address-table-cli.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/address-table/address-table-cli.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/address-table/address-table-cli.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/address-table/address-table.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/address-table/address-table.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/address-table/address-table.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/address-table/address-table.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/address-table/address-table.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/address-table/address-table.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/basic/basic-cb.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/basic/basic-cb.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/basic/basic-cb.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/basic/basic-cb.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/basic/basic-cb.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/basic/basic-cb.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/basic/basic.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/basic/basic.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/basic/basic.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/basic/basic.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/basic/basic.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/basic/basic.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/concentrator/concentrator-support-cli.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/concentrator/concentrator-support-cli.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/concentrator/concentrator-support-cli.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/concentrator/concentrator-support-cli.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/concentrator/concentrator-support-cli.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/concentrator/concentrator-support-cli.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/concentrator/concentrator-support.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/concentrator/concentrator-support.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/concentrator/concentrator-support.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/concentrator/concentrator-support.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/concentrator/concentrator-support.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/concentrator/concentrator-support.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/counters/af-counters.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/counters/af-counters.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/counters/af-counters.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/counters/af-counters.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/counters/af-counters.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/counters/af-counters.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/counters/counters-cb.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/counters/counters-cb.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/counters/counters-cb.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/counters/counters-cb.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/counters/counters-cb.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/counters/counters-cb.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/counters/counters-cli.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/counters/counters-cli.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/counters/counters-cli.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/counters/counters-cli.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/counters/counters-cli.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/counters/counters-cli.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/counters/counters-ota-host.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/counters/counters-ota-host.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/counters/counters-ota-host.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/counters/counters-ota-host.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/counters/counters-ota-host.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/counters/counters-ota-host.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/debug-print/sl_zigbee_debug_print.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/debug-print/sl_zigbee_debug_print.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/debug-print/sl_zigbee_debug_print.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/debug-print/sl_zigbee_debug_print.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/debug-print/sl_zigbee_debug_print.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/debug-print/sl_zigbee_debug_print.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/network-creator-security/network-creator-security-cli.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/network-creator-security/network-creator-security-cli.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/network-creator-security/network-creator-security-cli.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/network-creator-security/network-creator-security-cli.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/network-creator-security/network-creator-security-cli.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/network-creator-security/network-creator-security-cli.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/network-creator-security/network-creator-security.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/network-creator-security/network-creator-security.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/network-creator-security/network-creator-security.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/network-creator-security/network-creator-security.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/network-creator-security/network-creator-security.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/network-creator-security/network-creator-security.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/network-creator/network-creator-cb.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/network-creator/network-creator-cb.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/network-creator/network-creator-cb.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/network-creator/network-creator-cb.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/network-creator/network-creator-cb.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/network-creator/network-creator-cb.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/network-creator/network-creator-cli.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/network-creator/network-creator-cli.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/network-creator/network-creator-cli.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/network-creator/network-creator-cli.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/network-creator/network-creator-cli.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/network-creator/network-creator-cli.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/network-creator/network-creator.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/network-creator/network-creator.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/network-creator/network-creator.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/network-creator/network-creator.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/network-creator/network-creator.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/network-creator/network-creator.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/network-steering/network-steering-cb.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/network-steering/network-steering-cb.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/network-steering/network-steering-cb.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/network-steering/network-steering-cb.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/network-steering/network-steering-cb.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/network-steering/network-steering-cb.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/network-steering/network-steering-cli.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/network-steering/network-steering-cli.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/network-steering/network-steering-cli.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/network-steering/network-steering-cli.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/network-steering/network-steering-cli.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/network-steering/network-steering-cli.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/network-steering/network-steering-v2.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/network-steering/network-steering-v2.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/network-steering/network-steering-v2.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/network-steering/network-steering-v2.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/network-steering/network-steering-v2.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/network-steering/network-steering-v2.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/network-steering/network-steering.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/network-steering/network-steering.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/network-steering/network-steering.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/network-steering/network-steering.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/network-steering/network-steering.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/network-steering/network-steering.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/ota-common/ota-common.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-common/ota-common.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-common/ota-common.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-common/ota-common.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/ota-common/ota-common.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/ota-common/ota-common.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/ota-server-policy/ota-server-policy-cb.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-server-policy/ota-server-policy-cb.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-server-policy/ota-server-policy-cb.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-server-policy/ota-server-policy-cb.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/ota-server-policy/ota-server-policy-cb.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/ota-server-policy/ota-server-policy-cb.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/ota-server-policy/ota-server-policy.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-server-policy/ota-server-policy.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-server-policy/ota-server-policy.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-server-policy/ota-server-policy.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/ota-server-policy/ota-server-policy.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/ota-server-policy/ota-server-policy.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/ota-server/ota-server-cb.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-server/ota-server-cb.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-server/ota-server-cb.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-server/ota-server-cb.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/ota-server/ota-server-cb.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/ota-server/ota-server-cb.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/ota-server/ota-server-cli.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-server/ota-server-cli.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-server/ota-server-cli.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-server/ota-server-cli.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/ota-server/ota-server-cli.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/ota-server/ota-server-cli.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/ota-server/ota-server-dynamic-block-period.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-server/ota-server-dynamic-block-period.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-server/ota-server-dynamic-block-period.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-server/ota-server-dynamic-block-period.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/ota-server/ota-server-dynamic-block-period.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/ota-server/ota-server-dynamic-block-period.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/ota-server/ota-server-page-request.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-server/ota-server-page-request.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-server/ota-server-page-request.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-server/ota-server-page-request.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/ota-server/ota-server-page-request.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/ota-server/ota-server-page-request.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/ota-server/ota-server.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-server/ota-server.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-server/ota-server.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-server/ota-server.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/ota-server/ota-server.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/ota-server/ota-server.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/ota-storage-common/ota-storage-common-cli.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-storage-common/ota-storage-common-cli.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-storage-common/ota-storage-common-cli.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-storage-common/ota-storage-common-cli.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/ota-storage-common/ota-storage-common-cli.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/ota-storage-common/ota-storage-common-cli.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/ota-storage-common/ota-storage-common.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-storage-common/ota-storage-common.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-storage-common/ota-storage-common.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-storage-common/ota-storage-common.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/ota-storage-common/ota-storage-common.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/ota-storage-common/ota-storage-common.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/ota-storage-posix-filesystem/ota-storage-linux.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-storage-posix-filesystem/ota-storage-linux.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-storage-posix-filesystem/ota-storage-linux.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-storage-posix-filesystem/ota-storage-linux.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/ota-storage-posix-filesystem/ota-storage-linux.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/ota-storage-posix-filesystem/ota-storage-linux.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/reporting/reporting-cb.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/reporting/reporting-cb.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/reporting/reporting-cb.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/reporting/reporting-cb.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/reporting/reporting-cb.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/reporting/reporting-cb.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/reporting/reporting-cli.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/reporting/reporting-cli.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/reporting/reporting-cli.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/reporting/reporting-cli.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/reporting/reporting-cli.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/reporting/reporting-cli.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/reporting/reporting-default-configuration.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/reporting/reporting-default-configuration.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/reporting/reporting-default-configuration.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/reporting/reporting-default-configuration.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/reporting/reporting-default-configuration.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/reporting/reporting-default-configuration.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/reporting/reporting.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/reporting/reporting.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/reporting/reporting.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/reporting/reporting.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/reporting/reporting.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/reporting/reporting.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/scan-dispatch/scan-dispatch.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/scan-dispatch/scan-dispatch.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/scan-dispatch/scan-dispatch.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/scan-dispatch/scan-dispatch.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/scan-dispatch/scan-dispatch.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/scan-dispatch/scan-dispatch.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/trust-center-nwk-key-update-periodic/trust-center-nwk-key-update-periodic.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/trust-center-nwk-key-update-periodic/trust-center-nwk-key-update-periodic.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/trust-center-nwk-key-update-periodic/trust-center-nwk-key-update-periodic.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/trust-center-nwk-key-update-periodic/trust-center-nwk-key-update-periodic.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/trust-center-nwk-key-update-periodic/trust-center-nwk-key-update-periodic.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/trust-center-nwk-key-update-periodic/trust-center-nwk-key-update-periodic.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/trust-center-nwk-key-update-unicast/trust-center-nwk-key-update-unicast.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/trust-center-nwk-key-update-unicast/trust-center-nwk-key-update-unicast.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/trust-center-nwk-key-update-unicast/trust-center-nwk-key-update-unicast.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/trust-center-nwk-key-update-unicast/trust-center-nwk-key-update-unicast.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/trust-center-nwk-key-update-unicast/trust-center-nwk-key-update-unicast.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/trust-center-nwk-key-update-unicast/trust-center-nwk-key-update-unicast.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/update-tc-link-key/update-tc-link-key-cb.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/update-tc-link-key/update-tc-link-key-cb.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/update-tc-link-key/update-tc-link-key-cb.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/update-tc-link-key/update-tc-link-key-cb.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/update-tc-link-key/update-tc-link-key-cb.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/update-tc-link-key/update-tc-link-key-cb.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/update-tc-link-key/update-tc-link-key-cli.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/update-tc-link-key/update-tc-link-key-cli.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/update-tc-link-key/update-tc-link-key-cli.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/update-tc-link-key/update-tc-link-key-cli.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/update-tc-link-key/update-tc-link-key-cli.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/update-tc-link-key/update-tc-link-key-cli.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/update-tc-link-key/update-tc-link-key.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/update-tc-link-key/update-tc-link-key.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/update-tc-link-key/update-tc-link-key.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/update-tc-link-key/update-tc-link-key.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/update-tc-link-key/update-tc-link-key.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/update-tc-link-key/update-tc-link-key.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/zcl-framework-core/zcl-framework-core-cb.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/zcl-framework-core/zcl-framework-core-cb.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/zcl-framework-core/zcl-framework-core-cb.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/zcl-framework-core/zcl-framework-core-cb.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/zcl-framework-core/zcl-framework-core-cb.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/zcl-framework-core/zcl-framework-core-cb.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/security/af-node.o: $(SDK_PATH)/protocol/zigbee/app/framework/security/af-node.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/security/af-node.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/security/af-node.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/security/af-node.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/security/af-node.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/security/af-security-common.o: $(SDK_PATH)/protocol/zigbee/app/framework/security/af-security-common.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/security/af-security-common.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/security/af-security-common.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/security/af-security-common.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/security/af-security-common.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/security/af-trust-center.o: $(SDK_PATH)/protocol/zigbee/app/framework/security/af-trust-center.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/security/af-trust-center.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/security/af-trust-center.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/security/af-trust-center.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/security/af-trust-center.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/security/crypto-state.o: $(SDK_PATH)/protocol/zigbee/app/framework/security/crypto-state.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/security/crypto-state.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/security/crypto-state.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/security/crypto-state.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/security/crypto-state.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/service-function/sl_service_function.o: $(SDK_PATH)/protocol/zigbee/app/framework/service-function/sl_service_function.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/service-function/sl_service_function.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/service-function/sl_service_function.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/service-function/sl_service_function.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/service-function/sl_service_function.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/signature-decode/sl_signature_decode.o: $(SDK_PATH)/protocol/zigbee/app/framework/signature-decode/sl_signature_decode.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/signature-decode/sl_signature_decode.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/signature-decode/sl_signature_decode.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/signature-decode/sl_signature_decode.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/signature-decode/sl_signature_decode.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/af-common.o: $(SDK_PATH)/protocol/zigbee/app/framework/util/af-common.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/util/af-common.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/util/af-common.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/af-common.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/af-common.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/af-event.o: $(SDK_PATH)/protocol/zigbee/app/framework/util/af-event.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/util/af-event.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/util/af-event.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/af-event.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/af-event.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/af-host.o: $(SDK_PATH)/protocol/zigbee/app/framework/util/af-host.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/util/af-host.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/util/af-host.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/af-host.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/af-host.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/attribute-size.o: $(SDK_PATH)/protocol/zigbee/app/framework/util/attribute-size.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/util/attribute-size.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/util/attribute-size.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/attribute-size.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/attribute-size.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/attribute-storage.o: $(SDK_PATH)/protocol/zigbee/app/framework/util/attribute-storage.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/util/attribute-storage.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/util/attribute-storage.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/attribute-storage.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/attribute-storage.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/attribute-table.o: $(SDK_PATH)/protocol/zigbee/app/framework/util/attribute-table.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/util/attribute-table.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/util/attribute-table.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/attribute-table.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/attribute-table.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/client-api.o: $(SDK_PATH)/protocol/zigbee/app/framework/util/client-api.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/util/client-api.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/util/client-api.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/client-api.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/client-api.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/global-callback.o: $(SDK_PATH)/protocol/zigbee/app/framework/util/global-callback.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/util/global-callback.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/util/global-callback.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/global-callback.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/global-callback.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/global-other-callback.o: $(SDK_PATH)/protocol/zigbee/app/framework/util/global-other-callback.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/util/global-other-callback.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/util/global-other-callback.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/global-other-callback.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/global-other-callback.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/message.o: $(SDK_PATH)/protocol/zigbee/app/framework/util/message.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/util/message.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/util/message.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/message.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/message.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/multi-network.o: $(SDK_PATH)/protocol/zigbee/app/framework/util/multi-network.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/util/multi-network.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/util/multi-network.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/multi-network.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/multi-network.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/print-formatter.o: $(SDK_PATH)/protocol/zigbee/app/framework/util/print-formatter.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/util/print-formatter.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/util/print-formatter.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/print-formatter.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/print-formatter.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/print.o: $(SDK_PATH)/protocol/zigbee/app/framework/util/print.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/util/print.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/util/print.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/print.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/print.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/process-cluster-message.o: $(SDK_PATH)/protocol/zigbee/app/framework/util/process-cluster-message.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/util/process-cluster-message.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/util/process-cluster-message.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/process-cluster-message.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/process-cluster-message.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/process-global-message.o: $(SDK_PATH)/protocol/zigbee/app/framework/util/process-global-message.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/util/process-global-message.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/util/process-global-message.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/process-global-message.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/process-global-message.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/service-discovery-common.o: $(SDK_PATH)/protocol/zigbee/app/framework/util/service-discovery-common.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/util/service-discovery-common.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/util/service-discovery-common.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/service-discovery-common.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/service-discovery-common.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/service-discovery-host.o: $(SDK_PATH)/protocol/zigbee/app/framework/util/service-discovery-host.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/util/service-discovery-host.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/util/service-discovery-host.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/service-discovery-host.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/service-discovery-host.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/time-util.o: $(SDK_PATH)/protocol/zigbee/app/framework/util/time-util.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/util/time-util.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/util/time-util.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/time-util.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/time-util.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/util.o: $(SDK_PATH)/protocol/zigbee/app/framework/util/util.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/util/util.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/util/util.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/util.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/util.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/zcl-util.o: $(SDK_PATH)/protocol/zigbee/app/framework/util/zcl-util.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/util/zcl-util.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/util/zcl-util.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/zcl-util.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/zcl-util.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/common/library.o: $(SDK_PATH)/protocol/zigbee/app/util/common/library.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/util/common/library.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/util/common/library.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/common/library.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/common/library.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/ezsp/ezsp-callbacks.o: $(SDK_PATH)/protocol/zigbee/app/util/ezsp/ezsp-callbacks.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/util/ezsp/ezsp-callbacks.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/util/ezsp/ezsp-callbacks.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/ezsp/ezsp-callbacks.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/ezsp/ezsp-callbacks.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/ezsp/ezsp-enum-decode.o: $(SDK_PATH)/protocol/zigbee/app/util/ezsp/ezsp-enum-decode.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/util/ezsp/ezsp-enum-decode.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/util/ezsp/ezsp-enum-decode.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/ezsp/ezsp-enum-decode.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/ezsp/ezsp-enum-decode.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/ezsp/ezsp-frame-utilities.o: $(SDK_PATH)/protocol/zigbee/app/util/ezsp/ezsp-frame-utilities.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/util/ezsp/ezsp-frame-utilities.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/util/ezsp/ezsp-frame-utilities.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/ezsp/ezsp-frame-utilities.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/ezsp/ezsp-frame-utilities.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/ezsp/ezsp.o: $(SDK_PATH)/protocol/zigbee/app/util/ezsp/ezsp.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/util/ezsp/ezsp.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/util/ezsp/ezsp.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/ezsp/ezsp.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/ezsp/ezsp.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/ezsp/serial-interface-uart.o: $(SDK_PATH)/protocol/zigbee/app/util/ezsp/serial-interface-uart.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/util/ezsp/serial-interface-uart.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/util/ezsp/serial-interface-uart.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/ezsp/serial-interface-uart.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/ezsp/serial-interface-uart.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/security/security-address-cache-host.o: $(SDK_PATH)/protocol/zigbee/app/util/security/security-address-cache-host.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/util/security/security-address-cache-host.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/util/security/security-address-cache-host.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/security/security-address-cache-host.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/security/security-address-cache-host.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/serial/linux-serial.o: $(SDK_PATH)/protocol/zigbee/app/util/serial/linux-serial.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/util/serial/linux-serial.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/util/serial/linux-serial.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/serial/linux-serial.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/serial/linux-serial.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/serial/sl_zigbee_command_interpreter.o: $(SDK_PATH)/protocol/zigbee/app/util/serial/sl_zigbee_command_interpreter.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/util/serial/sl_zigbee_command_interpreter.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/util/serial/sl_zigbee_command_interpreter.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/serial/sl_zigbee_command_interpreter.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/serial/sl_zigbee_command_interpreter.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/zigbee-framework/zigbee-device-common.o: $(SDK_PATH)/protocol/zigbee/app/util/zigbee-framework/zigbee-device-common.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/util/zigbee-framework/zigbee-device-common.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/util/zigbee-framework/zigbee-device-common.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/zigbee-framework/zigbee-device-common.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/zigbee-framework/zigbee-device-common.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/zigbee-framework/zigbee-device-host.o: $(SDK_PATH)/protocol/zigbee/app/util/zigbee-framework/zigbee-device-host.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/app/util/zigbee-framework/zigbee-device-host.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/util/zigbee-framework/zigbee-device-host.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/zigbee-framework/zigbee-device-host.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/zigbee-framework/zigbee-device-host.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/stack/config/sl_zigbee_configuration_host_access.o: $(SDK_PATH)/protocol/zigbee/stack/config/sl_zigbee_configuration_host_access.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/stack/config/sl_zigbee_configuration_host_access.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/stack/config/sl_zigbee_configuration_host_access.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/config/sl_zigbee_configuration_host_access.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/config/sl_zigbee_configuration_host_access.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/stack/security/zigbee-security-manager-host.o: $(SDK_PATH)/protocol/zigbee/stack/security/zigbee-security-manager-host.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/protocol/zigbee/stack/security/zigbee-security-manager-host.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/stack/security/zigbee-security-manager-host.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/security/zigbee-security-manager-host.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/security/zigbee-security-manager-host.o

$(OUTPUT_DIR)/sdk/util/plugin/byte_utilities/byte-utilities.o: $(SDK_PATH)/util/plugin/byte_utilities/byte-utilities.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/plugin/byte_utilities/byte-utilities.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/plugin/byte_utilities/byte-utilities.c
CDEPS += $(OUTPUT_DIR)/sdk/util/plugin/byte_utilities/byte-utilities.d
OBJS += $(OUTPUT_DIR)/sdk/util/plugin/byte_utilities/byte-utilities.o

$(OUTPUT_DIR)/sdk/util/silicon_labs/silabs_core/event_queue/event-queue.o: $(SDK_PATH)/util/silicon_labs/silabs_core/event_queue/event-queue.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/silicon_labs/silabs_core/event_queue/event-queue.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/silicon_labs/silabs_core/event_queue/event-queue.c
CDEPS += $(OUTPUT_DIR)/sdk/util/silicon_labs/silabs_core/event_queue/event-queue.d
OBJS += $(OUTPUT_DIR)/sdk/util/silicon_labs/silabs_core/event_queue/event-queue.o

$(OUTPUT_DIR)/sdk/util/third_party/printf/printf.o: $(SDK_PATH)/util/third_party/printf/printf.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/printf/printf.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/printf/printf.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/printf/printf.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/printf/printf.o

$(OUTPUT_DIR)/sdk/util/third_party/printf/src/iostream_printf.o: $(SDK_PATH)/util/third_party/printf/src/iostream_printf.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/printf/src/iostream_printf.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/printf/src/iostream_printf.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/printf/src/iostream_printf.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/printf/src/iostream_printf.o

$(OUTPUT_DIR)/project/_/src/zigbee_host.o: ../src/zigbee_host.c
	@$(POSIX_TOOL_PATH)echo 'Building ../src/zigbee_host.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../src/zigbee_host.c
CDEPS += $(OUTPUT_DIR)/project/_/src/zigbee_host.d
OBJS += $(OUTPUT_DIR)/project/_/src/zigbee_host.o

$(OUTPUT_DIR)/project/_/src/zigbee_host_address_table.o: ../src/zigbee_host_address_table.c
	@$(POSIX_TOOL_PATH)echo 'Building ../src/zigbee_host_address_table.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../src/zigbee_host_address_table.c
CDEPS += $(OUTPUT_DIR)/project/_/src/zigbee_host_address_table.d
OBJS += $(OUTPUT_DIR)/project/_/src/zigbee_host_address_table.o

$(OUTPUT_DIR)/project/_/src/zigbee_host_attribute_mgmt.o: ../src/zigbee_host_attribute_mgmt.c
	@$(POSIX_TOOL_PATH)echo 'Building ../src/zigbee_host_attribute_mgmt.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../src/zigbee_host_attribute_mgmt.c
CDEPS += $(OUTPUT_DIR)/project/_/src/zigbee_host_attribute_mgmt.d
OBJS += $(OUTPUT_DIR)/project/_/src/zigbee_host_attribute_mgmt.o

$(OUTPUT_DIR)/project/_/src/zigbee_host_cli.o: ../src/zigbee_host_cli.c
	@$(POSIX_TOOL_PATH)echo 'Building ../src/zigbee_host_cli.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../src/zigbee_host_cli.c
CDEPS += $(OUTPUT_DIR)/project/_/src/zigbee_host_cli.d
OBJS += $(OUTPUT_DIR)/project/_/src/zigbee_host_cli.o

$(OUTPUT_DIR)/project/_/src/zigbee_host_cluster_command_parse.o: ../src/zigbee_host_cluster_command_parse.c
	@$(POSIX_TOOL_PATH)echo 'Building ../src/zigbee_host_cluster_command_parse.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../src/zigbee_host_cluster_command_parse.c
CDEPS += $(OUTPUT_DIR)/project/_/src/zigbee_host_cluster_command_parse.d
OBJS += $(OUTPUT_DIR)/project/_/src/zigbee_host_cluster_command_parse.o

$(OUTPUT_DIR)/project/_/src/zigbee_host_counters.o: ../src/zigbee_host_counters.c
	@$(POSIX_TOOL_PATH)echo 'Building ../src/zigbee_host_counters.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../src/zigbee_host_counters.c
CDEPS += $(OUTPUT_DIR)/project/_/src/zigbee_host_counters.d
OBJS += $(OUTPUT_DIR)/project/_/src/zigbee_host_counters.o

$(OUTPUT_DIR)/project/_/src/zigbee_host_discovery.o: ../src/zigbee_host_discovery.c
	@$(POSIX_TOOL_PATH)echo 'Building ../src/zigbee_host_discovery.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../src/zigbee_host_discovery.c
CDEPS += $(OUTPUT_DIR)/project/_/src/zigbee_host_discovery.d
OBJS += $(OUTPUT_DIR)/project/_/src/zigbee_host_discovery.o

$(OUTPUT_DIR)/project/_/src/zigbee_host_neighbor_table.o: ../src/zigbee_host_neighbor_table.c
	@$(POSIX_TOOL_PATH)echo 'Building ../src/zigbee_host_neighbor_table.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../src/zigbee_host_neighbor_table.c
CDEPS += $(OUTPUT_DIR)/project/_/src/zigbee_host_neighbor_table.d
OBJS += $(OUTPUT_DIR)/project/_/src/zigbee_host_neighbor_table.o

$(OUTPUT_DIR)/project/_/src/zigbee_host_ota_server.o: ../src/zigbee_host_ota_server.c
	@$(POSIX_TOOL_PATH)echo 'Building ../src/zigbee_host_ota_server.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../src/zigbee_host_ota_server.c
CDEPS += $(OUTPUT_DIR)/project/_/src/zigbee_host_ota_server.d
OBJS += $(OUTPUT_DIR)/project/_/src/zigbee_host_ota_server.o

$(OUTPUT_DIR)/project/_/src/zigbee_host_poll_checkin.o: ../src/zigbee_host_poll_checkin.c
	@$(POSIX_TOOL_PATH)echo 'Building ../src/zigbee_host_poll_checkin.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../src/zigbee_host_poll_checkin.c
CDEPS += $(OUTPUT_DIR)/project/_/src/zigbee_host_poll_checkin.d
OBJS += $(OUTPUT_DIR)/project/_/src/zigbee_host_poll_checkin.o

$(OUTPUT_DIR)/project/_/src/zigbee_host_sl_cli.o: ../src/zigbee_host_sl_cli.c
	@$(POSIX_TOOL_PATH)echo 'Building ../src/zigbee_host_sl_cli.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../src/zigbee_host_sl_cli.c
CDEPS += $(OUTPUT_DIR)/project/_/src/zigbee_host_sl_cli.d
OBJS += $(OUTPUT_DIR)/project/_/src/zigbee_host_sl_cli.o

$(OUTPUT_DIR)/project/_/src/zigbee_host_trust_center.o: ../src/zigbee_host_trust_center.c
	@$(POSIX_TOOL_PATH)echo 'Building ../src/zigbee_host_trust_center.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../src/zigbee_host_trust_center.c
CDEPS += $(OUTPUT_DIR)/project/_/src/zigbee_host_trust_center.d
OBJS += $(OUTPUT_DIR)/project/_/src/zigbee_host_trust_center.o

$(OUTPUT_DIR)/project/_/src/zigbee_host_util.o: ../src/zigbee_host_util.c
	@$(POSIX_TOOL_PATH)echo 'Building ../src/zigbee_host_util.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../src/zigbee_host_util.c
CDEPS += $(OUTPUT_DIR)/project/_/src/zigbee_host_util.d
OBJS += $(OUTPUT_DIR)/project/_/src/zigbee_host_util.o

$(OUTPUT_DIR)/project/_/src/zigbee_host_zcl_frame.o: ../src/zigbee_host_zcl_frame.c
	@$(POSIX_TOOL_PATH)echo 'Building ../src/zigbee_host_zcl_frame.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../src/zigbee_host_zcl_frame.c
CDEPS += $(OUTPUT_DIR)/project/_/src/zigbee_host_zcl_frame.d
OBJS += $(OUTPUT_DIR)/project/_/src/zigbee_host_zcl_frame.o

$(OUTPUT_DIR)/project/autogen/sl_cli_command_table.o: autogen/sl_cli_command_table.c
	@$(POSIX_TOOL_PATH)echo 'Building autogen/sl_cli_command_table.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ autogen/sl_cli_command_table.c
CDEPS += $(OUTPUT_DIR)/project/autogen/sl_cli_command_table.d
OBJS += $(OUTPUT_DIR)/project/autogen/sl_cli_command_table.o

$(OUTPUT_DIR)/project/autogen/sl_cli_instances.o: autogen/sl_cli_instances.c
	@$(POSIX_TOOL_PATH)echo 'Building autogen/sl_cli_instances.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ autogen/sl_cli_instances.c
CDEPS += $(OUTPUT_DIR)/project/autogen/sl_cli_instances.d
OBJS += $(OUTPUT_DIR)/project/autogen/sl_cli_instances.o

$(OUTPUT_DIR)/project/autogen/sl_cluster_service_gen.o: autogen/sl_cluster_service_gen.c
	@$(POSIX_TOOL_PATH)echo 'Building autogen/sl_cluster_service_gen.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ autogen/sl_cluster_service_gen.c
CDEPS += $(OUTPUT_DIR)/project/autogen/sl_cluster_service_gen.d
OBJS += $(OUTPUT_DIR)/project/autogen/sl_cluster_service_gen.o

$(OUTPUT_DIR)/project/autogen/sl_event_handler.o: autogen/sl_event_handler.c
	@$(POSIX_TOOL_PATH)echo 'Building autogen/sl_event_handler.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ autogen/sl_event_handler.c
CDEPS += $(OUTPUT_DIR)/project/autogen/sl_event_handler.d
OBJS += $(OUTPUT_DIR)/project/autogen/sl_event_handler.o

$(OUTPUT_DIR)/project/autogen/sl_iostream_handles.o: autogen/sl_iostream_handles.c
	@$(POSIX_TOOL_PATH)echo 'Building autogen/sl_iostream_handles.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ autogen/sl_iostream_handles.c
CDEPS += $(OUTPUT_DIR)/project/autogen/sl_iostream_handles.d
OBJS += $(OUTPUT_DIR)/project/autogen/sl_iostream_handles.o

$(OUTPUT_DIR)/project/autogen/sli_cli_hooks.o: autogen/sli_cli_hooks.c
	@$(POSIX_TOOL_PATH)echo 'Building autogen/sli_cli_hooks.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ autogen/sli_cli_hooks.c
CDEPS += $(OUTPUT_DIR)/project/autogen/sli_cli_hooks.d
OBJS += $(OUTPUT_DIR)/project/autogen/sli_cli_hooks.o

$(OUTPUT_DIR)/project/autogen/zigbee_common_callback_dispatcher.o: autogen/zigbee_common_callback_dispatcher.c
	@$(POSIX_TOOL_PATH)echo 'Building autogen/zigbee_common_callback_dispatcher.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ autogen/zigbee_common_callback_dispatcher.c
CDEPS += $(OUTPUT_DIR)/project/autogen/zigbee_common_callback_dispatcher.d
OBJS += $(OUTPUT_DIR)/project/autogen/zigbee_common_callback_dispatcher.o

$(OUTPUT_DIR)/project/autogen/zigbee_host_callback_dispatcher.o: autogen/zigbee_host_callback_dispatcher.c
	@$(POSIX_TOOL_PATH)echo 'Building autogen/zigbee_host_callback_dispatcher.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ autogen/zigbee_host_callback_dispatcher.c
CDEPS += $(OUTPUT_DIR)/project/autogen/zigbee_host_callback_dispatcher.d
OBJS += $(OUTPUT_DIR)/project/autogen/zigbee_host_callback_dispatcher.o

$(OUTPUT_DIR)/project/autogen/zigbee_stack_callback_dispatcher.o: autogen/zigbee_stack_callback_dispatcher.c
	@$(POSIX_TOOL_PATH)echo 'Building autogen/zigbee_stack_callback_dispatcher.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ autogen/zigbee_stack_callback_dispatcher.c
CDEPS += $(OUTPUT_DIR)/project/autogen/zigbee_stack_callback_dispatcher.d
OBJS += $(OUTPUT_DIR)/project/autogen/zigbee_stack_callback_dispatcher.o

$(OUTPUT_DIR)/project/autogen/zigbee_zcl_callback_dispatcher.o: autogen/zigbee_zcl_callback_dispatcher.c
	@$(POSIX_TOOL_PATH)echo 'Building autogen/zigbee_zcl_callback_dispatcher.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ autogen/zigbee_zcl_callback_dispatcher.c
CDEPS += $(OUTPUT_DIR)/project/autogen/zigbee_zcl_callback_dispatcher.d
OBJS += $(OUTPUT_DIR)/project/autogen/zigbee_zcl_callback_dispatcher.o

# Automatically-generated Simplicity Studio Metadata
# Please do not edit or delete these lines!
# SIMPLICITY_STUDIO_METADATA=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END_SIMPLICITY_STUDIO_METADATA
# END OF METADATA