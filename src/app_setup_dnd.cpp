
#include "app.hpp"
#include "export.hpp"
#include "logger.hpp"
#include "threadpool.hpp"
#include "unpack.hpp"
#include "utils.hpp"
#include "validators.hpp"

namespace fs = std::filesystem;

using namespace nnl;

namespace unit {

using namespace std::string_literals;

void App::RunDragAndDrop() {
  const std::set<std::string>& input_paths = this->input_paths;
  const ExportOptions& exp_opt = this->exp_opt;
  const UnpackOpt& unp_opt = this->unp_opt;

  if (input_paths.empty()) return;

  auto work_dir_path = fs::current_path();

  const fs::path output_path = work_dir_path / fs::u8path("unit_dnd");

  utl::CreateDir(output_path);

  std::vector<std::future<void>> tasks;

  ThreadPool pool(std::max(std::thread::hardware_concurrency(), 1U));

  UNIT_LOG_DEBUG("threads: " + std::to_string(pool.GetNumThreads()));

  for (const auto& dir_entry_str : input_paths) {
    auto dir_entry = fs::u8path(NormalizePath(dir_entry_str));

    if (!fs::is_regular_file(dir_entry)) throw unit::RuntimeError("the file does not exist " + dir_entry.u8string());

    tasks.push_back(pool.AddTask([dir_entry, output_path, &exp_opt, &unp_opt]() {
      ExportOptions new_exp_opt = exp_opt;

      new_exp_opt.input_path = dir_entry;
      // an individual directory should be created for each file
      new_exp_opt.output_path = output_path / new_exp_opt.input_path.filename() / new_exp_opt.input_path.filename();

      try {
        if (!Export(new_exp_opt, true)) {
          UNIT_LOG_INFO("trying to unpack...");

          UnpackOpt entry_unp_opt = unp_opt;

          entry_unp_opt.input_path = dir_entry;

          entry_unp_opt.output_path = output_path / entry_unp_opt.input_path.filename();
          Unpack(entry_unp_opt);
        }

      } catch (const std::exception& e) {
        UNIT_LOG_ERROR(e.what() + ("; " + new_exp_opt.input_path.u8string()));
      }
    }));
  }

  UNIT_LOG_DEBUG("tasks: " + std::to_string(tasks.size()));

  for (std::size_t i = 0; i < tasks.size(); i++) {
    tasks[i].get();

    UNIT_LOG_DEBUG("task completed: " + std::to_string(i));
  }
}

void App::SetUpDragAndDrop() {
  this->add_option("drag-and-drop", this->input_paths, "Drop files onto the executable or list them manually")
      ->group("")
      ->configurable(false);

  this->callback([this]() { this->RunDragAndDrop(); });
}

}  // namespace unit
