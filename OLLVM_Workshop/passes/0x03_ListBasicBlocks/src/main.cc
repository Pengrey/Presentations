#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {
    struct ListBasicBlocks : public PassInfoMixin<ListBasicBlocks> {
        PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM) {
            for (auto& F : M) {
                for (auto& BB : F) {
                    errs() << "Basic Block:\n" << BB << "\n";
                }
            }
            return PreservedAnalyses::all();
        };
    };
}

extern "C" LLVM_ATTRIBUTE_WEAK PassPluginLibraryInfo llvmGetPassPluginInfo() {
    return {
        .APIVersion = LLVM_PLUGIN_API_VERSION,
        .PluginName = "ListBasicBlocks",
        .PluginVersion = "v0.1",
        .RegisterPassBuilderCallbacks = [](PassBuilder &PB) {
            PB.registerPipelineStartEPCallback(
                [](ModulePassManager &MPM, OptimizationLevel Level) {
                    MPM.addPass(ListBasicBlocks());
                });
        }
    };
}
