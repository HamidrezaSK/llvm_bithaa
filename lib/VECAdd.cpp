//==============================================================================
// FILE:
//    VECAdd.cpp
//
// DESCRIPTION:
//    Obfuscation for integer sub instructions through Mixed Boolean Arithmetic
//    (MBA). This pass performs an instruction substitution based on this
//    equality:
//      a - b == (a + ~b) + 1
//
//
// USAGE:
//    1. Legacy pass manager:
//      $ opt -load <BUILD_DIR>/lib/libVECAdd.so --legacy-vec-add <bitcode-file>
//    2. New pass maanger:
//      $ opt -load-pass-plugin <BUILD_DIR>/lib/libVECAdd.so `\`
//        -passes=-"vec-add" <bitcode-file>
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
// VECSub Implementaion
//-----------------------------------------------------------------------------
bool VECAdd::runOnBasicBlock(BasicBlock &BB) {
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
    if (Opcode != Instruction::Add || !BinOp->getType()->isIntegerTy())
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
                                           
    //Val_Carry_temp = BinOp->getOperand(1) & Val_Carry_temp;
    // Create an instruction representing sum_temp = vin0[0]  ^ t0 ^ carry_temp
   
   
   Instruction *PartOne;
   Instruction *PartTwo;
   Value *carryTemp;
   if(flag == 1)
   {
      flag = 1;

            auto notVec1 = Builder.CreateNot(BinOp->getOperand(1));
           PartTwo = BinaryOperator::CreateOr(Builder.CreateAnd(BinOp->getOperand(0),notVec1),
                                   Builder.CreateOr(Builder.CreateAnd(BinOp->getOperand(0),carryTemp),Builder.CreateAnd(notVec1,carryTemp)));
	   carryTemp = Builder.CreateOr(Builder.CreateAnd(BinOp->getOperand(0),notVec1),
                                   Builder.CreateOr(Builder.CreateAnd(BinOp->getOperand(0),carryTemp),Builder.CreateAnd(notVec1,carryTemp)));
            BB.getInstList().insert(Inst, PartTwo);

            PartOne = BinaryOperator::CreateAnd(BinOp->getOperand(0),
                                        Builder.CreateAnd(Val_Carry_temp,
                                             Builder.CreateNot(BinOp->getOperand(1))));

//          Value *CarryTempAddr = Builder.CreatePtrToInt(PartTwo->getValue(), Builder.getInt64Ty());
            LLVM_DEBUG(dbgs() << *BinOp << " -> " << *PartOne << "\n");


            LLVM_DEBUG(dbgs() << "-*-" << " -> " << *PartTwo << "\n");

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
    	    PartTwo = BinaryOperator::CreateOr(Builder.CreateAnd(BinOp->getOperand(0),notVec1),
                                   Builder.CreateOr(Builder.CreateAnd(BinOp->getOperand(0),Val_Carry_temp),Builder.CreateAnd(notVec1,Val_Carry_temp)));
    	    BB.getInstList().insert(Inst, PartTwo);
    
	    PartOne = BinaryOperator::CreateAnd(BinOp->getOperand(0),
                                        Builder.CreateAnd(Val_Carry_temp,
                                             Builder.CreateNot(BinOp->getOperand(1))));

//	    Value *CarryTempAddr = Builder.CreatePtrToInt(PartTwo->getValue(), Builder.getInt64Ty());
	    LLVM_DEBUG(dbgs() << *BinOp << " -> " << *PartOne << "\n");

    
	    LLVM_DEBUG(dbgs() << "-*-" << " -> " << *PartTwo << "\n");
    
	    // Replacing the custom made instruction with the old one
    
	    ReplaceInstWithInst(BB.getInstList(), Inst, PartOne);
 
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
