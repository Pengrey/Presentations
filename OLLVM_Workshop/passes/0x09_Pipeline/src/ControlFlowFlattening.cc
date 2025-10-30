#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/FormatVariadic.h"
#include "llvm/Support/raw_ostream.h"

#include <vector>
#include <map>

using namespace llvm;

namespace {

    struct ControlFlowFlattening : public PassInfoMixin<ControlFlowFlattening> {
        PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM) {
            errs() << formatv("\n[>] Control Flow Flattening Pass\n");
            for (Function &F : M) {
                if (F.isDeclaration() || F.size() < 3) {
                    continue;
                }

                bool hasPHINodes = false;
                for (BasicBlock &BB : F) {
                    if (isa<PHINode>(BB.front())) {
                        hasPHINodes = true;
                        break;
                    }
                }

                if (hasPHINodes) {
                    errs() << formatv("[*] Skipping function {0,-25} (contains PHI nodes)\n", F.getName());
                    continue;
                }

                errs() << formatv("[*] Flattening function {0,-40}", F.getName());

                auto &CTX = F.getContext();
                IntegerType *int32Ty = IntegerType::getInt32Ty(CTX);

                // 2. Prepare blocks for flattening.
                BasicBlock *entryBlock = &F.getEntryBlock();

                if (entryBlock->getTerminator()->getNumSuccessors() != 1) {
                    entryBlock->splitBasicBlock(entryBlock->getTerminator(), "entry.split");
                }

                std::vector<BasicBlock *> originalBlocks;
                for (BasicBlock &BB : F) {
                    if (&BB != entryBlock) {
                        originalBlocks.push_back(&BB);
                    }
                }

                if (originalBlocks.empty()) {
                    continue;
                }

                // 3. Create the dispatcher and default blocks.
                BasicBlock *dispatcherBlock = BasicBlock::Create(CTX, "dispatcher", &F);
                BasicBlock *defaultBlock = BasicBlock::Create(CTX, "defaultCase", &F);

                new UnreachableInst(CTX, defaultBlock);

                dispatcherBlock->moveAfter(entryBlock);
                defaultBlock->moveAfter(dispatcherBlock);

                // 4. Create the state variable at the TOP of the entry block.
                IRBuilder<> allocaBuilder(&entryBlock->front());
                AllocaInst *stateVar = allocaBuilder.CreateAlloca(int32Ty, nullptr, "state");

                // 5. Get the first logical block and assign IDs to all blocks.
                Instruction *entryTerm = entryBlock->getTerminator();
                BasicBlock *firstBlock = entryTerm->getSuccessor(0);

                std::map<BasicBlock *, int> blockToIdMap;
                int currentId = 1;
                for (BasicBlock *BB : originalBlocks) {
                    blockToIdMap[BB] = currentId++;
                }

                // 6. Initialize state and rewire the entry block's TERMINATOR.
                IRBuilder<> termBuilder(entryTerm);
                termBuilder.CreateStore(ConstantInt::get(int32Ty, blockToIdMap[firstBlock]), stateVar);
                termBuilder.CreateBr(dispatcherBlock);
                entryTerm->eraseFromParent();

                // 7. Build the switch statement in the dispatcher block.
                IRBuilder<> dispatcherBuilder(dispatcherBlock);
                Value *loadedState = dispatcherBuilder.CreateLoad(int32Ty, stateVar, "loadedState");
                SwitchInst *dispatchSwitch = dispatcherBuilder.CreateSwitch(loadedState, defaultBlock, originalBlocks.size());

                BasicBlock* lastBlock = dispatcherBlock;
                for (auto const& [block, id] : blockToIdMap) {
                    dispatchSwitch->addCase(ConstantInt::get(int32Ty, id), block);
                    block->moveAfter(lastBlock);
                }

                // 8. Rewrite the terminators of all original blocks.
                for (BasicBlock *BB : originalBlocks) {
                    Instruction *terminator = BB->getTerminator();
                    IRBuilder<> builder(terminator);

                    if (isa<ReturnInst>(terminator) || isa<UnreachableInst>(terminator)) {
                        continue;
                    }

                    if (BranchInst *branch = dyn_cast<BranchInst>(terminator)) {
                        if (branch->isUnconditional()) {
                            BasicBlock *successor = branch->getSuccessor(0);
                            builder.CreateStore(ConstantInt::get(int32Ty, blockToIdMap[successor]), stateVar);
                            builder.CreateBr(dispatcherBlock);
                        } else {
                            BasicBlock *trueDest = branch->getSuccessor(0);
                            BasicBlock *falseDest = branch->getSuccessor(1);
                            Value *condition = branch->getCondition();
                            Value *trueId = ConstantInt::get(int32Ty, blockToIdMap[trueDest]);
                            Value *falseId = ConstantInt::get(int32Ty, blockToIdMap[falseDest]);
                            Value *nextState = builder.CreateSelect(condition, trueId, falseId, "nextState");
                            builder.CreateStore(nextState, stateVar);
                            builder.CreateBr(dispatcherBlock);
                        }
                        terminator->eraseFromParent();
                    }
                }

                // 9. Move any other stack allocations to the entry block.
                std::vector<AllocaInst*> AllocasToMove;
                for (BasicBlock &BB : F) {
                    if (&BB == entryBlock) continue;
                    for (Instruction &I : BB) {
                        if (AllocaInst *AI = dyn_cast<AllocaInst>(&I)) {
                            AllocasToMove.push_back(AI);
                        }
                    }
                }

                BasicBlock::iterator InsertPt = entryBlock->getFirstNonPHIOrDbgOrAlloca();
                for (AllocaInst *AI : AllocasToMove) {
                    AI->moveBefore(InsertPt);
                }

                errs() << "[Done]\n";
            }
            return PreservedAnalyses::none();
        }
    };

}
