#include "logger.h"

namespace cglang
{

logger::~logger()
{}

ostream_logger::~ostream_logger()
{}

void format_msg(std::ostream &os, const char* severity, const std::string &filename, const location &loc, const char *msg)
{
    os << '(' << severity << ") " << filename << ':' << loc << ' ' << msg << std::endl;
}

void ostream_logger::write_info(const location &loc, const char *msg)
{
    format_msg(_info, "info", _filename, loc, msg);
}

void ostream_logger::write_warning(const location &loc, const char *msg)
{
    format_msg(_warn, "warning", _filename, loc, msg);
}

void ostream_logger::write_error(const location &loc, const char *msg)
{
    format_msg(_err, "error", _filename, loc, msg);
}

}