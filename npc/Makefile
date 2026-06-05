NXDC_FILES = constr/top.nxdc
TOPNAME = top

# rules for NVBoard
include $(NVBOARD_HOME)/scripts/nvboard.mk
INCFLAGS = $(addprefix -I, $(INC_PATH))
CXXFLAGS += $(INCFLAGS) -DTOP_NAME=V$(TOPNAME)

all:

sim:$(NVBOARD_ARCHIVE)
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	python3 $(NVBOARD_HOME)/scripts/auto_pin_bind.py $(NXDC_FILES) ./csrc/auto_bind.cpp
	verilator --cc --exe --trace-vcd --build -j 0 -Wall \
		--top-module $(TOPNAME) \
		./vsrc/top.v ./csrc/main.cpp ./csrc/auto_bind.cpp $(NVBOARD_ARCHIVE) \
		$(addprefix -CFLAGS , $(CXXFLAGS)) \
		$(addprefix -LDFLAGS , $(LDFLAGS))
	./obj_dir/Vtop
include ../Makefile


