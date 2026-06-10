#pragma once
#include "NNL/nnl.hpp"
#include "app_options.hpp"

namespace unit {

namespace fs = std::filesystem;

using namespace nnl;

nnl::Buffer PackAsset(const PackOpt& opt);
nnl::Buffer PackCollection(const PackOpt& opt);
nnl::Buffer PackDigEntry(const PackOpt& opt);
nnl::Buffer PackDig(const PackOpt& opt);

}  // namespace unit
