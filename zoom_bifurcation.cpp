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
const int FRAMES = 900;        // Long movie to handle the deep zoom smoothly
const int ITERATIONS = 2000;   // High iterations needed for fine details at deep zoom

// --- Deep Zoom Targets ---
// We zoom into a specific substructure within the Period-3 window
const double START_LAM_CENTER = 3.2;
const double START_LAM_WIDTH = 1.6; 

// A specific point known to look like a "mini" bifurcation diagram
const double TARGET_LAM_CENTER = 3.854077963; 
const double TOTAL_ZOOM = 50000.0; // 50,000x magnification

// We also focus Y on the upper branch
const double START_X_CENTER = 0.5;
const double START_X_WIDTH = 1.0;

const double TARGET_X_CENTER = 0.514; // The specific branch we follow
const double TARGET_X_WIDTH = 0.15;   // We tighten the Y-axis too

// --- Color ---
struct Color { uint8_t r, g, b; };
const Color WHITE = {255, 255, 255};

Color get_heat_color(int i, int max_i) {
    // Non-linear color mapping to make the deep details pop
    double t = static_cast<double>(i) / max_i;
    t = std::pow(t, 0.7); // Gamma correction for better contrast
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
    std::cout << "Initializing Deep Zoom (x" << TOTAL_ZOOM << ")..." << std::endl;
    Canvas canvas(WIDTH, HEIGHT);

    for (int frame = 0; frame < FRAMES; ++frame) {
        
        // 1. Exponential Interpolation (The key to deep zooming)
        // t goes from 0 to 1 linear
        double t = static_cast<double>(frame) / (FRAMES - 1);
        
        // ease_t is exponential: This makes the zoom speed visually constant
        double zoom_factor = std::pow(TOTAL_ZOOM, t);

        // Calculate current view width
        double cur_lam_width = START_LAM_WIDTH / zoom_factor;
        double cur_x_width = START_X_WIDTH / std::pow(zoom_factor, 0.5); // Zoom Y slower than X looks better

        // Interpolate centers
        double cur_lam_center = START_LAM_CENTER + (TARGET_LAM_CENTER - START_LAM_CENTER) * t;
        double cur_x_center = START_X_CENTER + (TARGET_X_CENTER - START_X_CENTER) * t;

        // Derived bounds
        double cur_lam_min = cur_lam_center - cur_lam_width / 2.0;
        double cur_lam_max = cur_lam_center + cur_lam_width / 2.0;
        double cur_x_min = cur_x_center - cur_x_width / 2.0;
        double cur_x_max = cur_x_center + cur_x_width / 2.0;

        canvas.clear();

        // 2. Render
        #pragma omp parallel for
        for (int col = 0; col < WIDTH; ++col) {
            double col_t = static_cast<double>(col) / (WIDTH - 1);
            double lam = cur_lam_min + col_t * (cur_lam_max - cur_lam_min);

            double x = 0.5;

            // Iterate
            for (int i = 0; i < ITERATIONS; ++i) {
                x = lam * x * (1.0 - x);
                
                // Check bounds
                if (x >= cur_x_min && x <= cur_x_max) {
                    // Normalize to screen
                    double row_t = (x - cur_x_min) / (cur_x_max - cur_x_min);
                    int row = static_cast<int>(row_t * (HEIGHT - 1));
                    
                    // Draw with transparency simulation (simple overwrite here, but fine for video)
                    canvas.set_pixel(col, row, get_heat_color(i, ITERATIONS));
                }
            }
        }

        // 3. Save
        std::ostringstream filename;
        filename << "/tmp/frame_" << std::setw(4) << std::setfill('0') << frame << ".ppm";
        canvas.save_ppm(filename.str());

        if (frame % 20 == 0) {
            std::cout << "Frame " << frame << "/" << FRAMES 
                      << " [Zoom x" << (int)zoom_factor << "]\r" << std::flush;
        }
    }

    std::cout << "\nDeep Zoom Complete." << std::endl;
    return 0;
}