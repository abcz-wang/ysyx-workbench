#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "Vtop.h"
#include "verilated.h"
#include "verilated_vcd_c.h"

int main(int argc, char** argv){
      VerilatedContext* contextp = new VerilatedContext;
      contextp->commandArgs(argc, argv);
      contextp->traceEverOn(true);

      Vtop* top = new Vtop{contextp};
      Verilated::mkdir("logs");
      VerilatedVcdC* tfp = new VerilatedVcdC;
      top->trace(tfp, 99);
      tfp->open("logs/vlt_dump.vcd");

      for (int i = 0; i < 10; i++)
      { 
        int a = rand() & 1;
        int b = rand() & 1;
        top->a = a;
        top->b = b;
        top->eval();
        tfp->dump(contextp->time());
        contextp->timeInc(1);
        printf("a = %d, b = %d, f = %d\n", a, b, top->f);
        assert(top->f == (a ^ b));
      }
      top->final();
      tfp->close();
      delete tfp;
      delete top;
      delete contextp;
      return 0;

}
