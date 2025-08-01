#include <unistd.h>

#include <spdlog/spdlog.h>

#include <gtest/gtest.h>
//
//
// TEST(CacheTest, UsageTest) {
//     // spdlog::set_level(spdlog::level::trace);
//     lru_cache_t<int> cache(5, 100);
//     for (int i = 0; i < 5; ++i) {
//         cache.set(std::to_string(i), i);
//     }
//     for (int i = 0; i < 5; ++i) {
//         EXPECT_EQ(cache.get(std::to_string(i)), i);
//     }
//     cache.set("0", 0);
//     for (int i = 0; i < 5; ++i) {
//         EXPECT_EQ(cache.get(std::to_string(i)), i);
//     }
//     cache.set("0", 10);
//     EXPECT_EQ(cache.get("0"), 10);
//     EXPECT_EQ(cache.get("100"), std::nullopt);
// }
//
// TEST(CacheTest, DeletePoliticTest) {
//     lru_cache_t<int> cache(5);
//     for (int i = 0; i < 5; ++i) {
//         cache.set(std::to_string(i), i);
//     }
//     cache.set("test", 100);
//     EXPECT_EQ(cache.get("0"), std::nullopt);
//     cache.get("1");
//     // так как ранее прочли "1" то удалится не "1" а "2"
//     cache.set("test1", 100);
//     EXPECT_EQ(cache.get("2"), std::nullopt);
//     EXPECT_EQ(cache.get("1"), 1);
// }
//
// TEST(CacheTest, TimeToLiveTest) {
//     lru_cache_t<int> cache(2, 2);
//     for (int i = 0; i < 2; ++i) {
//         cache.set(std::to_string(i), i);
//         sleep(1);
//     }
//     EXPECT_EQ(cache.get("1"), 1);
//     EXPECT_EQ(cache.get("0"), std::nullopt);
// }
