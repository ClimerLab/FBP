#ifndef FBP_WORKER_H
#define FBP_WORKER_H

#include <vector>

#include "ConfigParser.h"
#include "ExprsData.h"

class FBP_Worker {
  private:
    const ExprsData data;
    const std::size_t world_rank;
    std::size_t ps;
    double threshold;
    std::size_t analyte_state;
    std::vector<std::size_t> pool;
    bool end_;
    std::vector<std::pair<double, std::vector<std::size_t>>> sol_pool;

    std::vector<std::size_t> pat;
    std::vector<std::size_t> index;
    std::size_t cur_ps;
    std::vector<std::vector<std::size_t>> g1;
    std::vector<std::size_t> g2;

    void solve();
    void receive_problem();
    void send_solution();

    void reset();
    void init_g1_g2();
    void calc_g1();
    double calc_f1() const;
    double calc_f2() const;
    bool get_next_index();
    bool get_next_pattern();
    bool add_marker();

  public:
    FBP_Worker(const ConfigParser &parser);
    ~FBP_Worker();

    bool end() const;
    void work();    
};

#endif