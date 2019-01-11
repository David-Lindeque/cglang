#include "logger.h"

namespace cglang
{

logger::~logger()
{}

ostream_logger::~ostream_logger()
{}

void format_msg(std::wostream &os, const wchar_t* severity, const std::string &filename, const location &loc, const wchar_t *msg)
{
    os << L'(' << severity << ") " << std::wstring(filename.begin(), filename.end()) << L':' << loc << L' ' << msg << std::endl;
}

void ostream_logger::write_info(const location &loc, const wchar_t *msg)
{
    format_msg(_info, L"info", _filename, loc, msg);
}

void ostream_logger::write_warning(const location &loc, const wchar_t *msg)
{
    format_msg(_warn, L"warning", _filename, loc, msg);
}

void ostream_logger::write_error(const location &loc, const wchar_t *msg)
{
    format_msg(_err, L"error", _filename, loc, msg);
}

}