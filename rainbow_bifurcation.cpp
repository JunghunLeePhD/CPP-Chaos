#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <string>
#include <iomanip>
#include <sstream>

// --- Configuration ---
const int WIDTH = 800;      // HD Resolution width
const int HEIGHT = 800;     // HD Resolution height
const int ITERATIONS = 60;  // How many points to draw per Lambda
const double LAMBDA_MIN = 1.0;
const double LAMBDA_MAX = 4.0;

// --- Color Management ---
struct Color { uint8_t r, g, b; };

// Helper: Linear Interpolation between Blue (start) and Red (end)
Color get_heat_color(int i, int max_i) {
    double t = static_cast<double>(i) / max_i;
    // Clamp t to [0, 1] just in case
    if (t < 0) t = 0; if (t > 1) t = 1;

    // Start (Blue): 0, 0, 255
    // End (Red):   255, 0, 0
    uint8_t r = static_cast<uint8_t>(255 * t);
    uint8_t g = 0; // Keep it dark/contrast
    uint8_t b = static_cast<uint8_t>(255 * (1.0 - t));
    
    return {r, g, b};
}

const Color WHITE = {255, 255, 255};

class Canvas {
    std::vector<Color> pixels;
    int width, height;
public:
    Canvas(int w, int h) : width(w), height(h), pixels(w * h, WHITE) {}

    void set_pixel(int x, int y, Color c) {
        // We flip Y so 0 is at the bottom
        int true_y = height - 1 - y;
        if (x >= 0 && x < width && true_y >= 0 && true_y < height) {
            pixels[true_y * width + x] = c;
        }
    }

    void save_ppm(const std::string& filename) {
        std::ofstream ofs(filename, std::ios::binary);
        ofs << "P6\n" << width << " " << height << "\n255\n";
        for (const auto& p : pixels) ofs << p.r << p.g << p.b;
        ofs.close();
    }
};

int main() {
    std::cout << "Initializing Rainbow Bifurcation canvas..." << std::endl;
    
    // We keep one persistent canvas and add to it frame by frame
    Canvas canvas(WIDTH, HEIGHT);

    std::cout << "Generating " << WIDTH << " frames (Scanning Lambda)..." << std::endl;

    // We treat every pixel column as a frame in the movie
    for (int col = 0; col < WIDTH; ++col) {
        
        // 1. Calculate Lambda for this specific column
        double t_lam = static_cast<double>(col) / (WIDTH - 1);
        double lam = LAMBDA_MIN + t_lam * (LAMBDA_MAX - LAMBDA_MIN);

        // 2. Run the Orbit
        double x = 0.5; // Start at critical point
        
        // DO NOT skip transients. Draw everything.
        for (int i = 0; i < ITERATIONS; ++i) {
            // Calculate pixel position
            int row = static_cast<int>(x * (HEIGHT - 1));
            
            // Calculate Color based on iteration "age"
            Color c = get_heat_color(i, ITERATIONS);

            // Draw
            canvas.set_pixel(col, row, c);

            // Iterate Map
            x = lam * x * (1.0 - x);
        }

        // 3. Save Frame
        // Optimization: Saving 1920 frames is slow. 
        // Let's save every 2nd frame to speed up rendering (960 frames total)
        if (col % 2 == 0) {
            std::ostringstream filename;
            // Frame number maps to column index
            filename << "/tmp/frame_" << std::setw(4) << std::setfill('0') << (col / 2) << ".ppm";
            canvas.save_ppm(filename.str());
        }

        if (col % 100 == 0) {
            std::cout << "Progress: " << col << "/" << WIDTH << " (Lambda=" << lam << ")\r" << std::flush;
        }
    }

    std::cout << "\nGeneration complete." << std::endl;
    return 0;
}