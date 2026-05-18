// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Design implementation internals
// See Vexample.h for the primary calling header

#include "Vexample__pch.h"

#ifdef VL_DEBUG
VL_ATTR_COLD void Vexample___024root___dump_triggers__ico(const VlUnpacked<QData/*63:0*/, 1> &triggers, const std::string &tag);
#endif  // VL_DEBUG

void Vexample___024root___eval_triggers__ico(Vexample___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vexample___024root___eval_triggers__ico\n"); );
    Vexample__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Body
    vlSelfRef.__VicoTriggered[0U] = ((0xfffffffffffffffeULL 
                                      & vlSelfRef.__VicoTriggered
                                      [0U]) | (IData)((IData)(vlSelfRef.__VicoFirstIteration)));
    vlSelfRef.__VicoFirstIteration = 0U;
#ifdef VL_DEBUG
    if (VL_UNLIKELY(vlSymsp->_vm_contextp__->debug())) {
        Vexample___024root___dump_triggers__ico(vlSelfRef.__VicoTriggered, "ico"s);
    }
#endif
}

bool Vexample___024root___trigger_anySet__ico(const VlUnpacked<QData/*63:0*/, 1> &in) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vexample___024root___trigger_anySet__ico\n"); );
    // Locals
    IData/*31:0*/ n;
    // Body
    n = 0U;
    do {
        if (in[n]) {
            return (1U);
        }
        n = ((IData)(1U) + n);
    } while ((1U > n));
    return (0U);
}

void Vexample___024root___ico_sequent__TOP__0(Vexample___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vexample___024root___ico_sequent__TOP__0\n"); );
    Vexample__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Body
    vlSelfRef.f = ((IData)(vlSelfRef.a) ^ (IData)(vlSelfRef.b));
}

void Vexample___024root___eval_ico(Vexample___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vexample___024root___eval_ico\n"); );
    Vexample__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Body
    if ((1ULL & vlSelfRef.__VicoTriggered[0U])) {
        Vexample___024root___ico_sequent__TOP__0(vlSelf);
    }
}

bool Vexample___024root___eval_phase__ico(Vexample___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vexample___024root___eval_phase__ico\n"); );
    Vexample__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Locals
    CData/*0:0*/ __VicoExecute;
    // Body
    Vexample___024root___eval_triggers__ico(vlSelf);
    __VicoExecute = Vexample___024root___trigger_anySet__ico(vlSelfRef.__VicoTriggered);
    if (__VicoExecute) {
        Vexample___024root___eval_ico(vlSelf);
    }
    return (__VicoExecute);
}

void Vexample___024root___eval(Vexample___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vexample___024root___eval\n"); );
    Vexample__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Locals
    IData/*31:0*/ __VicoIterCount;
    // Body
    __VicoIterCount = 0U;
    vlSelfRef.__VicoFirstIteration = 1U;
    do {
        if (VL_UNLIKELY(((0x00000064U < __VicoIterCount)))) {
#ifdef VL_DEBUG
            Vexample___024root___dump_triggers__ico(vlSelfRef.__VicoTriggered, "ico"s);
#endif
            VL_FATAL_MT("vsrc/example.v", 1, "", "Input combinational region did not converge after 100 tries");
        }
        __VicoIterCount = ((IData)(1U) + __VicoIterCount);
    } while (Vexample___024root___eval_phase__ico(vlSelf));
}

#ifdef VL_DEBUG
void Vexample___024root___eval_debug_assertions(Vexample___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vexample___024root___eval_debug_assertions\n"); );
    Vexample__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Body
    if (VL_UNLIKELY(((vlSelfRef.a & 0xfeU)))) {
        Verilated::overWidthError("a");
    }
    if (VL_UNLIKELY(((vlSelfRef.b & 0xfeU)))) {
        Verilated::overWidthError("b");
    }
}
#endif  // VL_DEBUG
