// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Design internal header
// See Vexample.h for the primary calling header

#ifndef VERILATED_VEXAMPLE___024ROOT_H_
#define VERILATED_VEXAMPLE___024ROOT_H_  // guard

#include "verilated.h"


class Vexample__Syms;

class alignas(VL_CACHE_LINE_BYTES) Vexample___024root final {
  public:

    // DESIGN SPECIFIC STATE
    VL_IN8(a,0,0);
    VL_IN8(b,0,0);
    VL_OUT8(f,0,0);
    CData/*0:0*/ __VstlFirstIteration;
    CData/*0:0*/ __VicoFirstIteration;
    VlUnpacked<QData/*63:0*/, 1> __VstlTriggered;
    VlUnpacked<QData/*63:0*/, 1> __VicoTriggered;

    // INTERNAL VARIABLES
    Vexample__Syms* vlSymsp;
    const char* vlNamep;

    // CONSTRUCTORS
    Vexample___024root(Vexample__Syms* symsp, const char* namep);
    ~Vexample___024root();
    VL_UNCOPYABLE(Vexample___024root);

    // INTERNAL METHODS
    void __Vconfigure(bool first);
};


#endif  // guard
