# FBP
Frequency Based Pruning (FBP) is a feature selection algorithm based upon maximizing the Youden J statistic. FBP intelligently enumerates through combinations of features, using the frequency of smaller patterns to prune away large regions of the solution space.

## To Use
Configure the Makefile with the locaion of open mpi libraries and binary

Compile with the Makefile by navigating to the root directory and entering: make

Update configuration file

Run the program. For an example enter: mpirun -np 4 ./fbp <cfg_file>

## Configuration
DATA_FILE - Tab seperated file where the first NUM_CASES columns are cases and the next NUM_CTRLS columns are controls. The row indicate features.

SCRATCH_DIR - Directoty where results are recorded

SOL_POOL_FILE - File File with the best and worst objective values from the solution pool of each pattern size.

RUN_TAG - Run tag appended to output file

RISK - Boolean that indicates if risk patterns (true) or protective patterns (false) should be found.

NUM_CASES - The number of cases in DATA_FILE.

NUM_CTRLS - The number of controls in DATA_FILE.

NUM_EXPRS - The number of features in DATA_FILE.

NUM_HEAD_ROWS - The number of header rows in DATA_FILE.

NUM_HEAD_COLS - The number of header columns in DATA_FILE.

PATTERN_SIZE - The number of marker states in the pattern(s) to be found.

MISSING_SYMBOL - String used to indicate missing data in DATA_FILE.

MAX_PS - Maximum size patterns to find

USE_SOLUTION_POOL - A boolean indicating if all pattens above a threshold should be found (true) or just an optimal solution.
If true, the worst objective value for each patter size from SOL_POOL_FILE is used as the lower bound.

HIGH_VALUE - Value in DATA_FILE that indicates high expression.

NORM_VALUE - Value in DATA_FILE that indicates normal expression.

LOW_VALUE - Value in DATA_FILE that indicates low expression.

SET_NA_TRUE - Boolean used to indicate if missing data is treated as both high and low.      

## Outputs
<run_tag>_runtimes.csv - File containing runtimes for each pattern size

PS#_<Risk/Prot>.log - File containing the selected feature sets of size #.

## Notes
Recommend using [sync-greedy](https://github.com/ClimerLab/sync-greedy) to generate SOL_POOL_FILE.

Requires Open MPI

DATA_FILE should be tab seperate, the columns represent individuals and the rows represent features