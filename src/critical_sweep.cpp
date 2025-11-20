#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <string>
#include <iomanip>
#include <sstream> 

// --- Configuration ---
const int WIDTH = 800;
const int HEIGHT = 800;
const int ITERATIONS = 100;  // More iterations to see the "dense" chaos better
const int FRAMES = 600;      // More frames for a smoother transition
const double START_LAM = 2.0;
const double END_LAM = 4.0;

// --- Colors ---
struct Color { uint8_t r, g, b; };
const Color WHITE = {255, 255, 255};
const Color BLACK = {0, 0, 0};
const Color GRAY = {220, 220, 220};
const Color BLUE = {0, 100, 255};

class Image {
    std::vector<Color> pixels;
    int width, height;
public:
    Image(int w, int h) : width(w), height(h), pixels(w * h, WHITE) {}
    void set_pixel(int x, int y, Color c) {
        if (x >= 0 && x < width && y >= 0 && y < height) pixels[y * width + x] = c;
    }
    void draw_line(int x0, int y0, int x1, int y1, Color c) {
        int dx = std::abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
        int dy = -std::abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
        int err = dx + dy, e2;
        while (true) {
            set_pixel(x0, y0, c);
            if (x0 == x1 && y0 == y1) break;
            e2 = 2 * err;
            if (e2 >= dy) { err += dy; x0 += sx; }
            if (e2 <= dx) { err += dx; y0 += sy; }
        }
    }
    void save_ppm(const std::string& filename) {
        std::ofstream ofs(filename, std::ios::binary);
        ofs << "P6\n" << width << " " << height << "\n255\n";
        for (const auto& p : pixels) ofs << p.r << p.g << p.b;
        ofs.close();
    }
};

// Math Helpers
double f(double x, double lam) { return lam * x * (1.0 - x); }
int to_screen_x(double x) { return static_cast<int>(x * (WIDTH - 1)); }
int to_screen_y(double y) { return HEIGHT - 1 - static_cast<int>(y * (HEIGHT - 1)); }

void draw_cobweb(Image& img, double x0, double lam, Color c) {
    double x = x0;
    int sx = to_screen_x(x);
    int sy = to_screen_y(0);
    
    // Draw initial line up to the curve
    int first_sy = to_screen_y(f(x, lam));
    img.draw_line(sx, sy, sx, first_sy, c);
    sy = first_sy;

    for (int i = 0; i < ITERATIONS; ++i) {
        double next_x = f(x, lam);
        
        // Horizontal: (x, f(x)) -> (f(x), f(x))
        int diag_sx = to_screen_x(next_x);
        int diag_sy = to_screen_y(next_x); // on diagonal, y = x
        img.draw_line(sx, sy, diag_sx, diag_sy, c);

        // Vertical: (f(x), f(x)) -> (f(x), f(f(x)))
        double next_y = f(next_x, lam);
        int curve_sx = diag_sx;
        int curve_sy = to_screen_y(next_y);
        img.draw_line(diag_sx, diag_sy, curve_sx, curve_sy, c);

        x = next_x;
        sx = curve_sx;
        sy = curve_sy;
    }
}

int main() {
    std::cout << "Generating sweep from Lambda " << START_LAM << " to " << END_LAM << "...\n";

    for (int frame = 0; frame < FRAMES; ++frame) {
        Image img(WIDTH, HEIGHT);
        
        // Calculate Lambda for this frame
        double t = static_cast<double>(frame) / (FRAMES - 1);
        double current_lam = START_LAM + t * (END_LAM - START_LAM);

        // 1. Draw Static Diagonal
        img.draw_line(to_screen_x(0), to_screen_y(0), to_screen_x(1), to_screen_y(1), GRAY);
        
        // 2. Draw Dynamic Parabola (Height changes with lambda!)
        for (int i = 0; i < WIDTH - 1; ++i) {
            double x_w = static_cast<double>(i) / (WIDTH - 1);
            double x_wn = static_cast<double>(i + 1) / (WIDTH - 1);
            img.draw_line(to_screen_x(x_w), to_screen_y(f(x_w, current_lam)),
                          to_screen_x(x_wn), to_screen_y(f(x_wn, current_lam)), BLACK);
        }

        // 3. Draw Critical Orbit (x0 = 0.5 fixed)
        draw_cobweb(img, 0.5, current_lam, BLUE);

        std::ostringstream filename;
        filename << "/tmp/frame_" << std::setw(4) << std::setfill('0') << frame << ".ppm";
        img.save_ppm(filename.str());
        
        if (frame % 50 == 0) std::cout << "Rendered frame " << frame << " (Lam=" << current_lam << ")\n";
    }
    return 0;
}