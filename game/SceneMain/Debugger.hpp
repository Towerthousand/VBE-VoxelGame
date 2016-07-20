#ifndef DEBUGGER_HPP
#define DEBUGGER_HPP
#include "commons.hpp"

class Debugger final : public Profiler {
    public:
        Debugger();
        ~Debugger();

        static int numChunksDrawn;
        static int numChunksSkipped;
    private:
        void renderCustomInterface() const override;
};

#endif // DEBUGGER_HPP
