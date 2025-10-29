#include <memory>
#include <atomic>
#include "ad_index.h"

// Atomic snapshot for lock-free reads
std::shared_ptr<AdIndex> current_index;