#pragma once

#include <CLI/CLI.hpp>
#include <filesystem>
#include <string>

namespace unit {

struct ExistingParentPathValidator : public CLI::Validator {
  ExistingParentPathValidator(const std::string& desc = "PATH")
      : Validator(desc) {
    name_ = "ExistingParentPath";
    func_ = [](const std::string& str) {
      std::filesystem::path p = std::filesystem::u8path(str).parent_path();

        if (p.empty()) {
            p = ".";
        }

      if (!std::filesystem::is_directory(p))
        return std::string("parent directory does not exist: " + p.u8string());
      else
        return std::string();
    };
  }
};

inline const ExistingParentPathValidator ExistingParentPath;

inline const ExistingParentPathValidator ExistingParentPathFile{"FILE"};

inline const ExistingParentPathValidator ExistingParentPathDir{"DIR"};

template <std::size_t min_val, std::size_t max_val>
class Pow2Range : public CLI::Validator {
 public:
  explicit Pow2Range(const std::string& validator_name = {})
      : Validator(validator_name) {
    static_assert(IsPow2(min_val) && IsPow2(max_val) && max_val > min_val);

    if (validator_name.empty()) {
      std::string out = "UINT in [" + std::to_string(min_val) + " - " +
                        std::to_string(max_val) + "]";
      description(out);
    }

    name_ = "POW2";
    func_ = [](std::string& input) {
      auto n = std::strtoul(input.c_str(), nullptr, 0);

      if (n < min_val || n > max_val || !Pow2Range::IsPow2(n)) {
        std::string allowed;
        auto val = min_val > 0 ? min_val : 1u;
        while (val <= max_val) {
          allowed += std::to_string(val) + " ";
          val *= 2;
        }

        return std::string("allowed values: " + allowed);
      }

      else
        return std::string();
    };
  }

 private:
  template <typename T>
  static constexpr bool IsPow2(T val) {
    return (val & (val - 1)) == 0;
  }
};

class EnumTransformer : public CLI::CheckedTransformer {
 public:
  template <typename T>
  explicit EnumTransformer(T mapping) : CheckedTransformer(mapping) {
    desc_function_ = [mapping]() {
      std::string out("VALUE in ");
      out += CLI::detail::generate_map(CLI::detail::smart_deref(mapping), true);
      return out;
    };
  }
};

struct ValidHexValidator : public CLI::Validator {
  ValidHexValidator() : Validator("HEX") {
    name_ = "IsHexString";
    func_ = [](const std::string& str) {
      const auto IsValidHex = [](char c) -> bool {
        return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') ||
               (c >= 'a' && c <= 'f') || (c == 32);
      };

      for (char c : str) {
        if (!IsValidHex(c)) return std::string("invalid hex character: ") + c;
      }

      return std::string();
    };
  }
};

const static ValidHexValidator ValidHex;

inline const auto NormalizePath = [](const std::string& str) -> std::string {
  auto full_path = std::filesystem::u8path(str);

  full_path = full_path.lexically_normal();

  full_path = std::filesystem::absolute(full_path);

  // remove trailing slash
  if (!full_path.has_filename()) full_path = full_path.parent_path();

  return full_path.u8string();
};
}  // namespace unit
