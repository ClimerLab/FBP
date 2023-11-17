#include "ConfigParser.h"
#include "Parallel.h"
#include "FBP_Controller.h"
#include "FBP_Worker.h"

int main(int argc, char **argv) {
  //*
  //* MPI Init
  //*
  MPI_Init(NULL, NULL);

  const int world_rank = Parallel::getWorldRank();
  const int world_size = Parallel::getWorldSize();

  try {
    // *
    // * Check for correct command line arguments
    // *
    if (world_rank == 0) {
      if (argc != 2) {
        fprintf(stderr, "Usage: %s <config file>\n", argv[0]);
        MPI_Abort(MPI_COMM_WORLD, 1);
      }
      if (world_size < 2) {
        fprintf(stderr, "world_size must be greater than 1\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
      }            
    }
    
    ConfigParser parser(argv[1]);

    switch (world_rank) {
      case 0: {
        FBP_Controller controller(parser);

        for (std::size_t PS = 3; PS <= parser.getSizeT("MAX_PS"); ++PS) {
          controller.set_ps(PS);
          controller.work();
        }

        controller.signal_workers_to_end();

        while (controller.workers_still_working()) {
          controller.wait_for_workers();
        }
        break;
      }

      default: {
        FBP_Worker worker(parser);

        while (!worker.end()) {
          worker.work();
        }      
        break;
      }
    }
  } catch (std::exception &e) {
    std::cout << "  *** Fatal error reported by rank_"
              << world_rank << ": " << e.what() << " ***" << std::endl;
    MPI_Abort(MPI_COMM_WORLD, 1);
  }
  MPI_Finalize();
}