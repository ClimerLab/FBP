#include "FBP_Controller.h"
#include "Parallel.h"
#include "FBP_Utils.h"
#include "Timer.h"
#include <algorithm>

FBP_Controller::FBP_Controller(const ConfigParser &parser) :  data(parser),
                                                              scratch_dir(parser.getString("SCRATCH_DIR")),
                                                              world_size(Parallel::getWorldSize()),
                                                              run_tag(parser.getString("RUN_TAG")),
                                                              runtime_file(run_tag + "_runtime.csv"),
                                                              obj_file(""),
                                                              sol_file(""),
                                                              ps(0),
                                                              marker_pairs(scratch_dir+"markerPairs.csv", data.get_num_bins()-1),
                                                              greedy_pool(),
                                                              use_sol_pool(parser.getBool("USE_SOLUTION_POOL")),
                                                              lb(0.0),
                                                              count_threshold(data.get_num_bins()) {
  for (std::size_t i = world_size - 1; i > 0; --i) {
    available_workers.push(i);
  }
}

FBP_Controller::~FBP_Controller() {}

void FBP_Controller::signal_workers_to_end() {
  char signal = 0;
  for (std::size_t i = 1; i < world_size; ++i) {
    MPI_Send(&signal, 1, MPI_CHAR, i, Parallel::CONVERGE_TAG, MPI_COMM_WORLD);
  }
}

bool FBP_Controller::workers_still_working() const {
  return (available_workers.size() < (world_size-1));
}

void FBP_Controller::wait_for_workers() {
  receive_completion();
}

void FBP_Controller::send_problem(const std::size_t ps, const std::size_t analyte_state, const std::vector<std::size_t> &pool) {
  // Wait until a worker is free
  if (available_workers.empty()) {
    receive_completion();
  }

  // Get worker id
  const int worker = available_workers.top();

  // Send pattern size
  MPI_Send(&ps, 1, CUSTOM_SIZE_T, worker, Parallel::SPARSE_PROB, MPI_COMM_WORLD);

  // Send solution pool threshold
  MPI_Send(&lb, 1, MPI_DOUBLE, worker, Parallel::SPARSE_PROB, MPI_COMM_WORLD);

  // Send analyte state
  MPI_Send(&analyte_state, 1, CUSTOM_SIZE_T, worker, Parallel::SPARSE_PROB, MPI_COMM_WORLD);

  // Send pool size
  const std::size_t pool_size = pool.size();
  MPI_Send(&pool_size, 1, CUSTOM_SIZE_T, worker, Parallel::SPARSE_PROB, MPI_COMM_WORLD);

  // Send pool
  MPI_Send(&pool[0], pool_size, CUSTOM_SIZE_T, worker, Parallel::SPARSE_PROB, MPI_COMM_WORLD);

  // Update workers
  available_workers.pop();
  unavailable_workers.insert(worker);
}

void FBP_Controller::receive_completion() {
  assert(available_workers.size() < world_size -1); // Cannot receive problem when no workers are working

  MPI_Status status;
  MPI_Probe(MPI_ANY_SOURCE, Parallel::SPARSE_SOL, MPI_COMM_WORLD, &status);

  std::size_t num_sol;
  std::vector<std::size_t> markers_in_sol(ps);
  double obj_value;
  bool updated_lb = false;

  // Receive number of patterns
  MPI_Recv(&num_sol, 1, CUSTOM_SIZE_T, MPI_ANY_SOURCE, Parallel::SPARSE_SOL, MPI_COMM_WORLD, &status);

  for (std::size_t i = 0; i < num_sol; ++i) {
    MPI_Recv(&obj_value, 1, MPI_DOUBLE, status.MPI_SOURCE, Parallel::SPARSE_SOL, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(&markers_in_sol[0], ps, CUSTOM_SIZE_T, status.MPI_SOURCE, Parallel::SPARSE_SOL, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    
    write_obj_to_file(obj_value);
    write_sol_to_file(markers_in_sol);

    if (!use_sol_pool) {
      lb = obj_value > lb ? obj_value : lb;
      updated_lb = true;
    }
  }

  if (updated_lb) {
    calc_threshold_from_lb();
  }
  
  // *
  // * Make the worker available again
  // *
  available_workers.push(status.MPI_SOURCE);
  unavailable_workers.erase(status.MPI_SOURCE);    
}

void FBP_Controller::set_ps(const std::size_t _ps) {
  ps = _ps; 
}

void FBP_Controller::init_lb_from_greedy() {
  std::string sol_pool_file_name = scratch_dir + "ps" + std::to_string(ps) + ".solPool";
  greedy_pool.read_from_file(sol_pool_file_name, data);

  if (use_sol_pool) {
    // lb = greedy_pool.get_max_obj() * .9;
    lb = greedy_pool.get_min_obj();
  } else {
    lb = greedy_pool.get_max_obj();
  }

  fprintf(stderr, "Using LB of %lf\n", lb);
}

void FBP_Controller::calc_threshold_from_lb() {
  count_threshold = static_cast<std::size_t>(std::ceil(lb * data.get_num_grp1()));
}

std::vector<std::size_t> FBP_Controller::update_markers_from_pair_counts() {
  std::vector<std::size_t> markers_set_to_zero;

  bool markers_updated = true;
  while (markers_updated) {
    markers_updated = false;

    for (std::size_t i = 0; i < data.get_num_bins()-1; ++i) {
      if (markers[i] != 0) {
        std::size_t num_pairs = marker_pairs.getNumPairsGteThresh(i, count_threshold, markers);
        if (num_pairs < ps-1) {
          markers[i] = 0;
          markers_set_to_zero.push_back(i);
          markers_updated = true;
        }
      }
    }
  }
  return markers_set_to_zero;
}

void FBP_Controller::update_pair_count() {
  for (std::size_t i = 0; i < pair_count.size(); ++i) {
    if (markers[i] != 0) {
      pair_count[i] = marker_pairs.getNumPairsGteThresh(i, count_threshold);
    }
  }
}

std::size_t FBP_Controller::get_next_marker() {
  std::size_t idx = data.get_num_bins();
  std::size_t best_count = data.get_num_bins();
  for (std::size_t i = 0; i < pair_count.size(); ++i) {
    if (markers[i] != 0 && (pair_count[i] >= ps-1) && (pair_count[i] < best_count)) {
      idx = i;
      best_count = pair_count[i];
    }
  }
  return idx;
}

void FBP_Controller::reset() {
  lb = 0.0;
  greedy_pool.clear();
}

void FBP_Controller::work() {
  init_lb_from_greedy();
  calc_threshold_from_lb();

  // fprintf(stderr, "G1 threshold: %u\n", count_threshold);
  
  std::string risk_str = data.get_risk() ? "Risk" : "Prot";
  obj_file = run_tag + "_PS" + std::to_string(ps) + "_" + risk_str + ".obj";
  create_new_file(obj_file);

  sol_file = run_tag +  "_PS" + std::to_string(ps) + "_" + risk_str + ".log";
  create_new_file(sol_file);

  Timer timer;
  timer.start();

  markers.clear();
  markers.resize(data.get_num_bins(), -1);
  update_markers_from_pair_counts();

  pair_count.clear();
  pair_count.resize(data.get_num_bins(), 0);
  update_pair_count();
  
  std::size_t num_valid = 0;
  for (std::size_t i = 0; i < pair_count.size(); ++i) {
    if (pair_count[i] >= ps-1) {
      ++num_valid;
    }
  }
  fprintf(stderr, "%lu states remaining\n", num_valid);

  while (true) {
    std::size_t marker_to_test = get_next_marker();

    if (marker_to_test == data.get_num_bins()) {
      break;
    }

    auto pairs = marker_pairs.getPairsGteThresh(marker_to_test, count_threshold, markers);

    if (num_valid % 100 == 0) {
      fprintf(stderr, "%lu states remaining\n", num_valid);
    }

    if (pairs.size() > ps-1) {
      send_problem(ps, marker_to_test, pairs);
    }

    markers[marker_to_test] = 0;
    --num_valid;

    pair_count[marker_to_test] = 0;
    for (auto p : pairs) {
      --pair_count[p];
      if (pair_count[p] < ps-1) {
        --num_valid;
        markers[p] = 0;
      }
    }
  }
  
  while (workers_still_working()) {
    wait_for_workers();
  }
  timer.stop();
  fprintf(stderr, "FPB PS=%lu run time: %lf\n\n\n", ps, timer.elapsed_cpu_time());
  write_runtime_to_file(timer.elapsed_cpu_time());
  reset();
}

void FBP_Controller::write_runtime_to_file(const double gs_time) const {
  FILE *output;
  if ((output = fopen(runtime_file.c_str(), "a+")) == nullptr) {
    fprintf(stderr, "ERROR - FBP_Controller::write_runtime_to_file - Could not open file %s\n", runtime_file.c_str());
    exit(1);
  }

  fprintf(output, "%lu,%lf\n", ps, gs_time);

  fclose(output);
}

void FBP_Controller::write_obj_to_file(const double obj) const {
  FILE *output;

  if ((output = fopen(obj_file.c_str(), "a+")) == nullptr) {
    fprintf(stderr, "ERROR - FBP_Controller::write_obj_to_file - Could not open file %s\n", obj_file.c_str());
    exit(1);
  }

  fprintf(output, "%lf\n", obj);

  fclose(output);
}

void FBP_Controller::write_sol_to_file(const std::vector<std::size_t> &sol) const {
  FILE *output;

  if ((output = fopen(sol_file.c_str(), "a+")) == nullptr) {
    fprintf(stderr, "ERROR - FBP_Controller::write_obj_to_file - Could not open file %s\n", sol_file.c_str());
    exit(1);
  }
  
  for (std::size_t i = 0; i < sol.size()-1; ++i) {
    fprintf(output, "%lu\t", sol[i]);
  }
  fprintf(output, "%lu\n", sol[sol.size()-1]);

  fclose(output);
}

void FBP_Controller::create_new_file(const std::string &file_name) const {
  FILE *output;

  if ((output = fopen(file_name.c_str(), "w")) == nullptr) {
    fprintf(stderr, "ERROR - FBP_Controller::write_obj_to_file - Could not open file %s\n", file_name.c_str());
    exit(1);
  }

  fclose(output);
}