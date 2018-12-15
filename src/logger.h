#ifndef __LOGGERH
#define __LOGGERH

#include "ast.h"

#include <string>
#include <ostream>
#include <memory>

namespace cglang {
        
    class logger {
    public:
        virtual ~logger();
        virtual void write_info(const location &loc, const char *msg) = 0;
        virtual void write_warning(const location &loc, const char *msg) = 0;
        virtual void write_error(const location &loc, const char *msg) = 0;
    };

    class ostream_logger : public logger {
    private:
        std::string _filename;
        std::ostream &_info;
        std::ostream &_warn;
        std::ostream &_err;
    public:
        ostream_logger() = delete;
        ostream_logger(const ostream_logger&) = delete;
        ostream_logger(ostream_logger&&) = delete;
        template<typename _Filename> explicit ostream_logger(_Filename &&filename, std::ostream &info, std::ostream &warn, std::ostream &err)
        : _filename(std::forward<_Filename>(filename)), _info(info), _warn(warn), _err(err)
        {}
        virtual ~ostream_logger();

        ostream_logger& operator=(const ostream_logger&)  = delete;
        ostream_logger& operator=(ostream_logger&&) = delete;

        virtual void write_info(const location &loc, const char *msg);
        virtual void write_warning(const location &loc, const char *msg);
        virtual void write_error(const location &loc, const char *msg);
    };
}

#endif