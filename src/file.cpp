#include "file.h"
#include <sys/stat.h>

namespace
{

inline bool file_exists(const std::string& name) {
    struct stat buffer;   
    return stat(name.c_str(), &buffer) == 0; 
}

bool try_file(std::string &result, const std::string &dir, const std::string &fn)
{
    // dir can be
    // ''                   ::= '<fn>'
    // './'                 ::= './<fn>'
    // '.'                  ::= './<fn>'
    // '...../dir/dir/'     ::= '...../dir/dir/<fn>'
    // '...../dir/dir'      ::= '...../dir/dir/<fn>'

    result = dir;
    if (result.empty()) {
        result = fn;
    }
    else {
        if (result.back() == '/') {
            result.append(fn);
        }
        else {
            result += '/';
            result.append(fn);
        }
    }
    return file_exists(result);
}

}

namespace cglang
{

void search_fn(const std::vector<std::string> &i_dirs, const std::string &pwd, const std::string &fn, std::set<std::string> &found, std::set<std::string> &tries)
{
    if (!fn.empty() && fn[0] == '/') {
        // absolute path - do not try dirs
        if (file_exists(fn)) {
            found.emplace(fn);
        }
        return;
    }

    std::string name = fn;
    std::string nfn;
    if (try_file(nfn, pwd, name)) {
        found.emplace(nfn);
    }
    tries.emplace(nfn);
    for(auto &d : i_dirs) {
        if (try_file(nfn, d, name)) {
            found.emplace(nfn);
        }
        tries.emplace(nfn);
    }
    name = fn + ".cgfeature";
    if (try_file(nfn, pwd, name)) {
        found.emplace(nfn);
    }
    tries.emplace(nfn);
    for(auto &d : i_dirs) {
        if (try_file(nfn, d, name)) {
            found.emplace(nfn);
        }
        tries.emplace(nfn);
    }
}

}