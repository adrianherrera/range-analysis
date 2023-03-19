#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/SourceMgr.h>

#ifndef NDEBUG
#include <llvm/IR/Verifier.h>
#endif

#include "RangeAnalysis.h"
#include "vSSA.h"

using namespace llvm;

namespace {
static cl::OptionCategory RACat("range analysis options");

static cl::opt<std::string> InFile(cl::Positional, cl::desc("<BC file>"),
                                   cl::value_desc("path"), cl::Required,
                                   cl::cat(RACat));
} // anonymous namespace

int main(int argc, char *argv[]) {
  // Parse command-line options
  cl::ParseCommandLineOptions(argc, argv, "range-analysis\n");

  LLVMContext Ctx;
  SMDiagnostic Err;
  auto Mod = parseIRFile(InFile, Err, Ctx);
  if (!Mod) {
    errs() << "Failed to parse `" << InFile << "`: " << Err.getMessage()
           << '\n';
    exit(1);
  }

  ModulePassManager MPM;
  LoopAnalysisManager LAM;
  FunctionAnalysisManager FAM;
  CGSCCAnalysisManager CGAM;
  ModuleAnalysisManager MAM;

  MPM.addPass(createModuleToFunctionPassAdaptor(vSSA()));

#ifndef NDEBUG
  MPM.addPass(VerifierPass());
#endif

  MAM.registerPass([] {
    return range_analysis::InterproceduralRA<range_analysis::Cousot>();
  });

  // Create the new pass manager builder
  llvm::PassBuilder PB;

  // Register all the basic analyses with the managers
  PB.registerModuleAnalyses(MAM);
  PB.registerCGSCCAnalyses(CGAM);
  PB.registerFunctionAnalyses(FAM);
  PB.registerLoopAnalyses(LAM);
  PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);

  MPM.run(*Mod, MAM);

  const auto &Ranges =
      MAM.getResult<range_analysis::InterproceduralRA<range_analysis::Cousot>>(
          *Mod);

  for (const auto &[V, R] : Ranges) {
    outs() << *V << " -> " << R << '\n';
  }

  return 0;
}
