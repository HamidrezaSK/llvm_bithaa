//==============================================================================
// FILE:
//    VECSub.cpp
//
// DESCRIPTION:
//    bitslicing normal vectorized addition instructions:
//      a + b ---->
//      t0 = ~vin1[0];
//      sum_temp = vin0[0]  ^ t0 ^ carry_temp;
//      carry_temp = (vin0[0]  & t0) | (vin0[0]  & carry_temp) | (t0  & carry_temp);
//      t0 = sum_temp;

//      vout[0] =  t0;
//
//
// USAGE:
//    1. Legacy pass manager:
//      $ opt -load <BUILD_DIR>/lib/libVECSub.so --legacy-vec-sub <bitcode-file>
//    2. New pass maanger:
//      $ opt -load-pass-plugin <BUILD_DIR>/lib/libVECsub.so `\`
//        -passes=-"vec-sub" <bitcode-file> -S -o <output-file>
//

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
  short flag = 0;
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
   
   
    Instruction *PartOne;
    Value *carryTemp;
    if(flag == 1)
    {
      flag = 1;

      auto notVec1 = Builder.CreateNot(BinOp->getOperand(1));
      PartOne = BinaryOperator::CreateXor(BinOp->getOperand(0),
                                  Builder.CreateXor(carryTemp,notVec1));
      carryTemp = Builder.CreateOr(Builder.CreateAnd(BinOp->getOperand(0),notVec1),
                              Builder.CreateOr(Builder.CreateAnd(BinOp->getOperand(0),carryTemp),Builder.CreateAnd(notVec1,carryTemp)));



      LLVM_DEBUG(dbgs() << *BinOp << " -> " << *PartOne << "\n");


      LLVM_DEBUG(dbgs() << "-*-" << " -> " << *carryTemp << "\n");

      // Replacing the custom made instruction with the old one

      ReplaceInstWithInst(BB.getInstList(), Inst, PartOne);

      Changed = true;


      // Update the statistics

      ++SubstCount;	
    }

   
   
    if(flag == 0)
    {
      flag = 1; 
 
	    auto notVec1 = Builder.CreateNot(BinOp->getOperand(1));
	    carryTemp = Builder.CreateOr(Builder.CreateAnd(BinOp->getOperand(0),notVec1),
                                   Builder.CreateOr(Builder.CreateAnd(BinOp->getOperand(0),Val_Carry_temp),Builder.CreateAnd(notVec1,Val_Carry_temp)));
      // BB.getInstList().insert(Inst, PartTwo);
    
	    PartOne = BinaryOperator::CreateXor(BinOp->getOperand(0),
                                        Builder.CreateXor(Val_Carry_temp,notVec1));

	    LLVM_DEBUG(dbgs() << *BinOp << " -> " << *PartOne << "\n");

    
	    LLVM_DEBUG(dbgs() << "-*-" << " -> " << *carryTemp << "\n");
    
	    // Replacing the custom made instruction with the old one
    
	    ReplaceInstWithInst(BB.getInstList(), Inst, PartOne);
 
	    Changed = true;

    
	    // Update the statistics
    
	    ++SubstCount;
    }
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
