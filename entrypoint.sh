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
    echo "  1. ./entrypoint.sh orbit <lambda>   (e.g., 3.9) -> Visualizes cobweb for specific lambda"
    echo "  2. ./entrypoint.sh sweep            -> Visualizes changing lambda from 2.8 to 4.0"
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

  *)
    usage
    ;;
esac

cleanup
echo "SUCCESS! Saved to: $OUTPUT"