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

## HELLO_WORLD PASS
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
In order to use this we have to generate IR format from input files in input and pass them to opt as input and our passes so file as pass pipeline like this.

```bash
cd ../inputs

/<path to your llvm .build folder>/bin/clang -emit-llvm -S -O1 hello.c
/<path to your llvm .build folder>/bin/opt -load-pass-plugin ../lib/libHelloWorld.so -passes=hello-world -disable-output hello.ll

```
## MBAADD a transformation PASS
this is a transformation pass that reads the file's 8bit add instructions and change them

## build
```bash
cd <path to project>

mkdir .build

cd .build

cmake -G "Ninja" ../

ninja
```
now all so files are created in lib folder.

## usage
```bash
cd ../inputs

/<path to your llvm .build folder>/bin/clang -emit-llvm -S -O1 input_for_mba.c 
/<path to your llvm .build folder>/bin/opt -load-pass-plugin ../lib/libMBAAdd.so -passes=mba-add input_for_mba.ll -S -o output_for_mba.ll

```