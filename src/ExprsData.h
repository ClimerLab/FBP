#ifndef EXPRS_DATA_H
#define EXPRS_DATA_H

#include <string>
#include <vector>

class ConfigParser;
class Pattern;

class ExprsData {
  private:
    const std::size_t num_cases;
    const std::size_t num_ctrls;
    const std::size_t num_analytes;
    const std::size_t num_header_rows;
    const std::size_t num_header_cols;
        
    const bool risk;
    const std::size_t grp1_total;
    const std::size_t grp1_start;
    const std::size_t grp1_stop;
    const std::size_t grp2_total;
    const std::size_t grp2_start;
    const std::size_t grp2_stop;
    const std::size_t num_bins_orig;
    
    const std::string data_file;
    const std::string MISSING_SYMBOL;
    const bool SET_NA_TRUE;
    const std::string HIGH_BIN;
    const std::string NORM_BIN;
    const std::string LOW_BIN;

    std::vector<std::vector<bool>> bin_data;
    std::vector<std::pair<std::size_t, std::vector<std::size_t>>> dups;
    std::vector<std::size_t> reduced_to_orig;
        
    void read_bin_data();
    std::size_t get_index_in_dup(const std::size_t idx) const;    
    
  public:
    std::vector<std::string> analyte_names;

    ExprsData(const ConfigParser &parser);
    ~ExprsData();
    const char * get_analyte_name(const std::size_t index) const;

    std::size_t get_num_grp1() const;
    std::size_t get_num_grp2() const;
    std::size_t get_num_bins() const;
    bool get_risk() const;
    std::size_t get_grp1_start() const;
    std::size_t get_grp1_stop() const;
    std::size_t get_grp2_start() const;
    std::size_t get_grp2_stop() const;
    bool get_bin(const std::size_t i, const std::size_t j) const;

    void print_bin_data(const std::string &file_name) const;
    std::size_t get_orig_index(const std::size_t idx) const;

    std::string get_pat_as_str(const std::vector<std::size_t> &pat) const;

    double get_grp1_freq(const std::vector<std::size_t> &pat) const;
    double get_grp2_freq(const std::vector<std::size_t> &pat) const;

    bool indiv_has_pat(const std::size_t ind, const std::vector<std::size_t> &pat) const;
};

#endif