#ifndef LK_TAXSCAN
#define LK_TAXSCAN

#include <string>
#include <vector>

#include "errors.h"
#include "core_types.h"
#include "state_machine.h"

struct FileTaxonomy {
	RoutineTaxonomy routine;
	std::vector<CompilationError> errors;

	bool operator==(const FileTaxonomy& other) const;
	friend std::ostream& operator<<(std::ostream& os, const FileTaxonomy& line);
};

FileTaxonomy scan_file(const std::vector<std:: string>& lines, StateMachine& machine);
FileTaxonomy empty_file_taxonomy();

#endif