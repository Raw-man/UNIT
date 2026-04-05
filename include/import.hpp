#pragma once

#include "app_options.hpp"
#include "NNL/nnl.hpp"

namespace unit {
namespace fs = std::filesystem;

using namespace nnl;

std::string GetAssetStats(const SAsset3D& sasset, bool all = true);

bool ImportModelContainer(const ImportMdlOpt& opt);

bool ImportCamera(const ImportCamOpt& opt);

bool ImportPosition(const ImportPosOpt& opt);

bool ImportMinimap(const ImportMinOpt& opt);

bool ImportLight(const ImportLitOpt& opt);

bool ImportPPHD8(const ImportSndOpt& opt);

bool ImportTextureContainer(const ImportImgOpt& opt);

bool ImportDialog(const ImportTxtOpt& opt);

bool ImportFog(const ImportFogOpt& opt);

bool ImportDistance(const ImportDisOpt& opt);

}  // namespace unit
