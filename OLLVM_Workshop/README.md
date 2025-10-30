# Workshop: OLLVM for the rest of us

This repository contains hands-on materials for the OLLVM workshop. The aborded topics are the following:

* Simple LLVM Hello World 
* Functions names listing
* Function, Instruction and Basic Block body listing
* Simple program operation modification
* Arithmetic Obfuscation
* Basic Block Splitting
* Control Flow Flattening

## Prerequisites
* Basic knowledge of C/C++ programming
* Docker/Podman installed on your machine

## Getting Started
1. Clone the repository:
```bash
git clone https://github.com/Pengrey/OLLVM_Workshop.git
cd OLLVM_Workshop
```

2. Build the Docker image for OLLVM pass compilation:
```bash
make pod-build
```

3. Compile passes:
```bash
make <pass-name>
```

4. Test the pass against the provided test program:
```bash
make test
```

## References
* [LLVM for Grad Students](https://www.cs.cornell.edu/~asampson/blog/llvm.html)
* [CS 6120: Lesson 6: Writing an LLVM Pass](https://vod.video.cornell.edu/media/CS+6120%3A+Lesson+6%3A+Writing+an+LLVM+Pass/1_4nrtmvc9/179754792)
* [Writing an LLVM Pass](https://releases.llvm.org/1.9/docs/WritingAnLLVMPass.html)
* [Open-Obfuscator](https://obfuscator.re/)

## License
This project is licensed under the MIT License. See the `LICENSE` file for details.
