#include "collision_json.hpp"
#include "export.hpp"
#include "logger.hpp"
#include "utils.hpp"
namespace unit {

using namespace std::string_literals;

bool ExportModelContainer(const asset::AssetView& asset_container, const ExportOptions& options) {
  using namespace asset;

  SAsset3D sasset;

  if (asset_container.count(Asset3D::kTextureContainer)) {
    try {
      auto textures = texture::Import(asset_container.at(Asset3D::kTextureContainer));

      sasset.textures = texture::Convert(textures);

      for (std::size_t t = 0; t < textures.size() && options.mipmaps && !options.pack_textures; t++) {
        auto& tex = textures.at(t);
        for (std::size_t lvl = 1; lvl < tex.texture_data.size(); lvl++) {
          auto stex = texture::Convert(tex, lvl);
          stex.name = nnl::utl::string::PrependZero(t, 2) + "_" + stex.name;
          stex.ExportPNG(options.output_path.parent_path() / nnl::utl::filesys::u8path(stex.name), options.flip);
        }
      }

    } catch (const std::exception& e) {
      UNIT_LOG_ERROR(e.what() + "; texture export failed: "s + options.input_path.u8string());
    }
  }

  if (asset_container.count(Asset3D::kAnimationContainer)) {
    try {
      auto animations = animation::Import(asset_container.at(Asset3D::kAnimationContainer));

      sasset.animations = animation::Convert(animations);
    } catch (const std::exception& e) {
      UNIT_LOG_ERROR(e.what() + "; animation export failed: "s + options.input_path.u8string());
    }
  }

  if (asset_container.count(Asset3D::kVisanimationContainer) && options.visibility) {
    try {
      auto subanimations = visanimation::Import(asset_container.at(Asset3D::kVisanimationContainer));

      sasset.visibility_animations = visanimation::Convert(subanimations);
    } catch (const std::exception& e) {
      UNIT_LOG_ERROR(e.what() + "; visibility animation export failed: "s + options.input_path.u8string());
    }
  }

  if (asset_container.count(Asset3D::kActionConfig)) {
    try {
      auto action = action::Import(asset_container.at(Asset3D::kActionConfig));
      auto animation_names = action::GetAnimationNames(action);

      for (std::size_t i = 0; i < sasset.animations.size(); i++) {
        auto& animation = sasset.animations[i];

        // Blender's name limit is 64 chars (with 0 terminator)
        if (animation_names.find(i) != animation_names.end()) {
          for (auto& name : animation_names.at(i)) {
            if (animation.name.size() + 1 + name.size() < (64 - sizeof("Armature"))) animation.name += "_" + name;
          }
        }
      }
    } catch (const std::exception& e) {
      UNIT_LOG_ERROR(e.what() + "; action config export failed: "s + options.input_path.u8string());
    }
  }

  if (asset_container.count(Asset3D::kCollision)) {
    try {
      SAsset3D scollision;

      auto collision = collision::Import(asset_container.at(Asset3D::kCollision));

      scollision.model = collision::Convert(collision);

      auto params = collision::GenerateConvertParam(collision);

      for (std::size_t i = 0; i < scollision.model.meshes.size(); i++) {
        ToSValue(scollision.model.meshes.at(i).extras, params.at(i));
      }

      auto new_path = unit::utl::ReplaceExtensionFront(options.output_path, fs::u8path(".glb"));

      new_path.replace_filename(
          fs::u8path(new_path.stem().u8string() + "_collision" + new_path.extension().u8string()));

      scollision.ExportGLB(new_path);
    } catch (const std::exception& e) {
      UNIT_LOG_ERROR(e.what() + "; collision export failed: "s + options.input_path.u8string());
    }
  }

  if (asset_container.count(Asset3D::kShadowCollision)) {
    try {
      SAsset3D scollision;

      auto shadow_collision = shadow_collision::Import(asset_container.at(Asset3D::kShadowCollision));

      scollision.model = shadow_collision::Convert(shadow_collision);

      auto new_path = unit::utl::ReplaceExtensionFront(options.output_path, fs::u8path(".glb"));

      new_path.replace_filename(
          fs::u8path(new_path.stem().u8string() + "_shadow_collision" + new_path.extension().u8string()));

      scollision.ExportGLB(new_path);
    } catch (const std::exception& e) {
      UNIT_LOG_ERROR(e.what() + "; shadow collision export failed: "s + options.input_path.u8string());
    }
  }

  auto model = model::Import(asset_container.at(Asset3D::kModel));

  sasset.model = model::Convert(model);

  sasset.ExportGLB(unit::utl::ReplaceExtensionFront(options.output_path, fs::u8path(".glb")), options.flip,
                   options.pack_textures);

  return true;
}

}  // namespace unit
