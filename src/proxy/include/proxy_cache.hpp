#pragma once

#include "cache.hpp"
#include "cached_item_t.hpp"

using cache_t = lru_cache_t<cached_item_t>;
