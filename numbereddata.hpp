#ifndef __NUMBERED_DATA_HPP__
#define __NUMBERED_DATA_HPP__

#include <string>
#include <memory>

typedef struct NumberedString
{
    unsigned int number;
    std::string data;
    NumberedString(unsigned int n, std::string d) : number(n), data(d) {}
} NumberedString;

struct NumberedStringCompare
{
    bool operator()(const std::shared_ptr<NumberedString> e1, const std::shared_ptr<NumberedString> e2) const
    {
        return e1->number > e2->number;
    }
};

#endif
