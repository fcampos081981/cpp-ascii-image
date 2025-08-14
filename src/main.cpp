
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cmath>

#define STB_IMAGE_STATIC
#include "stb_image.h"


struct Options {
    std::string inputPath;
    std::string outputPath;     // empty => stdout
    int width = 120;            // target character columns
    float charAspect = 0.5f;    // width/height of a terminal glyph (~0.5 for many fonts)
    bool invert = false;        // invert brightness -> chars
    std::string charset = "@%#*+=-:. "; // dark->light mapping
};

static void print_usage(const char* exe) {
    std::cerr
        << "Usage: " << exe << " <input.jpg> [options]\n"
        << "Options:\n"
        << "  -w, --width <cols>      Target ASCII width in characters (default 120)\n"
        << "  -a, --aspect <ratio>    Character width/height aspect ratio (default 0.5)\n"
        << "  -c, --charset <chars>   Characters from dark->light (default \"@%#*+=-:. \")\n"
        << "  -i, --invert            Invert mapping (light uses dense chars)\n"
        << "  -o, --output <file>     Write result to file instead of stdout\n"
        << "Examples:\n"
        << "  " << exe << " photo.jpg -w 100\n"
        << "  " << exe << " photo.jpg -w 80 -a 0.45 -c \"MWNXK0Okxol:,. \" -o out.txt\n";
}

static bool parse_args(int argc, char** argv, Options& opt) {
    if (argc < 2) {
        print_usage(argv[0]);
        return false;
    }
    opt.inputPath = argv[1];
    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        auto need_val = [&](const char* name)->bool {
            if (i + 1 >= argc) {
                std::cerr << "Missing value for " << name << "\n";
                return false;
            }
            return true;
        };
        if (arg == "-w" || arg == "--width") {
            if (!need_val(arg.c_str())) return false;
            opt.width = std::max(1, std::atoi(argv[++i]));
        } else if (arg == "-a" || arg == "--aspect") {
            if (!need_val(arg.c_str())) return false;
            opt.charAspect = std::max(0.05f, std::stof(argv[++i]));
        } else if (arg == "-c" || arg == "--charset") {
            if (!need_val(arg.c_str())) return false;
            opt.charset = argv[++i];
            if (opt.charset.empty()) {
                std::cerr << "Charset must not be empty.\n";
                return false;
            }
        } else if (arg == "-i" || arg == "--invert") {
            opt.invert = true;
        } else if (arg == "-o" || arg == "--output") {
            if (!need_val(arg.c_str())) return false;
            opt.outputPath = argv[++i];
        } else if (arg == "-h" || arg == "--help") {
            print_usage(argv[0]);
            return false;
        } else {
            std::cerr << "Unknown option: " << arg << "\n";
            return false;
        }
    }
    return true;
}

// Simple nearest-neighbor resize for grayscale image
static std::vector<unsigned char> resize_nn(
    const unsigned char* src, int sw, int sh, int dw, int dh)
{
    std::vector<unsigned char> dst(dw * dh);
    for (int y = 0; y < dh; ++y) {
        int sy = static_cast<int>( (y + 0.5f) * sh / static_cast<float>(dh) );
        sy = std::clamp(sy, 0, sh - 1);
        for (int x = 0; x < dw; ++x) {
            int sx = static_cast<int>( (x + 0.5f) * sw / static_cast<float>(dw) );
            sx = std::clamp(sx, 0, sw - 1);
            dst[y * dw + x] = src[sy * sw + sx];
        }
    }
    return dst;
}

static char map_intensity_to_char(unsigned char v, const std::string& charset, bool invert) {
    // v in [0,255] -> index in charset
    int n = static_cast<int>(charset.size());
    if (n == 1) return charset[0];
    float t = v / 255.0f; // 0 = dark, 1 = light
    if (invert) t = 1.0f - t;
    int idx = static_cast<int>(std::round(t * (n - 1)));
    idx = std::clamp(idx, 0, n - 1);
    return charset[idx];
}

int main(int argc, char** argv) {
    Options opt;
    if (!parse_args(argc, argv, opt)) {
        return argc < 2 ? 1 : 0;
    }

    int w = 0, h = 0, ch = 0;
    // Load as grayscale directly (1 channel)
    unsigned char* img = stbi_load(opt.inputPath.c_str(), &w, &h, &ch, 1);
    if (!img) {
        std::cerr << "Failed to load image: " << opt.inputPath << "\n";
        return 1;
    }

    // Determine output character dimensions
    int outW = std::max(1, opt.width);
    // Because characters are taller than they are wide, we scale height accordingly.
    // Each character cell roughly has aspect ratio (charWidth/charHeight) = opt.charAspect.
    // So, number of rows should be scaled by 1/charAspect.
    float scale = static_cast<float>(outW) / static_cast<float>(w);
    int outH = std::max(1, static_cast<int>(std::round(h * scale / std::max(0.01f, opt.charAspect))));

    auto resized = resize_nn(img, w, h, outW, outH);
    stbi_image_free(img);

    std::ostream* out = &std::cout;
    std::unique_ptr<std::ofstream> fout;
    if (!opt.outputPath.empty()) {
        fout = std::make_unique<std::ofstream>(opt.outputPath, std::ios::out | std::ios::trunc);
        if (!(*fout)) {
            std::cerr << "Failed to open output file: " << opt.outputPath << "\n";
            return 1;
        }
        out = fout.get();
    }

    // Emit ASCII lines
    for (int y = 0; y < outH; ++y) {
        for (int x = 0; x < outW; ++x) {
            unsigned char v = resized[y * outW + x];
            (*out) << map_intensity_to_char(v, opt.charset, opt.invert);
        }
        (*out) << '\n';
    }

    if (!opt.outputPath.empty()) {
        std::cerr << "Wrote ASCII art to: " << opt.outputPath << "\n";
    }

    return 0;
}