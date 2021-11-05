//==============================================================================
// FILE:
//    VECAdd.cpp
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
//      $ opt -load <BUILD_DIR>/lib/libVECAdd.so --legacy-vec-add <bitcode-file>
//    2. New pass maanger:
//      $ opt -load-pass-plugin <BUILD_DIR>/lib/libVECAdd.so `\`
//        -passes=-"vec-add" <bitcode-file> -S -o <output-file>
//

//==============================================================================
#include "VECAdd.h"

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include <random>

using namespace llvm;

#define DEBUG_TYPE "vec-add"

STATISTIC(SubstCount, "The # of substituted instructions");

//-----------------------------------------------------------------------------
// VECAdd Implementaion
//-----------------------------------------------------------------------------
bool VECAdd::runOnBasicBlock(BasicBlock &BB) {
  bool Changed = false;

  // Loop over all instructions in the block. Replacing instructions requires
  // iterators, hence a for-range loop wouldn't be suitable.
  short flag = 0;
  auto initCarryTemp = ConstantInt::get(BB.getContext(), llvm::APInt(64,
                                              0, false));
  for (auto Inst = BB.begin(), IE = BB.end(); Inst != IE; ++Inst) {

    // Skip non-binary (e.g. unary or compare) instruction.
    auto *BinOp = dyn_cast<BinaryOperator>(Inst);
    if (!BinOp)
      continue;

    /// Skip instructions other than integer add.
    unsigned Opcode = BinOp->getOpcode();
    if (Opcode != Instruction::Add || !BinOp->getType()->isIntegerTy())
      continue;

    // A uniform API for creating instructions and inserting
    // them into basic blocks.
    IRBuilder<> Builder(BinOp);

   
   
   Instruction *sumTemp;
   Value *carryTempValue;
   if(flag == 1)
   {
        flag = 1;
        sumTemp = BinaryOperator::CreateXor(BinOp->getOperand(0),
                                    Builder.CreateXor(carryTempValue,
                                            BinOp->getOperand(1)));
	    carryTempValue = Builder.CreateOr(Builder.CreateAnd(BinOp->getOperand(0),BinOp->getOperand(1)),Builder.CreateOr(Builder.CreateAnd(BinOp->getOperand(0),carryTempValue),Builder.CreateAnd(BinOp->getOperand(1),carryTempValue)));


        LLVM_DEBUG(dbgs() << *BinOp << " -> " << *sumTemp << "\n");


        LLVM_DEBUG(dbgs() << "-*-" << " -> " << *carryTempValue << "\n");

        // Replacing the custom made instruction with the old one

        ReplaceInstWithInst(BB.getInstList(), Inst, sumTemp);

        Changed = true;


        // Update the statistics

        ++SubstCount;	
   }

   
   
    if(flag == 0)
    {
        flag = 1;
 
	    carryTempValue = Builder.CreateAnd(BinOp->getOperand(0),BinOp->getOperand(1));
    //	BB.getInstList().insert(Inst, carryTempInstruction);
    
	    sumTemp = BinaryOperator::CreateXor(BinOp->getOperand(0),
                                        Builder.CreateXor(initCarryTemp,
                                             BinOp->getOperand(1)));

	    LLVM_DEBUG(dbgs() << *BinOp << " -> " << *sumTemp << "\n");

    
	    LLVM_DEBUG(dbgs() << "-*-" << " -> " << *carryTempValue << "\n");
    
	    // Replacing the custom made instruction with the old one
    
	    ReplaceInstWithInst(BB.getInstList(), Inst, sumTemp);
 
	    Changed = true;

    
	    // Update the statistics
    
	    ++SubstCount;
    }
  }
  return Changed;
}

PreservedAnalyses VECAdd::run(llvm::Function &F,
                              llvm::FunctionAnalysisManager &) {
  bool Changed = false;

  for (auto &BB : F) {
    Changed |= runOnBasicBlock(BB);
  }
  return (Changed ? llvm::PreservedAnalyses::none()
                  : llvm::PreservedAnalyses::all());
}

bool LegacyVECAdd::runOnFunction(llvm::Function &F) {
  bool Changed = false;

  for (auto &BB : F) {
    Changed |= Impl.runOnBasicBlock(BB);
  }
  return Changed;
}

//-----------------------------------------------------------------------------
// New PM Registration
//-----------------------------------------------------------------------------
llvm::PassPluginLibraryInfo getVECAddPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "vec-add", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "vec-add") {
                    FPM.addPass(VECAdd());
                    return true;
                  }
                  return false;
                });
          }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getVECAddPluginInfo();
}

//-----------------------------------------------------------------------------
// Legacy PM Registration
//-----------------------------------------------------------------------------
char LegacyVECAdd::ID = 0;

// Register the pass - required for (among others) opt
static RegisterPass<LegacyVECAdd> X(/*PassArg=*/"legacy-vec-add",
                                    /*Name=*/"VECAdd",
                                    /*CFGOnly=*/true,
                                    /*is_analysis=*/false);
