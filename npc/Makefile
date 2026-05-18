all:

sim:
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	verilator --cc --exe --build -j 0 -Wall --trace-vcd ./csrc/main.cpp ./vsrc/top.v
	./obj_dir/Vtop

include ../Makefile
