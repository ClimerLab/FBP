#ifndef SYNC_UTILS_H
#define SYNC_UTILS_H

#include <vector>
#include <boost/multiprecision/cpp_int.hpp>

class ExprsData;
class Pattern;

namespace FBP_Utils {
  struct SortPairByFirstItemDecreasing {
    template<typename T, typename U>
    bool operator()(const std::pair<T, U> &lhs, const std::pair<T, U> &rhs) const {
      return lhs.first > rhs.first;
    }
  };

  struct SortPairByFirstItemIncreasing {
    template<typename T, typename U>
    bool operator()(const std::pair<T, U> &lhs, const std::pair<T, U> &rhs) const {
      return lhs.first < rhs.first;
    }
  };

  struct SortPairBySecondItemDecreasing {
    template<typename T, typename U>
    bool operator()(const std::pair<T, U> &lhs, const std::pair<T, U> &rhs) const {
      return lhs.second > rhs.second;
    }
  };

  struct SortPairBySecondItemIncreasing {
    template<typename T, typename U>
    bool operator()(const std::pair<T, U> &lhs, const std::pair<T, U> &rhs) const {
      return lhs.second < rhs.second;
    }
  };
  
  void populate_pattern(const std::vector<std::size_t> &index,
                        const std::vector<std::size_t> &pool,
                        const std::size_t start_index,
                        std::vector<std::size_t> &pat);
  bool get_next_index(const std::vector<std::size_t> &pool, std::vector<std::size_t> &index);
  void init_index_from_trip_index(std::vector<std::size_t> &index, const std::vector<std::size_t> &trip_index);
  boost::multiprecision::cpp_int C(const std::size_t n, std::size_t k);
}

#endif