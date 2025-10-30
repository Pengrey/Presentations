# Arithmetic Obfuscation Pass
This is a simple implementation of a LLVM pass that obfuscates arithmetic operations in the LLVM IR code. The pass replaces the operations: +, -, ^, &, | with equivalent but more complex expressions.

The substitutions are as follows:

| Operation | MBA Transformation     |
|-----------|------------------------|
| X ^ Y     | (X \| Y) - (X & Y)     |
| X + Y     | (X & Y) + (X \| Y)     |
| X - Y     | (X ^ -Y) + 2*(X & -Y)  |
| X & Y     | (X + Y) - (X \| Y)     |
| X \| Y    | X + Y + 1 + (~X \| ~Y) |

For example, the expression `a + b` would be transformed to `(a & b) + (a | b)`. And if we consequently apply the transformation to `a & b` and `a | b`, we would get:

`a + b  ->  (a & b) + (a | b) ->  (a & b) + (a + b + 1 + (~a | ~b))`

We can apply the transformations multiple times to increase the complexity of the expressions, being the minimum recommended 2 iterations.

The implementation first iterates over all the functions present in the module, if it is a function declaration it skips it but if it is a function definition it iterates over all the basic blocks and instructions to find the binary operations we want to obfuscate. It saves them in a worklist to modify them later.

> Note: We save them to modify them later because if we modify them while iterating over the instructions we could end up in an infinite loop or corrupt the iteration, this is due to the fact that we are adding new instructions to the basic block while iterating over it.

```cpp
<SNIP>
    struct ArithmeticObf : public PassInfoMixin<ArithmeticObf> {
        PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM) {
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
                                        worklist.push_back(binOp);                                      // Save to modify later
                                }
                            }
                        }
                    }

                    if (worklist.empty()) continue;
```

After having our worklist, we iterate over it for new instructions to replace the original ones.

```cpp
                    errs() << formatv("[{0,2}/{1}] Targeting {2,10} instructions in function {3,-20}", i + 1, ITERNUM, worklist.size(), F.getName());

                    for (BinaryOperator *binOp : worklist) {
                        IRBuilder<NoFolder> builder(binOp);                                             // We use NoFolder to prevent constant folding
                        Value* lhs = binOp->getOperand(0);
                        Value* rhs = binOp->getOperand(1);
                        Value* newInst = nullptr;

                        switch (binOp->getOpcode()) {
                            <SNIP>
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
```

> Note: We use `NoFolder` with the `IRBuilder` to prevent constant folding, if we don't use it the compiler could optimize our new expressions and we would lose the obfuscation.

If the opcode matches one of the operations we want to obfuscate, we call the corresponding function to create the new instruction using the `IRBuilder`.

```cpp
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
```

> Note: The functions are templated to accept any type of `IRBuilder`, this allows us to use `NoFolder` as mentioned before.
