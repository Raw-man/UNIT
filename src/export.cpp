#include "export.hpp"

#include "logger.hpp"
#include "utils.hpp"
namespace unit {

bool Export(const ExportOptions& opt, bool exp_to_dir) {
  nnl::Buffer bin_asset = utl::LoadFile(opt.input_path);

  auto asset_type = format::Detect(bin_asset);

  if (asset_type == format::kUnknown) {
    UNIT_LOG_WARN("unknown asset type: " + opt.input_path.u8string());
    return false;
  }

  if (asset_type == format::kATRAC3 || asset_type == format::kPlainText) {
    UNIT_LOG_DEBUG("skipped: " + opt.input_path.u8string());
    return false;
  }

  if (asset_type == format::kAssetContainer) {
    auto asset_container = asset::ImportView(bin_asset);

    auto category = asset::Categorize(asset_container);

    if (exp_to_dir && (category != asset::Category::kUnknown && category != asset::Category::kPlaceholder))
      utl::CreateDir(opt.output_path.parent_path());

    if (category == asset::Category::kAsset3DAnim && opt.base_path.empty()) {
      UNIT_LOG_WARN(
          "the asset only stores animations. A base path to a "
          "complete asset with a model is required: " +
          opt.input_path.u8string());
      return false;
    }

    nnl::Buffer bin_base_asset;

    if (!opt.base_path.empty() && category == asset::Category::kAsset3DAnim) {
      bin_base_asset = utl::LoadFile(opt.base_path);

      if (!asset::IsOfType(bin_base_asset)) {
        UNIT_LOG_ERROR("the base asset is not an asset container: " + opt.base_path.u8string());
        return false;
      }

      auto base_container = asset::ImportView(bin_base_asset);

      if (base_container.size() < asset_container.size()) {
        UNIT_LOG_ERROR(
            "the original base container has fewer parts than the partial one. "
            "Try swapping -b and -i");
        return false;
      }

      if (base_container.count(asset::Asset3D::kModel) == 0 ||
          !model::IsOfType(base_container.at(asset::Asset3D::kModel))) {
        UNIT_LOG_ERROR("the base asset does not contain a model " + opt.base_path.u8string());
        return false;
      }

      for (auto& [key, new_buffer] : asset_container) {
        base_container[key] = new_buffer;
      }

      asset_container = std::move(base_container);

      category = asset::Categorize(asset_container);
    }

    if (category == asset::Category::kAsset3DModel) {
      return ExportModelContainer(asset_container, opt);
    }

    if (category == asset::Category::kBitmapTextFull) {
      return ExportDialog(asset_container, opt);
    }

    if (category == asset::Category::kSoundBank) {
      auto exp_opt = opt;
      // here output is not a concrete file but always a folder
      if (exp_to_dir) {
        // change the path back to a folder
        exp_opt.output_path = exp_opt.output_path.parent_path();
      }

      utl::CreateDir(exp_opt.output_path);

      return ExportPPHD8(asset_container, exp_opt);
    }

    if (category == asset::Category::kPlaceholder) {
      UNIT_LOG_WARN("this asset container is an empty placeholder: " + opt.input_path.u8string());
      return true;
    }

    UNIT_LOG_WARN(
        "this asset container cannot be exported. You may try the "
        "unpack command: " +
        opt.input_path.u8string());

    return false;
  }

  if (exp_to_dir) utl::CreateDir(opt.output_path.parent_path());

  if (asset_type == format::kTextureContainer) {
    auto exp_opt = opt;
    // here output is not a concrete file but always a folder
    if (exp_to_dir) {
      // change the path back to a folder
      exp_opt.output_path = exp_opt.output_path.parent_path();
    }
    utl::CreateDir(exp_opt.output_path);

    auto textures = texture::Import(bin_asset);

    return ExportTextureContainer(textures, exp_opt);
  }

  if (asset_type == format::kPositionData) {
    auto positions = posd::Import(bin_asset);

    return ExportPositionData(positions, opt);
  }

  if (asset_type == format::kRenderConfig) {
    auto distance = render::Import(bin_asset);

    return ExportDistance(distance, opt);
  }

  if (asset_type == format::kLit) {
    auto lit = lit::Import(bin_asset);

    return ExportLit(lit, opt);
  }

  if (asset_type == format::kFog) {
    auto fog = fog::Import(bin_asset);

    return ExportFog(fog, opt);
  }

  if (asset_type == format::kMinimapConfig) {
    if (opt.base_path.empty()) {
      UNIT_LOG_WARN(
          "the asset is a minimap config with no texture. A base path to "
          "a minimap texture (container) is required: " +
          opt.input_path.u8string());
      return false;
    }

    auto mini = minimap::Import(bin_asset);

    return ExportMinimap(mini, opt);
  }

  if (asset_type == format::kText) {
    UNIT_LOG_WARN("a text archive is outside of an asset container: " + opt.input_path.u8string());

    auto text = text::Import(bin_asset);

    return ExportDialogPartial(text, opt);
  }

  if (asset_type == format::kModel) {
    UNIT_LOG_WARN("a model is outside of an asset container: " + opt.input_path.u8string());

    auto model = model::Import(bin_asset);

    return ExportModelPartial(model, opt);
  }

  if (asset_type == format::kShadowCollision) {
    UNIT_LOG_WARN("a shadow collision is outside of an asset container: " + opt.input_path.u8string());

    auto collision = shadow_collision::Import(bin_asset);

    return ExportShadowCollisionPartial(collision, opt);
  }

  if (asset_type == format::kCollision) {
    UNIT_LOG_WARN("a collision is outside of an asset container: " + opt.input_path.u8string());

    auto collision = collision::Import(bin_asset);

    return ExportCollisionPartial(collision, opt);
  }

  if (asset_type == format::kDig || asset_type == format::kDigEntry || asset_type == format::kCollection) {
    UNIT_LOG_WARN("the file is a container, you can use the unpack command: " + opt.input_path.u8string());
    return false;
  }

  if (asset_type != format::kUnknown) {
    UNIT_LOG_WARN(
        "the asset type is known, but it can't be exported yet (or by "
        "itself): " +
        opt.input_path.u8string());
  }

  return false;
}

}  // namespace unit
