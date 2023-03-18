//===-------------------------- vSSA.h ------------------------------------===//
//
//					 The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
// Copyright (C) 2011-2012, 2014-2015	Victor Hugo Sperle Campos
//
//===----------------------------------------------------------------------===//

#include <llvm/IR/PassManager.h>
#include <llvm/Pass.h>

namespace llvm {
class DominatorTree;
class DominanceFrontier;
} // namespace llvm

class vSSA : public llvm::PassInfoMixin<vSSA> {
public:
  llvm::PreservedAnalyses run(llvm::Function &,
                              llvm::FunctionAnalysisManager &);
  bool convert(llvm::Function &, llvm::DominatorTree *,
               llvm::DominanceFrontier *);
  static llvm::StringRef name() { return "vSSA"; }

private:
  void createSigmasIfNeeded(llvm::BasicBlock *);
  void insertSigmas(llvm::Instruction *, llvm::Value *);
  void renameUsesToSigma(llvm::Value *, llvm::PHINode *);
  llvm::SmallVector<llvm::PHINode *, 24> insertPhisForSigma(llvm::Value *,
                                                            llvm::PHINode *);
  void insertPhisForPhi(llvm::Value *, llvm::PHINode *);
  void renameUsesToPhi(llvm::Value *, llvm::PHINode *);
  void insertSigmaAsOperandOfPhis(llvm::SmallVector<llvm::PHINode *, 24> &,
                                  llvm::PHINode *);
  void populatePhis(llvm::SmallVector<llvm::PHINode *, 24> &, llvm::Value *);
  bool dominateAny(llvm::BasicBlock *, llvm::Value *);
  bool dominateOrHasInFrontier(llvm::BasicBlock *, llvm::BasicBlock *,
                               llvm::Value *);
  bool verifySigmaExistance(llvm::Value *, llvm::BasicBlock *,
                            llvm::BasicBlock *);

  llvm::DominatorTree *DT_;
  llvm::DominanceFrontier *DF_;
};

class LegacyvSSA : public llvm::FunctionPass {
public:
  static char ID;

  LegacyvSSA() : llvm::FunctionPass(ID) {}
  void getAnalysisUsage(llvm::AnalysisUsage &) const override;
  bool runOnFunction(llvm::Function &) override;
};
