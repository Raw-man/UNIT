

#include "NNL/utility/utf8.hpp"
#include "import.hpp"
#include "logger.hpp"
#include "utils.hpp"
namespace unit {
bool ImportDialog(const ImportTxtOpt& opt) {
  asset::Asset asset;

  nnl::Buffer b = utl::LoadFile(opt.typeface_paths.at(0));

  if (asset::IsOfType(b)) {
    asset = asset::Import(b);
    auto cat = asset::Categorize(asset);

    if (cat != asset::Category::kBitmapTextFull)
      throw unit::RuntimeError("the base asset is not a complete text archive");
  }

  if (!asset.empty()) {
    if (opt.typeface_paths.size() > 1)
      throw unit::RuntimeError(
          "expected only the first base path but multiple paths were "
          "provided");
    if (opt.out_format != ImportTxtOpt::kNSUNI)
      throw unit::RuntimeError(
          "the base path is supported only for the NSUNI out format");
  }

  if (opt.typeface_paths.size() > 1 && opt.kerning) {
    throw unit::RuntimeError("--kerning is not supported for multiple fonts");
  }

  if (opt.typeface_paths.size() > 1 && opt.out_format == ImportTxtOpt::kNSUNI)
    UNIT_LOG_WARN(
        "multiple fonts are not supported for the output format, see --fmt");

  auto text = utl::LoadTextFile(opt.input_path);

  if (text.empty() || !nnl::utl::utf8::IsValid(text))
    throw unit::RuntimeError("text is not encoded with utf8: " +
                             opt.input_path.u8string());

  std::vector<std::string> strs;

  strs.reserve(512);

  std::size_t pos = 0;

  while (pos < text.size()) {
    auto start = text.find(u8"\u00ab", pos);
    auto end = text.find(u8"\u00bb", pos);

    if (start != std::string::npos && end != std::string::npos) {
      strs.push_back(text.substr(start + 2, end - start - 2));
      pos = end + 2;
    } else
      break;
  }

  if (strs.empty())
    throw unit::RuntimeError(
        u8"no text strings were found enclosed with \u00ab\u00bb: " +
        opt.input_path.u8string());

  if (!asset.empty()) {
    auto dialog = text::Import(asset[2]);
    dialog.strings.clear();
    dialog = text::Convert(strs, text::kSpecialCodeToString, dialog.characters);
    asset[2] = text::Export(dialog);
    auto buf = asset::Export(asset);
    utl::SaveFile(opt.output_path, buf);
    return true;
  }

  std::vector<std::pair<nnl::Buffer, nnl::Buffer>> fonts;
  fonts.reserve(opt.typeface_paths.size());
  auto dialog = text::Convert(strs);
  for (auto& font_path : opt.typeface_paths) {
    auto [stextures, tracking] = text::GenerateBitmapFont(
        dialog, font_path,
        {opt.quality, opt.columns, opt.opacity, opt.scale_factor,
         opt.tracking_offset, opt.kerning, opt.nearest,
         opt.texture_compress_lvl == 2U ? 16U : 256U});

    texture::ConvertParam param;

    param.max_mipmap_lvl = 0;

    param.swizzle = opt.swizzle;

    param.texture_format = opt.texture_compress_lvl == 2
                               ? texture::TextureFormat::kCLUT4
                               : (opt.texture_compress_lvl == 1
                                      ? texture::TextureFormat::kCLUT8
                                      : texture::TextureFormat::kRGBA8888);
    param.clut_format = texture::ClutFormat::kRGBA8888;

    std::vector<texture::ConvertParam> params(stextures.size(), param);

    fonts.push_back(
        {texture::Export(texture::Convert(std::move(stextures), params)),
         tracking});
  }

  switch (opt.out_format) {
    case ImportTxtOpt::OutFmt::kNSUNI: {
      auto& font = fonts.at(0);
      asset::Asset asset;
      asset[0] = std::move(font.first);   // textures
      asset[1] = std::move(font.second);  // tracking
      asset[2] = text::Export(dialog);
      auto buf = asset::Export(asset);
      utl::SaveFile(opt.output_path, buf);
      return true;
    }
    case ImportTxtOpt::OutFmt::kNSLAR: {
      auto txt = text::Export(dialog);
      utl::SaveFile(opt.output_path, txt);  // Main text
      const auto out_folder = opt.output_path.parent_path();
      for (std::size_t i = 0; i < fonts.size(); i++) {
        std::string out_name = opt.output_path.stem().u8string() + "_" +
                               std::to_string(i + 1) + ".bitmap_font";
        auto out_font_path = out_folder / fs::u8path(out_name);
        auto& font = fonts[i];
        asset::Asset asset;
        asset[0] = std::move(font.first);   // textures
        asset[1] = std::move(font.second);  // tracking

        auto buf = asset::Export(asset);
        utl::SaveFile(out_font_path, buf);
      }
      return true;
    }

    case ImportTxtOpt::OutFmt::kSplit: {
      auto txt = text::Export(dialog);
      utl::SaveFile(opt.output_path, txt);  // Main text
      const auto out_folder = opt.output_path.parent_path();
      for (std::size_t i = 0; i < fonts.size(); i++) {
        std::string out_name =
            opt.output_path.stem().u8string() + "_" + std::to_string(i + 1);

        std::string out_texture_name = out_name + "_0.texture";
        std::string out_spacing_name = out_name + "_1.spacing";
        auto out_texture_path = out_folder / fs::u8path(out_texture_name);
        auto out_spacing_path = out_folder / fs::u8path(out_spacing_name);
        auto& font = fonts[i];

        utl::SaveFile(out_texture_path, font.first);
        utl::SaveFile(out_spacing_path, font.second);
      }
      return true;
    }
    default:
      throw unit::RuntimeError("invalid output format");
  }

  return false;
}
}  // namespace unit
