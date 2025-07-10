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
- `i` is the **imaginary unit**, defined by the property ```i² = -1```

Imaginary numbers allow us to represent values that don’t exist on the traditional number line.  
For example, there's no real number whose square is -1, but in complex math, we define such a number as `i`.

```
i² = -1
```

You can’t find `i` on the real number line.. It’s a purely mathematical construct.  


## 2. What Is the Complex Plane?

The **complex plane** is a 2D plane used to visualize complex numbers:

- The horizontal axis represents the **real part**.
- The vertical axis represents the **imaginary part**.

Each complex number is thus a **point** in this 2D space.

Example:

```
z = 3 + 4i → 3 (real), 4 (imaginary)
```


```
     ^
     |        ● (3 + 4i)
     |        .
  Im |     .
     |  .
     +------------------>
         Real
```

Why is it called the *complex* plane?

- Because each point carries **two components** (real + imaginary), unlike the real number line which is **1-dimensional**.
- It bridges **algebra** with **geometry**: complex number operations like multiplication become geometric transformations (rotation and scaling).
- It enables 2D iteration with meaningful feedback, like detecting divergence.

---

## 3. Dimensionality and Fractals
Fractals often have **non-integer dimensions**.

- A **line** is 1D.
- A **plane** is 2D.
- A **cube** is 3D.

Fractals can be **1.3D**, **1.5D**, or anything in-between. This is called their **Hausdorff dimension** (a measure of how completely they fill space).

---

### What It Means?

- Fractals **don’t neatly fit** into regular geometry.
- They may **look like curves**, but contain too much detail to be just 1D.
- They may **cover area**, but not fully like a square.

This is why fractals are said to have **fractional dimensions**, their complexity lies between classic shapes.

---

```Note:``` The **Hausdorff dimension** is a mathematical way of measuring how much space a fractal actually “occupies,” even if it doesn’t fully fill a 2D region.

### Example: The Koch Snowflake

Start with a triangle and keep recursively replacing each side with smaller bumps.

- The perimeter grows infinitely.
- The area stays finite.
- The resulting shape has a dimension between 1 and 2.

Fractals like Mandelbrot and Julia sets, although drawn in a 2D plane, exhibit **self-similarity and recursive structure** that behaves like a fractional dimension.

---

## 4. Fractals in Nature and Math

Here are some real-world and mathematical examples of fractals:

| Fractal Name            | Source / Description                                     |
|-------------------------|----------------------------------------------------------|
| **Mandelbrot Set**      | Iteration of ```z = z² + c```; diverges or stays   |
| **Julia Set**           | Similar to Mandelbrot but varies ```c``` per image |
| **Koch Snowflake**      | Recursive triangle pattern with infinite edge            |
| **Sierpinski Triangle** | Triangle recursively subdivided                          |
| **Barnsley Fern**       | Uses affine transformations to simulate a fern           |
| **Romanesco Broccoli**  | Naturally self-similar structure in plants               |
| **Lichtenberg Figures** | Lightning-like paths formed by electricity               |
| **Lung Bronchial Tree** | Biological fractal structure                             |

---

## 5. Complex Iteration

A key idea in many fractals is **iteration**: applying the same function over and over.

```
zₙ₊₁ = zₙ² + c
```

Depending on the initial point `z₀` and parameter `c`, the sequence might:
- Escape to infinity (diverge)
- Stay within bounds (converge or orbit)

This divergence behavior is used to **color** each point on the complex plane.

### Mandelbrot Set

- Fix `z₀ = 0`
- Vary `c` based on pixel position
- Track whether the sequence diverges

### Julia Set

- Fix a complex constant `c`
- Vary `z₀` based on pixel
- Different `c` values generate very different patterns

---

## 6. Geometric Meaning of Multiplication in Complex Numbers

Complex multiplication combines **rotation** and **scaling**:

- ```z * z → rotates and stretches```
- This is why fractal zooms twist and spiral in
- Visual harmony emerges from consistent transformations

---

## 7. Dimensional Control and Zooming

When you zoom in on a fractal, you're magnifying a **very small region** of the complex plane.

- Due to infinite self-similarity, each zoom reveals **more detail**, without ever repeating.
- This is not just visual — the **math really does produce unique patterns** at each scale.

Example:
- Zooming on Mandelbrot’s edge reveals **Julia-like sets**
- These are sometimes referred to as “baby Mandelbrots”

---

## 8. Tips for the fractol project!

In the `fractol` project, every pixel corresponds to a **complex number** on the complex plane.

1. **Map** each screen pixel (x, y) to a complex number `z` or `c` (depending on the fractal).
2. **Iterate** a complex formula like `z = z² + c`.
3. **Color** the pixel based on how quickly the value “escapes”, or how fast its magnitude exceeds a threshold (typically 2).

This iterative process builds fractals that exhibit infinite complexity and self-similarity. With this approach, you can:
- Zoom forever into the image
- Alter parameters to explore new shapes
- Create effects using custom formulas

---

### Common Fractal Formulas

| Fractal Type     | Formula                         | Per-Pixel Variable | Constant Parameter | Description                          |
|------------------|---------------------------------|--------------------|--------------------|--------------------------------------|
| Mandelbrot Set   | `z = z² + c`                    | `c = pixel`        | `z = 0`            | The classic recursive set            |
| Julia Set        | `z = z² + c`                    | `z = pixel`        | `c = fixed`        | Varies shape based on `c`            |
| Burning Ship     | `z = (|Re(z)| + i|Im(z)|)² + c` | `c = pixel`        | `z = 0`            | Creates flame-like structures        |
| Multibrot        | `z = zⁿ + c` (e.g. `n = 3`)     | `c = pixel`        | `z = 0`            | Generalization of Mandelbrot         |
| Custom           | `z = sin(z) + c`                | `c` or `z`         | Depends            | Experiment! Try log, exp, etc.       |

---

