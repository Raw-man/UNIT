#pragma once
#include "app_options.hpp"
#include "NNL/nnl.hpp"

namespace unit {

namespace fs = std::filesystem;

using namespace nnl;


bool Unpack(const UnpackOpt& opt);

}
