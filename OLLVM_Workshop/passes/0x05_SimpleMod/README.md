# Simple Mod Pass
This pass is a simple implementation of an LLVM pass that modifies the LLVM IR code so that if an operation adds two integers, it will instead subtract them.

For this we access the instructions using the `BasicBlock` class, and then we check if the instruction is a binary operation and if it is an addition. If so, we change the opcode to subtraction.

```cpp
for (auto &I : BB) {
    if (auto *binOp = dyn_cast<BinaryOperator>(&I)) {
        if (binOp->getOpcode() == Instruction::Add) {
            IRBuilder<> builder(binOp);                                     // Create an IRBuilder to help with instruction creation  
            Value *newLHS = binOp->getOperand(0);                           // Get the left-hand side operand (in the test case, this is '2')
            Value *newRHS = binOp->getOperand(1);                           // Get the right-hand side operand (in the test case, this is '1')
            Value *newSub = builder.CreateSub(newLHS, newRHS);             // Create a new subtraction instruction (2 - 1)

            binOp->replaceAllUsesWith(newSub);                              // Redirect all uses of the original addition to the new subtraction
        }
    }
}
```