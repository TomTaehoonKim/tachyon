#ifndef TACHYON_BASE_CONTAINERS_CONTAINER_UTIL_H_
#define TACHYON_BASE_CONTAINERS_CONTAINER_UTIL_H_

#include <algorithm>
#include <iterator>
#include <utility>
#include <vector>

#include "tachyon/base/functional/functor_traits.h"
#include "tachyon/base/logging.h"
#include "tachyon/base/numerics/checked_math.h"

namespace tachyon::base {

template <typename T>
std::vector<T> CreateRangedVector(T start, T end, T step = 1) {
  CHECK_LT(start, end);
  CHECK_GT(step, T{0});
  std::vector<T> ret;
  size_t size = static_cast<size_t>((end - start + step - 1) / step);
  ret.reserve(size);
  T v = start;
  std::generate_n(std::back_inserter(ret), size, [&v, step]() {
    T ret = v;
    v += step;
    return ret;
  });
  return ret;
}

template <typename Generator,
          typename FunctorTraits = internal::MakeFunctorTraits<Generator>,
          typename RunType = typename FunctorTraits::RunType,
          typename ReturnType = typename FunctorTraits::ReturnType,
          typename ArgList = internal::ExtractArgs<RunType>,
          size_t ArgNum = internal::GetSize<ArgList>,
          std::enable_if_t<ArgNum == 0>* = nullptr>
std::vector<ReturnType> CreateVector(size_t size, Generator&& generator) {
  std::vector<ReturnType> ret;
  ret.reserve(size);
  std::generate_n(std::back_inserter(ret), size,
                  std::forward<Generator>(generator));
  return ret;
}

template <typename Generator,
          typename FunctorTraits = internal::MakeFunctorTraits<Generator>,
          typename RunType = typename FunctorTraits::RunType,
          typename ReturnType = typename FunctorTraits::ReturnType,
          typename ArgList = internal::ExtractArgs<RunType>,
          size_t ArgNum = internal::GetSize<ArgList>,
          std::enable_if_t<ArgNum == 1>* = nullptr>
std::vector<ReturnType> CreateVector(size_t size, Generator&& generator) {
  std::vector<ReturnType> ret;
  ret.reserve(size);
  size_t idx = 0;
  std::generate_n(
      std::back_inserter(ret), size,
      [&idx, generator = std::forward<Generator>(generator)]() mutable {
        return generator(idx++);
      });
  return ret;
}

template <typename T,
          std::enable_if_t<!internal::IsCallableObject<T>::value>* = nullptr>
std::vector<T> CreateVector(size_t size, const T& initial_value) {
  std::vector<T> ret;
  ret.resize(size, initial_value);
  return ret;
}

template <typename InputIterator, typename UnaryOp,
          typename FunctorTraits = internal::MakeFunctorTraits<UnaryOp>,
          typename ReturnType = typename FunctorTraits::ReturnType>
std::vector<ReturnType> Map(InputIterator begin, InputIterator end,
                            UnaryOp&& op) {
  std::vector<ReturnType> ret;
  ret.reserve(std::distance(begin, end));
  std::transform(begin, end, std::back_inserter(ret),
                 std::forward<UnaryOp>(op));
  return ret;
}

template <typename Container, typename UnaryOp>
auto Map(Container&& container, UnaryOp&& op) {
  return Map(std::begin(container), std::end(container),
             std::forward<UnaryOp>(op));
}

template <typename InputIterator, typename UnaryOp,
          typename FunctorTraits = internal::MakeFunctorTraits<UnaryOp>,
          typename ReturnType = typename FunctorTraits::ReturnType::value_type>
std::vector<ReturnType> FlatMap(InputIterator begin, InputIterator end,
                                UnaryOp&& op) {
  std::vector<std::vector<ReturnType>> tmp;
  tmp.reserve(std::distance(begin, end));
  std::transform(begin, end, std::back_inserter(tmp),
                 std::forward<UnaryOp>(op));

  base::CheckedNumeric<size_t> size = 0;
  for (size_t i = 0; i < tmp.size(); ++i) {
    size += tmp[i].size();
  }

  std::vector<ReturnType> ret;
  ret.reserve(size.ValueOrDie());
  std::for_each(tmp.begin(), tmp.end(), [&ret](std::vector<ReturnType>& vec) {
    ret.insert(ret.end(), std::make_move_iterator(vec.begin()),
               std::make_move_iterator(vec.end()));
  });
  return ret;
}

template <typename Container, typename UnaryOp>
auto FlatMap(Container&& container, UnaryOp&& op) {
  return FlatMap(std::begin(container), std::end(container),
                 std::forward<UnaryOp>(op));
}

}  // namespace tachyon::base

#endif  // TACHYON_BASE_CONTAINERS_CONTAINER_UTIL_H_
