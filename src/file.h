#ifndef __FILEH
#define __FILEH

#include <string>
#include <set>
#include <vector>

namespace cglang
{
    void search_fn(const std::vector<std::string> &i_dirs, const std::string &pwd, const std::string &fn, std::set<std::string> &found, std::set<std::string> &tries);
}

#endif