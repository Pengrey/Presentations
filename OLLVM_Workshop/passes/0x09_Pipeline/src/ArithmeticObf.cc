#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/NoFolder.h"
#include "llvm/Pass.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/FormatVariadic.h"

#include <vector>
#include <string>

#define ITERNUM 10

using namespace llvm;

namespace {
    // MBA for X ^ Y = (X | Y) - (X & Y)
    template <typename BuilderTy>
    Value* mba_xor(Value* X, Value* Y, BuilderTy &builder) {
        Value* orInst = builder.CreateOr(X, Y, "or_tmp");
        Value* andInst = builder.CreateAnd(X, Y, "and_tmp");
        return builder.CreateSub(orInst, andInst, "xor_mba");
    }

    // MBA for X + Y = (X & Y) + (X | Y)
    template <typename BuilderTy>
    Value* mba_add(Value* X, Value* Y, BuilderTy &builder) {
        Value* andInst = builder.CreateAnd(X, Y, "and_tmp");
        Value* orInst = builder.CreateOr(X, Y, "or_tmp");
        return builder.CreateAdd(andInst, orInst, "add_mba");
    }

    // MBA for X - Y = (X ^ -Y) + 2*(X & -Y)
    template <typename BuilderTy>
    Value* mba_sub(Value* X, Value* Y, BuilderTy &builder) {
        Value* negY = builder.CreateNeg(Y, "neg_tmp");
        Value* xorInst = builder.CreateXor(X, negY, "xor_tmp");
        Value* andInst = builder.CreateAnd(X, negY, "and_tmp");
        Value* shlInst = builder.CreateShl(andInst, ConstantInt::get(X->getType(), 1), "shl_tmp");
        return builder.CreateAdd(xorInst, shlInst, "sub_mba");
    }

    // MBA for X & Y = (X + Y) - (X | Y)
    template <typename BuilderTy>
    Value* mba_and(Value* X, Value* Y, BuilderTy &builder) {
        Value* addInst = builder.CreateAdd(X, Y, "add_tmp");
        Value* orInst = builder.CreateOr(X, Y, "or_tmp");
        return builder.CreateSub(addInst, orInst, "and_mba");
    }

    // MBA for X | Y = X + Y + 1 + (~X | ~Y)
    template <typename BuilderTy>
    Value* mba_or(Value* X, Value* Y, BuilderTy &builder) {
        Value* addInst = builder.CreateAdd(X, Y, "add_tmp");
        Value* notX = builder.CreateNot(X, "notX_tmp");
        Value* notY = builder.CreateNot(Y, "notY_tmp");
        Value* orInst = builder.CreateOr(notX, notY, "or_tmp");
        Value* addOne = builder.CreateAdd(addInst, ConstantInt::get(X->getType(), 1), "addOne_tmp");
        return builder.CreateAdd(addOne, orInst, "or_mba");
    }

    struct ArithmeticObf : public PassInfoMixin<ArithmeticObf> {
        PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM) {
            errs() << formatv("\n[>] Arithmetic Obfuscation Pass\n");
            for (unsigned i = 0; i < ITERNUM; ++i) {
                for (Function &F : M) {
                    if (F.isDeclaration()) continue;                                                    // Skip function declarations

                    std::vector<BinaryOperator*> worklist;                                              // List of instructions to modify

                    for (BasicBlock &BB : F) {
                        for (Instruction &I : BB) {
                            if (auto *binOp = dyn_cast<BinaryOperator>(&I)) {
                                if (binOp->getOpcode() == Instruction::Add ||
                                    binOp->getOpcode() == Instruction::Sub ||
                                    binOp->getOpcode() == Instruction::Xor ||
                                    binOp->getOpcode() == Instruction::And ||
                                    binOp->getOpcode() == Instruction::Or) {
                                    worklist.push_back(binOp);                                          // Save to modify later
                                    }
                            }
                        }
                    }

                    if (worklist.empty()) continue;

                    errs() << formatv("[*] Targeting {0,10} instrs in function {1,-20}", worklist.size(), F.getName());

                    for (BinaryOperator *binOp : worklist) {
                        IRBuilder<NoFolder> builder(binOp);                                             // We use NoFolder to prevent constant folding
                        Value* lhs = binOp->getOperand(0);
                        Value* rhs = binOp->getOperand(1);
                        Value* newInst = nullptr;

                        switch (binOp->getOpcode()) {
                            case Instruction::Add: newInst = mba_add(lhs, rhs, builder); break;
                            case Instruction::Sub: newInst = mba_sub(lhs, rhs, builder); break;
                            case Instruction::Xor: newInst = mba_xor(lhs, rhs, builder); break;
                            case Instruction::And: newInst = mba_and(lhs, rhs, builder); break;
                            case Instruction::Or:  newInst = mba_or(lhs, rhs, builder); break;
                            default: continue;
                        }
                        binOp->replaceAllUsesWith(newInst);
                        binOp->eraseFromParent();
                    }

                    errs() << "[Done]\n";
                }
            }
            return PreservedAnalyses::all();
        }
    };
}
