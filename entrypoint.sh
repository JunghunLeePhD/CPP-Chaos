#!/bin/bash
set -e

# --- CONFIGURATION ---
MODE=$1
PARAM=$2

# Helper function to clean up temporary files
cleanup() {
    echo ">>> Cleaning up temporary files..."
    rm -f /tmp/frame_*.ppm
    rm -f /tmp/*.ppm
    rm -f ./movie_gen_exec
}

# Display usage helper
usage() {
    echo "Error: Invalid or missing mode."
    echo "Usage:"
    echo "  1. bash entrypoint.sh orbit <lambda>   (e.g., 3.9) -> Visualizes cobweb for specific lambda"
    echo "  2. bash entrypoint.sh sweep            -> Visualizes changing lambda from 2.8 to 4.0"
    echo "  3. bash entrypoint.sh rainbow          -> Generates Bifurcation Movie (scanning left-to-right)"
    echo "  4. bash entrypoint.sh zoom          -> Generates Zoom Bifurcation Movie"
    exit 1
}

# --- MAIN LOGIC ---

case "$MODE" in

  "orbit")
    # ==========================================
    # MODE 1: Fixed Lambda, Moving Initial Point
    # ==========================================
    if [ -z "$PARAM" ]; then
        echo "Error: 'orbit' mode requires a Lambda value."
        echo "Try: ./entrypoint.sh orbit 3.9"
        exit 1
    fi

    echo ">>> [Orbit] Compiling C++ code..."
    g++ -O3 cobweb_movie.cpp -o movie_gen_exec

    echo ">>> [Orbit] Generating Frames (Lambda=$PARAM)..."
    ./movie_gen_exec $PARAM

    OUTPUT="logistic_orbit_lambda_${PARAM}.mp4"
    echo ">>> [Orbit] Rendering Movie: $OUTPUT"
    ffmpeg -y -framerate 30 -i /tmp/frame_%04d.ppm \
        -c:v libx264 -pix_fmt yuv420p "$OUTPUT" -loglevel error
    ;;
 
    "sweep")
    # ==========================================
    # MODE 2: Moving Lambda (Structural Stability)
    # ==========================================
    echo ">>> [Sweep] Compiling C++ code..."
    g++ -O3 critical_sweep.cpp -o movie_gen_exec

    echo ">>> [Sweep] Generating Frames (Lambda Sweep)..."
    ./movie_gen_exec

    OUTPUT="logistic_parameter_sweep.mp4"
    echo ">>> [Sweep] Rendering Movie: $OUTPUT"
    ffmpeg -y -framerate 30 -i /tmp/frame_%04d.ppm \
        -c:v libx264 -pix_fmt yuv420p "$OUTPUT" -loglevel error
    ;;

    "rainbow")
    # ==========================================
    # MODE 3: Rainbow Bifurcation Scanning Movie
    # ==========================================
    echo ">>> [Rainbow] Compiling C++ code..."
    g++ -O3 rainbow_bifurcation.cpp -o movie_gen_exec

    echo ">>> [Rainbow] Painting the chaos..."
    ./movie_gen_exec

    OUTPUT="logistic_rainbow_bifurcation.mp4"
    echo ">>> [Rainbow] Rendering Movie: $OUTPUT"
    # Using 60fps because this mode generates a lot of frames (scans width)
    ffmpeg -y -framerate 60 -i /tmp/frame_%04d.ppm \
        -c:v libx264 -pix_fmt yuv420p "$OUTPUT" -loglevel error
    ;;

    "rainbow_sweep")
    # --- MODE 4: Rainbow Sweep (Projected Lines) ---
    echo ">>> [Rainbow Sweep] Compiling..."
    g++ -O3 rainbow_sweep.cpp -o movie_gen_exec
    
    echo ">>> [Rainbow Sweep] Generating Frames..."
    ./movie_gen_exec
    
    OUTPUT="logistic_rainbow_sweep.mp4"
    echo ">>> [Rainbow Sweep] Rendering Movie: $OUTPUT"
    ffmpeg -y -framerate 30 -i /tmp/frame_%04d.ppm \
        -c:v libx264 -pix_fmt yuv420p "$OUTPUT" -loglevel error
    ;;

    "zoom")
    # --- MODE 5: Fractal Zoom Movie ---
    echo ">>> [Zoom] Compiling..."
    g++ -O3 zoom_bifurcation.cpp -o movie_gen_exec
    
    echo ">>> [Zoom] Rendering Zoom Sequence (This may take a moment)..."
    ./movie_gen_exec
    
    OUTPUT="logistic_fractal_zoom.mp4"
    echo ">>> [Zoom] Stitching Movie: $OUTPUT"
    ffmpeg -y -framerate 30 -i /tmp/frame_%04d.ppm \
        -c:v libx264 -pix_fmt yuv420p "$OUTPUT" -loglevel error
    ;;
    *)
    usage
    ;;
esac

cleanup
echo "SUCCESS! Saved to: $OUTPUT"