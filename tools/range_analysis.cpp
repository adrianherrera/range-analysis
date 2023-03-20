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
enum Analysis {
  INTRA_COUSOT,
  INTRA_CROP,
  INTER_COUSOT,
  INTER_CROP,
};

static cl::OptionCategory RACat("range analysis options");

static cl::opt<std::string> InFile(cl::Positional, cl::desc("<BC file>"),
                                   cl::value_desc("path"), cl::Required,
                                   cl::cat(RACat));

static cl::opt<Analysis> RAType(
    "range-analysis", cl::desc("Type of range analysis"),
    cl::values(
        clEnumValN(INTRA_COUSOT, "intra-cousot", "Intraprocedural (Cousot)"),
        clEnumValN(INTRA_CROP, "intra-cropt", "Intraprocedural (CropDFS)"),
        clEnumValN(INTER_COUSOT, "inter-cousot", "Interprocedural (Cousot)"),
        clEnumValN(INTER_CROP, "inter-crop", "Interprocedural (CropDFS)")));
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

  switch (RAType) {
  case Analysis::INTRA_COUSOT:
    FAM.registerPass([] {
      return range_analysis::IntraproceduralRA<range_analysis::Cousot>();
    });
    break;
  case Analysis::INTRA_CROP:
    FAM.registerPass([] {
      return range_analysis::IntraproceduralRA<range_analysis::CropDFS>();
    });
    break;
  case Analysis::INTER_COUSOT:
    MAM.registerPass([] {
      return range_analysis::InterproceduralRA<range_analysis::Cousot>();
    });
    break;
  case Analysis::INTER_CROP:
    MAM.registerPass([] {
      return range_analysis::InterproceduralRA<range_analysis::CropDFS>();
    });
    break;
  }

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
