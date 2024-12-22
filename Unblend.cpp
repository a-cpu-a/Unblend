
#include <vector>
#include <iostream>
#include <cmath>
#include <algorithm>

#define LODEPNG_COMPILE_DECODER
#include <lodepng/lodepng.h>
#include <lodepng/lodepng.cpp>

// Function to compute overlay and per-pixel alpha
void computeOverlay(std::vector<uint8_t>& overlay, const std::vector<uint8_t>& base, const std::vector<uint8_t>& result, uint32_t width, uint32_t height) {
    overlay.resize(base.size()); // Resize overlay to match input size

    for (uint32_t i = 0; i < width * height; i++)
    { // Iterate over each pixel
        uint32_t idx = i * 4;  // Start index of RGBA for this pixel

        uint8_t alpha = 0;

        // Compute alpha per pixel by considering the smallest valid alpha across RGB channels
        for (int c = 0; c < 3; c++)
        { // For R, G, B channels only
            if (result[idx + c] != base[idx + c])
            {
                double channel_alpha =
                    (result[idx + c] - base[idx + c]) /
                    (255.0 * (result[idx + c] > base[idx + c] ? 1.0 : -1.0));
                alpha = std::max(alpha, static_cast<uint8_t>(std::clamp(channel_alpha * 2.0 * 255.0, 0.0, 255.0)));
            }
            else
                alpha = 0;
        }

        if(alpha!=0)
        {
            // Compute overlay for each channel using the per-pixel alpha
            for (int c = 0; c < 3; c++)
            { // R, G, B channels
                overlay[idx + c] = static_cast<uint8_t>(
                    std::clamp((result[idx + c] - (1.0 - alpha / 255.0) * base[idx + c]) / (alpha / 255.0)+0.5, 0.0, 255.0));
            }
        }

        // Set the alpha channel of the overlay
        overlay[idx + 3] = alpha;
    }
}

int main() {
    // Load base and result images
    std::vector<uint8_t> base, result, overlay;
    uint32_t width, height;

    if (lodepng::decode(base, width, height, "base.png"))
    {
        std::cerr << "Error loading base.png" << std::endl;
        return 1;
    }
    if (lodepng::decode(result, width, height, "result.png"))
    {
        std::cerr << "Error loading result.png" << std::endl;
        return 1;
    }

    // Compute overlay
    computeOverlay(overlay, base, result, width, height);

    // Save the overlay image
    if (lodepng::encode("overlay.png", overlay, width, height))
    {
        std::cerr << "Error saving overlay.png" << std::endl;
        return 1;
    }

    std::cout << "Overlay image saved as overlay.png" << std::endl;
    return 0;
}