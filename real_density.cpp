#include <iostream>
#include <vector>
#include <complex>
#include <cmath>
#include <fstream>
#include <cstdlib>
#include <string>
#include <iomanip>
#include <sstream>

// --- Configuration ---
const int WIDTH = 1280;
const int HEIGHT = 720;
const int FRAMES = 600;
const int POINTS_PER_FRAME = 200000; // More points = brighter/clearer image

// Parameter Sweep
const double LAMBDA_START = 2.5; // Circle
const double LAMBDA_END = 4.1;   // Chaos / Real Line

struct Color { uint8_t r, g, b; };
const Color BLACK = {0, 0, 0};
const Color CYAN = {0, 200, 255};

// Map complex plane to screen
// Viewport: Real [-0.5, 1.5], Imag [-0.8, 0.8]
int to_screen_x(double re) {
    return static_cast<int>((re - (-0.5)) / (1.5 - (-0.5)) * (WIDTH - 1));
}
int to_screen_y(double im) {
    return HEIGHT - 1 - static_cast<int>((im - (-0.6)) / (0.6 - (-0.6)) * (HEIGHT - 1));
}

class Canvas {
    std::vector<Color> pixels;
    int width, height;
public:
    Canvas(int w, int h) : width(w), height(h), pixels(w * h, BLACK) {}

    void clear() { std::fill(pixels.begin(), pixels.end(), BLACK); }

    void set_pixel(int x, int y, Color c) {
        if (x >= 0 && x < width && y >= 0 && y < height) {
            pixels[y * width + x] = c;
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
    std::cout << "Initializing Inverse Julia Animation..." << std::endl;
    Canvas canvas(WIDTH, HEIGHT);
    
    // Seed random for the walker
    srand(42);

    for (int frame = 0; frame < FRAMES; ++frame) {
        
        double t = static_cast<double>(frame) / (FRAMES - 1);
        double lam_val = LAMBDA_START + t * (LAMBDA_END - LAMBDA_START);
        std::complex<double> lambda(lam_val, 0.0);

        canvas.clear();

        // --- INVERSE ITERATION ---
        // We start with a random point and iterate BACKWARDS.
        // Backward iteration naturally falls onto the Julia Set (the repeller).
        
        std::complex<double> z(0.5, 0.0); // Initial guess

        // 1. Warmup (get the point onto the set)
        for (int i = 0; i < 50; ++i) {
            std::complex<double> root = std::sqrt(1.0 - (4.0 * z / lambda));
            if (rand() % 2 == 0) z = (1.0 + root) / 2.0;
            else                 z = (1.0 - root) / 2.0;
        }

        // 2. Draw (Walk along the set)
        for (int i = 0; i < POINTS_PER_FRAME; ++i) {
            // The Inverse Formula for Logistic Map:
            // z = (1 +/- sqrt(1 - 4z/lambda)) / 2
            
            std::complex<double> root = std::sqrt(1.0 - (4.0 * z / lambda));
            
            // Randomly choose which branch to take
            if (rand() % 2 == 0) {
                z = (1.0 + root) / 2.0;
            } else {
                z = (1.0 - root) / 2.0;
            }

            // Plot
            int sx = to_screen_x(z.real());
            int sy = to_screen_y(z.imag());
            canvas.set_pixel(sx, sy, CYAN);
        }

        // Save Frame
        std::ostringstream filename;
        filename << "/tmp/frame_" << std::setw(4) << std::setfill('0') << frame << ".ppm";
        canvas.save_ppm(filename.str());

        if (frame % 10 == 0) {
            std::cout << "Frame " << frame << "/" << FRAMES 
                      << " [Lambda: " << std::fixed << std::setprecision(3) << lam_val << "]\r" << std::flush;
        }
    }

    std::cout << "\nInverse animation complete." << std::endl;
    return 0;
}