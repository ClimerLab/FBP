#include <assert.h>
#include "ExprsData.h"
#include "ConfigParser.h"

const std::size_t STRSIZE = 50;

ExprsData::ExprsData(const ConfigParser &parser) :  num_cases(parser.getSizeT("NUM_CASES")),
                                                    num_ctrls(parser.getSizeT("NUM_CTRLS")),
                                                    num_analytes(parser.getSizeT("NUM_EXPRS")),
                                                    num_header_rows(parser.getSizeT("NUM_HEAD_ROWS")),
                                                    num_header_cols(parser.getSizeT("NUM_HEAD_COLS")),
                                                    risk(parser.getBool("RISK")),
                                                    grp1_total(risk ? num_cases : num_ctrls),
                                                    grp1_start(risk ? 0 : num_cases),
                                                    grp1_stop(risk ? num_cases - 1 : num_cases + num_ctrls -1),
                                                    grp2_total(risk ? num_ctrls : num_cases),
                                                    grp2_start(risk ? num_cases: 0),
                                                    grp2_stop(risk ? num_cases + num_ctrls -1 : num_cases -1),
                                                    num_bins_orig(num_analytes * 2),
                                                    data_file(parser.getString("DATA_FILE")),
                                                    MISSING_SYMBOL(parser.getString("MISSING_SYMBOL")),
                                                    SET_NA_TRUE(parser.getBool("SET_NA_TRUE")),
                                                    HIGH_BIN(parser.getString("HIGH_VALUE")),
                                                    NORM_BIN(parser.getString("NORM_VALUE")),
                                                    LOW_BIN(parser.getString("LOW_VALUE")) {
  reduced_to_orig.resize(num_bins_orig);
  for (std::size_t i = 0; i < num_bins_orig; ++i) {
    reduced_to_orig[i] = i;
  }
  read_bin_data();    
}

ExprsData::~ExprsData() {}

void ExprsData::read_bin_data() {
  FILE *input;
  if ((input = fopen(data_file.c_str(), "r")) == nullptr) {
    perror("ExprsData::fopen");
    exit(EXIT_FAILURE);
  }

  char strng[STRSIZE];
  analyte_names.resize(num_analytes);
  std::vector<bool> data_row(num_cases+num_ctrls, false);
  for (std::size_t i = 0; i < num_bins_orig; ++i) {
    bin_data.push_back(data_row);
  }

	// read in header rows
	// read in first header row and disregard
	for (std::size_t j = 0; j < num_header_cols; ++j) {
		if (fscanf(input, "%s", strng) <= 0) {
      fprintf(stderr, "ERROR: Could not find all header columns (%s)\n", data_file.c_str());
      exit(EXIT_FAILURE);
    }
	}

  // Read in individuals
	for (std::size_t j = 0; j < num_cases + num_ctrls; ++j) {
    if (fscanf(input, "%s", strng) <= 0) {
      fprintf(stderr, "ERROR: Could not find all individuals (%s)\n", data_file.c_str());
      exit(EXIT_FAILURE);
    }
	}

	// read in rest of header rows and disregard
	for (std::size_t i = 1; i < num_header_rows; ++i) {
		for (std::size_t j = 0; j < num_header_cols + num_cases + num_ctrls; ++j) {
      if (fscanf(input, "%s", strng) <= 0) {
        fprintf(stderr, "ERROR: Could not find all header rows\n (%s)", data_file.c_str());
        exit(EXIT_FAILURE);
      }
    }
	}

	// read in data
	for (std::size_t i = 0; i < num_analytes; ++i) {
    // Read in header rows
		for (std::size_t j = 0; j < num_header_cols; ++j) {
			if (fscanf(input, "%s", strng) > 0) {
        if (j == 0)
          analyte_names[i] = strng;
      } else {
        fprintf(stderr, "ERROR: Could not find all analyte names\n (%s)", data_file.c_str());
        exit(EXIT_FAILURE);
      }
		}

		for (std::size_t j = 0; j < num_cases + num_ctrls; ++j) {
			if (feof(input)) {
        fprintf(stderr, "ERROR\n");
        exit(1);
      }
		
			if (fscanf(input, "%s", strng) > 0) {
        if (MISSING_SYMBOL.compare(strng) == 0) {
          if (SET_NA_TRUE) {
            bin_data[2*i][j] = true;
            bin_data[2*i+1][j] = true;
          }                    
        } else if (HIGH_BIN.compare(strng) == 0) {
          bin_data[2*i][j] = true;
        } else if (LOW_BIN.compare(strng) == 0) {
          bin_data[2*i+1][j] = true;
        } else if (NORM_BIN.compare(strng) != 0) {
          fprintf(stderr, "ERROR: Unknown data type '%s' (%s)\n", strng, data_file.c_str());
          exit(EXIT_FAILURE);
        }                
      } else {
        fprintf(stderr, "ERROR: Input file is missing data (%s)\n", data_file.c_str());
        exit(EXIT_FAILURE);
      }            
		}
  }
  fclose(input);
}

const char* ExprsData::get_analyte_name(const std::size_t index) const {
  assert(index < analyte_names.size());
  return analyte_names[index].c_str();
}

std::size_t ExprsData::get_num_grp1() const {
  return grp1_total;
}

std::size_t ExprsData::get_num_grp2() const {
  return grp2_total;
}

std::size_t ExprsData::get_num_bins() const {
  return bin_data.size();
}

bool ExprsData::get_risk() const {
  return risk;
}

std::size_t ExprsData::get_grp1_start() const {
  return grp1_start;
}

std::size_t ExprsData::get_grp1_stop() const {
  return grp1_stop;
}

std::size_t ExprsData::get_grp2_start() const {
  return grp2_start;
}

std::size_t ExprsData::get_grp2_stop() const {
  return grp2_stop;
}

bool ExprsData::get_bin(const std::size_t i, const std::size_t j) const {
  assert(i < bin_data.size());
  assert(j < num_cases + num_ctrls);
  return bin_data[i][j];
}

void ExprsData::print_bin_data(const std::string &file_name) const {
  FILE *output;

  if ((output = fopen(file_name.c_str(), "w")) == nullptr) {
    perror("ExprsData::print_bin_data - fopen");
    exit(EXIT_FAILURE);
  }

  for (std::size_t i = 0; i < bin_data.size(); ++i) {
    for (std::size_t j = 0; j < num_cases + num_ctrls; ++j) {
      fprintf(output, bin_data[i][j] ? "1 " : "0 ");
    }
    fprintf(output, "\n");
  }

  fclose(output);
}

std::size_t ExprsData::get_index_in_dup(const std::size_t idx) const {
  for (std::size_t i = 0; i < dups.size(); ++i) {
    if (dups[i].first == idx)
      return i;
  }
  return num_bins_orig;
}

std::size_t ExprsData::get_orig_index(const std::size_t idx) const {
  assert(idx < reduced_to_orig.size());    
  return reduced_to_orig[idx];
}

std::string ExprsData::get_pat_as_str(const std::vector<std::size_t> &pat) const {
  std::ostringstream oss;   
  std::size_t orig_idx, index;
  std::string bin, val;

  for (std::size_t i = 0; i < pat.size()-1; ++i) {
    orig_idx = get_orig_index(pat[i]);
    index = orig_idx / 2;
    bin = orig_idx%2==0 ? "HIGH" : "LOW";
    val = orig_idx%2==0 ? "1" : "-1";

    oss << analyte_names[index].c_str() << "\t";
    oss << bin.c_str() << "\t";
    oss << val.c_str() << "\t";
  }

  orig_idx = get_orig_index(pat[pat.size()-1]);
  index = orig_idx / 2;
  bin = orig_idx%2==0 ? "HIGH" : "LOW";
  val = orig_idx%2==0 ? "1" : "-1";

  oss << analyte_names[index].c_str() << "\t";
  oss << bin.c_str() << "\t";
  oss << val.c_str();
    
  return oss.str();
}

bool ExprsData::indiv_has_pat(const std::size_t ind, const std::vector<std::size_t> &pat) const {
  if (ind >= grp1_total + grp2_total) {
    fprintf(stderr, "ERROR - ExprsData::indiv_has_pat - Trying to access indivual at index %lu\n", ind);
    exit(1);
  }

  for (auto p : pat) {
    if (!get_bin(p, ind)) {
      return false;
    }
  }
  return true;
}

double ExprsData::get_grp1_freq(const std::vector<std::size_t> &pat) const {
  double f1 = 0.0;

  for (std::size_t ind = grp1_start; ind <= grp1_stop; ++ind) {
    if (indiv_has_pat(ind, pat)) {
      ++f1;
    }
  }
  return f1 / grp1_total;
}

double ExprsData::get_grp2_freq(const std::vector<std::size_t> &pat) const {
  double f2 = 0.0;

  for (std::size_t ind = grp2_start; ind <= grp2_stop; ++ind) {
    if (indiv_has_pat(ind, pat)) {
      ++f2;
    }
  }
  return f2 / grp2_total;
}