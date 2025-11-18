#include "nvpm.h"
#include <unordered_map>
#include <cstring>
#include <cstdint>

int NVPMAddCounter(uint32_t unCounterIndex) {
    return 0;
}

int NVPMBeginExperiment(int *pnNumPasses) {
    *pnNumPasses = 0;
    return 0;
}

int NVPMBeginObject(int nObjectID) {
    return 0;
}

int NVPMBeginPass(int nPass) {
    return 0;
}

int NVPMEndObject(int nObjectID) {
    return 0;
}

int NVPMEndPass(int nPass) {
    return 0;
}

int NVPMEndExperiment() {
    return 0;
}

int NVPMGetCounterDescription(uint32_t unCounterIndex, char *pcString, uint32_t *punLen) {
    *pcString = '\0';
    *punLen = 0;
    return 0;
}

int NVPMGetCounterIndex(char *pcString, uint32_t *punCounterIndex) {
    *pcString = '\0';
    *punCounterIndex = 0;
    return 0;
}

int NVPMGetCounterName(uint32_t unCounterIndex, char *pcString, uint32_t *punLen) {
    *pcString = '\0';
    *punLen = 0;
    return 0;
}

int NVPMGetCounterValueByName(char *pcName, int nObjectID, uint64_t *pulValue, uint64_t *pulCycles) {
    *pulValue = 0;
    *pulCycles = 0;
    return 0;
}

int NVPMGetCounterValue(uint32_t unCounterIndex, int nObjectID, uint64_t *pulValue, uint64_t *pulCycles) {
    *pulValue = 0;
    *pulCycles = 0;
    return 0;
}

int NVPMGetNumCounters(uint32_t *punCount) {
    *punCount = 0;
    return 0;
}

int NVPMInit() {
    return 0;
}

int NVPMRemoveAllCounters() {
    return 0;
}

int NVPMRemoveCounter(uint32_t unCounterIndex) {
    return 0;
}

int NVPMSample(void *pSamples, uint32_t *punCount) {
    *punCount = 0;
    return 0;
}

int NVPMShutdown() {
    return 0;
}

std::unordered_map<const char *, void *> NVPM_FUNCS_STUB = {
    {"NVPMAddCounter", (void *)NVPMAddCounter},
    {"NVPMBeginExperiment", (void *)NVPMBeginExperiment},
    {"NVPMBeginObject", (void *)NVPMBeginObject},
    {"NVPMBeginPass", (void *)NVPMBeginPass},
    {"NVPMEndExperiment", (void *)NVPMEndExperiment},
    {"NVPMEndObject", (void *)NVPMEndObject},
    {"NVPMEndPass", (void *)NVPMEndPass},
    {"NVPMGetCounterDescription", (void *)NVPMGetCounterDescription},
    {"NVPMGetCounterIndex", (void *)NVPMGetCounterIndex},
    {"NVPMGetCounterName", (void *)NVPMGetCounterName},
    {"NVPMGetCounterValue", (void *)NVPMGetCounterValue},
    {"NVPMGetCounterValueByName", (void *)NVPMGetCounterValueByName},
    {"NVPMGetNumCounters", (void *)NVPMGetNumCounters},
    {"NVPMInit", (void *)NVPMInit},
    {"NVPMRemoveAllCounters", (void *)NVPMRemoveAllCounters},
    {"NVPMRemoveCounter", (void *)NVPMRemoveCounter},
    {"NVPMSample", (void *)NVPMSample},
    {"NVPMShutdown", (void *)NVPMShutdown}
};

void *nvpm_resolve(const char *name)
{
    for (auto &it : NVPM_FUNCS_STUB)
    {
        if (strcmp(it.first, name) == 0)
            return it.second;
    }
    return NULL;
}