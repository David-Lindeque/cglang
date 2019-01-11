#ifndef __ASTH
#define __ASTH

#include <string>
#include <memory>
#include <ostream>

namespace cglang
{
    class cell;
    typedef std::unique_ptr<cell> pcell;
    typedef std::vector<pcell> cells;
    typedef std::unique_ptr<cells> pcells;
    typedef std::vector<pcells> rows;
    typedef std::unique_ptr<rows> prows;
    typedef prows table;

    struct location {
        int first_line;
        int first_column;
        int last_line;
        int last_column;
    };

    std::ostream& operator <<(std::ostream&, const location&);
    std::wostream& operator <<(std::wostream&, const location&);

    enum class step_kind {
        given,
        when,
        then
    };

    class step {
    private:
        step_kind _kind;
        std::wstring _text;
        location _span;
    public:
        step() = delete;
        step(const step&) = default;
        step(step&&) = default;
        template<typename _Text> explicit step(const step_kind &kind, _Text &&text, const location &span)
        : _kind(kind), _text(std::forward<_Text>(text)), _span(span)
        {}

        step& operator=(const step&) = default;
        step& operator=(step&&) = default;

        union {
            location _table_loc;
            location _mtext_loc;
        };
        table _table;
        std::wstring _mtext;

        const step_kind& kind() const { return _kind; }
        const std::wstring& text() const { return _text; }
        const location& span() const { return _span; }
    };

    typedef std::unique_ptr<step> pstep;
    typedef std::vector<pstep> steps;
    typedef std::unique_ptr<steps> psteps;

    class cell {
    private:
        std::wstring _value;
        location _span;
    public:
        cell() = delete;
        cell(const cell&) = default;
        cell(cell&&) = default;
        template<typename _Value> explicit cell(_Value &&value, const location &span)
        : _value(std::forward<_Value>(value)), _span(span)
        {}

        cell& operator=(const cell&) = default;
        cell& operator=(cell&&) = default;

        const std::wstring value() const { return _value; }
        const location span() const { return _span; }
    };
}

#endif