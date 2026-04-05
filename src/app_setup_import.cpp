
#include <CLI/CLI.hpp>
#include <string>

#include "app.hpp"
#include "import.hpp"
#include "validators.hpp"

namespace unit {

namespace fs = std::filesystem;

using namespace nnl;

void App::RunSubcmdImpMdl() { ImportModelContainer(this->imp_mdl_opt); }

void App::RunSubcmdImpCam() { ImportCamera(this->imp_cam_opt); }

void App::RunSubcmdImpPos() { ImportPosition(this->imp_pos_opt); }

void App::RunSubcmdImpMinimap() { ImportMinimap(this->imp_min_opt); }

void App::RunSubcmdImpLit() { ImportLight(this->imp_lit_opt); }

void App::RunSubcmdImpSnd() { ImportPPHD8(this->imp_snd_opt); }

void App::RunSubcmdImpImg() { ImportTextureContainer(this->imp_img_opt); }

void App::RunSubcmdImpFog() { ImportFog(this->imp_fog_opt); }

void App::RunSubcmdImpDis() { ImportDistance(this->imp_dis_opt); }

void App::RunSubcmdImpText() { ImportDialog(this->imp_txt_opt); };

template <bool extras = true, typename Opt>
void SetUpComTextureOpt(CLI::App* texture_group, Opt& imp_opt) {
  texture_group
      ->add_option("--tcomp-lvl", imp_opt.texture_compress_lvl,
                   "Set texture compression level: 0 - best quality, big "
                   "files, poor performance on PSP")
      ->check(CLI::Range(0, 2))
      ->capture_default_str();

  texture_group
      ->add_flag("--swizzle,!--no-swizzle", imp_opt.swizzle,
                 "Rearrange texture data for the PSP's GU: greatly improves "
                 "performance on PSP")
      ->default_val(imp_opt.swizzle);

  if constexpr (extras) {
    texture_group
        ->add_flag("--dither,!--no-dither", imp_opt.dither,
                   "Apply noise to create an illusion of a wider color range "
                   "when compression is used")
        ->default_val(imp_opt.dither);

    texture_group
        ->add_option("--max-wh", imp_opt.max_wh,
                     "Resize textures if width or height is too big; 512 is the "
                     "max value for PSP")
        ->check(Pow2Range<16, 1024>())
        ->capture_default_str();

    texture_group
        ->add_option("--max-mm", imp_opt.max_mm,
                     "Set the maximum mipmap level. Improves performance, "
                     "reduces artifacts at large distances")
        ->check(CLI::Range(0, 7))
        ->capture_default_str();

    texture_group
        ->add_option("--alpha", imp_opt.alpha_levels,
                     "Quantize the alpha level. This might improve texture "
                     "quality when compression is used")
        ->check(CLI::Range(1, 256))
        ->capture_default_str();
  }
}

void App::SetUpSubcmdImpMdl(CLI::App* sub) {
  auto* sub_mdl = sub->add_subcommand("mdl", "Import a 3d model");

  sub_mdl->alias("model");

  sub_mdl->usage(
      "\nTo properly reimport assets:"
      "\nSet Frame Rate to 30, Frame Start to 0, Frame End to 1000 before "
      "importing assets into Blender (4.4 is recommended)"
      "\nFor better color accuracy set the View Transform to Standard (under "
      "the Color Management section)"
      "\nThese settings can be saved as the defaults (File - Defaults - Save "
      "Startup File)"
      "\nSet Bone Dir to Blender, untoggle Guess Original Bind Pose when "
      "importing assets into Blender"
      "\nToggle Custom Properties when exporting from Blender"
      "\nSome custom properties can be found under the Object and Material tabs"
      "\nIf UV animation is used, set Animation Mode to NLA tracks and toggle "
      "Animation Pointer"
      "\nThese settings can be saved for the current project if you toggle "
      "Remember Export Settings"
      "\nReading the GLTF addon docs might be helpful:"
      "\nhttps://docs.blender.org/manual/en/latest/addons/import_export/"
      "scene_gltf2.html");

  sub_mdl->fallthrough();

  sub_mdl->callback([this, sub_mdl]() {
    if (this->print_config) this->LogInfo("\n\n" + sub_mdl->config_to_str(true));
    this->RunSubcmdImpMdl();
  });

  auto asset_group = sub_mdl->add_option_group("ASSET", "");

  auto model_group = sub_mdl->add_option_group("MODEL", "");

  auto texture_group = sub_mdl->add_option_group("TEXTURE", "");

  auto animation_group = sub_mdl->add_option_group("ANIMATION", "");

  auto collision_group = sub_mdl->add_option_group("COLLISION", "");

  auto& imp_opt = this->imp_mdl_opt;

  asset_group
      ->add_option("--scale", imp_opt.scale_factor,
                   "Scale the asset (model, skeleton, collisions). This won't scale "
                   "attached effects, hitboxes. Use Blender as an alternative")
      ->check(CLI::PositiveNumber)
      ->capture_default_str();

  asset_group
      ->add_flag("--blend,!--no-blend", imp_opt.blend,
                 "Set an appropriate blend mode for materials that don't use one. "
                 "This might solve blending issues.\nAlso, consider using --sort")
      ->default_val(false);

  asset_group
      ->add_flag("--sort,!--no-sort", imp_opt.sort,
                 "Sort meshes so that opaque meshes are drawn before transparent "
                 "ones. This might solve blending issues and improve performance")
      ->default_val(false);

  asset_group
      ->add_flag("--merge-mat,!--no-merge-mat", imp_opt.materials,
                 "Merge duplicate materials: might improve performance")
      ->default_val(false);

  asset_group->add_flag("--flip,!--no-flip", imp_opt.flip, "Flip textures and uvs")->default_val(true);
  asset_group
      ->add_flag("--sskeleton,!--no-sskeleton", imp_opt.sskeleton,
                 "Construct a new simplified skeleton that uses only translations for "
                 "bones. This might get rid of unwanted transformations that affect "
                 "hitboxes, attached effects, animations")
      ->default_val(false);

  sub_mdl->add_option("-i,--input,input", imp_opt.input_path, "An input path to a glb asset")
      ->required(true)
      ->transform(NormalizePath)
      ->check(CLI::ExistingFile);

  sub_mdl->add_option("-o,--output,output", imp_opt.output_path, "An output path to the resulting 3d model")
      ->required(true)
      ->transform(NormalizePath)
      ->check(ExistingParentPathFile);

  sub_mdl
      ->add_option("-b,--base,base", imp_opt.base_path,
                   "An input path to the original game asset. Required for "
                   "importing characters and effects")
      ->transform(NormalizePath)
      ->check(CLI::ExistingFile);

  auto c =
      collision_group
          ->add_option("-c,--collision", imp_opt.collision_path, "An input path to a glb asset to use for collisions")
          ->transform(NormalizePath)
          ->check(CLI::ExistingFile);

  collision_group
      ->add_option("-C,--col-shadow", imp_opt.shadow_path,
                   "An input path to a glb asset to use for shadow collisions "
                   "(NSLAR only!)")
      ->needs(c)
      ->transform(NormalizePath)
      ->check(CLI::ExistingFile);

  collision_group
      ->add_flag("--wall,!--no-wall", imp_opt.wall, "Automatically set collision flags for wall and ceiling triangles")
      ->default_val(true);

  collision_group
      ->add_flag("--cull,!--no-cull", imp_opt.cull, "Automatically remove non-ground triangles (when using -C)")
      ->default_val(true);

  model_group
      ->add_flag("--indexed,!--no-indexed", imp_opt.use_indices,
                 "Use indexed buffers for drawing: makes performance a bit "
                 "worse on PSP, results in smaller files")
      ->default_val(true);

  model_group
      ->add_flag("--optw,!--no-optw", imp_opt.optimize_weights,
                 "Remove unnecessary weights from vertex attributes: results "
                 "in smaller files (NSUNI only!)")
      ->default_val(false);

  model_group
      ->add_flag("--strip,!--no-strip", imp_opt.use_strips,
                 "Use triangle strips for drawing: improves performance on PSP")
      ->default_val(true);

  model_group
      ->add_flag("--stitch,!--no-stitch", imp_opt.stitch_strips,
                 "Stitch triangle strips using degenerate triangles: usually "
                 "improves performance")
      ->default_val(true);

  model_group
      ->add_option("--vcomp-lvl", imp_opt.compress_lvl,
                   "Compress vertex attributes: 0 - best quality, big files, "
                   "worse performance on PSP")
      ->check(CLI::Range(0, 2))
      ->capture_default_str();

  SetUpComTextureOpt(texture_group, imp_opt);

  animation_group->add_flag("--unbake,!--no-unbake", imp_opt.simplify, "Remove redundant keyframes")->default_val(true);
}

void App::SetUpSubcmdImpCam(CLI::App* sub) {
  auto* sub_cam = sub->add_subcommand("cam", "Import an animated camera");

  sub_cam->alias("camera");

  sub_cam->usage(
      "\nTo properly reimport assets:"
      "\nSet Frame Rate to 30, Frame Start to 0, Frame End to 1000 before "
      "importing assets into Blender (4.4 is recommended)"
      "\nToggle \"Cameras\" when exporting from Blender"
      "\nA game camera is technically just a 3d model with animations. Do not "
      "export anything unrelated to it");

  sub_cam->fallthrough();

  sub_cam->callback([this, sub_cam]() {
    if (this->print_config) this->LogInfo("\n\n" + sub_cam->config_to_str(true));
    this->RunSubcmdImpCam();
  });

  auto& imp_opt = this->imp_cam_opt;

  sub_cam->add_option("-i,--input,input", imp_opt.input_path, "An input path to a glb asset")
      ->required(true)
      ->transform(NormalizePath)
      ->check(CLI::ExistingFile);
  sub_cam->add_option("-o,--output,output", imp_opt.output_path, "An output path to the resulting camera (model) file")
      ->required(true)
      ->transform(NormalizePath)
      ->check(ExistingParentPathFile);
}

void App::SetUpSubcmdImpPos(CLI::App* sub) {
  auto* sub_pos = sub->add_subcommand("pos", "Import spawn positions");

  sub_pos->usage(
      "\nTo properly reimport positions toggle Custom Properties when "
      "exporting "
      "from Blender (4.4 is recommended)");

  sub_pos->fallthrough();

  sub_pos->alias("position");

  sub_pos->callback([this, sub_pos]() {
    if (this->print_config) this->LogInfo("\n\n" + sub_pos->config_to_str(true));
    this->RunSubcmdImpPos();
  });

  auto& imp_opt = this->imp_pos_opt;

  sub_pos
      ->add_option("-i,--input,input", imp_opt.input_path,
                   "An input path to a glb asset containing a node called positions")
      ->required(true)
      ->transform(NormalizePath)
      ->check(CLI::ExistingFile);

  sub_pos->add_option("-o,--output,output", imp_opt.output_path, "An output path to the resulting position file")
      ->required(true)
      ->transform(NormalizePath)
      ->check(ExistingParentPathFile);

  sub_pos->add_option("--scale", imp_opt.scale_factor, "Scale the asset")
      ->check(CLI::PositiveNumber)
      ->capture_default_str();
}

void App::SetUpSubcmdImpMinimap(CLI::App* sub) {
  auto* sub_min = sub->add_subcommand("mmap", "Import minimap anchors, markers");

  sub_min->usage(
      "Note: as of now, minimap configs are only available when repacking the games\n\n"
      "Setting Minimap Anchors in Blender:\n"
      "Add a Camera and change its type to Orthographic.\n"
      "Press Numpad 7 (Top View), then press Ctrl + Alt + Numpad 0 to snap the camera to your current view.\n"
      "In the Camera Data tab, set Orthographic Scale and Clip End to a high value (e.g., 2000+).\n"
      "In Object Properties, adjust the camera's location, set the Z-Location to 200 or more.\n"
      "In the Output tab, set your resolution:\n"
      "e.g., 128x128 for NSUNI, uniform PoT dimensions are preferred!\n"
      "Add an Empty and name it positions.\n"
      "Add two more Empties as children of positions. Name them 00_top_left and 01_top_right.\n"
      "In Camera View (Numpad 0), move the Empties so they align with the top left and right corners of "
      "the render region.\n"
      "Press F12 to render and save the minimap image.\n"
      "Export your scene as a .glb file (you may also limit the export only to selected nodes).\n"
      "Import the .glb file with this command.\n"
      "Import the new minimap texture with the imp tex command.");

  sub_min->fallthrough();

  sub_min->alias("minimap");

  sub_min->callback([this, sub_min]() {
    if (this->print_config) this->LogInfo("\n\n" + sub_min->config_to_str(true));
    this->RunSubcmdImpMinimap();
  });

  auto& imp_opt = this->imp_min_opt;

  sub_min
      ->add_option("-i,--input,input", imp_opt.input_path,
                   "An input path to a glb asset containing a node called positions")
      ->required(true)
      ->transform(NormalizePath)
      ->check(CLI::ExistingFile);

  sub_min->add_option("-o,--output,output", imp_opt.output_path, "An output path to the resulting minimap config")
      ->required(true)
      ->transform(NormalizePath)
      ->check(ExistingParentPathFile);

  sub_min->add_option("--width", imp_opt.texture_width, "The width of the respective minimap texture")
      ->required()
      ->check(CLI::Range(128, 1024));
}

void App::SetUpSubcmdImpLit(CLI::App* sub) {
  auto* sub_lit = sub->add_subcommand("lit", "Import lights");

  sub_lit->fallthrough();

  sub_lit->alias("light");

  sub_lit->usage(
      "\nTo properly reimport lights toggle Punctual Lights and Custom "
      "Properties when exporting from Blender (4.4 is recommended)");

  sub_lit->callback([this, sub_lit]() {
    if (this->print_config) this->LogInfo("\n\n" + sub_lit->config_to_str(true));
    this->RunSubcmdImpLit();
  });

  auto& imp_opt = this->imp_lit_opt;

  sub_lit
      ->add_option("-i,--input,input", imp_opt.input_path,
                   "An input path to a glb asset with up to 3 sun/directional lights")
      ->required(true)
      ->transform(NormalizePath)
      ->check(CLI::ExistingFile);

  sub_lit->add_option("-o,--output,output", imp_opt.output_path, "An output path to the resulting light file")
      ->required(true)
      ->transform(NormalizePath)
      ->check(ExistingParentPathFile);

  sub_lit->add_option("--brightness", imp_opt.character_brightness, "Set the brightness level of characters")
      ->check(CLI::Range(0.0f, 1.0f))
      ->capture_default_str();

  sub_lit->add_flag("--specular,!--no-specular", imp_opt.specular, "Enable the specular highlight")->default_val(false);
}

void App::SetUpSubcmdImpSnd(CLI::App* sub) {
  auto* sub_snd = sub->add_subcommand("snd", "Import a sound bank");

  sub_snd->fallthrough();

  sub_snd->alias("sound");

  sub_snd->usage("Note: numeric prefixes in the names of files must be preserved");

  sub_snd->callback([this, sub_snd]() {
    if (this->print_config) this->LogInfo("\n\n" + sub_snd->config_to_str(true));
    this->RunSubcmdImpSnd();
  });

  auto& imp_opt = this->imp_snd_opt;

  sub_snd->add_option("-i,--input,input", imp_opt.input_path, "An input path to a folder containing wav or mp3 files")
      ->required(true)
      ->transform(NormalizePath)
      ->check(CLI::ExistingDirectory);

  sub_snd
      ->add_option("-o,--output,output", imp_opt.output_path, "An output path to the resulting PHD/PBD sound bank file")
      ->required(true)
      ->transform(NormalizePath)
      ->check(ExistingParentPathFile);

  sub_snd
      ->add_option("-b,--base,base", imp_opt.base_path,
                   "An input path to the original PHD/PBD sound bank. Needed to copy "
                   "over some data")
      ->required(true)
      ->transform(NormalizePath)
      ->check(CLI::ExistingFile);

  sub_snd
      ->add_option("--max-sr", imp_opt.max_sr,
                   "Set the maximum sample rate. Audio files exceeding this "
                   "rate will be resampled.")
      ->check(CLI::IsMember({8000, 11025, 16000, 22050, 44100, 48000}))
      ->capture_default_str();
}

void App::SetUpSubcmdImpImg(CLI::App* sub) {
  auto* sub_ui = sub->add_subcommand("tex", "Import textures");

  sub_ui->fallthrough();

  sub_ui->alias("texture");

  sub_ui->usage(
      "\nNote: numeric prefixes in the names of files must be preserved. A "
      "patch is required to use upscaled ui textures (with the plugin).");

  sub_ui->callback([this, sub_ui]() {
    if (this->print_config) this->LogInfo("\n\n" + sub_ui->config_to_str(true));
    this->RunSubcmdImpImg();
  });

  auto& imp_opt = this->imp_img_opt;

  sub_ui->add_option("-i,--input,input", imp_opt.input_path, "An input path to a folder with images")
      ->required(true)
      ->transform(NormalizePath)
      ->check(CLI::ExistingDirectory);

  sub_ui->add_option("-o,--output,output", imp_opt.output_path, "An output path to the resulting texture container")
      ->required(true)
      ->transform(NormalizePath)
      ->check(ExistingParentPathFile);

  sub_ui
      ->add_option("-b,--base,base", imp_opt.base_path,
                   "An input path to the original texture container. Replacing a "
                   "texture "
                   "with 0-byte files allows to keep the original texture unchanged")
      ->transform(NormalizePath)
      ->check(CLI::ExistingFile);

  SetUpComTextureOpt(sub_ui, imp_opt);

  sub_ui
      ->add_flag("--nearest,!--no-nearest", imp_opt.nearest,
                 "Use the nearest filter for upscaling. This might reduce "
                 "scaling artifacts but could lead to a blocky look")
      ->default_val(false);
}

void App::SetUpSubcmdImpFog(CLI::App* sub) {
  auto* sub_fog = sub->add_subcommand("fog", "Import a fog config (NSLAR only!)");

  sub_fog->usage(" ");

  sub_fog->callback([this, sub_fog]() {
    std::vector<CLI::ConfigItem> values = config_formatter_->from_file(imp_fog_opt.input_path.u8string());
    _parse_config(values);

    if (this->print_config) this->LogInfo("\n\n" + sub_fog->config_to_str(true));
    this->RunSubcmdImpFog();
  });

  auto& imp_opt = this->imp_fog_opt;

  sub_fog->add_option("-i,--input,input", imp_opt.input_path, "An input path to the toml config")
      ->required(true)
      ->transform(NormalizePath)
      ->check(CLI::ExistingFile);

  sub_fog->add_option("-o,--output,output", imp_opt.output_path, "An output path to the resulting fog file")
      ->required(true)
      ->transform(NormalizePath)
      ->check(ExistingParentPathFile);

  sub_fog->add_option("--color", imp_opt.color, "Set the color of the fog")->capture_default_str();

  sub_fog->add_option("--near", imp_opt.near_, "Set the distance where the fog starts to cover objects")
      ->check(CLI::Range(-2048.0f, 512000.0f))
      ->capture_default_str();

  sub_fog->add_option("--far", imp_opt.far_, "Set the distance where the fog fully covers objects")
      ->check(CLI::Range(0.0f, 512000.0f))
      ->capture_default_str();
}

void App::SetUpSubcmdImpDis(CLI::App* sub) {
  auto* sub_dis = sub->add_subcommand("dis", "Import a distance config (NSUNI only!)")
                      ->excludes(sub->get_subcommand_ptr("fog").get());

  sub_dis->alias("distance");

  sub_dis->usage(" ");

  sub_dis->callback([this, sub_dis]() {
    std::vector<CLI::ConfigItem> values = config_formatter_->from_file(imp_dis_opt.input_path.u8string());
    _parse_config(values);

    if (this->print_config) this->LogInfo("\n\n" + sub_dis->config_to_str(true));
    this->RunSubcmdImpDis();
  });

  auto& imp_opt = this->imp_dis_opt;

  sub_dis->add_option("-i,--input,input", imp_opt.input_path, "An input path to the toml config")
      ->required(true)
      ->transform(NormalizePath)
      ->check(CLI::ExistingFile);

  sub_dis->add_option("-o,--output", imp_opt.output_path, "An output path to the resulting distance file")
      ->required(true)
      ->transform(NormalizePath)
      ->check(ExistingParentPathFile);

  sub_dis->add_option("--color", imp_opt.color, "Set the color of the fog")->capture_default_str();

  sub_dis
      ->add_option("--near", imp_opt.near_,
                   "Set the distance where the fog starts to cover spawned "
                   "entities (characters, crates)")
      ->check(CLI::Range(-2048.0f, 512000.0f))
      ->capture_default_str();

  sub_dis->add_option("--far", imp_opt.far_, "Set the rendering distance where the fog fully covers objects")
      ->check(CLI::Range(0.0f, 512000.0f))
      ->capture_default_str();

  sub_dis
      ->add_option("--bloom", imp_opt.bloom,
                   "Set the opacity of the bloom effect. Requires buffered "
                   "rendering in PPSSPP")
      ->check(CLI::Range(0.0f, 1.0f))
      ->capture_default_str();

  sub_dis
      ->add_option("--transition", imp_opt.transition,
                   "Set the opacity of the transition effect on the distance "
                   "border. Use 0 for the max render distance")
      ->capture_default_str();

  sub_dis->add_option("--bias", imp_opt.bias, "Set the mipmap bias. The bigger the bias the stronger the blur")
      ->check(CLI::Range(-5.0f, 5.0f))
      ->capture_default_str();

  sub_dis->add_option("--slope", imp_opt.slope, "Set the mipmap slope")
      ->check(CLI::Range(-5.0f, 5.0f))
      ->capture_default_str();
}

void App::SetUpSubcmdImpText(CLI::App* sub) {
  auto* sub_txt = sub->add_subcommand("txt", "Import text");

  sub_txt->fallthrough();

  sub_txt->alias("text");

  sub_txt->usage("\nA patch is required to use bitmaps with width > 128");

  sub_txt->callback([this, sub_txt]() {
    if (this->print_config) this->LogInfo("\n\n" + sub_txt->config_to_str(true));

    this->RunSubcmdImpText();
  });

  auto& imp_opt = this->imp_txt_opt;

  sub_txt->add_option("-i,--input,input", imp_opt.input_path, "An input path to a .txt file ")
      ->required(true)
      ->transform(NormalizePath)
      ->check(CLI::ExistingFile);

  sub_txt->add_option("-o,--output,output", imp_opt.output_path, "An output path to the resulting game text file")
      ->required(true)
      ->transform(NormalizePath)
      ->check(ExistingParentPathFile);

  sub_txt
      ->add_option("-t,-b,--typeface,--base,typeface", imp_opt.typeface_paths,
                   "An input path or paths to .ttf fonts (font family) that have glyphs "
                   "for the language of your choice\n"
                   "You can also specify the path to "
                   "the original game text archive to use the original bitmap font")
      ->required(true)
      ->transform(NormalizePath)
      ->check(CLI::ExistingFile);

  std::vector<std::pair<std::string, ImportTxtOpt::OutFmt>> mapping{
      {"NSUNI", ImportTxtOpt::kNSUNI}, {"NSLAR", ImportTxtOpt::kNSLAR}, {"split", ImportTxtOpt::kSplit}};

  sub_txt
      ->add_option("--fmt", imp_opt.out_format,
                   "Set the format of the output. The appropriate selection "
                   "depends on the target game")
      ->transform(EnumTransformer(mapping))
      ->capture_default_str();

  auto text_group = sub_txt->add_option_group("TEXT", "");

  auto bitmap_group = sub_txt->add_option_group("FONT", "");

  auto texture_group = sub_txt->add_option_group("TEXTURE", "");

  text_group
      ->add_flag("--kerning,!--no-kerning", imp_opt.kerning,
                 "Simulate kerning. This adjusts the spacing between pairs of "
                 "some characters")
      ->default_val(false);

  text_group->add_option("--spacing", imp_opt.tracking_offset, "A value added to space between glyphs")
      ->check(CLI::Range(-127, 127))
      ->capture_default_str();

  bitmap_group
      ->add_option("--columns", imp_opt.columns,
                   "A number of glyphs that generated bitmaps have in a row. "
                   "It's usually 8 or 4")
      ->check(Pow2Range<2, 16>())
      ->capture_default_str();

  bitmap_group->add_option("--scale", imp_opt.scale_factor, "A factor by which glyphs are forcibly scaled")
      ->check(CLI::Range(0.01f, 10.0f))
      ->capture_default_str();

  bitmap_group->add_option("--opacity", imp_opt.opacity, "A factor by which colors are multiplied")
      ->check(CLI::Range(0.01f, 10.0f))
      ->capture_default_str();

  bitmap_group->add_option("--quality", imp_opt.quality, "The width and height of the bitmap")
      ->check(Pow2Range<128, 1024>())
      ->capture_default_str();

  SetUpComTextureOpt<false>(texture_group, imp_opt);

  texture_group
      ->add_flag("--nearest,!--no-nearest", imp_opt.nearest,
                 "Use the nearest filter for upscaling. This might reduce "
                 "scaling artifacts but could lead to a blocky look")
      ->default_val(false);
}

CLI::App* App::SetUpSubcmdImport() {
  auto* sub = this->add_subcommand("imp", "Import assets");

  sub->group("MAIN SUBCOMMANDS");

  sub->usage("\nunit imp <SUBCOMMAND> <INPUT> <OUTPUT> [OPTIONS]");

  sub->alias("import");

  // block propagation
  sub->add_option("export-files-block-2", this->input_paths)
      ->group("")
      ->check([](const std::string& str) { return "The following argument was not expected: " + str; })
      ->configurable(false);

  sub->fallthrough(true);

  sub->require_subcommand(1);

  return sub;
}
}  // namespace unit
