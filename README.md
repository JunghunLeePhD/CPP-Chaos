# CPP-Chaos: High-Performance Logistic Map Visualizations

 **CPP-Chaos** is a high-performance C++ visualization engine for exploring Chaos Theory. It focuses on the **Logistic Map** (xn+1​=λxn​(1−xn​)) and its connection to **Julia Sets** in the complex plane.By leveraging optimized C++ pixel manipulation and **FFmpeg**, this project generates high-resolution, high-framerate scientific animations that are computationally expensive to produce in interpreted languages like Python.***

## 🚀 Run Instantly in GitHub Codespaces (Recommended)

 You can run this project directly in your web browser. GitHub Codespaces will automatically set up the C++ compiler, FFmpeg, and all dependencies for you.

### Step 1: Launch the Environment

1. Click the green **Code** button at the top right of this repository.

2. Select the **Codespaces** tab.

3. Click **"Create codespace on main"**.

4. _Wait a moment for the container to build and initialize._

### Step 2: Generate a Movie

Once the terminal is ready, run the entrypoint script with your desired mode. For example, to see the chaos fractal zoom:Bash    bash entrypoint.sh zoom

### Step 3: Watch the Result

1. When the script finishes, a file (e.g., `logistic_fractal_zoom.mp4`) will appear in the **File Explorer** on the left side of the screen.

2. **Right-click** the file and select **Download**.

3. Open the file on your local computer to watch the animation.***

## 🛠️ Run on Local Machine (Docker)

 If you have Docker installed, you can run this locally without installing C++ or FFmpeg on your host machine.

### 1. Build the Image

 ```bash
 docker build -f .devcontainer/Dockerfile -t chaos-movie-maker .
 ```

### 2. Run the Container

Map your current directory to the container so the video file is saved to your hard drive.Bash    # Syntax: docker run --rm -v "$(pwd):/workspace" chaos-movie-maker bash entrypoint.sh <MODE> [PARAMS]

    # Example: Generate the Julia Set movie
    docker run --rm -v "$(pwd):/workspace" chaos-movie-maker bash entrypoint.sh chaos***
========================================================================================

## 🎥 Visualization Modes

 The engine supports 5 distinct visualization modes. Run the script using `bash entrypoint.sh <mode>`.
 | Mode        | Command          | Description                                                                                                                                             |
| ----------- | ---------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **Orbit**   | `orbit <lambda>` | **Cobweb Plot Dynamics.** Visualizes the trajectory of a single orbit for a specific parameter λ. _Example:_ `bash entrypoint.sh orbit 3.9`             |
| **Sweep**   | `sweep`          | **Bifurcation Evolution.** Animates the structural change of the attractor as λ increases from 2.8 to 4.0 (Period Doubling → Chaos).                    |
| **Rainbow** | `rainbow`        | **Rainbow Bifurcation.** A scanning visualization where iterations are colored from Blue (start) to Red (end) to show the time-evolution of the system. |
| **Zoom**    | `zoom`           | **Fractal Zoom.** Performs a deep zoom into the self-similar structures of the Bifurcation Diagram (specifically the Period-3 window).                  |
| **Chaos**   | `chaos`          | **Julia Sets.** Visualizes the "Filled Julia Set" in the complex plane (zn+1​=λzn​(1−zn​)), showing the boundary of bounded orbits.                     |Export to Sheets***

## 🧠 Mathematical Background

# This project explores the transition from order to chaos through two lenses:

1) **The Real Line (Bifurcation):**
    $$
        x_{n+1}​=λx_n​(1−x_n​)
    $$
    This maps the population density over discrete time steps. We observe period-doubling bifurcations accumulating at the Feigenbaum point ($\lambda \approx 3.5699$), followed by chaotic regimes.

2) **The Complex Plane (Julia Sets):**
    $$
        z_{n+1}​=λz_n​(1−z_n​)
    $$
    By extending the variable to the complex plane ($z \in \mathbb{C}$), we visualize the **Julia Set**—the boundary between points that remain bounded and points that escape to infinity. This reveals that the simple 1D bifurcation diagram is actually a cross-section of a complex fractal structure.
