#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <nvboard.h>
#include "Vtop.h"
#include "verilated.h"
#include "verilated_vcd_c.h"

void nvboard_bind_all_pins(TOP_NAME* top);

int main(int argc, char** argv){
      VerilatedContext* contextp = new VerilatedContext;
      contextp->commandArgs(argc, argv);
      contextp->traceEverOn(true);

      TOP_NAME* dut = new TOP_NAME{contextp};
      nvboard_bind_all_pins(dut);
      nvboard_init();

      Verilated::mkdir("logs");
      VerilatedVcdC* tfp = new VerilatedVcdC;
      dut->trace(tfp, 99);
      tfp->open("logs/vlt_dump.vcd");

      while(1)
      { 
        nvboard_update();
        dut->eval();
        assert(dut->f == (dut->a ^ dut->b));
        tfp->dump(contextp->time());
        contextp->timeInc(1);
        nvboard_update();
      }
      dut->final();
      tfp->close();
      delete tfp;
      delete dut;
      delete contextp;
      nvboard_quit();

      return 0;

}
