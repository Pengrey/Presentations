#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

using namespace llvm;

#include "ControlFlowFlattening.cc"
#include "SplitBasicBlocks.cc"
#include "ArithmeticObf.cc"

extern "C" LLVM_ATTRIBUTE_WEAK PassPluginLibraryInfo llvmGetPassPluginInfo() {
    return {
        .APIVersion = LLVM_PLUGIN_API_VERSION,
        .PluginName = "Pipeline",
        .PluginVersion = "v0.1",
        .RegisterPassBuilderCallbacks = [](PassBuilder &PB) {
            PB.registerPipelineStartEPCallback(
                [](ModulePassManager &MPM, OptimizationLevel Level) {
                    // They will run in this order
                    MPM.addPass(ControlFlowFlattening());
                    MPM.addPass(SplitBasicBlocks());
                    MPM.addPass(ArithmeticObf());
                });
        }
    };
}
