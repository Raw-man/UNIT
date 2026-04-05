#include <filesystem>

#include "export.hpp"

namespace unit {
bool ExportTextureContainer(const texture::TextureContainer& textures,
                            const ExportOptions& options) {
  auto stextures = texture::Convert(textures, options.mipmaps);

  const fs::path output_dir_path = options.output_path;

  for (STexture& stexture : stextures) {
    stexture.ExportPNG(output_dir_path / fs::u8path(stexture.name), false);
  }

  return true;
}

}  // namespace unit
