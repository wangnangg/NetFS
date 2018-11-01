#include "range.hpp"

bool RangeList::overlap(Range r1, Range r2)
{
    if (r1.start <= r2.start)
    {
        if (r1.end >= r2.start)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        if (r2.end >= r1.start)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
}

void RangeList::insertRange(size_t start, size_t end)
{
    assert(start < end);
    auto new_range = Range{start, end};
    int over_start = -1;
    int over_end = -1;
    size_t new_start = start;
    size_t new_end = end;
    for (size_t i = 0; i < _ranges.size(); i++)
    {
        auto r = _ranges[i];
        if (overlap(r, new_range))
        {
            if (over_start == -1)
            {
                over_start = i;
            }
            over_end = i + 1;
            new_start = std::min(new_start, r.start);
            new_end = std::max(new_end, r.end);
        }
    }
    if (over_start >= 0)
    {
        _ranges.erase(_ranges.begin() + over_start,
                      _ranges.begin() + over_end);
        _ranges.insert(_ranges.begin() + over_start,
                       Range{new_start, new_end});
    }
    else
    {
        auto iter = _ranges.begin();
        while (iter != _ranges.end() && iter->start < new_start)
        {
            iter++;
        }
        _ranges.insert(iter, Range{new_start, new_end});
    }
}

void overlay(const char* upper_layer, Range range, char* lower_layer)
{
    for (size_t i = range.start; i < range.end; i++)
    {
        lower_layer[i] = upper_layer[i];
    }
}
void overlay(const char* upper_layer, const RangeList& ranges,
             char* lower_layer)
{
    for (auto r : ranges)
    {
        overlay(upper_layer, r, lower_layer);
    }
}

std::ostream& operator<<(std::ostream& os, const Range& rg)
{
    os << "[" << rg.start << "-" << rg.end << "]";
    return os;
}
