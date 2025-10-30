# List Function Names Pass
This pass lists the names of all functions in a given LLVM module.

We can access functions using the `Module` class, which provides an iterator to traverse all functions within a module:
```cpp
for (auto &F : M) {
    // Process each function F
}
```

To print the function names, we can use the `getName()` method of the `Function` class, which returns the name of the function as a `StringRef`.

> Note: In C++, the function names can be mangled. For this you can tell the compiler to not mangle the names by using `extern "C"` in your function declarations.