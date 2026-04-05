
#include <CLI/CLI.hpp>
#include <CLI/Timer.hpp>
#include <string>

#include "app.hpp"

int main(int argc, char **argv) {
  unit::App &app = unit::App::GetInstance();

  try {
    CLI::Timer timer{"Done! Time taken"};
    app.Parse(argc, argv);
    app.LogInfo(timer.to_string());

  } catch (const CLI::ParseError &e) {
    return app.GetExitErrCode(e);
  } catch (const std::exception &e) {
    app.LogError(e.what());
    return 1;
  }

  return 0;
}
