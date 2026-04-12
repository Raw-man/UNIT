#pragma once

#include <CLI/Formatter.hpp>

namespace unit {

class Formatter : public CLI::Formatter {


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

  std::string get_type_name(const CLI::Option* opt) const {
    std::string full_type_name = opt->get_type_name();

    std::size_t multi_type = full_type_name.find(":",1);

    if(multi_type != std::string::npos && multi_type+1 != full_type_name.size()){
      full_type_name = full_type_name.substr(multi_type);
    }

    return full_type_name;
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

  // std::string make_option_opts(const CLI::Option *opt)  const override{
  //   std::stringstream out;
  //   // Help output should be stable across runs, so sort pointer-based sets by option name before printing.
  //   const auto print_option_set = [&out](const std::set<CLI::Option *> &options) {
  //     std::vector<const CLI::Option *> sorted(options.begin(), options.end());
  //     std::sort(sorted.begin(), sorted.end(), [](const CLI::Option *lhs, const CLI::Option *rhs) {
  //       return lhs->get_name() < rhs->get_name();
  //     });
  //     for(const CLI::Option *op : sorted)
  //       out << " " << op->get_name();
  //   };

  //   if(!opt->get_option_text().empty()) {
  //     out << " " << opt->get_option_text();
  //   } else {
  //     if(opt->get_type_size() != 0) {
  //       if(enable_option_type_names_) {
  //         std::string type_name = get_type_name(opt);

  //         if(!type_name.empty())
  //           out << " " << type_name;
  //       }
  //       if(enable_option_defaults_) {
  //         if(!opt->get_default_str().empty())
  //           out << " [" << opt->get_default_str() << "] ";
  //       }
  //       if(opt->get_expected_max() == CLI::detail::expected_max_vector_size)
  //         out << " ...";
  //       else if(opt->get_expected_min() > 1)
  //         out << " x " << opt->get_expected();

  //       if(opt->get_required())
  //         out << " " << get_label("REQUIRED");
  //     }
  //     if(!opt->get_envname().empty())
  //       out << " (" << get_label("Env") << ":" << opt->get_envname() << ")";
  //     if(!opt->get_needs().empty()) {
  //       out << " " << get_label("Needs") << ":";
  //       print_option_set(opt->get_needs());
  //     }
  //     if(!opt->get_excludes().empty()) {
  //       out << " " << get_label("Excludes") << ":";
  //       print_option_set(opt->get_excludes());
  //     }
  //   }
  //   return out.str();
  // }
};
}
