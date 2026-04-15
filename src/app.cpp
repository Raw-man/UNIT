#include "app.hpp"

#include <mutex>

#include "rang.hpp"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#define NNL_WINDOWS
#include "Windows.h"
#endif

namespace unit {
std::mutex mut_cout;

void App::Parse(int argc, char** argv) {
  if (argc < 2) {
    throw CLI::CallForHelp();
  }

  auto utf8_argv = this->ensure_utf8(argv);

  this->parse(argc, utf8_argv);

  if (this->get_option("export-files")->count() == 0 && this->get_subcommands().size() == 0) {
    throw CLI::RequiredError("A subcommand or a file path");
  }
}

App::App() {
  SetUpGeneral();

  SetUpSubcmdExport();

  SetUpDragAndDrop();

  auto* sub = SetUpSubcmdImport();

  SetUpSubcmdImpMdl(sub);

  SetUpSubcmdImpCam(sub);

  SetUpSubcmdImpPos(sub);

  SetUpSubcmdImpMinimap(sub);

  SetUpSubcmdImpLit(sub);

  SetUpSubcmdImpText(sub);

  SetUpSubcmdImpSnd(sub);

  SetUpSubcmdImpImg(sub);

  SetUpSubcmdImpFog(sub);

  SetUpSubcmdImpDis(sub);

  SetUpSubcmdUnpack();

  auto* pack = SetUpSubcmdPack();

  SetUpSubcmdPackDig(pack);

  SetUpSubcmdPackDigEntry(pack);

  SetUpSubcmdPackCollection(pack);

  SetUpSubcmdPackAsset(pack);

  SetUpSubcmdMD5();

  auto* loc = SetUpSubcmdLocate();

  SetUpSubcmdLocFile(loc);

  SetUpSubcmdLocHex(loc);

  SetUpSubcmdLocStr(loc);

  SetUpSubcmdDetect();
}

bool App::IsLogLevelEnabled(App::LogType level) { return level >= log_lvl; }

void App::SetLogLevel(App::LogType level) {
  std::scoped_lock l(mut_cout);
  log_lvl = level;
}

void App::Log(std::string_view msg, App::LogType type) {
  if (!IsLogLevelEnabled(type)) return;

  std::scoped_lock l(mut_cout);

  switch (type) {
    case LogType::kError:
      std::cout << rang::fgB::red << "[ERROR]" << rang::fg::reset << " " << msg << std::endl;
      break;

    case LogType::kWarn:
      std::cout << rang::fgB::yellow << "[WARN]" << rang::fg::reset << " " << msg << std::endl;
      break;

    case LogType::kInfo:
      std::cout << rang::fgB::green << "[INFO]" << rang::fg::reset << " " << msg << std::endl;
      break;

    case LogType::kDebug:
      std::cout << rang::fgB::gray << rang::bg::yellow << "[DEBUG]" << rang::fg::reset << rang::bg::reset << " " << msg
                << std::endl;
      break;
  }
}

void App::LogDebug(std::string_view msg) { App::Log(msg, LogType::kDebug); }

void App::LogInfo(std::string_view msg) { App::Log(msg, LogType::kInfo); }

void App::LogWarn(std::string_view msg) { App::Log(msg, LogType::kWarn); }

void App::LogError(std::string_view msg) { App::Log(msg, LogType::kError); }

}  // namespace unit
