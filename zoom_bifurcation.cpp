#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <string>
#include <iomanip>
#include <sstream>

// --- Configuration ---
const int WIDTH = 800;        
const int HEIGHT = 600;       
const int FRAMES = 450;       // Total frames in the zoom movie
const int ITERATIONS = 800;   // High iterations needed to see detail when zoomed

// --- Zoom Targets (Period 3 Window) ---
// We start at the full view and zoom into this specific window
const double START_LAM_MIN = 2.8;
const double START_LAM_MAX = 4.0;
const double TARGET_LAM_MIN = 3.8284;  // Start of Period 3
const double TARGET_LAM_MAX = 3.8571;  // End of Period 3

// We also zoom X slightly to frame the middle branch
const double START_X_MIN = 0.0;
const double START_X_MAX = 1.0;
const double TARGET_X_MIN = 0.45;
const double TARGET_X_MAX = 0.56;

// --- Color Management ---
struct Color { uint8_t r, g, b; };
const Color WHITE = {255, 255, 255};
const Color BLACK = {0, 0, 0};

// Rainbow Heatmap (Blue -> Red)
Color get_heat_color(int i, int max_i) {
    double t = static_cast<double>(i) / max_i;
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

    // Reset canvas to white for the next frame
    void clear() {
        std::fill(pixels.begin(), pixels.end(), WHITE);
    }

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

// Linear interpolation helper
double lerp(double start, double end, double t) {
    return start + t * (end - start);
}

int main() {
    std::cout << "Initializing Zoom Sequence..." << std::endl;
    Canvas canvas(WIDTH, HEIGHT);

    for (int frame = 0; frame < FRAMES; ++frame) {
        
        // 1. Calculate Zoom Factor (Exponential Ease-In)
        // t goes from 0.0 to 1.0 over the course of the movie
        double t = static_cast<double>(frame) / (FRAMES - 1);
        // We square t to make the zoom start slow and speed up (ease-in)
        // or we can use linear. Let's use linear for predictable scaling.
        
        // 2. Calculate Current Viewport Bounds
        double cur_lam_min = lerp(START_LAM_MIN, TARGET_LAM_MIN, t);
        double cur_lam_max = lerp(START_LAM_MAX, TARGET_LAM_MAX, t);
        double cur_x_min   = lerp(START_X_MIN, TARGET_X_MIN, t);
        double cur_x_max   = lerp(START_X_MAX, TARGET_X_MAX, t);

        canvas.clear();

        // 3. Render the entire diagram for this frame
        // We iterate over screen columns (pixels)
        for (int col = 0; col < WIDTH; ++col) {
            
            // Map pixel col -> Lambda
            double col_t = static_cast<double>(col) / (WIDTH - 1);
            double lam = cur_lam_min + col_t * (cur_lam_max - cur_lam_min);

            double x = 0.5; // Always start at critical point

            // Run orbit
            for (int i = 0; i < ITERATIONS; ++i) {
                x = lam * x * (1.0 - x);
                
                // Map value x -> pixel row
                // We must check if x is within our CURRENT zoomed view
                if (x >= cur_x_min && x <= cur_x_max) {
                    double row_t = (x - cur_x_min) / (cur_x_max - cur_x_min);
                    int row = static_cast<int>(row_t * (HEIGHT - 1));
                    
                    Color c = get_heat_color(i, ITERATIONS);
                    canvas.set_pixel(col, row, c);
                }
            }
        }

        // 4. Save Frame
        std::ostringstream filename;
        filename << "/tmp/frame_" << std::setw(4) << std::setfill('0') << frame << ".ppm";
        canvas.save_ppm(filename.str());

        if (frame % 10 == 0) {
            std::cout << "Rendered Frame " << frame << "/" << FRAMES 
                      << " (Zoom: " << std::fixed << std::setprecision(4) 
                      << (START_LAM_MAX - START_LAM_MIN) / (cur_lam_max - cur_lam_min) 
                      << "x)\r" << std::flush;
        }
    }

    std::cout << "\nZoom generation complete." << std::endl;
    return 0;
}