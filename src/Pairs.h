#ifndef PAIRS_H
#define PAIRS_H

#include <vector>
#include <string>

class Pairs {
  private:
    std::size_t size;
    std::vector<std::vector<unsigned int>> values;

    void clearValues();

  public:
    Pairs();
    Pairs(const std::string &filename, const std::size_t _size);
    ~Pairs();

    void set_size(const std::size_t _size);
    void read(const std::string &filename);
    void print() const;

    std::vector<std::size_t> getPairsGteThresh(const std::size_t idx, const unsigned int threshold) const;
    std::vector<std::size_t> getPairsGteThresh(const std::size_t idx, const unsigned int threshold, const std::vector<bool> valid) const;
    std::vector<std::size_t> getPairsGteThresh(const std::size_t idx, const unsigned int threshold, const std::vector<int> valid) const;

    std::vector<std::size_t> get_pool(const std::size_t idx, const unsigned int threshold, const std::vector<int> valid) const;

    std::vector<std::size_t> getPairsLtThresh(const std::size_t idx, const unsigned int threshold) const;
    std::vector<std::size_t> getPairsLtThresh(const std::size_t idx, const unsigned int threshold, const std::vector<bool> valid) const;
    std::vector<std::size_t> getPairsLtThresh(const std::size_t idx, const unsigned int threshold, const std::vector<int> valid) const;

    std::size_t getNumPairsGteThresh(const std::size_t idx, const unsigned int threshold) const;
    std::size_t getNumPairsGteThresh(const std::size_t idx, const unsigned int threshold, const std::vector<bool> valid) const;
    std::size_t getNumPairsGteThresh(const std::size_t idx, const unsigned int threshold, const std::vector<int> valid) const;

    std::size_t getNumPairsLtThresh(const std::size_t idx, const unsigned int threshold) const;
    std::size_t getNumPairsLtThresh(const std::size_t idx, const unsigned int threshold, const std::vector<bool> valid) const;
    std::size_t getNumPairsLtThresh(const std::size_t idx, const unsigned int threshold, const std::vector<int> valid) const;
  
    std::size_t get_size() const;

    std::vector<unsigned int> get_values(const std::size_t idx) const;
};

#endif