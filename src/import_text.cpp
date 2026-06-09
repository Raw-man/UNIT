
#include <charconv>
#include <pugixml.hpp>

#include "NNL/utility/utf8.hpp"
#include "import.hpp"
#include "logger.hpp"
#include "utils.hpp"

// An older "format"
std::vector<std::string> ParseTXT(const std::filesystem::path& p) {
  std::vector<std::string> strs;
  strs.reserve(128);

  auto text = unit::utl::LoadTextFile(p);

  if (text.empty() || !nnl::utl::utf8::IsValid(text))
    throw unit::RuntimeError("the input text file is not UTF-8 encoded: " + p.u8string());

  std::string_view text_view = text;

  std::size_t start_str = std::string::npos;
  std::size_t end_str = std::string::npos;
  std::size_t line = 0;
  int unmatched = 0;

  for (std::size_t i = 0; i < text_view.size();) {
    std::size_t utf8_size = nnl::utl::utf8::GetSize(text_view, i);

    assert(i + utf8_size <= text_view.size());

    std::string_view utf8code = text_view.substr(i, utf8_size);

    char32_t codepoint = nnl::utl::utf8::Decode(utf8code);

    if (codepoint == '\n') {
      line++;
    }

    if (codepoint == 0xAB) {
      unmatched++;
      start_str = i;
    }

    if (codepoint == 0xBB) {
      unmatched--;
      end_str = i;
    }

    if (unmatched < 0 || unmatched > 1) {
      break;
    }

    if (end_str != std::string::npos && start_str != std::string::npos) {
      assert(end_str >= start_str + 2);

      strs.push_back(text.substr(start_str + 2, end_str - (start_str + 2)));

      end_str = std::string::npos;
      start_str = std::string::npos;
    }

    i += utf8_size;
  }

  if (unmatched != 0) {
    throw unit::RuntimeError(u8"unmatched \u00ab or \u00bb at line " + std::to_string(line + 1) + ": " + p.u8string());
  }

  return strs;
}

std::vector<std::string> ParseXLIFF(const std::filesystem::path& p) {
  std::vector<std::string> strs;
  strs.reserve(128);

  pugi::xml_document doc;

  pugi::xml_parse_result result = doc.load_file(p.c_str(), pugi::parse_default | pugi::parse_ws_pcdata);

  if (!result) {
    throw unit::RuntimeError("error parsing XLIFF: " + std::string(result.description()) + "; " + p.u8string());
  }

  pugi::xml_node xliff = doc.child("xliff");

  if (!xliff) {
    throw unit::RuntimeError("error parsing XLIFF: no <xliff> tag found; " + p.u8string());
  }

  std::string version = xliff.attribute("version").as_string();

  if (version.empty() || version[0] != '2') {
    throw unit::RuntimeError("unsupported version of XLIFF: " + version + "; " + p.u8string());
  }

  pugi::xml_node file = xliff.child("file");

  auto units = file.children("unit");

  std::size_t num_source = 0;
  std::size_t num_target = 0;

  for (auto& unit : units) {
    std::string unit_id_str = unit.attribute("id").as_string();
    unit_id_str = unit_id_str.size() > 4 ? unit_id_str.substr(4) : unit_id_str;  // str_x

    std::size_t unit_id = 0;

    auto [ptr, ec] = std::from_chars(unit_id_str.data(), unit_id_str.data() + unit_id_str.size(), unit_id);

    if (ec != std::errc{} || ptr != unit_id_str.data() + unit_id_str.size()) {
      throw unit::RuntimeError("a <unit> tag has an invalid id: " + unit_id_str);
    }

    std::string str;
    str.reserve(128);

    std::size_t num_segments = 0;

    for (pugi::xml_node segment : unit.children("segment")) {
      num_segments++;

      pugi::xml_node source = segment.child("source");
      pugi::xml_node target = segment.child("target");

      pugi::xml_node src_node;

      if (target && source) {
        src_node = target;
        num_target++;
        num_source++;
      } else if (source) {
        src_node = source;
        num_source++;
      } else {
        UNIT_LOG_WARN("a <unit> tag without a <source> tag. unit id: " + unit_id_str);
        continue;
      }

      if (target && target.text().empty() && !source.text().empty()) {
        UNIT_LOG_WARN("an empty <target> tag for a non-empty <source> tag. unit id: " + unit_id_str);
      }

      for (pugi::xml_node child : src_node.children()) {
        if (child.type() == pugi::node_pcdata) {
          str += child.value();
        } else if (std::string(child.name()) == "ph") {
          auto equiv = child.attribute("equiv");
          if (!equiv.empty()) {
            str += equiv.value();
          } else {
            UNIT_LOG_WARN("a <ph> tag without the equiv attribute. unit id: " + unit_id_str);
          }
        } else {
          UNIT_LOG_WARN(std::string("ignored the tag: <") + child.name() + ">. unit id: " + unit_id_str);
        }
      }
    }

    if (num_segments > 1) UNIT_LOG_WARN("multiple <segment> tags. unit id: " + unit_id_str);

    if (strs.size() < unit_id + 1) strs.resize(unit_id + 1);

    auto& str_d = strs.at(unit_id);

    if (!str_d.empty()) {
      UNIT_LOG_WARN("a duplicate <unit> id detected " + unit_id_str);
    }

    str_d = std::move(str);
  }

  UNIT_LOG_INFO(std::to_string(num_source) + " source strings; " + std::to_string(num_target) +
                " corresponding target strings");

  return strs;
};

namespace unit {

struct BinBitmapFont {
  nnl::Buffer textures;
  nnl::Buffer advance_widths;
};

bool ImportText(const ImportTxtOpt& opt) {
  nnl::Buffer b = utl::LoadFile(opt.typeface_paths.at(0));

  if (opt.typeface_paths.size() > 1 && opt.kerning) {
    throw unit::RuntimeError("--kerning is not supported when multiple fonts are used");
  }

  if (opt.typeface_paths.size() > 1 && opt.out_format == ImportTxtOpt::kNSUNI) {
    UNIT_LOG_WARN("multiple fonts are not supported for the output format, see --fmt");
  }

  std::string ext = nnl::utl::string::ToLower(opt.input_path.extension().u8string());

  std::vector<std::string> strs = ext == ".txt" ? ParseTXT(opt.input_path) : ParseXLIFF(opt.input_path);

  if (strs.empty()) throw unit::RuntimeError(u8"no text strings were found: " + opt.input_path.u8string());

  text::Text dialog;
  std::vector<BinBitmapFont> bin_fonts;
  bin_fonts.reserve(opt.typeface_paths.size());

  const bool is_unbundled_text = text::IsOfType(b);
  const bool is_asset_container = !is_unbundled_text && asset::IsOfType(b);

  if ((is_unbundled_text || is_asset_container) && opt.typeface_paths.size() > 1) {
    throw unit::RuntimeError("expected only one path for --base");
  }

  if (is_unbundled_text) {
    if (opt.out_format == ImportTxtOpt::kNSUNI) {
      throw unit::RuntimeError(
          "the base asset is an incomplete text archive (no bitmap font), --fmt=NSUNI cannot be used.");
    }

    dialog = text::Import(b);
    dialog.strings.clear();
    dialog = text::Convert(strs, text::kSpecialCodeToString, dialog.characters);

    UNIT_LOG_WARN("the base asset is an incomplete text archive (no bitmap font); the output is also a text archive");

    auto txt = text::Export(dialog);
    utl::SaveFile(opt.output_path, txt);  // Main text
    return true;
  }

  if (is_asset_container) {
    asset::Asset asset = asset::Import(b);

    auto cat = asset::Categorize(asset);

    if (cat != asset::Category::kBitmapTextFull)
      throw unit::RuntimeError("the base asset container is not a text archive");

    dialog = text::Import(asset[asset::BitmapText::kText]);
    dialog.strings.clear();
    dialog = text::Convert(strs, text::kSpecialCodeToString, dialog.characters);

    auto& bin_texture_container = asset[asset::BitmapText::kTextureContainer];
    auto& bin_advance_widths = asset[asset::BitmapText::kAdvanceWidth];
    bin_fonts.push_back({std::move(bin_texture_container), std::move(bin_advance_widths)});
  }

  if (bin_fonts.empty()) {
    dialog = text::Convert(strs);

    for (auto& font_path : opt.typeface_paths) {
      auto [stextures, tracking] =
          text::GenerateBitmapFont(dialog, font_path,
                                   {opt.quality, opt.columns, opt.opacity, opt.scale_factor, opt.tracking_offset,
                                    opt.kerning, opt.nearest, opt.texture_compress_lvl == 2U ? 16U : 256U});

      texture::ConvertParam param;

      param.max_mipmap_lvl = 0;

      param.swizzle = opt.swizzle;

      switch (opt.texture_compress_lvl) {
        case 0:
          param.texture_format = texture::TextureFormat::kRGBA8888;
          break;
        case 1:
          param.texture_format = texture::TextureFormat::kCLUT8;
          break;
        default:
          param.texture_format = texture::TextureFormat::kCLUT4;
          break;
      }

      param.clut_format = texture::ClutFormat::kRGBA8888;

      bin_fonts.push_back({texture::Export(texture::Convert(std::move(stextures), param)), tracking});
    }
  }

  switch (opt.out_format) {
    case ImportTxtOpt::OutFmt::kNSUNI: {
      auto& font = bin_fonts.at(0);
      asset::Asset asset;
      asset[asset::BitmapText::kTextureContainer] = std::move(font.textures);
      asset[asset::BitmapText::kAdvanceWidth] = std::move(font.advance_widths);
      asset[asset::BitmapText::kText] = text::Export(dialog);
      auto buf = asset::Export(asset);
      utl::SaveFile(opt.output_path, buf);
      return true;
    }
    case ImportTxtOpt::OutFmt::kNSLAR: {
      auto txt = text::Export(dialog);
      utl::SaveFile(opt.output_path, txt);  // Main text
      const auto out_folder = opt.output_path.parent_path();

      for (std::size_t i = 0; i < bin_fonts.size(); i++) {
        std::string out_name = opt.output_path.stem().u8string() + "_" + std::to_string(i + 1) + ".bitmap_font";
        auto out_font_path = out_folder / fs::u8path(out_name);
        auto& font = bin_fonts[i];
        asset::Asset asset;
        asset[asset::BitmapText::kTextureContainer] = std::move(font.textures);
        asset[asset::BitmapText::kAdvanceWidth] = std::move(font.advance_widths);

        auto buf = asset::Export(asset);
        utl::SaveFile(out_font_path, buf);
      }
      return true;
    }

    case ImportTxtOpt::OutFmt::kSplit: {
      auto txt = text::Export(dialog);
      utl::SaveFile(opt.output_path, txt);  // Main text
      const auto out_folder = opt.output_path.parent_path();
      for (std::size_t i = 0; i < bin_fonts.size(); i++) {
        std::string out_name = opt.output_path.stem().u8string() + "_" + std::to_string(i + 1);

        std::string out_texture_name = out_name + "_0.texture";
        std::string out_spacing_name = out_name + "_1.spacing";
        auto out_texture_path = out_folder / fs::u8path(out_texture_name);
        auto out_spacing_path = out_folder / fs::u8path(out_spacing_name);
        auto& font = bin_fonts[i];

        utl::SaveFile(out_texture_path, font.textures);
        utl::SaveFile(out_spacing_path, font.advance_widths);
      }
      return true;
    }
    default:
      throw unit::RuntimeError("invalid output format");
  }

  return false;
}
}  // namespace unit
