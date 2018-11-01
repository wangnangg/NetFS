#pragma once
#include <cassert>
#include <iostream>
#include <vector>

struct Range
{
    size_t start;
    size_t end;
};

class RangeList
{
public:
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
    auto begin() const { return _ranges.begin(); }
    auto begin() { return _ranges.begin(); }
    auto end() const { return _ranges.end(); }
    auto end() { return _ranges.end(); }
    size_t count() const { return _ranges.size(); }
};

void overlay(const char* upper_layer, Range range, char* lower_layer);

void overlay(const char* upper_layer, const RangeList& ranges,
             char* lower_layer);

std::ostream& operator<<(std::ostream& os, const Range& rg);
