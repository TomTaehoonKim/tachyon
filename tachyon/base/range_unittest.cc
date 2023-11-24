#include "tachyon/base/range.h"

#include <vector>

#include "gtest/gtest.h"

#include "tachyon/base/containers/container_util.h"

namespace tachyon::base {

template <typename RangeType>
class RangeTest : public testing::Test {};

using RangeTypes =
    testing::Types<Range<int, false, false>, Range<int, false, true>,
                   Range<int, true, false>, Range<int, true, true>>;
TYPED_TEST_SUITE(RangeTest, RangeTypes);

TYPED_TEST(RangeTest, Iterator) {
  using RangeType = TypeParam;

  RangeType range(1, 5);
  std::vector<int> elements =
      base::Map(range.begin(), range.end(), [](int v) { return v; });
  std::vector<int> expected;
  if constexpr (RangeType::kIsStartInclusive) {
    expected.push_back(1);
  }
  for (int i = 2; i < 5; ++i) {
    expected.push_back(i);
  }
  if constexpr (RangeType::kIsEndInclusive) {
    expected.push_back(5);
  }
  EXPECT_EQ(elements, expected);
}

TYPED_TEST(RangeTest, IsEmpty) {
  using RangeType = TypeParam;

  RangeType range(3, 3);
  if constexpr (RangeType::kIsStartInclusive && RangeType::kIsEndInclusive) {
    EXPECT_FALSE(range.IsEmpty());
  } else {
    EXPECT_TRUE(range.IsEmpty());
  }

  range = RangeType(3, 4);
  EXPECT_FALSE(range.IsEmpty());
}

TYPED_TEST(RangeTest, Contains) {
  using RangeType = TypeParam;

  RangeType range(3, 4);
  EXPECT_FALSE(range.Contains(2));
  if constexpr (RangeType::kIsStartInclusive) {
    EXPECT_TRUE(range.Contains(3));
  } else {
    EXPECT_FALSE(range.Contains(3));
  }
  if constexpr (RangeType::kIsEndInclusive) {
    EXPECT_TRUE(range.Contains(4));
  } else {
    EXPECT_FALSE(range.Contains(4));
  }
  EXPECT_FALSE(range.Contains(5));
}

TYPED_TEST(RangeTest, Intersect) {
  using RangeType = TypeParam;

  RangeType range(1, 7);
  RangeType range2(5, 10);
  RangeType expected(5, 7);
  EXPECT_EQ(range.Intersect(range2), expected);
  EXPECT_EQ(range2.Intersect(range), expected);
}

TYPED_TEST(RangeTest, GetSize) {
  using RangeType = TypeParam;

  RangeType range(3, 4);
  if constexpr (RangeType::kIsStartInclusive) {
    if constexpr (RangeType::kIsEndInclusive) {
      EXPECT_EQ(range.GetSize(), 2);
    } else {
      EXPECT_EQ(range.GetSize(), 1);
    }
  } else {
    if constexpr (RangeType::kIsEndInclusive) {
      EXPECT_EQ(range.GetSize(), 1);
    } else {
      EXPECT_EQ(range.GetSize(), 0);
    }
  }
}

}  // namespace tachyon::base
