# List Basic Blocks Pass

This pass lists all basic blocks in each function of the input LLVM IR code. Basic blocks are fundamental units of code in LLVM, representing a sequence of instructions with a single entry point and a single exit point.

We can access basic blocks using the `Function` class, which provides an iterator to traverse all basic blocks within a function:
```cpp
for (auto &BB : F) {
    // Process each basic block BB
}
```
