#ifndef BENCHMARK_H
#define BENCHMARK_H

// resolutions + benchmark scoring

struct Resolution {
    const char* name;
    const char* size;
    int width;
    int height;
    float scoreWeight;  // pixels compared to 720p
};

inline const Resolution resolutions[] = {
    { "720p",  "1280 × 720",  1280, 720,  1.0f  },
    { "1080p", "1920 × 1080", 1920, 1080, 2.25f },
    { "1440p", "2560 × 1440", 2560, 1440, 4.0f  },
    { "4K",    "3840 × 2160", 3840, 2160, 9.0f  },
    { "8K",    "7680 × 4320", 7680, 4320, 36.0f },
};
const int RESOLUTION_COUNT = sizeof(resolutions) / sizeof(resolutions[0]);

// cube, torus, armada
inline const float sceneScoreWeights[] = { 0.25f, 2.0f, 2.0f };

struct BenchmarkResult {
    int score = 0;
    double avgFps = 0.0;
    int scene = 0;
    int resIndex = 0;
};

struct Benchmark {
    static constexpr double DURATION = 30.0;

    bool running = false;
    bool hasResult = false;
    double startTime = 0.0;
    int frames = 0;
    BenchmarkResult result;

    void start(double now) {
        running = true;
        startTime = now;
        frames = 0;
    }

    void cancel() { running = false; }

    double elapsed(double now) const { return now - startTime; }

    // call every frame
    void update(double now, int scene, int resIndex) {
        if (!running) return;
        frames++;

        double seconds = elapsed(now);
        if (seconds >= DURATION) {
            running = false;
            hasResult = true;
            result.avgFps = frames / seconds;
            result.scene = scene;
            result.resIndex = resIndex;
            // score = avg fps scaled by resolution and scene
            result.score = (int)(result.avgFps * 10.0 * resolutions[resIndex].scoreWeight * sceneScoreWeights[scene]);
        }
    }
};

#endif // BENCHMARK_H
