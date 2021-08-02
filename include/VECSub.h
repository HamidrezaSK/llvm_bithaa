//========================================================================
// FILE:
//    VECSub.h
//
// DESCRIPTION:
//    Declares the VECSub pass for the new and the legacy pass managers.
//
//========================================================================
#ifndef LLVM_TUTOR_VEC_SUB_H
#define LLVM_TUTOR_VEC_SUB_H

#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

// PassInfoMixIn is a CRTP mix-in to automatically provide informational APIs
// needed for passes. Currently it provides only the 'name' method.
struct VECSub : public llvm::PassInfoMixin<VECSub> {
  llvm::PreservedAnalyses run(llvm::Function &F,
                              llvm::FunctionAnalysisManager &);
  bool runOnBasicBlock(llvm::BasicBlock &B);
};

struct LegacyVECSub : public llvm::FunctionPass {
  // The address of this static is used to uniquely identify this pass in the
  // pass registry. The PassManager relies on this address to find instance of
  // analyses passes and build dependencies on demand.
  // The value does not matter.
  static char ID;
  LegacyVECSub() : FunctionPass(ID) {}
  bool runOnFunction(llvm::Function &F) override;

  VECSub Impl;
};
#endif
