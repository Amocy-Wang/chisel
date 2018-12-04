#include <string>

#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/spdlog.h>

//#include "dce.h"
#include "FileManager.h"
#include "Frontend.h"
#include "GlobalReduction.h"
#include "LocalReduction.h"
#include "OptionManager.h"
#include "Profiler.h"
#include "Reduction.h"
#include "Report.h"
#include "StatsManager.h"

void initialize() {
  auto Logger = spdlog::stdout_logger_mt("Logger");
  if (OptionManager::Verbose) {
    Logger->set_level(spdlog::level::debug);
  } else {
    Logger->set_level(spdlog::level::info);
  }

  FileManager::Initialize();
  Profiler::Initialize();
}

void finalize() {
  FileManager::Finalize();
  Profiler::Finalize();
}

int main(int argc, char *argv[]) {
  OptionManager::handleOptions(argc, argv);
  initialize();

  Profiler::GetInstance()->beginChisel();

  StatsManager::ComputeStats(OptionManager::InputFile);
  int wc0 = std::numeric_limits<int>::max();
  int wc = StatsManager::GetNumOfWords();

  if (OptionManager::Stat) {
    StatsManager::Print();
    return 0;
  }

  int Iteration = 0;
  while (wc < wc0) {
    Iteration++;
    llvm::outs() << "Iteration " << Iteration << " (Word: " << wc << ")\n";
    wc0 = wc;
    if (!OptionManager::SkipGlobal) {
      llvm::outs() << "Start global reduction\n";
      Reduction *GR = new GlobalReduction();
      Frontend::Parse(OptionManager::InputFile, GR);
      llvm::outs() << "\n";
    }
    if (!OptionManager::SkipLocal) {
      llvm::outs() << "Start local reduction\n";
      Reduction *LR = new LocalReduction();
      Frontend::Parse(OptionManager::InputFile, LR);
      llvm::outs() << "\n";
    }
    //    if (!OptionManager::skipDCE) {
    //      DCE::removeDeadcode(bestNow.c_str());
    //    }
    StatsManager::ComputeStats(OptionManager::InputFile);
    wc = StatsManager::GetNumOfWords();
  }
  //  DCE::removeVacuousElements(bestNow.c_str());
  Profiler::GetInstance()->endChisel();

  Report::print();
  finalize();

  return 0;
}
