#ifndef GPU_MONITOR_H
#define GPU_MONITOR_H

#include <windows.h>
#include <string>

typedef int nvmlReturn_t;
#define NVML_SUCCESS 0
#define NVML_TEMPERATURE_GPU 0

typedef void* nvmlDevice_t;

typedef struct {
    unsigned int gpu;
    unsigned int memory;
} nvmlUtilization_t;

typedef nvmlReturn_t (*pfn_nvmlInit)(void);
typedef nvmlReturn_t (*pfn_nvmlShutdown)(void);
typedef nvmlReturn_t (*pfn_nvmlDeviceGetHandleByIndex)(unsigned int, nvmlDevice_t*);
typedef nvmlReturn_t (*pfn_nvmlDeviceGetTemperature)(nvmlDevice_t, int, unsigned int*);
typedef nvmlReturn_t (*pfn_nvmlDeviceGetUtilizationRates)(nvmlDevice_t, nvmlUtilization_t*);
typedef nvmlReturn_t (*pfn_nvmlDeviceGetPowerUsage)(nvmlDevice_t, unsigned int*);

class GPUMonitor {
public:
    bool isAvailable = false;
    unsigned int temperatureC = 0;
    unsigned int utilizationGpu = 0;
    unsigned int utilizationMem = 0;
    float powerWatts = 0.0f;

private:
    HMODULE hNvml = NULL;
    nvmlDevice_t device = NULL;
    pfn_nvmlInit nvmlInit = NULL;
    pfn_nvmlShutdown nvmlShutdown = NULL;
    pfn_nvmlDeviceGetHandleByIndex nvmlDeviceGetHandleByIndex = NULL;
    pfn_nvmlDeviceGetTemperature nvmlDeviceGetTemperature = NULL;
    pfn_nvmlDeviceGetUtilizationRates nvmlDeviceGetUtilizationRates = NULL;
    pfn_nvmlDeviceGetPowerUsage nvmlDeviceGetPowerUsage = NULL;

public:
    GPUMonitor() {
        init();
    }

    ~GPUMonitor() {
        shutdown();
    }

    bool init() {
        hNvml = LoadLibraryA("nvml.dll");
        if (!hNvml) return false;

        // Support both _v2 and standard entry points
        nvmlInit = (pfn_nvmlInit)GetProcAddress(hNvml, "nvmlInit_v2");
        if (!nvmlInit) nvmlInit = (pfn_nvmlInit)GetProcAddress(hNvml, "nvmlInit");

        nvmlShutdown = (pfn_nvmlShutdown)GetProcAddress(hNvml, "nvmlShutdown");

        nvmlDeviceGetHandleByIndex = (pfn_nvmlDeviceGetHandleByIndex)GetProcAddress(hNvml, "nvmlDeviceGetHandleByIndex_v2");
        if (!nvmlDeviceGetHandleByIndex) nvmlDeviceGetHandleByIndex = (pfn_nvmlDeviceGetHandleByIndex)GetProcAddress(hNvml, "nvmlDeviceGetHandleByIndex");

        nvmlDeviceGetTemperature = (pfn_nvmlDeviceGetTemperature)GetProcAddress(hNvml, "nvmlDeviceGetTemperature");
        nvmlDeviceGetUtilizationRates = (pfn_nvmlDeviceGetUtilizationRates)GetProcAddress(hNvml, "nvmlDeviceGetUtilizationRates");
        nvmlDeviceGetPowerUsage = (pfn_nvmlDeviceGetPowerUsage)GetProcAddress(hNvml, "nvmlDeviceGetPowerUsage");

        if (!nvmlInit || !nvmlDeviceGetHandleByIndex || !nvmlDeviceGetTemperature || !nvmlDeviceGetUtilizationRates) {
            FreeLibrary(hNvml);
            hNvml = NULL;
            return false;
        }

        if (nvmlInit() != NVML_SUCCESS) {
            FreeLibrary(hNvml);
            hNvml = NULL;
            return false;
        }

        if (nvmlDeviceGetHandleByIndex(0, &device) != NVML_SUCCESS) {
            if (nvmlShutdown) nvmlShutdown();
            FreeLibrary(hNvml);
            hNvml = NULL;
            return false;
        }

        isAvailable = true;
        update();
        return true;
    }

    void update() {
        if (!isAvailable || !device) return;

        unsigned int temp = 0;
        if (nvmlDeviceGetTemperature && nvmlDeviceGetTemperature(device, NVML_TEMPERATURE_GPU, &temp) == NVML_SUCCESS) {
            temperatureC = temp;
        }

        nvmlUtilization_t util = {0, 0};
        if (nvmlDeviceGetUtilizationRates && nvmlDeviceGetUtilizationRates(device, &util) == NVML_SUCCESS) {
            utilizationGpu = util.gpu;
            utilizationMem = util.memory;
        }

        unsigned int powerMw = 0;
        if (nvmlDeviceGetPowerUsage && nvmlDeviceGetPowerUsage(device, &powerMw) == NVML_SUCCESS) {
            powerWatts = (float)powerMw / 1000.0f;
        }
    }

    void shutdown() {
        if (isAvailable && nvmlShutdown) {
            nvmlShutdown();
        }
        if (hNvml) {
            FreeLibrary(hNvml);
            hNvml = NULL;
        }
        isAvailable = false;
        device = NULL;
    }
};

#endif // GPU_MONITOR_H
