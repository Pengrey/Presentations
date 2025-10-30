#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Support/FormatVariadic.h"
#include "llvm/Support/raw_ostream.h"

#include <cstdlib>
#include <ctime>

#define SPLIT_CHANCE_PERCENT 50 // 50% chance that any given eligible block will be split

using namespace llvm;

namespace {
    struct SplitBasicBlocks : public PassInfoMixin<SplitBasicBlocks> {
        PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM) {
            errs() << formatv("\n[>] Split Basic Blocks Pass\n");
            srand(time(NULL));

            auto &CTX = M.getContext();
            IntegerType *int32Ty = IntegerType::getInt32Ty(CTX);

            FunctionCallee randFunc = M.getOrInsertFunction("rand", int32Ty);

            auto containsPHI = [](const BasicBlock *BB) {
                for (const Instruction &I : *BB) {
                    if (isa<PHINode>(&I)) return true;
                }
                return false;
            };

            for (Function &F : M) {
                if (F.isDeclaration()) continue;

                F.addFnAttr(Attribute::NoInline);

                std::vector<BasicBlock *> worklist;
                for (BasicBlock &BB : F) {
                    if (BB.size() >= 3 && !containsPHI(&BB)) {
                        worklist.push_back(&BB);                                                                    // Save to modify later
                    }
                }

                if (worklist.empty()) continue;

                errs() << formatv("[*] Targeting {0,10} blocks in function {1,-20}", worklist.size(), F.getName());

                for (BasicBlock *BB : worklist) {
                    if ((rand() % 100) >= SPLIT_CHANCE_PERCENT) {
                        continue;
                    }

                    unsigned splitIdx = 1 + (rand() % (BB->size() - 2));                                            // Get index to split BB
                    auto splitIt = std::next(BB->begin(), splitIdx);

                    BasicBlock *successor = BB->splitBasicBlock(splitIt, BB->getName() + ".split");
                    Instruction *oldTerminator = BB->getTerminator();

                    BasicBlock *dummyBlock = BasicBlock::Create(CTX, BB->getName() + ".dummy", &F, successor);
                    IRBuilder<>(dummyBlock).CreateBr(successor);

                    bool condition = (rand() % 2 == 0);
                    Value* fixedCond = condition ? ConstantInt::getTrue(CTX) : ConstantInt::getFalse(CTX);

                    IRBuilder<> builder(oldTerminator);
                    builder.CreateCondBr(fixedCond, successor, dummyBlock);
                    oldTerminator->eraseFromParent();

                    //errs() << formatv("[REPLACED]: Block was slpitted\t");
                }

                errs() << "[Done]\n";
            }
            return PreservedAnalyses::none();
        }
    };
}
