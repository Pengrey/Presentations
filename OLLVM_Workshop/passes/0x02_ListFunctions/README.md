# List Functions Pass

This pass lists all functions in the input LLVM IR code. While traversing the module class, we can output the functions body by using it as a stream.

```cpp
for (auto &F : M) {
    errs() << F << "\n"; // Print the function body
}
```