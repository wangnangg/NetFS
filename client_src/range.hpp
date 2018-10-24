#pragma once
#include <cassert>
#include <vector>

class RangeList
{
public:
    struct Range
    {
        size_t start;
        size_t end;
    };

private:
    std::vector<Range> _ranges;

    bool overlap(Range r1, Range r2);

public:
    RangeList() = default;
    RangeList(size_t start, size_t end) : _ranges()
    {
        assert(start < end);
        _ranges.push_back({start, end});
    }
    // insert a range in a sorted, not overlapped sequence of ranges. Possibly
    // merge with other ranges
    void insertRange(size_t start, size_t end);
    const std::vector<Range>& ranges() const { return _ranges; }
};

void overlay(const char* upper_layer, size_t size, const RangeList& ranges,
             char* lower_layer);
