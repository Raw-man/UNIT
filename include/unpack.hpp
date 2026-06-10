#pragma once
#include "NNL/nnl.hpp"
#include "app_options.hpp"

namespace unit {

namespace fs = std::filesystem;

using namespace nnl;

bool Unpack(const UnpackOpt& opt);

}  // namespace unit
