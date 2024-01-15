#include "FBP_Worker.h"
#include "Parallel.h"
#include "FBP_Utils.h"

FBP_Worker::FBP_Worker(const ConfigParser &parser) :  data(parser),
                                                      world_rank(Parallel::getWorldRank()),
                                                      ps(0),
                                                      threshold(0.0),
                                                      analyte_state(0),
                                                      end_(false) {}

FBP_Worker::~FBP_Worker() {}

bool FBP_Worker::end() const {
  return end_;
}

void FBP_Worker::work() {
  receive_problem();
  if (end_) {
    return;
  }

  solve();
  send_solution();
  sol_pool.clear();
}

void FBP_Worker::reset() {
  pat.clear();
  pat.resize(ps);

  index.clear();
  index.resize(ps-1, 0);

  std::vector<std::size_t> vec;
  for (auto g : g1) {
    g.clear();
  }
  g1.clear();
  for (std::size_t i = 0; i < ps; ++i) {
    g1.push_back(vec);
  }
  
  g2.clear();
}

void FBP_Worker::init_g1_g2() {
  for (std::size_t j = data.get_grp1_start(); j <= data.get_grp1_stop(); ++j) {
    if (data.get_bin(pat[0], j)) {
      g1[0].push_back(j);
    }
  }
  for (std::size_t j = data.get_grp2_start(); j <= data.get_grp2_stop(); ++j) {
    if (data.get_bin(pat[0], j)) {
      g2.push_back(j);
    }
  }
}

void FBP_Worker::calc_g1() {
  if (cur_ps == 0) {
    fprintf(stderr, "ERROR - FBP_Worker::calc_g1 - trying to access vector at index -1\n");
    exit(EXIT_FAILURE);
  }

  g1[cur_ps].clear();

  for (auto j : g1[cur_ps-1]) {
    if (data.get_bin(pat[cur_ps], j)) {
      g1[cur_ps].push_back(j);
    }
  }
}

double FBP_Worker::calc_f1() const {
  return static_cast<double>(g1[cur_ps].size()) / data.get_num_grp1();
}

double FBP_Worker::calc_f2() const {
  double count = g2.size();
  for (auto j : g2) {
    for (std::size_t idx = 1; idx < ps; ++idx) {
      std::size_t i = pat[idx];
      if (!data.get_bin(i,j)) {
        --count;
        break;
      }
    }
  }
  return count / data.get_num_grp2();
}

bool FBP_Worker::get_next_index() {
  if (cur_ps == 0) {
    fprintf(stderr, "ERROR - FBP_Worker::get_next_index - Trying to access vector at index -1\n");
    return false;
  }

  std::size_t i = cur_ps-1;
  std::size_t max = pool.size() - ps + 1 + i;
  ++index[i];

  while (index[i] > max) {
    if (i == 0) {
      return false;
    }

    --cur_ps;
    --i;
    ++index[i];
    max = pool.size() - ps + 1 + i;
  }
  
  return true;
}

bool FBP_Worker::get_next_pattern() {
  if (!get_next_index()) {
    return false;
  }
  for (std::size_t i = 1; i <= cur_ps; ++i) {
    pat[i] = pool[index[i-1]];
  }
  return true;
}

bool FBP_Worker::add_marker() {
  if (cur_ps == 0) {
    fprintf(stderr, "ERROR - cur_ps needs to be > 0\n");
    exit(1);
  }
  ++cur_ps;
  std::size_t i = cur_ps-1;
  std::size_t max = pool.size() - ps + 1 + i;
  
  index[i] = index[i-1] + 1;
  if (index[i] > max) {
    --cur_ps;
    
    if(!get_next_index()) {
      return false;
    }
  }

  pat[cur_ps] = pool[index[cur_ps-1]];
  return true;
}

void FBP_Worker::solve() {
  double f1, obj_value;

  reset();

  pat[0] = analyte_state;
  pat[1] = pool[0];
  init_g1_g2();  

  cur_ps = 1;
  bool valid_pattern = true;
  while (valid_pattern) {
    calc_g1();
    f1 = calc_f1();

    if (f1 < threshold) { // Go to next pattern
      valid_pattern = get_next_pattern();
    } else if (cur_ps == ps-1) { // Test pattern
      obj_value = f1 - calc_f2();
      if (obj_value >= threshold) {
        sol_pool.push_back(std::make_pair(obj_value, pat));
      }
      valid_pattern = get_next_pattern();
    } else { // Add marker
      valid_pattern = add_marker();
    }
  }
}

// This function recieves the problem from the conroller, and saves the values to local variables
void FBP_Worker::receive_problem() {
  std::size_t pool_size;
  MPI_Status status;

  // Check if received a signal to end    
  MPI_Probe(0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
  if (status.MPI_TAG == Parallel::CONVERGE_TAG) {
    char signal;
    MPI_Recv(&signal, 1, MPI_CHAR, 0, Parallel::CONVERGE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    end_ = true;
    return;
  }
  
  // Receive pattern size
  MPI_Recv(&ps, 1, CUSTOM_SIZE_T, 0, Parallel::SPARSE_PROB, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  
  // Receive solution pool threshold
  MPI_Recv(&threshold, 1, MPI_DOUBLE, 0, Parallel::SPARSE_PROB, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

  // Receive analyte state
  MPI_Recv(&analyte_state, 1, CUSTOM_SIZE_T, 0, Parallel::SPARSE_PROB, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

  // Receive pool size
  MPI_Recv(&pool_size, 1, CUSTOM_SIZE_T, 0, Parallel::SPARSE_PROB, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  pool.resize(pool_size);
      
  // Receive pool
  MPI_Recv(&pool[0], pool_size, CUSTOM_SIZE_T, 0, Parallel::SPARSE_PROB, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
}

// This function sends any solutions back to the controller.
void FBP_Worker::send_solution() {
  // Send number of solutions in pool
  const std::size_t num_sol = sol_pool.size();
  MPI_Send(&num_sol, 1, CUSTOM_SIZE_T, 0, Parallel::SPARSE_SOL, MPI_COMM_WORLD);

  // Send each solution
  for (std::size_t i = 0; i < num_sol; ++i) {
    MPI_Send(&sol_pool[i].first, 1, MPI_DOUBLE, 0, Parallel::SPARSE_SOL, MPI_COMM_WORLD);
    MPI_Send(&sol_pool[i].second[0], ps, CUSTOM_SIZE_T, 0, Parallel::SPARSE_SOL, MPI_COMM_WORLD);    
  }
}