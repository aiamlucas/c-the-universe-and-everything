# Complex Numbers and Fractals

Fractals like the **Mandelbrot** and **Julia sets** are visual representations of complex mathematics.  
To understand how they’re generated, we need to understand **complex numbers**.

---

## 1. What Are Complex Numbers?

A **complex number** is a number that has both a **real part** and an **imaginary part**.

```
z = a + bi
```

Where:
- `a` is the **real part**
- `b` is the **imaginary part**
- `i` is the **imaginary unit**, defined by the property ``` i² = -1 ```

Imaginary numbers allow us to represent values that don’t exist on the traditional number line.  
For example, there's no real number whose square is -1, but in complex math, we define such a number as `i`.

```
i² = -1
```

---

## 2. What Is the Complex Plane?

The **complex plane** is a 2D coordinate system used to visualize complex numbers:

- The horizontal axis represents the **real part**.
- The vertical axis represents the **imaginary part**.

Each complex number is a **point** in this 2D space.

Example:

```
z = 3 + 4i → 3 (real), 4 (imaginary)
```

Visual layout:

```
     ^
     |        ● (3 + 4i)
     |        .
  Im |     .
     |  .
     +------------------>
         Real
```

Why use a plane?

- It bridges **algebra** with **geometry**: operations like multiplication become **rotations and scalings**.
- It enables iterative functions (like ``` z = z² + c ```) to be interpreted **visually**, pixel-by-pixel.

---

## 3. Complex Iteration

A key idea in many fractals is **iteration**: applying the same function repeatedly.

```
zₙ₊₁ = zₙ² + c
```

Depending on the initial value `z₀` and parameter `c`, the sequence can:

- Escape to infinity (diverge)
- Stay bounded (converge or orbit in a loop)

This behavior is used to **color** each pixel on the screen.

### Mandelbrot Set

- Fix ``` z₀ = 0 ```
- Vary `c` for each pixel (based on screen coordinates)
- If `|z|` exceeds 2 → escape → color based on how fast

### Julia Set

- Fix `c` as a constant (input parameter)
- Vary `z₀` per pixel
- Very different results depending on the constant `c`

---

## 4. Geometric Meaning of Complex Multiplication

Complex multiplication combines **scaling and rotation**.

For example:

```
z = r * e^(iθ)   →  polar form (Euler's formula)
z² = r² * e^(2iθ)
```

This means:

- Squaring a complex number **doubles its angle** and **squares its magnitude**
- The result is a **spiral-out** or **spiral-in** transformation depending on |z|

In fractals:
- Each iteration rotates and stretches points.
- Spirals, bulbs, and branches emerge from these repeated transformations.

---

## 5. Dimensional Control and Zooming

When you zoom into a fractal, you’re magnifying a **tiny region** of the complex plane.

- Each zoom level reveals **new, unique structure**
- Patterns do not repeat, they are **infinitely complex**
- This is called **self-similarity**, but it's not *identical* self-similarity like a square grid

Example:
- Zooming into the Mandelbrot boundary reveals **Julia-like shapes**
- These are often referred to as **“baby Mandelbrots”**

---

## 6. Dimensionality and Fractals

Fractals often have **non-integer dimensions**.

| Shape   |           Dimension |
|---------|---------------------|
| Line    | 1D                  |
| Plane   | 2D                  |
| Cube    | 3D                  |
| Fractal | 1.3D–2.0D (depends) |

This fractional measure is called the **Hausdorff dimension**.  
It quantifies how a fractal “fills” space:

- A **line** is 1D (grows linearly)
- A **square** is 2D (area = side²)
- A **fractal** might grow faster than 1D but slower than 2D

### Example: Koch Snowflake

- Start with a triangle.
- Recursively replace each side with a "bump".
- The **perimeter grows infinitely** but **area stays finite**.
- Hausdorff dimension: ≈ 1.2619

---

## 7. Julia Set Connectivity and Dimension

Depending on the constant `c`, the Julia set can be:

### Connected

- Appears as a single coherent shape (spirals, blobs)
- Happens when `c` is inside the Mandelbrot set
- Hausdorff dimension typically between 1.0 and 2.0

### Disconnected ("Cantor Dust")

- Appears as isolated point (like static)
- Happens when `c` is outside the Mandelbrot set
- Dimension often < 1.0 It doesn't fill space

---

## 8. Mandelbrot Set and Dimensionality

- The Mandelbrot set itself is **connected** and **compact**
- Its **area is finite**, but its **boundary is infinitely detailed**
- The boundary has Hausdorff dimension = **2.0**

This means:
- It “almost” fills 2D space, like a sponge filled with detail
- Its edge contains *all* complexity, including infinite small Julia sets

---

## 9. Summary

- Complex numbers allow geometry + algebra to mix.
- Iterating ``` z = z² + c ``` builds feedback loops that create fractals.
- The Mandelbrot and Julia sets are **different views** of the same core idea.
- Dimensionality in fractals isn’t just visual, it's measurable through Hausdorff metrics.

---


## One idea how to approach the `fract-ol` project

---

### Step 0: Understand the Fractals

Before coding, understand what you're plotting:

**Mandelbrot**
- Iterate: z = z² + c
- Start: z = 0, c = pixel coordinate
- Escape condition: |z| > 2

**Julia**
- Iterate: z = z² + c
- Start: z = pixel coordinate, c = constant (input)

Visuals come from iterating and checking escape speed.

---

### Step 1: Project Setup

**Goal**: Set up a minimal buildable project

1. Create a new folder/repo
2. Setup file structure:
    - ``` fractol.c ```
    - ``` fractol.h ```
    - ``` Makefile ```
    - ``` src/ ```
    - ``` minilibx-linux/ ```

3. In the Makefile:
    - Use ``` -lmlx_Linux -lXext -lX11 -lm ```
    - Ensure compilation works

Test:
- Write a ``` main() ``` that just opens a blank window and closes on ESC.

---

### Step 2: Initialize MiniLibX

**Goal**: Open an MLX window with title

1. Use ``` mlx_init(), mlx_new_window() ```
2. Use ``` mlx_loop() ``` to keep it open
3. Add ESC key to exit with ``` mlx_hook() ```

Test:
- Build and verify that ESC closes the window.

---

### Step 3: Draw Pixels on Image

**Goal**: Render to a pixel buffer and show it

1. Create ``` mlx_new_image() ```
2. Get data pointer with ``` mlx_get_data_addr() ```
3. Write ``` my_pixel_put(x, y, color) ```
4. Use ``` mlx_put_image_to_window() ``` to display it

Test:
- Fill the screen with random colors or a gradient.

---

### Step 4: Implement Mandelbrot Set

**Goal**: Render Mandelbrot using math

---

#### 4.1 Define Window Size

In `fractol.h`, define your rendering resolution:

```
#define WIDTH 800
#define HEIGHT 800
```

This creates an 800×800 pixel canvas.

---

#### 4.2 Understand the Mandelbrot Complex Plane

The Mandelbrot set is rendered in this region of the complex plane:

```
Real part (cx):       from -2.0 to +2.0
Imaginary part (cy):  from -2.0 to +2.0
```

This corresponds to a square region: `[-2.0, 2.0] × [-2.0, 2.0]`.

> Why this range?

Because:

- **The Mandelbrot set is bounded.**
- Any `c` that causes the iterative sequence `zₙ₊₁ = zₙ² + c` to diverge (escape to infinity) will do so quickly **once `|z| > 2`**.
- So there's no need to compute values outside a radius of 2 from the origin.

This means:
- All interesting behavior (bounded vs. escaping) occurs inside this box.
- It covers the entire set and some surrounding space for visualization.

We **convert pixel coordinates `(x, y)` into complex numbers `(cx, cy)`** that fall within this range, so each pixel represents a unique `c`.

Once mapped, we iterate the function:

```
z₀ = 0
zₙ₊₁ = zₙ² + c
```

If `|zₙ| > 2` at any point, we say the point **escapes**.  
If the value stays bounded (never escapes) after `MAX_ITER` steps, it’s **considered part of the Mandelbrot set**.

---

#### 4.3 Convert Pixels to Complex Coordinates

You can map the pixel grid to complex numbers using **linear interpolation**.

```
double map(double unscaled, double new_min, double new_max, double old_min, double old_max)
{
    return (new_max - new_min) * (unscaled - old_min) / (old_max - old_min) + new_min;
}
```

And use it like:

```
cx = map(x, -2.0, 2.0, 0, WIDTH);
cy = map(y,  2.0, -2.0, 0, HEIGHT);
```

> The `cy` mapping uses `2.0 → -2.0` because screen Y-coordinates increase **downward**, while the imaginary axis increases **upward**.

---

#### 4.4 Iterate the Mandelbrot Function

Each pixel represents a complex number `c = cx + i*cy`. To test if `c` belongs to the Mandelbrot set:

```
... 
double zx_sq = zx * zx;
double zy_sq = zy * zy;

while (i < max_iter)
{
    zx_sq = zx * zx;
    zy_sq = zy * zy;

    if (zx_sq + zy_sq > 4)
        break;

    tmp = zx_sq - zy_sq + cx;
    zy = 2 * zx * zy + cy;
    zx = tmp;

    i++;
}
..
```

- Stop early if `zx² + zy² > 4`: this means the value **escaped**.
- Otherwise, after the max iterations, consider it **part of the set**.

---

#### 4.5 Color the Pixel

Use the number of iterations to assign a color:

```
int color = map(i, 0x000000, 0xFFFFFF, 0, MAX_ITER);
pixel_put(x, y, &img, color);
```

---

#### 4.6 Test Your Result

Once implemented, you should see a Mandelbrot "beetle"-like shape centered in the window.

If your output looks off:

- **Upside down** → you inverted the `cy` mapping incorrectly.
- **Stretched/squashed** → use equal `WIDTH` and `HEIGHT`.
- **Off-center** → review your `shift_x`/`shift_y` or mapping range.

---

### Step 5: Add Julia Set

**Goal**: Implement rendering for Julia fractals

1. Detect ``` argv[1] == "julia" ```
2. Parse ``` argv[2], argv[3] ``` as double values
3. Use ``` z = pixel coordinate ```, ``` c = constant ```
4. Reuse the same iteration code

Test:
- Try ``` ./fractol julia -0.8 0.156 ``` and other values.

```
./fractol julia -0.8 0.156	Classic “spiral nebula” look
./fractol julia 0.285 0.01	Spiderweb-like symmetric shapes
./fractol julia 0.285 0.0	Starburst with symmetry
./fractol julia 0.285 0.01	Slightly twisted variation
./fractol julia -0.4 0.6	Multiple filaments branching out
./fractol julia -0.70176 -0.3842	Lightning-like fractal arms
./fractol julia 0.3 0.5	Bulbous shapes with fine detail
./fractol julia -0.835 -0.2321	Spiral with multiple layers
```
---

### Step 6: Add Navigation

**Goal**: Move view using arrow keys

1. Use ``` mlx_hook() ``` with ``` KeyPress ```
2. On arrow keys:
    - Update ``` shift_x, shift_y ```
    - Re-render fractal

3. Add iteration increase/decrease with ``` +/- ``` keys

Test:
- You can explore different areas of the fractal.

---

### Step 7: Add Zoom with Mouse

**Goal**: Zoom in/out with scroll wheel

1. Add ``` mlx_hook() ``` for ``` ButtonPress ```
2. On scroll:
    - Multiply ``` zoom *= 0.95 or *= 1.05 ```
    - Optionally shift the view toward mouse

3. Improve zoom by updating shift to center zoom on cursor:
    - Optional bonus!

Test:
- Fractal zooms in and out properly.

---

### Step 8: Optimize Rendering

**Goal**: Avoid unnecessary recalculations

1. Precompute scale factors:
    - ``` double scale_x = 4.0 / WIDTH * zoom; ```
2. Avoid recomputing ``` zx * zx + zy * zy ``` repeatedly
3. Optimize ``` map() ``` use

Optional:
- Add bailout optimization for large |z| values early

---

### Step 9: Handle User Input

**Goal**: Validate command-line arguments

1. Accept:
    - ``` ./fractol mandelbrot ```
    - ``` ./fractol julia <x> <y> ```
2. Print usage message on error

Example:

```
Please enter:
  ./fractol mandelbrot
  ./fractol julia <value_1> <value_2>
```

---

### Step 10: Bonus Features

Optional bonuses for extra points:

- [ ] Add **Burning Ship** or other fractal
- [ ] Make zoom follow **mouse position**
- [ ] Animate color shifts (e.g. color += offset)
- [ ] Track Julia constant with mouse (MotionNotify)

---

### Step 11: Refactor & Organize

1. Split into modules:

    - ``` fractol.c ``` → main
    - ``` mandelbrot.c ```
    - ``` julia.c ```
    - ``` events.c ```
    - ``` render.c ```

2. Keep header organized: constants, structs, function prototypes
3. Write helper functions: ``` atodbl(), ft_strncmp(), ft_putstr_fd() ```

---

### Step 12: Test & Polish

- [ ] Try extreme zoom
- [ ] Validate memory (Valgrind)
- [ ] Test invalid inputs
- [ ] Comment key functions
- [ ] Clean up Makefile

---
