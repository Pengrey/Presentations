NAME	:= ollvm

LLVM_V	:= 20

define log_info
	echo -e "[\033[0;33m*\033[0m] $(1)"
endef

define log_success
	echo -e "[\033[0;32m+\033[0m] Done"
endef

define compile_pass
	podman run --rm -v $(PWD):/usr/local/src llvm-dev sh -c "clang++ -std=c++20 -fPIC -shared passes/$(1)/src/*.cc -o bin/$(NAME).so \`llvm-config --cxxflags --ldflags --libs core support\`"
endef

define compile_code
	podman run --rm -v $(PWD):/usr/local/src llvm-dev sh -c "clang -fpass-plugin=bin/$(NAME).so test/test.cc -o test/test"
endef

0x00_SimplePass: clean
	@ $(call log_info,Compiling...)
	@ $(call compile_pass,0x00_SimplePass)
	@ $(call log_success)

0x01_ListFunctionNames: clean
	@ $(call log_info,Compiling...)
	@ $(call compile_pass,0x01_ListFunctionNames)
	@ $(call log_success)

0x02_ListFunctions: clean
	@ $(call log_info,Compiling...)
	@ $(call compile_pass,0x02_ListFunctions)
	@ $(call log_success)

0x03_ListBasicBlocks: clean
	@ $(call log_info,Compiling...)
	@ $(call compile_pass,0x03_ListBasicBlocks)
	@ $(call log_success)

0x04_ListInstructions: clean
	@ $(call log_info,Compiling...)
	@ $(call compile_pass,0x04_ListInstructions)
	@ $(call log_success)

0x05_SimpleMod: clean
	@ $(call log_info,Compiling...)
	@ $(call compile_pass,0x05_SimpleMod)
	@ $(call log_success)

0x06_ArithmeticObf: clean
	@ $(call log_info,Compiling...)
	@ $(call compile_pass,0x06_ArithmeticObf)
	@ $(call log_success)

0x07_SplitBasicBlocks: clean
	@ $(call log_info,Compiling...)
	@ $(call compile_pass,0x07_SplitBasicBlocks)
	@ $(call log_success)

0x08_ControlFlowFlattening: clean
	@ $(call log_info,Compiling...)
	@ $(call compile_pass,0x08_ControlFlowFlattening)
	@ $(call log_success)

0x09_Pipeline: clean
	@ $(call log_info,Compiling...)
	@ $(call compile_pass,0x09_Pipeline)
	@ $(call log_success)

test:
	@ $(call log_info,Compiling test...)
	@ $(call compile_code)
	@ $(call log_success)

pod-build:
	@ $(call log_info, Building Podman image...)
	@ podman build --build-arg LLVM_V=$(LLVM_V) --quiet -t llvm-dev . --format docker
	@ $(call log_success)

pod-clean:
	@ $(call log_info, Deleting Podman image...)
	@ podman image rm llvm-dev
	@ $(call log_success)

clean:
	@ $(call log_info,Cleaning build artifacts)
	@ rm -f bin/*.so test/test
	@ $(call log_success)

.PHONY: test clean pod-build pod-clean
