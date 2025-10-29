#pragma once
#include <memory>

struct AdIndex;

// Atomic snapshot handle declared here, defined in a single .cpp
extern std::shared_ptr<AdIndex> current_index;