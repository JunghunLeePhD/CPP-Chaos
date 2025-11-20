#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <string>
#include <iomanip>
#include <sstream> 

const int WIDTH = 800;
const int HEIGHT = 800;
const int ITERATIONS = 60;
const int FRAMES = 300;

struct Color { uint8_t r, g, b; };
const Color WHITE = {255, 255, 255};
const Color BLACK = {0, 0, 0};
const Color GRAY = {200, 200, 200};
const Color RED = {255, 0, 0};
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

double f(double x, double lam) { return lam * x * (1.0 - x); }
int to_screen_x(double x) { return static_cast<int>(x * (WIDTH - 1)); }
int to_screen_y(double y) { return HEIGHT - 1 - static_cast<int>(y * (HEIGHT - 1)); }

void draw_cobweb(Image& img, double x0, double lam, Color c) {
    double x = x0;
    int sx = to_screen_x(x);
    int sy = to_screen_y(0);
    for (int i = 0; i < ITERATIONS; ++i) {
        double next_x = f(x, lam);
        int curve_sx = to_screen_x(x);
        int curve_sy = to_screen_y(next_x);
        img.draw_line(sx, sy, curve_sx, curve_sy, c);
        int diag_sx = to_screen_x(next_x);
        int diag_sy = to_screen_y(next_x);
        img.draw_line(curve_sx, curve_sy, diag_sx, diag_sy, c);
        x = next_x;
        sx = diag_sx;
        sy = diag_sy;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Error: Please provide the Lambda parameter (e.g., 3.9)\n";
        return 1;
    }

    double lambda = std::stod(argv[1]);
    std::cout << "Generating " << FRAMES << " frames for Lambda = " << lambda << "...\n";

    for (int frame = 0; frame < FRAMES; ++frame) {
        Image img(WIDTH, HEIGHT);
        double current_x0 = static_cast<double>(frame) / (FRAMES - 1);

        // Background
        img.draw_line(to_screen_x(0), to_screen_y(0), to_screen_x(1), to_screen_y(1), GRAY);
        for (int i = 0; i < WIDTH - 1; ++i) {
            double x_w = static_cast<double>(i) / (WIDTH - 1);
            double x_wn = static_cast<double>(i + 1) / (WIDTH - 1);
            img.draw_line(to_screen_x(x_w), to_screen_y(f(x_w, lambda)),
                          to_screen_x(x_wn), to_screen_y(f(x_wn, lambda)), BLACK);
        }

        // Blue: Critical Point Orbit (0.5)
        draw_cobweb(img, 0.5, lambda, BLUE);
        // Red: Sweeping Initial Condition (current_x0)
        draw_cobweb(img, current_x0, lambda, RED);

        std::ostringstream filename;
        // Save to /tmp so we don't clutter the volume
        filename << "/tmp/frame_" << std::setw(4) << std::setfill('0') << frame << ".ppm";
        img.save_ppm(filename.str());
    }
    return 0;
}