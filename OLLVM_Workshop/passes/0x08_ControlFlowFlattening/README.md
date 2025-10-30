# Control Flow Flattening Pass

This is naive implementation of a Control Flow Flattening obfuscation pass. The core idea of the pass is to eliminate direct branches between basic blocks and instead route all execution through a central "dispatcher" block. This dispatcher uses a state variable to determine which block to execute next, effectively turning the control flow into a large switch statement.

The pass firstly identifies suitable functions for flattening. It skips functions that are merely declarations or are too small to benefit from this transformation. A critical precondition for this implementation is the absence of `PHINode` instructions, as they complicate the process of reordering basic blocks and we are trying to implement a simple version here.

All basic blocks, except for the entry block, are collected into a list. The entry block is treated specially to ensure a clean start to the flattened control flow.

```cpp
<SNIP>
        PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM) {
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
                    errs() << formatv("Skipping function {0,-25} (contains PHI nodes)\n", F.getName());
                    continue;
                }

                errs() << formatv("Flattening function {0,-20}", F.getName());
<SNIP>
```

After retrieving the original basic blocks, a `dispatcher` and a `default` basic block are created. The dispatcher will contain the central switch statement, while the default case of this switch will lead to an `unreachable` instruction, handling any invalid states.

A `state` variable is allocated on the stack at the beginning of the entry block. This variable will hold the ID of the next basic block to be executed.

```cpp
<SNIP>
                BasicBlock *dispatcherBlock = BasicBlock::Create(CTX, "dispatcher", &F);
                BasicBlock *defaultBlock = BasicBlock::Create(CTX, "defaultCase", &F);

                new UnreachableInst(CTX, defaultBlock);

                dispatcherBlock->moveAfter(entryBlock);
                defaultBlock->moveAfter(dispatcherBlock);

                IRBuilder<> allocaBuilder(&entryBlock->front());
                AllocaInst *stateVar = allocaBuilder.CreateAlloca(int32Ty, nullptr, "state");
<SNIP>
```

Each of the original basic blocks is assigned a unique integer ID. The entry block is then modified to initialize the `state` variable with the ID of what was originally the first block to be executed. The entry block's terminator is replaced with an unconditional branch to the `dispatcher` block.

```cpp
<SNIP>
                Instruction *entryTerm = entryBlock->getTerminator();
                BasicBlock *firstBlock = entryTerm->getSuccessor(0);

                std::map<BasicBlock *, int> blockToIdMap;
                int currentId = 1;
                for (BasicBlock *BB : originalBlocks) {
                    blockToIdMap[BB] = currentId++;
                }

                IRBuilder<> termBuilder(entryTerm);
                termBuilder.CreateStore(ConstantInt::get(int32Ty, blockToIdMap[firstBlock]), stateVar);
                termBuilder.CreateBr(dispatcherBlock);
                entryTerm->eraseFromParent();
<SNIP>
```

The `dispatcher` block is populated with a `switch` statement. This switch loads the current value of the `state` variable and, based on that value, jumps to the corresponding original basic block.

```cpp
<SNIP>
                IRBuilder<> dispatcherBuilder(dispatcherBlock);
                Value *loadedState = dispatcherBuilder.CreateLoad(int32Ty, stateVar, "loadedState");
                SwitchInst *dispatchSwitch = dispatcherBuilder.CreateSwitch(loadedState, defaultBlock, originalBlocks.size());

                BasicBlock* lastBlock = dispatcherBlock;
                for (auto const& [block, id] : blockToIdMap) {
                    dispatchSwitch->addCase(ConstantInt::get(int32Ty, id), block);
                    block->moveAfter(lastBlock);
                }
<SNIP>
```

The terminators of all the original basic blocks are rewritten. Instead of branching directly to their successors, they now update the `state` variable with the ID of the next block and then branch back to the `dispatcher`.

For unconditional branches, the successor's ID is stored directly. For conditional branches, a `select` instruction is used to choose between the true and false successor IDs based on the original branch condition. After the new logic is inserted, the old terminator instruction is removed.

```cpp
<SNIP>
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
<SNIP>
                            Value *nextState = builder.CreateSelect(condition, trueId, falseId, "nextState");
                            builder.CreateStore(nextState, stateVar);
                            builder.CreateBr(dispatcherBlock);
                        }
                        terminator->eraseFromParent();
                    }
                }
<SNIP>
```

To maintain correct LLVM IR structure, any `alloca` instructions found in other basic blocks are moved to the top of the entry block. This ensures that all stack allocations happen at the beginning of the function execution.

```cpp
<SNIP>
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
<SNIP>
```
