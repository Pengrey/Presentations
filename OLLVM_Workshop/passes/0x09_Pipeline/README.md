# LLVM Obfuscation Pass Pipeline

This code serves as a plugin entry point for registering a custom pipeline of LLVM obfuscation passes. The goal is to apply the previous three passes into one pass pipeline that can be easily invoked.

The passes are registered within the `RegisterPassBuilderCallbacks` lambda. This callback is invoked by the `PassBuilder` at the beginning of the optimization pipeline construction (`PipelineStartEPCallback`).

Inside the callback, each custom pass is added to the `ModulePassManager` (`MPM`). The order in which `MPM.addPass(...)` is called dictates the execution order of the passes.

```cpp
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
```

As defined in the code, the obfuscation passes will run in the following sequence: `ControlFlowFlattening` > `SplitBasicBlocks` > `ArithmeticObf`. This order is chosen to first flatten the control flow, then split basic blocks to increase complexity, and finally apply arithmetic obfuscation to further obscure the program's logic.

> Note: In this case we use `#inlucde "*.cc"` to include the pass source files directly for simplicity, but it is recommended to use header files for better modularity and maintainability in larger projects.