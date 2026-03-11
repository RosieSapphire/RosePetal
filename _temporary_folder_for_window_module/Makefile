#################
# CONFIGURATION #
#################
DEBUG_MODE  ?= 1
VERBOSE     ?=
WANT_COLORS ?= 1
SUCCESS_MSG := Built successfully!

# Verbosity
V := @
ifdef VERBOSE
	V :=
endif

# Terminal Colors
ifneq ($(and                                                        \
       $(if $(WANT_COLORS),1,),                                     \
       $(if $(NO_COLOR),,1),                                        \
       $(filter-out dumb,$(TERM)),                                  \
       $(shell command -v tput 2>/dev/null),                        \
       $(filter-out 0,$(shell tput colors 2>/dev/null))),)
	COL_RED := $(shell tput setaf 1)
	COL_GRN := $(shell tput setaf 2)
	COL_YEL := $(shell tput setaf 3)
	COL_BLU := $(shell tput setaf 4)
	COL_PUR := $(shell tput setaf 5)
	COL_GRY := $(shell tput setaf 6)
	COL_END := $(shell tput sgr0)
endif

# Debug mode changes
ifdef DEBUG_MODE
	T3DE_CFLAGS := -O0 -ggdb3 -D_DEBUG
	SUCCESS_MSG +=  $(COL_RED)[DEBUG BUILD]$(COL_END)
else
	T3DE_CFLAGS := -Ofast -g0 -DNDEBUG
endif


###########
# GENERAL #
###########
BUILD_DIR := build
CC        := gcc

################
# DEPENDENCIES #
################

# Common
COMMON_DIR       := ../../deps/common
COMMON_INC_FLAGS := -I$(COMMON_DIR)/include

# glad
GLAD_CFLAGS    := --no-warnings -g0 -Ofast
GLAD_DIR       := ../../deps/glad
GLAD_INC_FLAGS := -I$(GLAD_DIR)/include
GLAD_SLIB      := $(GLAD_DIR)/libglad.a
GLAD_LNK_FLAGS := -L$(GLAD_DIR) -l$(patsubst lib%.a,%,$(notdir $(GLAD_SLIB)))

# GLFW
GLFW_DIR       := deps/GLFW
GLFW_INC_FLAGS := -I$(GLFW_DIR)/include
GLFW_SLIB      := $(GLFW_DIR)/libglfw3.a
GLFW_LNK_FLAGS := -L$(GLFW_DIR) -l$(patsubst lib%.a,%,$(notdir $(GLFW_SLIB)))

# cglm
CGLM_DIR       := deps/cglm
CGLM_INC_FLAGS := -I$(CGLM_DIR)/include
CGLM_SLIB      := $(CGLM_DIR)/libcglm.a
CGLM_LNK_FLAGS := -L$(CGLM_DIR) -l$(patsubst lib%.a,%,$(notdir $(CGLM_SLIB)))

# TODO: Turn this into an object file and link it with the program
# nuklear
NUKLEAR_DIR       := deps/nuklear
NUKLEAR_INC_FLAGS := -I$(NUKLEAR_DIR)
# NUKLEAR_C_FILE    := $(NUKLEAR_DIR)/nuklear.c
# NUKLEAR_O_FILE    := $(BUILD_DIR)/nuklear.o
# NUKLEAR_SLIB      := $(BUILD_DIR)/libnuklear.a
# NUKLEAR_LNK_FLAGS := -L $(BUILD_DIR) -l$(notdir $(NUKLEAR_SLIB:lib%.a=%))

##############
# T3D EDITOR #
##############

T3DE_EXE       := $(BUILD_DIR)/t3d_editor
T3DE_INC_FLAGS := -Isrc                \
	          $(GLAD_INC_FLAGS)    \
		  $(GLFW_INC_FLAGS)    \
		  $(CGLM_INC_FLAGS)    \
	          $(NUKLEAR_INC_FLAGS) \
		  $(COMMON_INC_FLAGS)  \
		  $(T3D_INC_FLAGS)
T3DE_CFLAGS    += $(T3DE_INC_FLAGS)
T3DE_C_FILES   := $(foreach dir,src,$(wildcard $(dir)/*.c))
T3DE_O_FILES   := $(T3DE_C_FILES:%.c=$(BUILD_DIR)/%.o)

######################
# BUILD INSTRUCTIONS #
######################

.PHONY: all clean

all: $(T3DE_EXE)
	@echo "$(COL_GRN)$(SUCCESS_MSG)$(COL_END)"

$(T3DE_EXE): $(T3DE_O_FILES)
	@mkdir -p $(dir $@)
	@echo "$(COL_BLU)    [LD] $@$(COL_END)"
	$(V)$(CC) $(T3DE_CFLAGS) -o $@ $^ \
		$(GLAD_LNK_FLAGS) \
		$(GLFW_LNK_FLAGS) \
		$(CGLM_LNK_FLAGS) \
		-lm
		
$(BUILD_DIR)/src/%.o: src/%.c
	@mkdir -p $(dir $@)
	@echo "$(COL_YEL)    [CC] $<$(COL_END)"
	$(V)$(CC) $(T3DE_CFLAGS) -o $@ -c $^

clean:
	@echo "Cleaning previous build..."
	$(V)rm -rf $(BUILD_DIR) .cache/ compile_commands.json
