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

void FBP_Worker::solve() {
  double grp1_freq, obj_value;
  bool more_trips = true;

  std::vector<std::size_t> pat(ps);
  pat[0] = analyte_state;
  
  std::vector<std::size_t> index(ps-1, 0);
  for (std::size_t i = 0; i < index.size(); ++i) {
    index[i] = i;
    pat[i+1] = pool[i];
  }

  std::vector<std::size_t> trip_index = {0,1};
  std::vector<std::size_t> trip = {analyte_state, pool[0], pool[1]};

  if (3 + pool.size() - ps <= 0) {
    fprintf(stderr, "trip size non-positive\n");
  }
  std::size_t trip_pool_size = pool.size() - (ps - 3);
  std::size_t num_trips = (std::size_t)FBP_Utils::C(trip_pool_size, 2);

  std::vector<std::size_t> trip_pool(trip_pool_size);
  for (std::size_t i = 0; i < trip_pool_size; ++i) {
    trip_pool[i] = pool[i];
  }
    
  // Loop through all possible patterns of size 3
  for (std::size_t i_trip = 0; i_trip < num_trips; ++i_trip) {
    // Check if the ps=3 pattern has a grp1 frequency >= threshold
    if (data.get_grp1_freq(trip) >= threshold) {
      FBP_Utils::init_index_from_trip_index(index, trip_index);
      FBP_Utils::populate_pattern(index, pool, 1, pat);
      std::size_t num_pats_to_test = (std::size_t)FBP_Utils::C(pool.size() - trip_index[1]-1, ps-3);

      // Loop through all patterns built on the ps=3 pattern
      for (std::size_t i_pat = 0; i_pat < num_pats_to_test; ++i_pat) {
        grp1_freq = data.get_grp1_freq(pat);
        if (grp1_freq >= threshold) {
          obj_value = grp1_freq - data.get_grp2_freq(pat);
          if (obj_value >= threshold) {
            sol_pool.push_back(std::make_pair(obj_value, pat));
          }
        }

        if (FBP_Utils::get_next_index(pool, index)) {
          FBP_Utils::populate_pattern(index, pool, 1, pat);                
        }
      }
    }

    if ((more_trips = FBP_Utils::get_next_index(trip_pool, trip_index))) {
      FBP_Utils::populate_pattern(trip_index, trip_pool, 1, trip);
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