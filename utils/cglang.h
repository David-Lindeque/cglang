#ifndef __CGLANG_H
#define __CGLANG_H

#include <iterator>

namespace cglang
{
namespace utils
{

template<unsigned int _Cols, class _CharT = char>
struct table
{
public:
    struct column_iterator
    {
    private:
        const _CharT **_indexes;
        unsigned int _index;
        explicit column_iterator(const _CharT **indexes, const unsigned int &index)
        : _indexes(indexes), _index(index)
        {}
    public:
        typedef std::random_access_iterator_tag iterator_category;
        typedef const _CharT* value_type;
        typedef int difference_type;
        typedef const value_type* pointer;
        typedef const value_type& reference;

        friend bool operator==(const column_iterator &lhs, const column_iterator &rhs)
        {
            return lhs._index == rhs._index;
        }
        friend bool operator!=(const column_iterator &lhs, const column_iterator &rhs)
        {
            return lhs._index != rhs._index;
        }
        friend bool operator<(const column_iterator &lhs, const column_iterator &rhs)
        {
            return lhs._index < rhs._index;
        }
        friend bool operator>(const column_iterator &lhs, const column_iterator &rhs)
        {
            return lhs._index > rhs._index;
        }
        friend bool operator<=(const column_iterator &lhs, const column_iterator &rhs)
        {
            return lhs._index <= rhs._index;
        }
        friend bool operator>=(const column_iterator &lhs, const column_iterator &rhs)
        {
            return lhs._index >= rhs._index;
        }

        column_iterator& operator ++()
        {
            _index += _Cols;
            return *this;
        }

        column_iterator operator ++(int)
        {
            column_iterator ci(_indexes, _index);
            ++ci;
            return ci;
        }
        column_iterator& operator --()
        {
            _index -= _Cols;
            return *this;
        }
        column_iterator operator --(int)
        {
            column_iterator ci(_indexes, _index);
            --ci;
            return ci;
        }
        column_iterator& operator +=(difference_type ofs)
        {
            _index += (_Cols * ofs);
            return *this;
        }
        friend column_iterator operator +(const column_iterator &lhs, difference_type rhs)
        {
            column_iterator ci(lhs._indexes, lhs._index);
            ci += rhs;
            return ci;
        }
        friend column_iterator operator +(difference_type lhs, const column_iterator &rhs)
        {
            return rhs+lhs;
        }
        column_iterator& operator -=(difference_type ofs)
        {
            _index -= (_Cols * ofs);
            return *this;
        }
        friend column_iterator operator -(const column_iterator &lhs, difference_type rhs)
        {
            column_iterator ci(lhs._indexes, lhs._index);
            ci -= rhs;
            return ci;
        }
        friend difference_type operator -(const column_iterator &lhs, const column_iterator &rhs)
        {
            // it1 - it2 = n
            // it1 = it2 + n
            return (lhs._index - rhs._index) / _Cols;
        }
        reference operator[](difference_type index)
        {
            return _indexes[_index + (_Cols * index)];
        }
        reference operator*()
        {
            return _indexes[_index];
        }

        friend struct table<_Cols, _CharT>;
    };
private:
    const _CharT **_indexes;
    unsigned int _rows;
public:
    explicit table(const _CharT **indexes, const unsigned int &rows)
    : _indexes(indexes), _rows(rows)
    {}

    unsigned int rows() const { return _rows; }
    unsigned int cols() const { return _Cols; }
    const _CharT* get(const unsigned int &row, const unsigned int &col) const
    {
        return _indexes[(row*_Cols) + col];
    }
    column_iterator begin_column(const unsigned int &col, const unsigned int &ofs) const
    {
        return column_iterator(_indexes, (ofs*_Cols) + col);
    }
    column_iterator end_column(const unsigned int &col) const
    {
        return column_iterator(_indexes, (_rows*_Cols) + col);
    }
};

} // namespace utils

} // namespace cglang

#endif
