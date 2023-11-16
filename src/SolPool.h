#ifndef SOL_POOL_H
#define SOL_POOL_H

#include <list>
#include <vector>
#include <string>
#include "ExprsData.h"

class SolPool {
  private:
    std::size_t max_size;

    std::list<std::pair<double, std::vector<std::size_t>>> pool;

    void sort_pool();
    void trim_to_max_size();

  public:
    SolPool(const std::size_t _max_size = 0);
    ~SolPool();

    void add_solution(const std::pair<double, std::vector<std::size_t>> &obj_sol_pair);
    void add_solution(const double obj, const std::vector<std::size_t> &sol);

    void read_from_file(const std::string &file_name, const ExprsData &data);
    void write_to_file(const std::string &file_name);

    void clear();

    double get_max_obj() const;
    double get_min_obj() const;
    std::size_t size() const;
    std::size_t get_max_size() const;
    std::pair<double, std::vector<std::size_t>> get_obj_sol_pair(const std::size_t idx) const;
    std::vector<std::size_t> get_sol(const std::size_t idx) const;
};

#endif