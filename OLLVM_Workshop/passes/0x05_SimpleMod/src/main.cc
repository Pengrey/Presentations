#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {
    struct SimpleMod : public PassInfoMixin<SimpleMod> {
        PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM) {
            for (auto& F : M) {
                for (auto& BB : F) {
                    for (auto& I : BB) {
                        if (auto *binOp = dyn_cast<BinaryOperator>(&I)) {                       // Check if the instruction is a binary operation
                            if (binOp->getOpcode() == Instruction::Add) {                       // Check if it's an addition operation
                                errs() << "Found add instruction: " << *binOp << "\n";

                                IRBuilder<> builder(binOp);                                     // Create an IRBuilder to help with instruction creation  
                                Value *newLHS = binOp->getOperand(0);                           // Get the left-hand side operand (in the test case, this is '2')
                                Value *newRHS = binOp->getOperand(1);                           // Get the right-hand side operand (in the test case, this is '1')
                                Value *newSub = builder.CreateSub(newLHS, newRHS);              // Create a new subtraction instruction (2 - 1)

                                binOp->replaceAllUsesWith(newSub);                              // Redirect the old instruction to use the new one
                                errs() << "Replaced with sub instruction: " << *newSub << "\n";
                            }
                        }
                    }
                }
            }
            return PreservedAnalyses::none();                                                   // We modified the IR, so we return none
        };
    };
}

extern "C" LLVM_ATTRIBUTE_WEAK PassPluginLibraryInfo llvmGetPassPluginInfo() {
    return {
        .APIVersion = LLVM_PLUGIN_API_VERSION,
        .PluginName = "SimpleMod",
        .PluginVersion = "v0.1",
        .RegisterPassBuilderCallbacks = [](PassBuilder &PB) {
            PB.registerPipelineStartEPCallback(
                [](ModulePassManager &MPM, OptimizationLevel Level) {
                    MPM.addPass(SimpleMod());
                });
        }
    };
}
