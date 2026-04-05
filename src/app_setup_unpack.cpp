#include <CLI/CLI.hpp>

#include "app.hpp"
#include "logger.hpp"
#include "threadpool.hpp"
#include "unpack.hpp"
#include "utils.hpp"
#include "validators.hpp"
namespace unit {

void App::RunSubcmdUnpack() {
  const UnpackOpt& unp_opt = this->unp_opt;

  if (fs::is_directory(unp_opt.input_path)) {
    utl::CreateDir(unp_opt.output_path);

    std::vector<std::future<void>> tasks;

    ThreadPool pool(std::max(std::thread::hardware_concurrency(), 1U));

    UNIT_LOG_DEBUG("threads: " + std::to_string(pool.GetNumThreads()));

    auto dir_entries = utl::GetDirEntries(unp_opt.input_path);

    for (const auto& dir_entry : dir_entries) {
      if (!fs::is_regular_file(dir_entry)) continue;

      tasks.push_back(pool.AddTask([&unp_opt, dir_entry]() {
        UnpackOpt entry_unp_opt = unp_opt;

        entry_unp_opt.input_path = dir_entry;

        // an individual directory should be created for each file
        entry_unp_opt.output_path = unp_opt.output_path / entry_unp_opt.input_path.filename();

        try {
          Unpack(entry_unp_opt);

        } catch (const std::exception& e) {
          UNIT_LOG_ERROR(e.what() + ("; " + entry_unp_opt.input_path.u8string()));
        };
      }));
    }

    if (tasks.empty()) throw unit::RuntimeError("nothing to unpack");

    UNIT_LOG_DEBUG("tasks: " + std::to_string(tasks.size()));

    for (std::size_t i = 0; i < tasks.size(); i++) {
      tasks[i].get();

      UNIT_LOG_DEBUG("task completed: " + std::to_string(i));
    }

  } else if (fs::is_regular_file(unp_opt.input_path)) {
    if (!Unpack(unp_opt)) {
      throw unit::RuntimeError("failed to unpack: " + unp_opt.input_path.u8string());
    }
  }
}

void App::SetUpSubcmdUnpack() {
  auto* sub = this->add_subcommand("unpack", "Unpack various containers");

  sub->group("ADVANCED SUBCOMMANDS");

  sub->alias("unp");

  sub->fallthrough(true);

  sub->usage("\nunit unp <INPUT> <OUTPUT> [OPTIONS]");

  sub->add_option("-i,--input,input", unp_opt.input_path, "An input path to a container")
      ->required()
      ->transform(NormalizePath)
      ->check(CLI::ExistingPath);
  sub->add_option("-o,--output,output", unp_opt.output_path, "An output path to the resulting folder")
      ->required()
      ->transform(NormalizePath)
      ->check(ExistingParentPathDir);

  std::vector<std::pair<std::string, UnpackOpt::Naming>> naming{{"default", UnpackOpt::Naming::kDefault},
                                                                {"plugin", UnpackOpt::Naming::kPlugin},
                                                                {"old_plugin", UnpackOpt::Naming::kOldPlugin}};

  sub->add_option("--naming", unp_opt.naming, "Set file names when unpacking .bin/.dig")
      ->transform(EnumTransformer(naming))
      ->capture_default_str();

  sub->add_flag("--recursive,!--no-recursive", unp_opt.recursive,
                "Unpack files from the container that are also simple "
                "containers. --recursive=2 unpacks everything (not "
                "recommended). This option should be "
                "used consistently during both packing and unpacking.")
      ->default_val(false);

  // block propagation
  sub->add_option("export-files-block-6", this->input_paths)
      ->group("")
      ->check([](const std::string& str) { return "The following argument was not expected: " + str; })
      ->configurable(false);

  sub->callback([this, sub]() {
    if (this->print_config) this->LogInfo("\n\n" + sub->config_to_str(true));
    this->RunSubcmdUnpack();
  });
}

}  // namespace unit
