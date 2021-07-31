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
## HELLOWORLD PASS
this is just an analysis pass that reads the file's functions and returns its name and number of it's arguments

## build

```bash
export LLVM_DIR=/<path to your llvm .build folder>/lib/cmake/llvm/

cd HelloWorld

mkdir .build

cd .build

cmake -G "Ninja" ../

ninja
```
now we have our libHelloWorld.so file which we can pass to out opt tool to create the pass pipline.