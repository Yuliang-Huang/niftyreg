#pragma once

#include "Kernel.h"

class LtsKernel: public Kernel {
public:
    static std::string GetName() {
        return "LtsKernel";
    }
    virtual void Calculate(bool affine) = 0;
};
