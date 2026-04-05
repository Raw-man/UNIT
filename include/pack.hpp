#pragma once
#include "app_options.hpp"
#include "NNL/nnl.hpp"

namespace unit {

namespace fs = std::filesystem;

using namespace nnl;


nnl::Buffer PackAsset(const PackOpt& opt);
nnl::Buffer PackCollection(const PackOpt& opt);
nnl::Buffer PackDigEntry(const PackOpt& opt);
nnl::Buffer PackDig(const PackOpt& opt);

}
