
TOPNAME = top
##NXDC_FILES = constr/top.nxdc
INC_PATH ?=
VERILATOR = verilator
VERILATOR_CFLAGS += -MMD --build -cc -O3 --x-assign fast --x-initial fast --noassert --trace
PROJECT_ROOT := $(abspath .)
BUILD_DIR = ./build
OBJ_DIR = $(BUILD_DIR)/obj_dir
BIN = $(BUILD_DIR)/$(TOPNAME)

IMG ?= 
override IMG := $(IMG)


default: $(BIN)

$(shell mkdir -p $(BUILD_DIR))

# constraint file
##SRC_AUTO_BIND = $(abspath $(BUILD_DIR)/auto_bind.cpp)
##$(SRC_AUTO_BIND): $(NXDC_FILES)
##	python3 $(NVBOARD_HOME)/scripts/auto_pin_bind.py $^ $@

# project source
VSRCS = $(shell find $(abspath ./vsrc) -name "*.v")
CSRCS = $(shell find $(abspath ./csrc) -name "*.c" -or -name "*.cc" -or -name "*.cpp")
CSRCS += $(SRC_AUTO_BIND)


#capstone
LIBCAPSTONE = $(NEMU_HOME)/tools/capstone/repo/libcapstone.so.5
CXXFLAGS += -I$(NEMU_HOME)/tools/capstone/repo/include
csrc/tools/disasm.c: $(LIBCAPSTONE)
$(LIBCAPSTONE):
	$(MAKE) -C $(NEMU_HOME)/tools/capstone
###


# rules for NVBoard
##include $(NVBOARD_HOME)/scripts/nvboard.mk
# rules for verilator

INCFLAGS = $(addprefix -I, $(INC_PATH))
CXXFLAGS += $(INCFLAGS) -DTOP_NAME="\"V$(TOPNAME)\"" \
			-I$(PROJECT_ROOT)/include \
			-DNPC_IMAGE_PATH="\\\"$(IMG)\\\""\
			-I$(NEMU_HOME)/include
LDFLAGS += -L$(NEMU_HOME)/tools/capstone/repo -lcapstone \
			-lreadline
#           -L$(NEMU_HOME)/build -lriscv32-nemu-interpreter \
           
			 



$(BIN): $(VSRCS) $(CSRCS) $(NVBOARD_ARCHIVE) 
	@rm -rf $(OBJ_DIR)
	$(VERILATOR) $(VERILATOR_CFLAGS) \
		--top-module $(TOPNAME) $^ \
		$(addprefix -CFLAGS , $(CXXFLAGS)) $(addprefix -LDFLAGS , $(LDFLAGS)) \
		--Mdir $(OBJ_DIR) --exe -o $(abspath $(BIN))
	$(info ..[DEBUG] IMG = $(IMG))

all: default

clean-run: clean run


run: $(BIN)
	$(info [DEBUG] IMG = $(IMG))
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	@$^
clean:
	rm -rf $(BUILD_DIR)

.PHONY: default all clean run clean-run
