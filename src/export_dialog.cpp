
#include "export.hpp"
#include "utils.hpp"
namespace unit {

bool ExportDialog(const asset::AssetView& asset_container,
                  const ExportOptions& options) {
  auto text = text::Import(asset_container.at(2));

  auto textures = texture::Import(asset_container.at(0));

  auto stextures = texture::Convert(textures);

  auto stext = text::Convert(text);

  std::string txt;

  txt.reserve(stext.size() * 128);

  for (auto& str : stext) txt += u8"\u00ab" + str + u8"\u00bb\n\n";

  utl::SaveFile(
      utl::ReplaceExtensionFront(options.output_path, fs::u8path(".txt")), txt);

  auto parent_path = options.output_path.parent_path();

  for (STexture& stexture : stextures) {
    stexture.ExportPNG(parent_path / fs::u8path(stexture.name), false);
  }

  return true;
}

}  // namespace unit
