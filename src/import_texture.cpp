#include "import.hpp"
#include "logger.hpp"
#include "threadpool.hpp"
#include "utils.hpp"
namespace unit {
bool ImportTextureContainer(const ImportImgOpt& opt) {
  std::vector<std::future<void>> tasks;

  auto image_files = utl::GetSortedDirEntries(opt.input_path, {".png", ".jpg", ".jpeg", ".bmp"});

  if (image_files.empty())
    throw unit::RuntimeError("no images were found in the directory " + opt.input_path.u8string());

  UNIT_LOG_INFO("num entries: " + std::to_string(image_files.size()));

  ThreadPool pool(
      std::min<std::size_t>(std::max<std::size_t>(std::thread::hardware_concurrency(), 1U), image_files.size()));

  UNIT_LOG_DEBUG("threads: " + std::to_string(pool.GetNumThreads()));

  texture::TextureContainer textures;

  if (!opt.base_path.empty()) {
    auto bin_base_texture = utl::LoadFile(opt.base_path);

    if (!texture::IsOfType(bin_base_texture))
      throw unit::RuntimeError("the base is not a texture container " + opt.base_path.u8string());

    textures = texture::Import(bin_base_texture);
  }

  if (image_files.size() > textures.size()) {
    if (!opt.base_path.empty()) {
      UNIT_LOG_WARN("the base container has fewer textures, errors may occur");
    }

    textures.resize(image_files.size());
  }

  try {
    std::size_t i = 0;

    for (const auto& input_img_path : image_files) {
      tasks.push_back(pool.AddTask([&textures, &opt, input_img_path, i]() {
        std::size_t size = std::filesystem::file_size(input_img_path);

        if (size < 67) {
          UNIT_LOG_DEBUG("a placeholder: " + input_img_path.u8string());

          if (opt.base_path.empty()) throw unit::RuntimeError("a placeholder texture detected but -b was not provided");

          return;
        }

        STexture stexture = STexture::Import(input_img_path);

        stexture.min_filter = opt.nearest ? STextureFilter::kNearest : STextureFilter::kLinear;
        stexture.mag_filter = opt.nearest ? STextureFilter::kNearest : STextureFilter::kLinear;

        if (!nnl::utl::math::IsPow2(stexture.width)) {
          UNIT_LOG_WARN("texture " + stexture.name + ": width is not a power of 2: " + std::to_string(stexture.width) +
                        "x" + std::to_string(stexture.height));

          stexture.AlignWidth();
        }

        stexture.QuantizeAlpha(opt.alpha_levels);

        texture::ConvertParam param;

        param.max_width_height = opt.max_wh;

        param.max_mipmap_lvl = opt.max_mm;

        param.swizzle = opt.swizzle;

        param.clut_dither = opt.dither;

        param.texture_format =
            opt.texture_compress_lvl == 2
                ? texture::TextureFormat::kCLUT4
                : (opt.texture_compress_lvl == 1 ? texture::TextureFormat::kCLUT8 : texture::TextureFormat::kRGBA8888);
        param.clut_format =
            opt.texture_compress_lvl == 2 ? texture::ClutFormat::kRGBA5551 : texture::ClutFormat::kRGBA8888;

        textures.at(i) = texture::Convert(std::move(stexture), param);
      }));

      i++;
    }

    UNIT_LOG_DEBUG("tasks: " + std::to_string(tasks.size()));

    for (std::size_t i = 0; i < tasks.size(); i++) {
      tasks[i].get();

      UNIT_LOG_DEBUG("task completed: " + std::to_string(i));
    }

    auto bin_texture = texture::Export(textures);

    if (bin_texture.size() > 25_MiB)
      UNIT_LOG_WARN("the output file is too big (" + utl::BytesToMegabytes(bin_texture.size()) +
                    ") and might not be loaded");

    utl::SaveFile(opt.output_path, bin_texture);
  } catch (const std::exception& e) {
    pool.Shutdown();
    throw;
  }

  return true;
}
}  // namespace unit
