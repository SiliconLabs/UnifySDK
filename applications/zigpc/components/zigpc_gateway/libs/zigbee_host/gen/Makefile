####################################################################
# Library Makefile for Host Systems                                #
####################################################################

-include libzigbee_host.Makefile

AR_FLAGS = rs

debug_lib: C_FLAGS += $(C_FLAGS_DEBUG)
debug_lib: CXX_FLAGS += $(CXX_FLAGS_DEBUG)
debug_lib: ASM_FLAGS += $(ASM_FLAGS_DEBUG)
debug_lib: $(OUTPUT_DIR)/$(PROJECTNAME).a

release_lib: C_FLAGS += $(C_FLAGS_RELEASE)
release_lib: CXX_FLAGS += $(CXX_FLAGS_RELEASE)
release_lib: ASM_FLAGS += $(ASM_FLAGS_RELEASE)
release_lib: $(OUTPUT_DIR)/$(PROJECTNAME).a

$(OUTPUT_DIR)/$(PROJECTNAME).a: $(OBJS) $(LIB_FILES)
	@echo 'Archiving $(OUTPUT_DIR)/$(PROJECTNAME).a'
	$(ECHO)$(AR) $(AR_FLAGS) $(OUTPUT_DIR)/$(PROJECTNAME).a $(OBJS)
	@echo 'Done.'
