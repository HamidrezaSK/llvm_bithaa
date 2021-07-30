# LLVM_Pass

We are trying to write a simple out of tree llvm pass

## build llvm



```bash
git clone https://github.com/llvm/llvm-project.git
mkdir .build
cd .build
cmake -G "Ninja" -DCMAKE_BUILD_TYPE=RelWithDebInfo -DLLVM_ENABLE_PROJECTS="clang" -DLLVM_TARGETS_TO_BUILD="X86" -DLLVM_USE_LINKER=gold -DLLVM_PARALLEL_LINK_JOBS=2 ../llvm-project/llvm
ninja -j8 all
```
