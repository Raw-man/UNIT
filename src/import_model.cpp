#include <future>

#include "collision_json.hpp"
#include "import.hpp"
#include "logger.hpp"
#include "threadpool.hpp"
#include "utils.hpp"

namespace unit {

enum class STransparencyType {
  kOpaque,  // alpha is ~255
  kBinary,  //~0 or ~255
  kVaried
};

STransparencyType DetermineTransparency(const STexture& stexture) {
  auto& width = stexture.width;
  auto& height = stexture.height;
  auto& bitmap = stexture.bitmap;
  assert(width * height == bitmap.size());

  std::size_t num_fully_opaque = 0;
  std::size_t num_fully_transparent = 0;
  std::size_t num_semi_transparent = 0;
  std::size_t num_transparent = 0;
  std::size_t num_total = bitmap.size();

  for (const auto& pixel : bitmap) {
    bool is_fully_opaque = pixel.a >= NNL_ALPHA_OPAQ;
    bool is_fully_transparent = pixel.a <= NNL_ALPHA_TRANSP;
    bool is_semi_transparent = !is_fully_opaque && !is_fully_transparent;

    num_fully_opaque += is_fully_opaque;
    num_fully_transparent += is_fully_transparent;
    num_semi_transparent += is_semi_transparent;
  }
  num_transparent = num_fully_transparent + num_semi_transparent;

  float opaque_ratio = num_fully_opaque / (float)num_total;

  if (opaque_ratio > 0.999f) return STransparencyType::kOpaque;

  float full_transparent_ratio = num_fully_transparent / (float)num_transparent;

  float semi_transparent_ratio = num_semi_transparent / (float)(num_fully_opaque + num_semi_transparent);

  if (full_transparent_ratio > 0.75f && semi_transparent_ratio < 0.45f)
    return STransparencyType::kBinary;
  else
    return STransparencyType::kVaried;
}

void SetupMaterialAlpha(SAsset3D& sasset) {
  auto& textures = sasset.textures;
  auto& model = sasset.model;
  std::vector<STransparencyType> transparency_types(textures.size());

  for (std::size_t i = 0; i < textures.size(); i++) {
    const auto& stexture = textures.at(i);
    transparency_types.at(i) = DetermineTransparency(stexture);
  }

  for (const auto& smesh : model.meshes) {
    auto& smat = model.materials.at(smesh.material_id);

    if (smat.alpha_mode != SBlendMode::kOpaque) continue;

    if (smat.opacity < 0.99f) {
      smat.alpha_mode = SBlendMode::kAlpha;
      continue;
    }

    if (smat.texture_id != -1) {
      auto transparancy_type = transparency_types.at(smat.texture_id);

      switch (transparancy_type) {
        case STransparencyType::kBinary:
          smat.alpha_mode = SBlendMode::kClip;
          continue;
        case STransparencyType::kVaried:
          smat.alpha_mode = SBlendMode::kAlpha;
          continue;
        case STransparencyType::kOpaque:
          break;
      }
    }

    if (smesh.HasAlphaVertex()) {
      smat.alpha_mode = SBlendMode::kAlpha;
    }
  }
}

bool ImportModelContainer(const ImportMdlOpt& opt) {
  std::mutex mut;

  std::vector<std::future<void>> tasks;

  asset::Asset asset_container;

  SAsset3D sasset;

  texture::TextureContainer texture_cont;

  ThreadPool pool(std::max(std::thread::hardware_concurrency(), 1U));

  UNIT_LOG_DEBUG("threads: " + std::to_string(pool.GetNumThreads()));

  if (!opt.base_path.empty()) {
    auto bin_asset = utl::LoadFile(opt.base_path);

    if (!asset::IsOfType(bin_asset)) {
      throw unit::RuntimeError("the base asset is not a model: " + opt.input_path.u8string());
    }

    asset_container = asset::Import(bin_asset);

    if (asset_container.count(asset::Asset3D::kModel) == 0 ||
        !model::IsOfType(asset_container.at(asset::Asset3D::kModel)))
      throw unit::RuntimeError("the base asset is not a model: " + opt.input_path.u8string());

    if (asset_container.count(asset::Asset3D::kActionConfig) == 0 &&
        asset_container.count(asset::Asset3D::kColboxConfig) == 0)
      UNIT_LOG_WARN("the base asset does not contain an action or a hitbox config: " + opt.base_path.u8string());
  }

  const bool is_character = asset_container.count(asset::Asset3D::kColboxConfig);
  const bool use_bbox = !opt.collision_path.empty() || asset_container.count(asset::Asset3D::kCollision);

  // patch parts of the base asset that come from other configs
  if ((!opt.input_path.empty() || !opt.base_path.empty()) && !opt.output_path.empty()) {
    if (!opt.collision_path.empty()) {
      tasks.push_back(pool.AddTask([&opt, &mut, &asset_container]() {
        SAsset3D scollision = SAsset3D::Import(opt.collision_path, false, false);

        UNIT_LOG_INFO(scollision.name + " (collision):\n" + GetAssetStats(scollision));

        if (opt.scale_factor != 1.0f) scollision.Scale(opt.scale_factor);

        if (!scollision.model.meshes.empty()) {
          std::vector<collision::ConvertParam> params(scollision.model.meshes.size());

          for (std::size_t i = 0; i < scollision.model.meshes.size(); i++) {
            FromSValue(scollision.model.meshes.at(i).extras, params.at(i));
          }

          auto collision = collision::Convert(std::move(scollision.model), params, opt.wall);
          auto bin_collision = collision::Export(collision);
          std::scoped_lock l{mut};
          asset_container[asset::Asset3D::kCollision] = std::move(bin_collision);
        } else {
          UNIT_LOG_WARN(opt.collision_path.u8string() + ": has no meshes");
        }
      }));
    }

    if (!opt.shadow_path.empty()) {
      tasks.push_back(pool.AddTask([&mut, &opt, &asset_container]() {
        SAsset3D sshadow_col = SAsset3D::Import(opt.shadow_path, false, false);

        UNIT_LOG_INFO(sshadow_col.name + " (shadow):\n" + GetAssetStats(sshadow_col));

        if (opt.scale_factor != 1.0f) sshadow_col.Scale(opt.scale_factor);

        if (!sshadow_col.model.meshes.empty()) {
          auto shadow_collision = shadow_collision::Convert(std::move(sshadow_col.model), opt.cull);
          auto bin_shadow_collision = shadow_collision::Export(shadow_collision);
          std::scoped_lock l{mut};
          asset_container[asset::Asset3D::kShadowCollision] = std::move(bin_shadow_collision);
        } else {
          UNIT_LOG_WARN(opt.shadow_path.u8string() + ": has no meshes");
        }
      }));
    }
  }

  // patch parts of base asset that come from a model
  if (!opt.input_path.empty() && !opt.output_path.empty()) {
    sasset = SAsset3D::Import(opt.input_path, opt.flip, opt.sort || opt.blend);

    UNIT_LOG_INFO(sasset.name + ":\n" + GetAssetStats(sasset));

    if (opt.sskeleton) sasset.TrySimplifySkeleton();
    if (opt.scale_factor != 1.0f) sasset.Scale(opt.scale_factor);
    if (opt.blend) SetupMaterialAlpha(sasset);
    if (opt.sort) sasset.SortForBlending(sasset.model.CalculateCenter());

    if (!sasset.model.meshes.empty()) {
      tasks.push_back(pool.AddTask([&mut, &opt, &asset_container, &sasset, is_character, use_bbox]() {
        std::size_t num_mat = sasset.model.materials.size();

        if (opt.materials) sasset.model.RemoveDuplicateMaterials();

        if (std::size_t n = num_mat - sasset.model.materials.size(); n != 0)
          UNIT_LOG_INFO("merged materials: " + std::to_string(n));

        model::ConvertParam param;

        param.join_submeshes = true;

        param.stitch_strips = opt.stitch_strips;

        param.use_bbox = use_bbox;

        param.use_strips = opt.use_strips;

        param.indexed = opt.use_indices;

        param.optimize_weights = opt.optimize_weights;

        param.compress_lvl = opt.compress_lvl == 0
                                 ? model::CompLvl::kNone
                                 : (opt.compress_lvl == 1 ? model::CompLvl::kMedium : model::CompLvl::kMax);

        std::vector<model::ConvertParam> params(sasset.model.meshes.size(), param);

        model::Model model = model::Convert(std::move(sasset.model), params, is_character);

        auto bin_model = model::Export(model);
        std::scoped_lock l{mut};
        asset_container[asset::Asset3D::kModel] = std::move(bin_model);
      }));

    } else {
      throw unit::RuntimeError(sasset.name + ": no meshes were found");
    }

    if (!sasset.animations.empty()) {
      tasks.push_back(pool.AddTask([&mut, &opt, &asset_container, &sasset, is_character]() {
        animation::ConvertParam param;
        param.unbake = opt.simplify;

        animation::AnimationContainer animation = animation::Convert(std::move(sasset.animations), param, is_character);
        auto bin_animation = animation::Export(animation);
        std::scoped_lock l{mut};

        if (animation.animations.size() > 1 && asset_container.count(asset::Asset3D::kActionConfig) == 0)
          UNIT_LOG_WARN(sasset.name +
                        ": has multiple animations but an action config was "
                        "not found (use -b or use the \"Active Actions "
                        "merged\" option when exporting)");

        asset_container[asset::Asset3D::kAnimationContainer] = std::move(bin_animation);
      }));
    }

    if (!sasset.visibility_animations.empty()) {
      tasks.push_back(pool.AddTask([&mut, &asset_container, &sasset]() {
        visanimation::AnimationContainer animation = visanimation::Convert(std::move(sasset.visibility_animations));
        auto bin_animation = visanimation::Export(animation);
        std::scoped_lock l{mut};
        asset_container[asset::Asset3D::kVisanimationContainer] = std::move(bin_animation);
      }));
    }

    if (!sasset.textures.empty()) {
      texture::ConvertParam param;

      param.texture_format =
          opt.texture_compress_lvl == 2
              ? texture::TextureFormat::kCLUT4
              : (opt.texture_compress_lvl == 1 ? texture::TextureFormat::kCLUT8 : texture::TextureFormat::kRGBA8888);
      param.clut_format =
          opt.texture_compress_lvl == 2 ? texture::ClutFormat::kRGBA5551 : texture::ClutFormat::kRGBA8888;
      param.swizzle = opt.swizzle;
      param.max_mipmap_lvl = is_character ? 0 : opt.max_mm;
      param.max_width_height = opt.max_wh;

      param.clut_dither = opt.dither;

      texture_cont.resize(sasset.textures.size());

      for (std::size_t i = 0; i < sasset.textures.size(); i++) {
        tasks.push_back(
            pool.AddTask([&sasset, &texture_cont, param, i, flip = opt.flip, alpha_lvl = opt.alpha_levels]() {
              STexture& stexture = sasset.textures.at(i);

              if (stexture.width == 0) {
                nnl::BufferView buffer(stexture.bitmap);

                stexture = STexture::Import(buffer, flip);
                stexture.AlignWidth();
                stexture.AlignHeight();
              }

              sasset.textures.at(i).QuantizeAlpha(alpha_lvl);

              texture_cont.at(i) = texture::Convert(std::move(sasset.textures.at(i)), param);
            }));
      }
    }
  }

  UNIT_LOG_DEBUG("tasks: " + std::to_string(tasks.size()));

  for (std::size_t i = 0; i < tasks.size(); i++) {
    tasks[i].get();

    UNIT_LOG_DEBUG("task completed: " + std::to_string(i));
  }

  if (!texture_cont.empty()) {
    asset_container[asset::Asset3D::kTextureContainer] = texture::Export(texture_cont);
  }

  assert(asset_container->size() > 0);

  auto bin_container = asset::Export(asset_container);

  if (bin_container.size() > 30_MiB)
    UNIT_LOG_WARN("the output file is too big (" + unit::utl::BytesToMegabytes(bin_container.size()) +
                  ") and might not be loaded");

  utl::SaveFile(opt.output_path, bin_container);

  return true;
}
}  // namespace unit
