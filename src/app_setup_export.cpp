
#include <CLI/CLI.hpp>
#include <string>

#include "app.hpp"
#include "export.hpp"
#include "logger.hpp"
#include "threadpool.hpp"
#include "utils.hpp"
#include "validators.hpp"

namespace fs = std::filesystem;

using namespace nnl;

namespace unit {

using namespace std::string_literals;

void App::RunSubcmdExport() {
  const ExportOptions& exp_opt = this->exp_opt;

  if (fs::is_directory(exp_opt.input_path)) {
    utl::CreateDir(exp_opt.output_path);

    std::vector<std::future<void>> tasks;

    ThreadPool pool(std::max(std::thread::hardware_concurrency(), 1U));

    UNIT_LOG_DEBUG("threads: " + std::to_string(pool.GetNumThreads()));

    auto dir_entries = utl::GetDirEntries(exp_opt.input_path);

    for (const auto& dir_entry : dir_entries) {
      if (!fs::is_regular_file(dir_entry)) continue;

      tasks.push_back(pool.AddTask([&exp_opt, dir_entry]() {
        ExportOptions entry_exp_opt = exp_opt;

        entry_exp_opt.input_path = dir_entry;

        // an individual directory should be created for each file
        entry_exp_opt.output_path =
            exp_opt.output_path / entry_exp_opt.input_path.filename() / entry_exp_opt.input_path.filename();

        try {
          Export(entry_exp_opt, true);

        } catch (const std::exception& e) {
          UNIT_LOG_ERROR(e.what() + "; "s + entry_exp_opt.input_path.u8string());
        };
      }));
    }

    if (tasks.empty()) throw unit::RuntimeError("nothing to export");

    UNIT_LOG_DEBUG("tasks: " + std::to_string(tasks.size()));

    for (std::size_t i = 0; i < tasks.size(); i++) {
      tasks[i].get();

      UNIT_LOG_DEBUG("task completed: " + std::to_string(i));
    }

  } else if (fs::is_regular_file(exp_opt.input_path)) {
    if (!Export(exp_opt)) {  // single file export
      throw unit::RuntimeError("failed to export: " + exp_opt.input_path.u8string());
    };
  }
}

void App::SetUpSubcmdExport() {
  auto* sub = this->add_subcommand("exp", "Export game assets");

  sub->group("MAIN SUBCOMMANDS");

  sub->alias("export");

  sub->usage("\nunit exp <INPUT> <OUTPUT> [OPTIONS]");

  sub->add_option("-i,--input,input", exp_opt.input_path, "An input path to a game asset or a folder with assets")
      ->required()
      ->transform(NormalizePath)
      ->check(CLI::ExistingPath);

  sub->add_option("-o,--output,output", exp_opt.output_path, "An output path to the resulting file or a folder")
      ->required()
      ->transform(NormalizePath)
      ->check(ExistingParentPath);

  sub->add_flag("--flip,!--no-flip", exp_opt.flip, "Flip textures and uv's")->default_val(true);

  sub->add_flag("--visibility,!--no-visibility", exp_opt.visibility,
                "Merge meshes by their respective groups and export mesh group "
                "visibility animations via KHR_animation_pointer "
                "and KHR_node_visibility. Note: Blender currently lacks "
                "KHR_node_visibility support. Works with Godot 4.6+")
      ->default_val(false);

  sub->add_flag("--pack,!--no-pack", exp_opt.pack_textures, "Embed textures into .glb files")->default_val(false);

  sub->add_flag("--mipmaps,!--no-mipmaps", exp_opt.mipmaps, "Export mipmaps")->default_val(false);

  auto advanced_group = sub->add_option_group("ADVANCED");

  advanced_group
      ->add_option("-b,--base,base", exp_opt.base_path,
                   "An input path to a game asset to use for "
                   "exporting partial assets that store only some data "
                   "(e.g. animations for a complete 3d model). Note: careless use may "
                   "result in errors")
      ->transform(NormalizePath)
      ->check(CLI::ExistingFile);

  sub->fallthrough(true);

  // block propagation
  sub->add_option("export-files-block-1", this->input_paths)
      ->group("")
      ->check([](const std::string& str) { return "The following argument was not expected: " + str; })
      ->configurable(false);

  sub->callback([this, sub]() {
    if (this->print_config) this->LogInfo("\n\n" + sub->config_to_str(true));
    this->RunSubcmdExport();
  });
}

}  // namespace unit
