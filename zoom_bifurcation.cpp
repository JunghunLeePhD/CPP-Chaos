#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <string>
#include <iomanip>
#include <sstream>

// --- Configuration ---
const int WIDTH = 1280;
const int HEIGHT = 720;
const int FRAMES = 500;       

// --- HIGH DENSITY Iteration Settings ---
const int BASE_ITERATIONS = 10000;    // Start with a very solid image
const double ITERATION_SCALE = 20.0;  // Aggressive scaling
// Math: At 100,000x zoom, this runs ~2,000,000 iterations per pixel column.

// --- Steady Zoom Targets ---
// Period 3 Accumulation Point
const double CENTER_LAM = 3.8540779635; 
const double CENTER_X = 0.500;

const double START_LAM_WIDTH = 3.0;
const double START_X_HEIGHT  = 1.2;
const double ZOOM_FACTOR = 50000.0; 

// --- Color ---
struct Color { uint8_t r, g, b; };
const Color WHITE = {255, 255, 255};

Color get_heat_color(int i, int current_max_i) {
    double t = static_cast<double>(i) / current_max_i;
    
    // Aggressive Gamma Correction
    // With 2 million iterations, the early "blue" points are a tiny fraction.
    // We use pow(t, 0.5) to stretch the blue/purple range so it remains visible.
    t = std::pow(t, 0.5); 
    
    if (t < 0) t = 0; if (t > 1) t = 1;
    
    uint8_t r = static_cast<uint8_t>(255 * t);
    uint8_t g = 0;
    uint8_t b = static_cast<uint8_t>(255 * (1.0 - t));
    return {r, g, b};
}

class Canvas {
    std::vector<Color> pixels;
    int width, height;
public:
    Canvas(int w, int h) : width(w), height(h), pixels(w * h, WHITE) {}

    void clear() { std::fill(pixels.begin(), pixels.end(), WHITE); }

    void set_pixel(int x, int y, Color c) {
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
    std::cout << "Initializing HIGH DENSITY Adaptive Zoom..." << std::endl;
    Canvas canvas(WIDTH, HEIGHT);

    for (int frame = 0; frame < FRAMES; ++frame) {
        
        double t = static_cast<double>(frame) / (FRAMES - 1);

        // 1. Calculate Zoom
        double current_zoom = std::pow(ZOOM_FACTOR, t);

        // 2. Calculate ADAPTIVE Iteration Count
        // Will reach approx 2,000,000 at the end
        long long current_iter_limit = BASE_ITERATIONS + static_cast<long long>(current_zoom * ITERATION_SCALE);

        // 3. Viewport Bounds (Fixed Center)
        double cur_lam_width = START_LAM_WIDTH / current_zoom;
        double cur_x_height  = START_X_HEIGHT / std::pow(current_zoom, 0.85); 

        double min_lam = CENTER_LAM - cur_lam_width / 2.0;
        double max_lam = CENTER_LAM + cur_lam_width / 2.0;
        double min_x   = CENTER_X - cur_x_height / 2.0;
        double max_x   = CENTER_X + cur_x_height / 2.0;

        canvas.clear();

        // Render Loop
        for (int col = 0; col < WIDTH; ++col) {
            
            double col_t = static_cast<double>(col) / (WIDTH - 1);
            double lam = min_lam + col_t * (max_lam - min_lam);
            double x = 0.5; 

            // Dynamic Loop Length
            for (long long i = 0; i < current_iter_limit; ++i) {
                x = lam * x * (1.0 - x);
                
                if (x >= min_x && x <= max_x) {
                    double row_t = (x - min_x) / (max_x - min_x);
                    int row = static_cast<int>(row_t * (HEIGHT - 1));
                    
                    // Use the full iteration limit for coloring to maintain gradient consistency
                    canvas.set_pixel(col, row, get_heat_color(i, current_iter_limit));
                }
            }
        }

        std::ostringstream filename;
        filename << "/tmp/frame_" << std::setw(4) << std::setfill('0') << frame << ".ppm";
        canvas.save_ppm(filename.str());

        if (frame % 10 == 0) {
            std::cout << "Frame " << frame << "/" << FRAMES 
                      << " [Zoom x" << (int)current_zoom 
                      << " | Iters: " << current_iter_limit << "]\r" << std::flush;
        }
    }

    std::cout << "\nHigh Density Zoom complete." << std::endl;
    return 0;
}