#include "SolPool.h"
#include <sstream>
#include <fstream>
#include <algorithm>
#include <iterator>
#include "FBP_Utils.h"

SolPool::SolPool(const std::size_t _max_size) : max_size(_max_size) {}

SolPool::~SolPool() {}

void SolPool::sort_pool() {
  pool.sort(FBP_Utils::SortPairByFirstItemDecreasing());
}

void SolPool::trim_to_max_size() {
  if (pool.size() > max_size) {
    pool.resize(max_size);
  }
}

void SolPool::add_solution(const std::pair<double, std::vector<std::size_t>> &obj_sol_pair) {
  std::pair<double, std::vector<std::size_t>> new_pair = obj_sol_pair;
  std::sort(new_pair.second.begin(), new_pair.second.end());

  if (pool.empty()) {
    pool.push_front(new_pair);
    return;
  }

  // Check if the new obj-sol pair has a smaller obj than the last element in the list
  if (new_pair.first < pool.back().first) {
    if (pool.size() < max_size) { // Make sure pool is less than max size
      pool.push_back(new_pair); // Add element after last element
    }
    return;
  }

  // Starting at the beginning, check if any solution with samle obj value has matching
  // markers
  for (auto itr = pool.begin(); itr != pool.end(); itr++) {
    if ((new_pair.second == (*itr).second)) { // (new_pair.first == (*itr).first) && 
      return;
    }
  }
  
  for (auto itr = pool.begin(); itr != pool.end(); itr++) {
    if (new_pair.first >= (*itr).first) {
      pool.insert(itr, new_pair); // Insert
      break;
    }
  }  

  // If pool is greater than max_size, remove last item
  if (pool.size() > max_size) {
    pool.pop_back();
  }
}

void SolPool::add_solution(const double obj, const std::vector<std::size_t> &sol) {
  const std::pair<double, std::vector<std::size_t>> new_sol = std::make_pair(obj, sol);

  add_solution(new_sol);
}

void SolPool::read_from_file(const std::string &file_name, const ExprsData &data) {
  std::string tmpStr, s;
	std::istringstream iss;
	std::ifstream input;

  // Open the file
  input.open(file_name.c_str());

  // Check if file opened
  if (!input) {
    fprintf(stderr, "ERROR - SolPool::read_from_file - Could not open file (%s)\n", file_name.c_str());		
    exit(1);
  }

  // Loop through file, reading each line
  while (std::getline(input, tmpStr)) {
    // Count number of spaces
    std::size_t ps = std::count(tmpStr.begin(), tmpStr.end(), '\t') + 1;
    std::vector<std::size_t> pat;
    pat.reserve(ps);

    // Clear and update istringstream
    iss.clear();
    iss.str(tmpStr);

    while (std::getline(iss, s, '\t')) {
      pat.push_back(std::stoul(s));
    }

    double obj = data.get_grp1_freq(pat) - data.get_grp2_freq(pat);

    pool.push_back(std::make_pair(obj, pat));
  }
  input.close();
  
  max_size = pool.size();
  sort_pool();
  trim_to_max_size();
}

void SolPool::write_to_file(const std::string &file_name) {
  FILE *output;
  if ((output = fopen(file_name.c_str(), "w")) == nullptr) {
    fprintf(stderr, "ERROR - SolPool::write_to_file - Could not open file %s\n", file_name.c_str());
    exit(1);
  }

  for (auto pat : pool) {
    for (std::size_t i = 0; i < pat.second.size()-1; ++i) {
      fprintf(output, "%lu ", pat.second[i]);
    }
    fprintf(output, "%lu\n", pat.second[pat.second.size()-1]);
  }
  fclose(output);
}

void SolPool::clear() {
  for (auto s : pool) {
    s.second.clear();
  }
  pool.clear();
}

double SolPool::get_max_obj() const {
  return pool.front().first;
}

double SolPool::get_min_obj() const {
  return pool.back().first;
}

std::size_t SolPool::size() const {
  return pool.size();
}

std::size_t SolPool::get_max_size() const {
  return max_size;
}

std::pair<double, std::vector<std::size_t>> SolPool::get_obj_sol_pair(const std::size_t idx) const {
  if (idx >= pool.size()) {
    fprintf(stderr, "ERROR - SolPool::get_obj_sol_pair - Trying to access obj_sol_pair at index %lu.", idx);
    fprintf(stderr, " Solution pool only has %lu elements.\n", pool.size());
    exit(EXIT_FAILURE);
  }

  auto itr = pool.begin();
  std::advance(itr, idx);
  return *itr;
}

std::vector<std::size_t> SolPool::get_sol(const std::size_t idx) const {
  if (idx >= pool.size()) {
    fprintf(stderr, "ERROR - SolPool::get_sol - Trying to access sol at index %lu.", idx);
    fprintf(stderr, " Solution pool only has %lu elements.\n", pool.size());
    exit(EXIT_FAILURE);
  }

  auto itr = pool.begin();
  std::advance(itr, idx);
  return (*itr).second;
}