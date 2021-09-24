//==============================================================================
// FILE:
//    MBASub.cpp
//
// DESCRIPTION:
//    Obfuscation for integer sub instructions through Mixed Boolean Arithmetic
//    (MBA). This pass performs an instruction substitution based on this
//    equality:
//      a - b == (a + ~b) + 1
//    See formula 2.2 (j) in [1].
//
// USAGE:
//    1. Legacy pass manager:
//      $ opt -load <BUILD_DIR>/lib/libMBASub.so --legacy-mba-sub <bitcode-file>
//    2. New pass maanger:
//      $ opt -load-pass-plugin <BUILD_DIR>/lib/libMBASub.so `\`
//        -passes=-"mba-sub" <bitcode-file>
//
//  [1] "Hacker's Delight" by Henry S. Warren, Jr.
//
// License: MIT
//==============================================================================
#include "VECSub.h"

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include <random>

using namespace llvm;

#define DEBUG_TYPE "vec-sub"

STATISTIC(SubstCount, "The # of substituted instructions");

//-----------------------------------------------------------------------------
// VECSub Implementaion
//-----------------------------------------------------------------------------
bool VECSub::runOnBasicBlock(BasicBlock &BB) {
  bool Changed = false;

  // Loop over all instructions in the block. Replacing instructions requires
  // iterators, hence a for-range loop wouldn't be suitable.
  auto Val_Carry_temp = ConstantInt::get(BB.getContext(), llvm::APInt(64,
                                              0xffffffffffffffff, false));
  for (auto Inst = BB.begin(), IE = BB.end(); Inst != IE; ++Inst) {

    // Skip non-binary (e.g. unary or compare) instruction.
    auto *BinOp = dyn_cast<BinaryOperator>(Inst);
    if (!BinOp)
      continue;

    /// Skip instructions other than integer sub.
    unsigned Opcode = BinOp->getOpcode();
    if (Opcode != Instruction::Sub || !BinOp->getType()->isIntegerTy())
      continue;

    // A uniform API for creating instructions and inserting
    // them into basic blocks.
    IRBuilder<> Builder(BinOp);
    // Constants used in building the instruction for substitution
   // auto Val_Carry_temp = ConstantInt::get(BinOp->getContext(), llvm::APInt(64,  
     //                                         0xffffffffffffffff, false));
    // auto Oprand_One_64Cast= ConstantInt::get(BinOp->getContext(), llvm::APInt(64,  
    //                                           BinOp->getOperand(1)->getvalue, true));
    // Create an instruction representing t0 = ~vin1[0]
    Instruction *PartOne = BinaryOperator::CreateAnd(BinOp->getOperand(0),
		    			Builder.CreateAnd(Val_Carry_temp,
                                             Builder.CreateNot(BinOp->getOperand(1))));
    //Val_Carry_temp = BinOp->getOperand(1) & Val_Carry_temp;
    // Create an instruction representing sum_temp = vin0[0]  ^ t0 ^ carry_temp
    Instruction *PartTwo = BinaryOperator::CreateOr(Builder.CreateAnd(BinOp->getOperand(0),Builder.CreateNot(BinOp->getOperand(1))),
                                              Builder.CreateOr(Builder.CreateAnd(BinOp->getOperand(0),Val_Carry_temp),Builder.CreateAnd(Builder.CreateNot(BinOp->getOperand(1)),Val_Carry_temp)));

    // Create an instruction representing carry_temp
    // Instruction *PartCarry = 
    // Instruction *NewValue = BinaryOperator::CreateXor(
    //     BinOp->getOperand(1),);
        
    // Create an instruction representing (a + ~b) + 1
   // Instruction *NewValue = BinaryOperator::CreateAdd(
    //    Builder.CreateAdd(BinOp->getOperand(0),
      //                    Builder.CreateNot(BinOp->getOperand(1))),
       // ConstantInt::get(BinOp->getType(), 1));

    // The following is visible only if you pass -debug on the command line
    // *and* you have an assert build.
    LLVM_DEBUG(dbgs() << *BinOp << " -> " << *PartOne << "\n");

    // Replace `(a - b)` (original instructions) with `(a + ~b) + 1`
    // (the new instruction)
    ReplaceInstWithInst(BB.getInstList(), Inst, PartOne);
    Changed = true;

    // Update the statistics
    ++SubstCount;
  }
  return Changed;
}

PreservedAnalyses VECSub::run(llvm::Function &F,
                              llvm::FunctionAnalysisManager &) {
  bool Changed = false;

  for (auto &BB : F) {
    Changed |= runOnBasicBlock(BB);
  }
  return (Changed ? llvm::PreservedAnalyses::none()
                  : llvm::PreservedAnalyses::all());
}

bool LegacyVECSub::runOnFunction(llvm::Function &F) {
  bool Changed = false;

  for (auto &BB : F) {
    Changed |= Impl.runOnBasicBlock(BB);
  }
  return Changed;
}

//-----------------------------------------------------------------------------
// New PM Registration
//-----------------------------------------------------------------------------
llvm::PassPluginLibraryInfo getVECSubPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "vec-sub", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "vec-sub") {
                    FPM.addPass(VECSub());
                    return true;
                  }
                  return false;
                });
          }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getVECSubPluginInfo();
}

//-----------------------------------------------------------------------------
// Legacy PM Registration
//-----------------------------------------------------------------------------
char LegacyVECSub::ID = 0;

// Register the pass - required for (among others) opt
static RegisterPass<LegacyVECSub> X(/*PassArg=*/"legacy-vec-sub",
                                    /*Name=*/"VECSub",
                                    /*CFGOnly=*/true,
                                    /*is_analysis=*/false);
