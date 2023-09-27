#include <sstream>
#include "taxscan.h"



bool FileTaxonomy::operator==(const FileTaxonomy& other) const {
    return this->routine == other.routine
        && this->errors == other.errors;
}

std::ostream& operator<<(std::ostream& os, const FileTaxonomy& file) {
    os << std::endl << "{" << std::endl;
    os << indent(1) << "routine=";
    to_stream(os, file.routine, 1);
    if (!file.errors.empty()) {
      os << indent(1) << "errors=[" << std::endl;
      for (CompilationError error : file.errors) {
        os << indent(2) << error << std::endl;
      }
      os << indent(1) << "]" << std::endl;
    }
    os << "}" << std::endl;
    return os;
}
