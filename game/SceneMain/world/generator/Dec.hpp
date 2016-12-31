#ifndef DEC_HPP
#define DEC_HPP
#include "ColumnGenerator.hpp"

class Dec { //abstract
    public:
        Dec() : Dec() {}
        virtual ~Dec() {}

        virtual void decorate(ColumnGenerator::ColumnData* col) = 0;
};

#endif // DEC_HPP
