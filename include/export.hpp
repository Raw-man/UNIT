#pragma once

#include "app_options.hpp"

#include "NNL/nnl.hpp"

namespace unit {

namespace fs = std::filesystem;

using namespace nnl;


bool ExportModelContainer(const asset::AssetView& asset_container,
                          const ExportOptions& options);

bool ExportModelPartial(const model::Model& model,
                        const ExportOptions& options);

bool ExportCollisionPartial(const collision::Collision& collision,
                            const ExportOptions& options);

bool ExportShadowCollisionPartial(const shadow_collision::Collision& collision,
                                  const ExportOptions& options);

bool ExportDialog(const asset::AssetView& asset_container,
                  const ExportOptions& options);

bool ExportDialogPartial(const text::Text& text, const ExportOptions& options);

bool ExportPPHD8(const asset::AssetView& asset, const ExportOptions& options);

bool ExportTextureContainer(const texture::TextureContainer& textures,
                            const ExportOptions& options);

bool ExportPositionData(const posd::PositionData& positions,
                        const ExportOptions& options);

bool ExportLit(const lit::Lit& lit, const ExportOptions& options);

bool ExportFog(const fog::Fog& fog, const ExportOptions& options);

bool ExportMinimap(const minimap::MinimapConfig& minimap, const ExportOptions& options);

bool ExportDistance(const render::RenderConfig& d, const ExportOptions& options);

bool Export(const ExportOptions& options, bool exp_to_dir = false);

}  // namespace unit
