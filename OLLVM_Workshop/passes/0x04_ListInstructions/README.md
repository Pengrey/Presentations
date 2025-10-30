# List Instructions Pass
This pass lists all instructions in each basic block of every function in the input LLVM IR code. Instructions are the fundamental operations that make up the body of a function. We can access instructions using the `BasicBlock` class, which provides an iterator to traverse all instructions within a basic block:
```cpp
for (auto &I : BB) {
    // Process each instruction I
}
```
