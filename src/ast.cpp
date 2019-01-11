#include "ast.h"


namespace cglang
{
    std::ostream& operator <<(std::ostream& os, const location& loc)
    {
        return os 
            << loc.first_line << ':' << loc.first_column 
            << '-' 
            << loc.last_line << ':' << loc.last_column;
    }

    std::wostream& operator <<(std::wostream& os, const location& loc)
    {
        return os 
            << loc.first_line << L':' << loc.first_column 
            << L'-' 
            << loc.last_line << L':' << loc.last_column;
    }

}