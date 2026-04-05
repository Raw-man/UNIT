#include "detect.hpp"

#include <map>

namespace unit {
std::string GetExtension(format::FileFormat type) {
  static const std::map<format::FileFormat, std::string> ext{{format::kUnknown, ".unk"},
                                                             {format::kPGD, ".pgd"},
                                                             {format::kDig, ".dig"},
                                                             {format::kDigEntry, ".dig_entry"},
                                                             {format::kCollection, ".collection"},
                                                             {format::kAssetContainer, ".asset_container"},
                                                             {format::kModel, ".model"},
                                                             {format::kTextureContainer, ".texture"},
                                                             {format::kAnimationContainer, ".animation"},
                                                             {format::kActionConfig, ".action"},
                                                             {format::kColboxConfig, ".colbox"},
                                                             {format::kVisanimationContainer, ".visib_animation"},
                                                             {format::kCollision, ".collision"},
                                                             {format::kShadowCollision, ".shadow_collision"},
                                                             {format::kText, ".text"},
                                                             {format::kATRAC3, ".at3"},
                                                             {format::kFog, ".fog"},
                                                             {format::kPositionData, ".posd"},
                                                             {format::kLit, ".lit"},
                                                             {format::kRenderConfig, ".distance"},
                                                             {format::kPHD, ".phd"},
                                                             {format::kPNG, ".png"},
                                                             {format::kCCSF, ".ccsf"},
                                                             {format::kUIConfig, ".user_interface"},
                                                             {format::kPSF, ".sfo"},
                                                             {format::kPSPELF, ".elf"},
                                                             {format::kELF, ".elf"},
                                                             {format::kPlainText, ".txt"},
                                                             {format::kMinimapConfig, ".minimap"}};

  return ext.at(type);
}

std::string GetDescription(format::FileFormat type) {
  static const std::map<format::FileFormat, std::string> ext{
      {format::kUnknown, "Unknown game asset type."},
      {format::kPGD,
       "Protected Game Data. Needs to be decrypted first. Usually, it contains "
       "the main game archive."},
      {format::kDig, "A .dig/.bin file - a main game archive."},
      {format::kDigEntry, "An entry in a .dig/.bin archive - an archive in itself."},
      {format::kCollection, "A container that stores similar but usually distinct assets."},
      {format::kAssetContainer, "A container that stores related parts of a single asset."},
      {format::kModel, "Unbundled model data. Usually part of an asset container."},
      {format::kTextureContainer, "Unbundled texture container. Usually part of an asset container."},
      {format::kAnimationContainer,
       "Unbundled skeletal animation data.  Usually part of an asset "
       "container."},
      {format::kActionConfig, "Unbundled action config. Usually part of an asset container."},
      {format::kColboxConfig,
       "Unbundled collision box (hitbox) config.  Usually part of an asset "
       "container."},
      {format::kVisanimationContainer,
       "Unbundled mesh group visibility animation data. Usually part of an "
       "asset container."},
      {format::kCollision,
       "Unbundled collision geometry for maps or map objects. Usually part of "
       "an asset container."},
      {format::kShadowCollision,
       "Unbundled geometry that is used for receiving blob/planar shadows "
       "(NSLAR). Usually part of an asset container."},
      {format::kText,
       "Unbundled game text archive (no bitmap font). Usually part of another "
       "container "
       "(typically, an asset container in NSUNI)"},
      {format::kATRAC3, "ATRAC3 audio. Used for background music or character dialogs"},
      {format::kFog, "A fog config file. Used in NSLAR"},
      {format::kPositionData, "Spawn position data"},
      {format::kLit, "A light config."},
      {format::kRenderConfig, "A render/distance config (view distance, mipmaps, bloom). Used in NSUNI"},
      {format::kPHD,
       "A header file (PHD) of a PHD/PBD soundbank. Part of an asset container that "
       "also includes ADPCM audio data (PBD)"},
      {format::kPNG, "A PNG image."},
      {format::kCCSF,
       "An asset format developed by CyberConnect2. It's used for finishing "
       "blow cutscenes in NSUNI. See "
       "https://github.com/MiguelQueiroz010/CCSF-Asset-Explorer/"},

      {format::kUIConfig,
       "Unbundled ui config (no textures). Usually part of "
       "an asset container."},

      {format::kPSF,
       "A metadata file. See "
       "https://github.com/xXxTheDarkprogramerxXx/PS3Tools"},
      {format::kPSPELF,
       "An encrypted executable file. See "
       "https://github.com/Linblow/pspdecrypt"},
      {format::kELF, "An executable file"},
      {format::kPlainText, "A plain text file"},
      {format::kMinimapConfig, "A minimap config"}};

  return ext.at(type);
}

std::string GetCatExtension(asset::Category type) {
  static const std::map<asset::Category, std::string> cat{
      {asset::Category::kUnknown, ".unk"},
      {asset::Category::kPlaceholder, ".placeholder"},
      {asset::Category::kAsset3DModel, ".3d.mdl"},
      {asset::Category::kAsset3DAnim, ".3d.anim"},
      {asset::Category::kAsset3DAction, ".3d.act"},
      {asset::Category::kAsset3DEffect, ".3d.vfx"},
      {asset::Category::kBitmapTextFull, ".bitmap_text"},
      {asset::Category::kBitmapTextFont, ".bitmap_font"},
      {asset::Category::kSoundBank, ".sound_bank"},
      {asset::Category::kUIConfigs, ".ui_configs"},
      {asset::Category::kUIConfigTextureContainer, ".ui"},
  };

  return cat.at(static_cast<asset::Category>(type));
}

std::string GetCatDescription(asset::Category type) {
  static const std::map<asset::Category, std::string> cat{
      {asset::Category::kUnknown, "Unknown category of an asset container"},
      {asset::Category::kPlaceholder, "An empty placeholder"},
      {asset::Category::kAsset3DModel,
       "A complete 3D model (character, map, map prop, animated camera in "
       "NSUNI)"},
      {asset::Category::kAsset3DAnim, "Animation data for another 3D model"},
      {asset::Category::kAsset3DAction,
       "Stores only an action config. May be used for short camera animation "
       "in NSUNI"},
      {asset::Category::kAsset3DEffect, "A 3D effect (not a model!)"},
      {asset::Category::kBitmapTextFull, "A complete text archive (with a bitmap font). Typical for NSUNI"},
      {asset::Category::kBitmapTextFont,
       "A bitmap font (without any textual data). Usually part of a collection "
       "(in NSLAR)"},
      {asset::Category::kSoundBank, "A PHD/PBD sound bank (contains SFX)"},

      {asset::Category::kUIConfigs, "A container of ui configs (no textures)"},
      {asset::Category::kUIConfigTextureContainer, "A container of pairs of ui configs and textures"}};

  return cat.at(type);
}
}  // namespace unit
