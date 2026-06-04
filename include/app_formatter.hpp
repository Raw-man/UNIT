#pragma once

#include <CLI/Formatter.hpp>

namespace unit {

class Formatter : public CLI::Formatter {

 public:
  Formatter(){
    this->label("TEXT:FILE", "PATH:FILE");

    this->label("TEXT:DIR", "PATH:DIR");

    this->label("TEXT:PATH", "PATH");

    this->label("TEXT:PATH(existing)", "PATH(existing)");

    this->label("TEXT:DIR:PATH(non-existing)", "PATH:DIR(non-existing)");

    this->enable_default_flag_values(false);
  }

 private:

  std::string make_positional_option(const CLI::Option *opt) const {
    std::stringstream out;

    auto fn = opt->get_fnames();
    auto sn = opt->get_snames();
    auto ln = opt->get_lnames();
    bool is_positional_only = fn.empty() && sn.empty() && ln.empty();

    const std::string left = "  " + make_option_name(opt, true) + make_option_opts(opt);
    const std::string desc = is_positional_only ? make_option_desc(opt) : "";

    out << std::setw(static_cast<int>(column_width_)) << std::left << left;

    if (!desc.empty()) {
      bool skipFirstLinePrefix = true;
      if (left.length() >= column_width_) {
        out << '\n';
        skipFirstLinePrefix = false;
      }
      CLI::detail::streamOutAsParagraph(out, desc, right_column_width_, std::string(column_width_, ' '),
                                        skipFirstLinePrefix);
    }

    out << '\n';
    return out.str();
  }


  std::string make_group(std::string group, bool is_positional,
                         std::vector<const CLI::Option *> opts) const override {
    std::stringstream out;

    out << "\n" << group << ":\n";
    for (const CLI::Option *opt : opts) {
      out << (is_positional ? make_positional_option(opt) : make_option(opt, false));
    }

    return out.str();
  }

};
}
