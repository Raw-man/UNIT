#pragma once

#include <filesystem>
#include <vector>
namespace fs = std::filesystem;

namespace unit {
struct ExportOptions {
  fs::path input_path;
  fs::path output_path;
  fs::path base_path;
  bool flip = true;
  bool visibility = false;
  bool pack_textures = false;
  bool mipmaps = false;
};

struct ImportMdlOpt {
  fs::path input_path;
  fs::path base_path;
  fs::path output_path;
  fs::path collision_path;
  fs::path shadow_path;
  fs::path action_path;

  // vertex
  unsigned int compress_lvl = 1;
  bool use_strips = true;
  bool stitch_strips = true;
  bool use_indices = true;
  bool optimize_weights = false;

  // texture
  unsigned int texture_compress_lvl = 1;

  bool swizzle = true;
  bool dither = true;
  unsigned int max_wh = 512;
  unsigned int max_mm = 7;
  unsigned int alpha_levels = 256;

  // collisions
  bool wall = true;
  bool cull = true;

  // general
  bool flip = true;
  bool simplify = true;
  bool materials = false;
  bool blend = false;
  bool sort = false;
  bool sskeleton = false;
  float scale_factor = 1.0f;
};

struct ImportCamOpt {
  fs::path input_path;
  fs::path output_path;
  bool simplify = true;
};

struct ImportPosOpt {
  fs::path input_path;
  fs::path output_path;
  float scale_factor = 1.0f;
};

struct ImportMinOpt {
  fs::path input_path;
  fs::path output_path;
  unsigned int texture_width = 0;
};

struct ImportLitOpt {
  fs::path input_path;
  fs::path output_path;
  bool specular = false;
  float character_brightness = 1.0f;
};

struct ImportTxtOpt {
  enum OutFmt { kNSUNI, kNSLAR, kSplit };
  fs::path input_path;
  fs::path output_path;
  std::vector<fs::path> typeface_paths;
  OutFmt out_format = kNSUNI;

  float scale_factor = 1.0f;
  bool kerning = false;
  unsigned int quality = 128;
  unsigned int columns = 8;
  int tracking_offset = 0;
  float opacity = 1.5f;

  //texture opt
  unsigned int texture_compress_lvl = 1;
  bool nearest = false;
  bool swizzle = true;
};

struct ImportSndOpt {
  fs::path input_path;
  fs::path output_path;
  fs::path base_path;
  unsigned int max_sr = 22050;
};

struct ImportImgOpt {
  fs::path input_path;
  fs::path output_path;
  fs::path base_path;

  unsigned int texture_compress_lvl = 1;
  bool swizzle = true;
  bool dither = false;
  unsigned int max_wh = 512;
  unsigned int max_mm = 0;
  unsigned int alpha_levels = 256;
  bool nearest = false;
};

struct ImportFogOpt {
  fs::path input_path;
  fs::path output_path;
  std::string color = "#FFFFFF";
  float near_ = 100.0f;
  float far_ = 900.0f;
};

struct ImportDisOpt {
  fs::path input_path;
  fs::path output_path;
  std::string color = "#FFFFFF";
  float near_ = 100.0f;
  float far_ = 900.0f;

  float bloom = 0.0f;
  float bias = 0.1f;
  float slope = 0.002f;
  float transition = 0.0f;
};

struct UnpackOpt {
  enum Naming { kDefault, kPlugin, kOldPlugin };
  fs::path input_path;
  fs::path output_path;
  int recursive = false;
  Naming naming = kDefault;
};

struct PackOpt {
  fs::path input_path;
  fs::path output_path;
  bool recursive = false;
  bool compress = false;
};

struct MD5ListOpt {
  fs::path input_path;
  fs::path output_path;
};

struct LocateOpt {
  fs::path input_path;
  std::string input_string;
  fs::path search_path;
  std::size_t needle_size = 0;
  bool caseless = false;
};

struct DetectOpt {
  fs::path input_path;
};
}  // namespace unit
