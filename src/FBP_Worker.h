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

    void solve();
    void receive_problem();
    void send_solution();

  public:
    FBP_Worker(const ConfigParser &parser);
    ~FBP_Worker();

    bool end() const;
    void work();    
};

#endif