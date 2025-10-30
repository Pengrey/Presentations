# Simple Hello World Pass

This is a simple LLVM pass that prints "Hello, World!" to the standard output when run.

LLVM passes are typically written in C++ and can be executed through the compiler infrastructure by first compiling them into a shared library and then loading them using the `-fpass-plugin=<path-to-pass>` option with `clang`.

To first create the pass, we need to first include the necessary LLVM headers and define our pass class:

```cpp
#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;                                                               // Use the llvm namespace to avoid prefixing llvm:: everywhere

namespace {
    struct HelloWorldPass : public PassInfoMixin<HelloWorldPass> {                  // Use PassInfoMixin to simplify pass creation (new pass manager style <= LLVM 10)
        PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM) {               // The run method is where the pass logic is implemented
            errs() << "Hello, World!\n";
            return PreservedAnalyses::all();                                        // Indicate that we did not modify the IR (all analyses are preserved)
        };
    };
}

extern "C" LLVM_ATTRIBUTE_WEAK PassPluginLibraryInfo llvmGetPassPluginInfo() {      // This function is required for LLVM to recognize the pass plugin
    return {
        .APIVersion = LLVM_PLUGIN_API_VERSION,                                      // Use the current LLVM plugin API version   
        .PluginName = "HelloWorldPass",                                             // Name of the pass
        .PluginVersion = "v0.1",                                                    // Version of the pass
        .RegisterPassBuilderCallbacks = [](PassBuilder &PB) {                       // Register the pass with the PassBuilder
            PB.registerPipelineStartEPCallback(
                [](ModulePassManager &MPM, OptimizationLevel Level) {               
                    MPM.addPass(HelloWorldPass());                                  // Add our pass to the module pass manager   
                });
        }
    };
}
  
  
