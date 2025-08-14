#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <cstdint>

#if defined(__unix__) || defined(__APPLE__)
#include <sys/ioctl.h>
#include <unistd.h>
#endif

struct TermSize {
    int cols;
    int rows;
};

static TermSize getTerminalSize() {
    TermSize ts{100, 30};
#if defined(__unix__) || defined(__APPLE__)
    struct winsize w{};
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
        if (w.ws_col > 0) ts.cols = w.ws_col;
        if (w.ws_row > 0) ts.rows = w.ws_row;
    }
#endif
    return ts;
}

static inline uint8_t clampU8(int v) {
    if (v < 0) return 0;
    if (v > 255) return 255;
    return static_cast<uint8_t>(v);
}


static inline uint8_t luminance(const unsigned char* p, int channels) {

    float r = 0.f, g = 0.f, b = 0.f, a = 1.f;
    if (channels == 1) {

        r = g = b = p[0] / 255.0f;
    } else if (channels == 2) {

        float gray = p[0] / 255.0f;
        a = p[1] / 255.0f;
        r = g = b = gray * a;
    } else if (channels >= 3) {
        r = p[0] / 255.0f;
        g = p[1] / 255.0f;
        b = p[2] / 255.0f;
        if (channels >= 4) {
            a = p[3] / 255.0f;
            r *= a; g *= a; b *= a;
        }
    }
    float Y = 0.2126f * r + 0.7152f * g + 0.0722f * b;
    int yi = static_cast<int>(std::round(Y * 255.0f));
    return clampU8(yi);
}

int main(int argc, char** argv) {
    std::string path = "PUT_YOUR_IMAGE_PATH_HERE.png";
    if (argc > 1) path = argv[1];

    int width = 0, height = 0, channels = 0;
    stbi_uc* img = stbi_load(path.c_str(), &width, &height, &channels, 0);
    if (img == nullptr) {
        std::cerr << "Error loading image: " << stbi_failure_reason() << "\n";
        std::cerr << "Tried: " << path << "\n";
        return 1;
    }


    TermSize ts = getTerminalSize();

    int targetCols = std::max(20, ts.cols - 2);


    const float charAspect = 2.0f;

    float scale = static_cast<float>(targetCols) / static_cast<float>(width);
    int targetRows = std::max(1, static_cast<int>(std::round((height * scale) / charAspect)));


    const std::string ramp = " .:-=+*#%@";
    const int rampN = static_cast<int>(ramp.size());


    std::vector<char> line(static_cast<size_t>(targetCols) + 1, '\0');

    for (int y = 0; y < targetRows; ++y) {

        int sy = std::min(height - 1, std::max(0, static_cast<int>(std::round((y + 0.5f) * (height / (targetRows * charAspect)) - 0.5f))));
        for (int x = 0; x < targetCols; ++x) {
            int sx = std::min(width - 1, std::max(0, static_cast<int>(std::round((x + 0.5f) * (static_cast<float>(width) / targetCols) - 0.5f))));
            const unsigned char* p = img + (static_cast<size_t>(sy) * width + sx) * channels;
            uint8_t lum = luminance(p, channels);

            int idx = (lum * (rampN - 1)) / 255;
            line[static_cast<size_t>(x)] = ramp[idx];
        }
        line[static_cast<size_t>(targetCols)] = '\0';
        std::cout << line.data() << "\n";
    }

    stbi_image_free(img);
    return 0;
}