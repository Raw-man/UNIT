#include "export.hpp"
#include "utils.hpp"
namespace unit {

bool ExportDialogPartial(const text::Text& text, const ExportOptions& options) {
  auto stext = text::Convert(text);

  std::string txt;

  txt.reserve(stext.size() * 128);

  for (auto& str : stext) txt += u8"\u00ab" + str + u8"\u00bb\n\n";

  utl::SaveFile(
      utl::ReplaceExtensionFront(options.output_path, fs::u8path(".txt")), txt);

  return true;
}

}  // namespace unit
