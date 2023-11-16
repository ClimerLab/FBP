#ifndef FBP_CONTROLLER_H
#define FBP_CONTROLLER_H

#include <stack>
#include <set>

#include "ConfigParser.h"
#include "ExprsData.h"
#include "Pairs.h"
#include "SolPool.h"

class ConfigParser;

class FBP_Controller {
  private:
    const ExprsData data;
    const std::string scratch_dir;
    const std::size_t world_size;
    const std::string run_tag;
    const std::string runtime_file;
    std::string obj_file;
    std::string sol_file;

    std::stack<int> available_workers;
    std::set<int> unavailable_workers;

    std::size_t ps;

    Pairs marker_pairs;
    SolPool greedy_pool;
    bool use_sol_pool;

    double lb;
    unsigned int count_threshold;
    std::vector<int> markers;
    std::vector<std::size_t> pair_count;

    void init_lb_from_greedy();
    void calc_threshold_from_lb();
    std::vector<std::size_t> update_markers_from_pair_counts();
    void update_pair_count();
    std::size_t get_next_marker();
    void reset();

    void write_runtime_to_file(const double gs_time) const;
    void write_obj_to_file(const double obj) const;
    void write_sol_to_file(const std::vector<std::size_t> &sol) const;
    void create_new_file(const std::string &file_name) const;

  public:
    FBP_Controller(const ConfigParser &parser);
    ~FBP_Controller();

    bool done() const;
    void signal_workers_to_end();
    bool workers_still_working() const;
    void wait_for_workers();
    void send_problem(const std::size_t ps, const std::size_t analyte_state, const std::vector<std::size_t> &pool);
    void receive_completion();

    void set_ps(const std::size_t _ps);
    
    void work();
    

    void record_time() const;
    void record_obj(const double obj) const;

    
    std::size_t get_ps() const;
};

#endif
