#pragma once

#include "cache.hpp"
#include "cached_item_t.hpp"

using cache_t = lru_cache_t<std::shared_ptr<cached_item_t>>;
