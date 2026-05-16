
#include <pugixml.hpp>

#include "export.hpp"
#include "logger.hpp"
#include "utils.hpp"

namespace unit {

void ToXMLNode(pugi::xml_node& unit, const text::Text& text, std::size_t str_id) {
  const auto& replacements = text::kSpecialCodeToString;

  pugi::xml_node segment = unit.append_child("segment");
  pugi::xml_node source = segment.append_child("source");

  source.append_attribute("xml:space") = "preserve";

  auto& str = text.strings.at(str_id);
  std::string buffer;
  buffer.reserve(str.size() * 4);

  std::size_t ph_tag_id = 0;

  std::map<std::string_view, std::size_t> tag_ids;

  if (str.empty() || str.front() == 0x8000) {
    unit.append_attribute("translate") = "no";
  }

  for (auto character_index : str) {
    if ((character_index & text::kSpecialCodeMask) == 0) {
      buffer += nnl::utl::utf8::Encode(text.characters.at(character_index));
    } else if (replacements.find(character_index) != replacements.end()) {
      std::string_view replacement = replacements.at(character_index);

      if (replacement.size() <= 1) {
        buffer += replacement;  // new line, space, etc...
        continue;
      }

      if (!buffer.empty()) {
        source.append_child(pugi::node_pcdata).set_value(buffer.c_str());
        buffer.clear();
      }

      pugi::xml_node ph = source.append_child("ph");

      std::string ph_id = std::string(replacement.substr(1));
      ph_id.pop_back();

      ph_id += "." + std::to_string(ph_tag_id);

      ph.append_attribute("id") = ph_tag_id;

      ph_tag_id++;

      ph.append_attribute("disp") = ph_id;

      ph.append_attribute("equiv") = replacement;

      if ((character_index & 0x1000) != 0) {
        ph.append_attribute("type") = "ui";
      } else if (character_index > 0x8038) {
        ph.append_attribute("type") = "ui";
        ph.append_attribute("subType") = "xlf:var";
      } else if (character_index > 0x8001) {
        ph.append_attribute("type") = "fmt";
      }

    } else {
      UNIT_LOG_WARN("unknown special code ignored: " + nnl::utl::string::IntToHex(character_index, true));
    }
  }

  if (!buffer.empty()) {
    source.append_child(pugi::node_pcdata).set_value(buffer.c_str());
  };
}

void SerializeAsXLIFF(const text::Text& text, const std::filesystem::path& out) {
  pugi::xml_document doc;

  pugi::xml_node decl = doc.prepend_child(pugi::node_declaration);
  decl.append_attribute("version") = "1.0";
  decl.append_attribute("encoding") = "UTF-8";

  pugi::xml_node xliff = doc.append_child("xliff");
  xliff.append_attribute("xmlns") = "urn:oasis:names:tc:xliff:document:2.0";
  xliff.append_attribute("version") = "2.1";
  xliff.append_attribute("srcLang") = "und";

  pugi::xml_node file = xliff.append_child("file");
  file.append_attribute("id") = out.stem().u8string();

  for (std::size_t i = 0; i < text.strings.size(); i++) {
    pugi::xml_node unit = file.append_child("unit");
    unit.append_attribute("id") = "str_" + std::to_string(i);

    ToXMLNode(unit, text, i);
  }

  if (!doc.save_file(out.c_str(), "  ", pugi::format_raw)) {
    throw unit::RuntimeError("failed to save " + out.u8string());
  }
}

bool ExportDialog(const asset::AssetView& asset_container, const ExportOptions& options) {
  auto text = text::Import(asset_container.at(2));

  auto textures = texture::Import(asset_container.at(0));

  auto stextures = texture::Convert(textures);

  SerializeAsXLIFF(text, utl::ReplaceExtensionFront(options.output_path, fs::u8path(".xlf")));

  auto parent_path = options.output_path.parent_path();

  for (STexture& stexture : stextures) {
    stexture.ExportPNG(parent_path / fs::u8path(stexture.name), false);
  }

  return true;
}

}  // namespace unit
