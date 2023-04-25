#include "processor.h"

// Optional TODO: Change here to implement predictions
class FrontendWithPredict : public Frontend {
protected:
    BranchPredictBundle bpuFrontendUpdate(unsigned int pc) override;
    BTBSlot btb[BTB_SIZE];
public:
    FrontendWithPredict(const std::vector<unsigned> &inst);
    unsigned calculateNextPC(unsigned pc) const override;
    void bpuBackendUpdate(const BpuUpdateData &x) override;
};

class ProcessorWithPredict {
    FrontendWithPredict frontend;
    Backend backend;
    RegisterFile regFile;

public:
    ProcessorWithPredict(const std::vector<unsigned> &inst,
                         const std::vector<unsigned> &data,
                         unsigned entry);
    bool step();
    unsigned readMem(unsigned addr) const;
    unsigned readReg(unsigned addr) const;
};
