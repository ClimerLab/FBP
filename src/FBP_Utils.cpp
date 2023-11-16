#include "FBP_Utils.h"
#include "ExprsData.h"

void FBP_Utils::populate_pattern(const std::vector<std::size_t> &index,
                      const std::vector<std::size_t> &pool,
                      const std::size_t start_index,
                      std::vector<std::size_t> &pat) {
  if (index[index.size()-1] >= pool.size()) {
    fprintf(stderr, "Last index: %lu - pool size: %lu\n", index[index.size()-1], pool.size());
    fprintf(stderr, "Index: ");
    for (std::size_t i = 0; i < index.size(); ++i) {
      fprintf(stderr, " %lu", index[i]);
    }
    fprintf(stderr, "\n");
  }
  if (index.size() + start_index > pat.size()) {
    fprintf(stderr, "Index size: %lu - start index: %lu - pat len: %lu\n", index.size(), start_index, pat.size());
  }

  assert(index[index.size()-1] < pool.size());
  assert(index.size() + start_index <= pat.size());

  for (std::size_t i = 0; i < index.size(); ++i) {
    pat[i+start_index] = pool[index[i]];
  }
}

bool FBP_Utils::get_next_index(const std::vector<std::size_t> &pool,
                               std::vector<std::size_t> &index) {
  std::size_t i = index.size()-1;
  std::size_t max = pool.size()-1;
  std::size_t roll_over_index = index.size();

  ++index[i];

  while (index[i] > max) {
    if (i == 0) {
      return false;
    }
    roll_over_index = i;
    --i;
    --max;
    ++index[i];
  }

  i = roll_over_index;
  while ( i < index.size()) {
    index[i] = index[i-1] + 1;
    ++i;
  }
    
  return true;
}

void FBP_Utils::init_index_from_trip_index(std::vector<std::size_t> &index,
                                           const std::vector<std::size_t> &trip_index) {
  for (std::size_t i = 0; i < trip_index.size(); ++i) {
    index[i] = trip_index[i];
  }
  for (std::size_t i = trip_index.size(); i < index.size(); ++i) {
    index[i] = index[i-1] +1;
  }
}

boost::multiprecision::cpp_int FBP_Utils::C(const std::size_t n, std::size_t k) {
  if (k > n)  {
    return 0;
  }    

  if (k > n - k){ // C(n, k) = C(n, n-k)
    k = n - k;
  }

  boost::multiprecision::cpp_int result = 1;

  for (std::size_t i = 0; i < k; ++i) {
    result *= (n - i);
    result /= (i + 1);
  }

  return result;
}