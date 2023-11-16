#include "Pairs.h"
#include <sstream>
#include <fstream>
#include <algorithm>
#include <assert.h>

Pairs::Pairs() : size(0) {}

Pairs::Pairs(const std::string &filename, const std::size_t _size) :  size(_size) {
  read(filename);
}

Pairs::~Pairs() {}

void Pairs::clearValues() {
  for (auto v : values) {
    v.clear();
  }
  values.clear();
}

void Pairs::set_size(const std::size_t _size) {
  size = _size;
}

void Pairs::read(const std::string &filename) {
  assert(size > 0);
  
  std::string tmpStr, s;
	std::istringstream iss;
	std::ifstream input;

  clearValues();
  std::vector<unsigned int> valuesRow;
  for (std::size_t i = 0; i < size; ++i) {
    values.push_back(valuesRow);
  }

	// Open the file
	input.open(filename.c_str());

	// Check if file opened
	if (!input) {
    fprintf(stderr, "ERROR - Could not open file (%s)\n", filename.c_str());		
		exit(1);
	}

  // Loop through file, reading each line
  while (std::getline(input, tmpStr)) {
    // Count number of comas
    std::size_t numPairs = std::count(tmpStr.begin(), tmpStr.end(), ',') + 1;
    // Clear and update istringstream
    iss.clear();
    iss.str(tmpStr);

    while (std::getline(iss, s, ',')) {
      values[size-numPairs].push_back(std::stoi(s));
    }
  }

  input.close();
}

void Pairs::print() const{
  for (auto vec : values) {
    for (auto v : vec) {
      fprintf(stderr, "%u ", v);
    }
    fprintf(stderr, "\n");
  }
}

std::vector<std::size_t> Pairs::getPairsGteThresh(const std::size_t idx, const unsigned int threshold) const {
  assert(idx < values.size());
  std::vector<std::size_t> pairs;

  for (std::size_t i = 0; i < idx; ++i) {
    if (values[i][idx-i-1] >= threshold) {
      pairs.push_back(i);
    }
  }

  for (std::size_t j = 0; j < values[idx].size(); ++j) {
    if (values[idx][j] >= threshold) {
      pairs.push_back(idx + 1 + j);
    }
  }
  return pairs;
}

std::vector<std::size_t> Pairs::getPairsGteThresh(const std::size_t idx, const unsigned int threshold, const std::vector<bool> valid) const {
    assert(idx < values.size());
  
  std::vector<std::size_t> pairs;

  for (std::size_t i = 0; i < idx; ++i) {
    if (valid[i] && values[i][idx-i-1] >= threshold) {
      pairs.push_back(i);
    }
  }

  if (idx < values.size()) {
    for (std::size_t j = 0; j < values[idx].size(); ++j) {
      if (valid[idx+1+j] && values[idx][j] >= threshold) {
        pairs.push_back(idx + 1 + j);
      }
    }
  }
  return pairs;
}

std::vector<std::size_t> Pairs::getPairsGteThresh(const std::size_t idx, const unsigned int threshold, const std::vector<int> valid) const {
  assert(idx < values.size());
  
  std::vector<std::size_t> pairs;

  for (std::size_t i = 0; i < idx; ++i) {
    if (valid[i] != 0 && values[i][idx-i-1] >= threshold) {
      pairs.push_back(i);
    }
  }

  if (idx < values.size()) {
    for (std::size_t j = 0; j < values[idx].size(); ++j) {
      if (valid[idx+1+j] != 0 && values[idx][j] >= threshold) {
        pairs.push_back(idx + 1 + j);
      }
    }
  }
  return pairs;
}

std::vector<std::size_t> Pairs::get_pool(const std::size_t idx, const unsigned int threshold, const std::vector<int> valid) const {
  assert(idx < values.size());
  fprintf(stderr, "Creating pool for %lu\n", idx);
  std::vector<std::size_t> pool;
  if (idx < values.size()) {
    for (std::size_t j = 0; j < values[idx].size(); ++j) {
      if (valid[idx+1+j] != 0 && values[idx][j] >= threshold) {
        fprintf(stderr, "Adding %lu to pool\n", idx+1+j);
        pool.push_back(idx + 1 + j);
      }
    }
  }
  return pool;
}

std::vector<std::size_t> Pairs::getPairsLtThresh(const std::size_t idx, const unsigned int threshold) const {
  assert(idx < values.size());
  std::vector<std::size_t> pairs;

  for (std::size_t i = 0; i < idx; ++i) {
    if (values[i][idx-i-1] < threshold) {
      pairs.push_back(i);
    }
  }

  for (std::size_t j = 0; j < values[idx].size(); ++j) {
    if (values[idx][j] < threshold) {
      pairs.push_back(idx + 1 + j);
    }
  }
  return pairs;
}

std::vector<std::size_t> Pairs::getPairsLtThresh(const std::size_t idx, const unsigned int threshold, const std::vector<bool> valid) const {
  assert(idx < values.size());
  
  std::vector<std::size_t> pairs;

  for (std::size_t i = 0; i < idx; ++i) {
    if (valid[i] && values[i][idx-i-1] < threshold) {
      pairs.push_back(i);
    }
  }

  if (idx < values.size()) {
    for (std::size_t j = 0; j < values[idx].size(); ++j) {
      if (valid[idx+1+j] && values[idx][j] < threshold) {
        pairs.push_back(idx + 1 + j);
      }
    }
  }
  return pairs;
}

std::vector<std::size_t> Pairs::getPairsLtThresh(const std::size_t idx, const unsigned int threshold, const std::vector<int> valid) const {
    assert(idx < values.size());
  
  std::vector<std::size_t> pairs;

  for (std::size_t i = 0; i < idx; ++i) {
    if ((valid[i] != 0) && (values[i][idx-i-1] < threshold)) {
      pairs.push_back(i);
    }
  }

  if (idx < values.size()) {
    for (std::size_t j = 0; j < values[idx].size(); ++j) {
      if ((valid[idx+1+j] != 0) && (values[idx][j] < threshold)) {
        pairs.push_back(idx + 1 + j);
      }
    }
  }
  return pairs;
}

std::size_t Pairs::getNumPairsGteThresh(const std::size_t idx, const unsigned int threshold) const {
  if (idx >= size) {
    fprintf(stderr, "Trying to access pair outside of bounds.\n");
    fprintf(stderr, "Trying to access pairs for idx %lu, but only %lu exists.\n", idx, size);
    exit(1);
  }

  std::size_t count = 0;

  for (std::size_t i = 0; i < idx; ++i) {
    if (values[i][idx-i-1] >= threshold) {
      ++count;
    }
  }

  if (idx < values.size()) {
    for (std::size_t j = 0; j < values[idx].size(); ++j) {
      if (values[idx][j] >= threshold) {
        ++count;
      }
    }
  }
  return count;
}

std::size_t Pairs::getNumPairsGteThresh(const std::size_t idx, const unsigned int threshold, const std::vector<bool> valid) const {  
  assert(idx < valid.size());

  if (!valid[idx]) {
    return 0;
  }

  std::size_t count = 0;

  for (std::size_t i = 0; i < idx; ++i) {
    if (valid[i] && values[i][idx-i-1] >= threshold) {
      ++count;
    }
  }

  if (idx < values.size()) {
    for (std::size_t j = 0; j < values[idx].size(); ++j) {
      if (valid[idx + 1 + j] && values[idx][j] >= threshold) {
        ++count;
      }
    }
  }
  
  return count;
}

std::size_t Pairs::getNumPairsGteThresh(const std::size_t idx, const unsigned int threshold, const std::vector<int> valid) const {
  assert(idx < valid.size());

  if (valid[idx] == 0) {
    return 0;
  }

  std::size_t count = 0;

  for (std::size_t i = 0; i < idx; ++i) {
    if (valid[i] != 0 && values[i][idx-i-1] >= threshold) {
      ++count;
    }
  }

  if (idx < values.size()) {
    for (std::size_t j = 0; j < values[idx].size(); ++j) {
      if (valid[idx + 1 + j] != 0 && values[idx][j] >= threshold) {
        ++count;
      }
    }
  }
  
  return count;
}

std::size_t Pairs::getNumPairsLtThresh(const std::size_t idx, const unsigned int threshold) const {
  if (idx >= size) {
    fprintf(stderr, "Trying to access pair outside of bounds.\n");
    fprintf(stderr, "Trying to access pairs for idx %lu, but only %lu exists.\n", idx, size);
    exit(1);
  }

  std::size_t count = 0;

  for (std::size_t i = 0; i < idx; ++i) {
    if (values[i][idx-i-1] < threshold) {
      ++count;
    }
  }

  if (idx < values.size()) {
    for (std::size_t j = 0; j < values[idx].size(); ++j) {
      if (values[idx][j] < threshold) {
        ++count;
      }
    }
  }
  return count;
}

std::size_t Pairs::getNumPairsLtThresh(const std::size_t idx, const unsigned int threshold, const std::vector<bool> valid) const {
  assert(idx < valid.size());

  if (!valid[idx]) {
    return 0;
  }

  std::size_t count = 0;

  for (std::size_t i = 0; i < idx; ++i) {
    if (valid[i] && values[i][idx-i-1] < threshold) {
      ++count;
    }
  }

  if (idx < values.size()) {
    for (std::size_t j = 0; j < values[idx].size(); ++j) {
      if (valid[idx + 1 + j] && values[idx][j] < threshold) {
        ++count;
      }
    }
  }
  
  return count;
}

std::size_t Pairs::getNumPairsLtThresh(const std::size_t idx, const unsigned int threshold, const std::vector<int> valid) const {
  assert(idx < valid.size());

  if (valid[idx] == 0) {
    return 0;
  }

  std::size_t count = 0;

  for (std::size_t i = 0; i < idx; ++i) {
    if (valid[i] != 0 && values[i][idx-i-1] < threshold) {
      ++count;
    }
  }

  if (idx < values.size()) {
    for (std::size_t j = 0; j < values[idx].size(); ++j) {
      if (valid[idx + 1 + j] != 0 && values[idx][j] < threshold) {
        ++count;
      }
    }
  }
  
  return count;
}

std::size_t Pairs::get_size() const {
  return size;
}

std::vector<unsigned int> Pairs::get_values(const std::size_t idx) const {
  std::vector<unsigned int> pairs;

  for (std::size_t i = 0; i < idx; ++i) {
    pairs.push_back(values[i][idx-i-1]);
  }

  if (idx < values.size()) {
    for (std::size_t j = 0; j < values[idx].size(); ++j) {
      pairs.push_back(values[idx][j]);
    }
  }
  return pairs;
}

