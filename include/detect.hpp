#pragma once

#include <string>
#include "NNL/game_asset/container/asset.hpp"
#include "NNL/game_asset/format.hpp"
namespace unit {

using namespace nnl;


std::string GetExtension(format::FileFormat type);

std::string GetDescription(format::FileFormat type);

std::string GetCatExtension(asset::Category cat);

std::string GetCatDescription(asset::Category cat);

}
