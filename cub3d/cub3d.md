# cub3D

A first-person ~3D engine written in C, built from scratch.
The world is a 2D grid. The 3D effect comes entirely from raycasting
casting one ray per screen column to find walls, then drawing vertical
strips scaled by distance. No OpenGL, no game engine, no 3D math.

This document focuses on the architecture and rendering pipeline.
Parsing the `.cub` map file is not covered here.

---

## 1. After parsing — closer look in the memory

The parser reads the `.cub` file and produces a single config struct.
After that, the file can be forgotten and only these fields matter:

```
config
├── 4 texture paths        NO, SO, EA, WE   ← strings until MLX loads them
├── floor color            RGB triplet (0–255)
├── ceiling color          RGB triplet (0–255)
├── map                    array of strings, one per row
├── map_width / height
├── player_x / player_y    grid cell coordinates of the spawn point
└── player_dir             'N' | 'S' | 'E' | 'W'
```

The map is stored as `char **map` an array of pointers, each pointing
to one row string. The pointers are contiguous in memory, but each string
lives at its own location:

```
map → [ ptr0 | ptr1 | ptr2 | ptr3 | ptr4 | NULL ]
         ↓      ↓      ↓      ↓      ↓
      "1111" "1000" "100N" "1100" "1111"
      (each string is a separate allocation)
```

Visually the map looks like this:

```
map[0] = "1111111111"
map[1] = "1000000001"
map[2] = "1000N00001"   ← player spawn at grid cell (4, 2)
map[3] = "1100000001"
map[4] = "1111111111"

         '1' = wall    '0' = empty floor    'N/S/E/W' = spawn
```

`player_x` and `player_y` are the grid cell coordinates of the spawn
point read from the map. After `init_player` they get converted to pixel
space and are no longer used directly.

### Map indexing — `map[row][col]` = `map[y][x]`

The map is an array of strings, each entry is one full row of
characters. Indexing follows the natural storage order: outer index
selects the row, inner index selects the character within that row:

```
map[2]      → "1000N00001"            (the entire row at index 2)
map[2][4]   → 'N'                     (character 4 of that row)

         outer index = row    = y
         inner index = column = x

         → map[y][x]
```

Reversing to `map[x][y]` would mean "the y-th character of column x",
which doesn't fit how rows of strings are laid out in memory. Beyond
that, `[row][col]` is the standard convention for matrices and image
buffers across almost all of computing — once the engine starts treating
the screen buffer as `[y][x]` too, having the map use the same
convention keeps everything consistent.

Two more useful properties of this layout:

1. The parser can use string functions (`ft_strlen`, `ft_strchr`, etc.)
   directly on each row.
2. The map is read-only after parsing — nothing modifies it during
   gameplay. No allocations, no extra struct per cell, just two integers
   indexing into a fixed grid.

---

## 2. Two coordinate systems

A raycaster constantly switches between two ways of measuring position:

```
GRID space                            PIXEL space
─────────────────                     ─────────────────
integer cell indices                  exact float position
unit = 1 cell                         unit = 1 pixel
```

The bridge is `BLOCK_SIZE` how many pixels make up one cell. This
implementation uses 64. Conversion is one division or multiplication:

```
grid → pixel:  pixel = grid * BLOCK_SIZE
pixel → grid:  grid  = (int)(pixel / BLOCK_SIZE)
```

Pixel space is what the player lives in: positions are stored as floating-point numbers so
movement is smooth and rays travel through it as continuous lines.
Grid space is what the **map** lives in and it's also where DDA
operates: the algorithm marches the ray cell by cell to find a wall hit,
which is far faster on integer coordinates than on pixels. Both spaces
will keep showing up; the engine constantly converts back and forth.

**Y grows downward** screen convention. North means *decreasing* Y,
south means *increasing* Y. 

---

## 3. MLX — the graphics library

### What MLX is

MLX (MiniLibX) is 42's small graphics library. It's a simplified wrapper
around **X11**, the windowing protocol that has powered Unix and Linux
desktops since 1984. X11 is a client-server design: every graphical
program is a *client* that talks to an *X server*, which owns the screen,
the keyboard, and the mouse. The client sends drawing commands; the
server displays them and forwards input events back.

Working directly with X11 is verbose, opening a single window takes
dozens of lines of boilerplate. MLX collapses the whole thing into a
small set of C functions: open a window, get a pixel buffer, draw,
flush, listen for keys. That's the entire surface area used in cub3D.

### The off-screen buffer pattern

MLX gives us a window, but **drawing pixel-by-pixel directly into the
window is slow and flickers**. The fix is to draw into an off-screen
image first, then push the whole image to the window in a single call:

```c
/* once at startup */
img.img  = mlx_new_image(mlx, WIN_WIDTH, WIN_HEIGHT);
img.addr = mlx_get_data_addr(img.img,
              &img.bits_per_pixel,
              &img.line_len,
              &img.endian);

/* every frame, after the buffer has been drawn into */
mlx_put_image_to_window(mlx, win, img.img, 0, 0);
```

`mlx_new_image` allocates the buffer. `mlx_get_data_addr` returns a
pointer to its first byte, plus three numbers describing its layout.
After that, MLX is out of the way until the flush, drawing is just
direct writes to memory.

### The buffer layout — one big 1D array

The buffer is one long flat array of bytes, laid out row by row:

```
addr →  [px(0,0)][px(1,0)] ... [px(W-1,0)]   [px(0,1)][px(1,1)] ... [px(W-1,1)] ...
         ──────── row 0 ──────────────────   ──────── row 1 ──────────────────
```

The three numbers from `mlx_get_data_addr` are everything needed to
navigate it:

```
bits_per_pixel    32 (= 4 bytes per pixel)
line_len          bytes per row = WIN_WIDTH × 4
addr              pointer to the very first byte
```

Reaching pixel `(x, y)` is one formula:

```
address of (x, y) = addr + y * line_len + x * 4
```

Visually:

```
   "skip y full rows"          "skip x pixels into the row"
        ↓                                  ↓
addr + (y * line_len)        +        (x * 4)
```

For a Full-HD window (1920 × 1080):

```
total pixels     = 1920 × 1080       = 2,073,600
bytes per pixel  = 4
total buffer     = 2,073,600 × 4     = 8,294,400 bytes ≈ 8 MB
line_len         = 1920 × 4          = 7,680 bytes per row
```

### Writing a pixel

A pixel is 4 bytes (32 bits): one byte for blue, one for green, one for
red, one unused. Instead of writing four separate bytes, the address
gets cast to a 32-bit integer pointer and all four bytes are written in
one operation. It's faster and matches MLX's expected format.

The address must be inside the buffer bounds, checking before writing
is mandatory. Writing one pixel past the end is a segfault.

### Packing RGB into an int

Floor and ceiling colors come from the parser as three numbers (R, G, B,
each 0–255). MLX expects one 32-bit integer in the layout:

```
  byte 3      byte 2      byte 1      byte 0
[ unused  ] [    R    ] [    G    ] [    B    ]
 00000000    RRRRRRRR    GGGGGGGG    BBBBBBBB
```

Three bit shifts position each channel, then OR operator combines them:

```
R << 16   slides red into byte 2
G << 8    slides green into byte 1
B         stays in byte 0

color = (R << 16) | (G << 8) | B
```

The result is one integer that can be assigned directly to a pixel
address.

### One canvas, one flush

Everything drawn: floor, ceiling, walls, minimap 
goes into this one buffer.
Once a frame is fully composed, **one** call to
`mlx_put_image_to_window` shows it. The next frame overwrites the
buffer from scratch. No flickering, no per-pixel calls into MLX.

---

## 4. The frame loop — what happens 60 times per second

```
1. draw floor and ceiling      ← repaints the entire buffer
2. for each screen column:
     - cast a ray
     - find the wall it hits (DDA)
     - draw a textured vertical strip
3. draw the minimap on top
4. update player position from keys held
5. flush the buffer to the window
```

Step 1 acts as the buffer clear. Filling the top half with ceiling color
and the bottom half with floor color overwrites everything from the
previous frame. Walls are drawn on top in step 2, covering the
floor/ceiling pixels they need to.

Step 2 is the heart of raycasting. The whole 3D illusion is just
hundreds of vertical strips at different heights, drawn side by side.

---

## 5. Floor and ceiling

Top half of the screen is ceiling, bottom half is floor:

```
y = 0
                    ── ceiling ──             fill every pixel with ceil_color
y = WIN_HEIGHT / 2  ─────────────
                    ── floor ──               fill every pixel with floor_color
y = WIN_HEIGHT - 1
```

Two loops over `y`. Each loop calls a row-fill helper that walks across
all `WIN_WIDTH` pixels and writes the same color. This is a clean,
isolated example of the address arithmetic from section 3 before any
wall math gets involved.

---

## 6. The column-based render (general idea)

Think of the screen as a fence of vertical columns:

```
col 0  col 1  col 2  ...  col WIN_WIDTH-1
  │      │      │              │
  │      │      │              │
  │      │      │              │
```

For every column, **one ray** gets fired from the player into the 2D
world. That ray hits some wall. Based on how far the wall is, a thin
vertical strip is drawn in that column. Taller if the wall is close,
shorter if it's far:

```
            wall close            wall far
              ┌──┐
              │  │
              │  │                ┌──┐
              │  │                └──┘
              └──┘
              col x                col x'
```

That's the entire 3D illusion. No 3D math anywhere, just hundreds of
2D rays and "near things look bigger".

What each column needs from its ray:

- the **distance** to the wall (sets strip height)
- which **face** of the wall got hit (picks the texture: NO/SO/EA/WE)
- **where on the wall** the hit landed (picks which texture column to use)

---

## 7. The player — what's needed to cast rays

To cast 1920 rays per frame, the player needs more than just a position:

```
player
├── x, y            position in pixel space (doubles)
├── angle           where the player faces, in radians
├── dir_x, dir_y    unit vector pointing forward
├── cplane_x/y      camera plane vector (perpendicular to dir)
└── key flags       up, down, left, right, rotate L, rotate R
```

### The angle

One full circle = 2π ≈ 6.283 radians = 360°. Cardinal directions map
to radians like this (Y grows down):

```
         3π/2 (north / up screen)
              ^
              |
   π  --------+--------  0
(west)        |        (east)
              v
          π/2 (south / down screen)

angle = 0    → facing East
angle = π/2  → facing South
angle = π    → facing West
angle = 3π/2 → facing North
```

### The direction vector

Every vector in 2D has two components: how much it moves
horizontally (x) and how much vertically (y). The direction
vector is a unit vector (length exactly 1.0) pointing in
the direction the player faces. 
Given an angle, `cos` and `sin`
give those two components:

```
dir_x = cos(angle)   ← horizontal component
dir_y = sin(angle)   ← vertical component
```
One step forward moves exactly one unit in the direction of
`(dir_x, dir_y)`, regardless of which way the player faces.

### The camera plane

A vector perpendicular to `dir` that represents the width of the
screen projected into the 2D world. Every ray fired this frame
passes through a different point along it:

```
      left edge      center      right edge
          ├────────────┼────────────┤   ← camera plane
           \           |           /
            \          |          /
             \         |         /       rays
              \        |        /
               \       |       /
                \      |      /
                      [P]
```

Always perpendicular to `dir`, scaled by the FOV (Field of View) factor:

```
cplane_x = -sin(angle) × FOV_FACTOR
cplane_y =  cos(angle) × FOV_FACTOR
```

`FOV_FACTOR = 0.66` gives approximately 66°, a typical value
for early first-person games of the 1990s. \
Increase it for a wider view, decrease it for a narrower one.


### Recompute, don't accumulate

Every frame, `dir` and `cplane` are recomputed from `angle` from scratch.
They are never modified directly. The angle wraps to `[0, 2π]` to keep
the number small and prevent floating-point precision loss over time:

```c
/* wrap angle to [0, 2π] before recomputing */
if (game->player.angle > 2 * PI)
    game->player.angle = 0;
if (game->player.angle < 0)
    game->player.angle = 2 * PI;
game->player.dir_x    = cos(game->player.angle);
game->player.dir_y    = sin(game->player.angle);
game->player.cplane_x = -sin(game->player.angle) * FOV_FACTOR;
game->player.cplane_y =  cos(game->player.angle) * FOV_FACTOR;
```

This guarantees they stay perpendicular and unit-length forever
no floating-point drift over thousands of frames.

---

## 8. Movement and sliding collision

Moving the player is two operations:

**Rotation.** Update `angle` by a small constant per frame, then
recompute `dir` and `cplane`. Wrap the angle to `[0, 2π]` so it never
grows huge and loses precision.

**Translation.** Add a fraction of `dir` to the position:

```
forward:    pos += dir × speed
backward:   pos -= dir × speed

speed = MOVEMENT_SPEED = 1 pixel per frame
      → at 60fps: 60 pixels/second ≈ 0.9 grid cells/second
```

For strafing, we need a vector 90° to the side of `dir`. That's just
`dir` rotated by ±π/2 — the same cos/sin trick with a phase shift:

```
strafe_left vector = (cos(angle - π/2), sin(angle - π/2))
```

Visually, for a player facing East (angle = 0):

```
   strafe_left  ↑  (points North)
                |
   West  ←──── P ────→  East  (dir)
                |
   strafe_right ↓  (points South)

lx = cos(angle - π/2) = sin(angle)
ly = sin(angle - π/2) = -cos(angle)
right strafe = (-lx, -ly)  ← just negate, no extra trig needed
```

The player's facing angle does **not** change while strafing — only
the position shifts sideways.

### The sliding trick

The naive collision rule "if the new position is in a wall, don't move"
gets the player stuck on diagonal contact. Better: check **X and Y
independently**.

```
if X axis is clear → move X
if Y axis is clear → move Y
```

Hitting a wall while moving diagonally:

```
   P →→→ [WALL]
    ↘
    X blocked  →  X stays
    Y free     →  Y updates
    ↓
    ↓     ← player slides downward
    P
```

One axis succeeds while the other fails. The player slides along the
wall instead of freezing.

**Look-ahead margin.** The collision check happens a few pixels *ahead*
of the actual move, not at the move destination. Without this margin
the player can clip a pixel into the wall before the check fires. With
it, collision stops the player cleanly with a small visual buffer.

---

## 9. Casting a ray for one column

The player is now set up. For every screen column `x`, the ray
direction is computed by mapping the column index to a value in
`[-1, +1]` along the camera plane:

```
column 0          camera_x = -1     (far left of cplane)
column WIN_W/2    camera_x =  0     (straight ahead)
column WIN_W-1    camera_x ≈ +1     (far right of cplane)
```

Then:

```
ray_dir = dir + cplane × camera_x
```

When `camera_x = 0`, `ray_dir = dir` exactly. When `camera_x = ±1`,
the ray points to the corresponding edge of the camera plane:

```
        camera_x = -1     0     +1
                    \     |     /
                     \    |    /
                      \   |   /
                       \  |  /
                        \ | /
                         \|/
                          P
```

That single formula generates all 1920 ray directions from one `dir`
and one `cplane`. Smooth, evenly fanned across the FOV.

---

## 10. DDA — finding which wall the ray hits

The ray now has a direction. The question: **which wall cell does it
hit first, and how far away is it?**

The brute-force approach: step a tiny amount along the ray, check the cell,
repeat. This is slow and can skip cells when the ray is nearly axis-aligned.
DDA is the better approach: jump directly **from gridline to gridline**.

```
Brute Force:

P ··········································► [WALL]
^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^
check at every tiny step
```

```
DDA algorithm

P ─────┬──┬──┬► [WALL]
│      │  │  │
│      │  │  └── hit wall, stop
│      │  └── crossed Y gridline
│      └── crossed X gridline
└── crossed X gridline

4 checks instead of 40. Exact. Never skips.
```

### The two key quantities

DDA pre-computes two distances that make the algorithm efficient.

**`delta_dist`** how far along the ray we walk to cross **one full cell**:

```
delta_dist_x = 1 / |ray_dir_x|     ← ray-distance per X gridline
delta_dist_y = 1 / |ray_dir_y|     ← ray-distance per Y gridline
```

If `ray_dir_x` is exactly 0 (a perfectly vertical ray), `delta_dist_x`
gets set to a huge number, that ray will never cross an X gridline,
so its X side never gets picked.

**`side_dist`** — how far along the ray to the **next** gridline:

```
            P starts at pos_x = 6.3, ray going right (step_x = +1)

            |─────|─────|─────|─────|
            5     6   6.3 P   7     8

                          next vertical gridline at x = 7
                          distance in X     = 7 - 6.3 = 0.7
                          distance in ray   = 0.7 × delta_dist_x

                          → side_dist_x = 0.7 × delta_dist_x
```

### The step

Each iteration of DDA:

```
1. compare side_dist_x and side_dist_y
2. whichever is smaller → that gridline is closer
3. jump to it: advance map_x or map_y by ±1, depending on ray direction
4. add delta_dist to that side_dist (it now points to the NEXT gridline)
5. record which side was crossed (X-face or Y-face)
6. check map[map_y][map_x] — if it's a wall, stop
```

A walked example, ray going right and slightly down:

```
Iter 1:  side_dist_x smaller → step right, map_x++,  side = X
Iter 2:  side_dist_y smaller → step down,  map_y++,  side = Y
Iter 3:  side_dist_x smaller → step right, map_x++,  side = X  → WALL!
```

The final state of the ray after the loop:

```
ray
├── map_x, map_y         the wall cell that was hit
├── side                  X-face (0) or Y-face (1)
├── step_x, step_y       stepping direction (still ±1)
├── side_dist_x/y         pushed one past the hit
└── delta_dist_x/y        unchanged
```

These five pieces give the wall renderer everything it needs.

---

## 11. From hit to wall column — perspective

We now know which wall got hit and how far. Drawing the strip is three
sub-problems.

### Wall height

The illusion of depth comes from one division:

```
line_height = WIN_HEIGHT / perp_dist
```

`perp_dist` is the **perpendicular distance** measured at a right angle
to the camera plane, not the straight-line distance from the player to
the wall hit point.

```
perp_dist = 0.5  →  strip is twice the screen height  (very close)
perp_dist = 1.0  →  strip fills the screen
perp_dist = 4.0  →  strip is 1/4 the screen height    (far away)
```

### The fisheye fix


If you measure the actual ray length, edge rays travel further than
the center ray, a flat wall bows outward at the edges.
That is the fisheye effect:


```
WRONG — Euclidean                  RIGHT — perpendicular

         /─── wall                          │── wall
        /                                   │
       /                                    │
      / d (longer at edges)                 │ d (same everywhere)
     /                                      │
    P                                       P
```

Instead of the ray length, we use the distance perpendicular to
the camera plane. Every ray projects onto the same forward axis
a flat wall in front always produces equal distances across all
columns.

This distance comes directly from values DDA already computed
no extra math needed:

```
hit on X-side:   perp_dist = side_dist_x - delta_dist_x
hit on Y-side:   perp_dist = side_dist_y - delta_dist_y
```

After DDA exits, `side_dist` is one step past the wall.
Subtracting `delta_dist` rolls it back to the exact hit point
and that distance is automatically perpendicular.

### Centering the strip on screen

Every wall strip is drawn centered on the screen horizon.
`line_height` is the total height of the strip in pixels:

```
y = 0          ─┤
                │  ceiling
draw_start     ─┼──────────┐
                │          │  ← wall strip, line_height pixels tall
draw_end       ─┼──────────┘
                │  floor
y = WIN_HEIGHT ─┤
```

```
draw_start = WIN_HEIGHT / 2 - line_height / 2
draw_end   = WIN_HEIGHT / 2 + line_height / 2
```

A very close wall produces a `line_height` taller than the screen.
In that case `draw_start` and `draw_end` are snapped to the screen
edges before drawing:

```c
col->draw_start = WIN_HEIGHT / 2 - line_height / 2;
if (col->draw_start < 0)
    col->draw_start = 0;
col->draw_end = WIN_HEIGHT / 2 + line_height / 2;
if (col->draw_end >= WIN_HEIGHT)
    col->draw_end = WIN_HEIGHT - 1;
```

The drawing loop uses the corrected values, nothing is drawn
outside the screen. But the texture coordinates are calculated
from the original `draw_start` and `draw_end`, so the texture
is always sampled at the correct height on the wall.

---

## 12. Texture mapping — putting pixels in the strip

Each wall face has a different texture. The texture files are XPM images
a plain-text format where each character in a grid represents one pixel:

```
/* XPM */
static char *north[] = {
"64 64 4 1 ",           ← 64×64 pixels, 4 colors, 1 char per pixel
"  c #000000",          ← color table
". c #020818",
"+ c #030D22",
"M c #FF00FF",
/* pixels */
"++++++++++++++++++++...",   ← row 0
...
```

Two questions for each column:

1. **Which texture?** From `side` and `step_x/step_y`, the face of the
   wall that got hit is determined. Four cases, four textures (NO/SO/EA/WE).
2. **Which pixel of the texture goes where on the strip?** Two
   coordinates: `tex_x` (column inside the texture) and `tex_y` (row).

### `tex_x` — where along the wall did we hit

The hit point along the wall, normalized to `[0, 1]`:

```
                wall_x = 0                        wall_x = 1
                │                                  │
       wall ────┤·····*····························│
                      ↑
                      wall_x = 0.3
```

The fractional part of the hit position along the wall axis. Multiply
by the texture's pixel width and that's the texture column.

Two of the four wall faces need their `tex_x` flipped (mirrored). Without
flipping, two of the four wall textures appear backwards.

### `tex_y` — walking down the strip

`tex_y` changes as we walk down the screen column:

```
step = tex_height / line_height       texture pixels per screen pixel
```

If a wall is 200 pixels tall on screen and the texture is 64 pixels
tall, each screen pixel covers 0.32 of a texture pixel. As we walk
down the column, `step` is added to a running `tex_y` value at each row.

### Texture sampling

For each row `y` in `[draw_start, draw_end]`:

1. compute `tex_y` from the running counter
2. look up `texture[tex_y][tex_x]` — that's a 32-bit color
3. write it to the image buffer at `(x, y)`

The texture is also a flat 1D array, just like the screen buffer. The
address math is identical.

### A subtle point — the clamped draw_start

When the wall is taller than the screen, `draw_start` was clamped to 0.
But `tex_y` should *not* start at 0 — it should start at "whatever
texture row would have been sampled at the unclamped draw_start".
That's a one-line offset added to the initial `tex_y`. Easy to miss;
without it, very close walls look stretched.

---

## 13. The minimap

The minimap is a top-down view drawn on top of the 3D scene. Same
buffer, same pixel writes: just a smaller area in the corner.

What it shows:

```
   ┌─────────┐         ← border around the map
   │  ▓ ▓▓   │
   │     ▓   │         ▓ = walls (each grid cell scaled down)
   │ \|/     │
   │  P      │         P = player position (small green square)
   └─────────┘
                       \|/ = a few rays showing the FOV cone
```

It draws in three layers:

1. **The map cells** walk every cell in `map[][]`. For each `'1'`,
   draw a small filled square at the scaled position.
2. **The player marker** a small green square at the player's scaled
   position.
3. **The FOV rays** about 15 rays (not 1920, just enough to
   visualize the cone) cast through the **same DDA** as the 3D render,
   then a line from the player to each hit point.

---

## 14. The full picture

Putting it all together, one frame of the engine:

```
parse the .cub file              (once, at startup)
  │
  ▼
init MLX + load 4 textures        (once, at startup)
  │
  ▼
each frame:
  ├── draw floor + ceiling        (repaints the buffer)
  ├── for each screen column:
  │     ├── compute ray direction  (player.dir + cplane × camera_x)
  │     ├── DDA until wall hit
  │     ├── compute strip height   (WIN_HEIGHT / perp_dist)
  │     ├── pick texture           (from side + step)
  │     ├── compute texture coords (wall_x → tex_x, step + text_pos)
  │     └── draw textured strip into buffer
  ├── draw minimap on top
  ├── update player position from keys held
  └── flush buffer to window in one MLX call
```

Three layers of abstraction, each handing the next a small struct:

```
parsing  →  config        (map, textures, colors, spawn)
player   →  ray_dir       (direction for each column)
DDA      →  ray_hit       (cell, side, perp_dist, wall_x)
renderer →  pixels        (in the image buffer)
window   →  the screen
```

---

## 15. Optimizations

A raycaster runs one ray per screen column per frame. At 1920×1080
and 60 fps that is **115,200 rays per second**, each one writing
dozens of pixels into the buffer. Tiny per-pixel costs compound fast.

---

### 1. Draw into memory, flush once

Every pixel write goes into an off-screen buffer in RAM. When the
frame is complete, one single call copies the entire buffer to the
screen:

```c
/* once at startup — allocate the buffer */
img.img  = mlx_new_image(mlx, WIN_WIDTH, WIN_HEIGHT);
img.addr = mlx_get_data_addr(img.img, &img.bits_per_pixel,
                              &img.line_len, &img.endian);

/* once per frame — flush the finished buffer to the window */
mlx_put_image_to_window(mlx, win, img.img, 0, 0);
```

The alternative — `mlx_pixel_put` — makes a system call for every
single pixel. At 2 million pixels per frame that is 2 million system
calls per frame. The program would run at single-digit FPS and
flicker badly.

---

### 2. Write one pixel in one operation

Each pixel is a 32-bit integer (4 bytes — R, G, B, and one unused).
Writing it as a single 32-bit value is one CPU instruction:

```c
/* one instruction — writes all 4 bytes at once */
*(unsigned int *)(addr + y * line_len + x * 4) = color;
```

Writing it byte by byte would be four instructions. Same result,
four times the work. Across 2 million pixels per frame this matters.

---

### 3. Floor and ceiling double as the frame clear

At the start of every frame, the floor and ceiling are drawn by
filling the entire buffer with two flat colors, top half ceiling,
bottom half floor. This overwrites everything from the previous frame.

A separate "clear the buffer" step before drawing would be redundant —
every byte cleared would immediately be overwritten anyway. One pass
instead of two.

---

### 4. Axis-aligned rays — avoid a check every iteration

The DDA loop runs 5 to 20 times per ray. On every iteration it
compares two distances to decide which gridline to cross next. When
a ray travels perfectly horizontally it will never cross a vertical
gridline — one of those distances would be infinite.

Instead of adding an `if` check inside the loop to handle this case,
the infinite distance is approximated with a very large number at
setup:

```c
if (ray->dir_x == 0)
    ray->delta_dist_x = 1e30;   /* never cross a vertical gridline */
else
    ray->delta_dist_x = fabs(1.0 / ray->dir_x);
```

In the DDA comparison `side_dist_x < side_dist_y`, a huge number
always loses — the unused axis is silently skipped on every iteration
without any extra branching inside the loop.

---

### 5. Perpendicular distance without `sqrt`

The wall strip height depends on how far away the wall is. The naive
way requires a `sqrt` and a `cos` per ray to get the correct distance
and fix the fisheye effect — roughly 20–100 CPU cycles each.

Instead, the perpendicular distance falls directly out of values DDA
has already computed:

```c
if (ray->side == 0)
    perp_dist = ray->side_dist_x - ray->delta_dist_x;
else
    perp_dist = ray->side_dist_y - ray->delta_dist_y;
```

One subtraction replaces a `sqrt` and a `cos`. At 115,200 rays per
second that is a significant saving — and the fisheye correction
comes for free because perpendicular distance is automatically
camera-plane-aligned.

---

### 6. Texture rows — accumulate, don't recompute

For each pixel in a wall strip, the corresponding row in the texture
must be looked up. The straightforward approach recomputes it from
scratch for every pixel:

```c
/* slow — one multiplication per pixel */
tex_y = (int)((row - draw_start) * step);
```

Instead, a running value is initialized once per strip and a fixed
increment is added at each row:

```c
/* fast — one addition per pixel */
text_pos += step;
tex_y = (int)text_pos;
```

One addition replaces one multiplication. Across hundreds of pixels
per strip and 1920 strips per frame the difference is measurable.

---

### 7. Texture row wrapping with bitwise AND

The running texture row value must wrap back to zero when it exceeds
the texture height. Modulo does this but costs a full division on the
CPU (~20–40 cycles) and requires a negative guard:

```c
/* slow — division + negative guard */
tex_y = (int)text_pos % tex->height;
if (tex_y < 0)
    tex_y += tex->height;
```

Because texture height is always a power of two (64, 128, 256),
bitwise AND can replace it. In binary, a power of two minus one is
all 1s in the lower bits — ANDing with it keeps only those bits,
which is exactly the wrap to the valid range:

```
64  =  01000000
63  =  00111111  ← all bits below 64

 70 & 63:          -5 & 63:
  70 = 01000110    -5 = 11111011  (simplified)
  63 = 00111111    63 = 00111111
     = 00000110       = 00111011
     = 6              = 59    (no negative guard needed)
```

```c
/* fast — one instruction, handles negatives automatically */
tex_y = (int)text_pos & (tex->height - 1);
```

One cycle instead of 20–40, and negative values wrap correctly
for free.

---

### 8. Recompute direction vectors from the angle every frame

The player's direction and camera plane are two vectors that must
always be perpendicular and unit-length. Rotating them incrementally
each frame using a rotation matrix causes floating-point rounding to
accumulate over thousands of frames — the vectors slowly drift and
walls start rendering at wrong heights.

The stable approach: store only the angle, recompute both vectors
from `cos` and `sin` every frame:

```c
dir_x    = cos(angle);
dir_y    = sin(angle);
cplane_x = -sin(angle) * FOV_FACTOR;
cplane_y =  cos(angle) * FOV_FACTOR;
```

Two trigonometric calls per frame is negligible. The vectors are
always exactly perpendicular and exactly unit-length.

---

### 9. Wrap the angle to keep numbers small

The player's angle grows or shrinks each frame as they rotate.
Without wrapping, after several minutes of play the angle could
reach into the thousands of radians. Trigonometric functions still
produce correct results for large inputs, but floating-point
precision degrades as numbers grow:

```
angle = 0.03        → all 15 significant digits available
angle = 1080.03     → 4 digits used on the integer part, 11 left
angle = 99999.03    → fewer digits still for the fractional part
```

Wrapping back to `[0, 2π]` keeps the number small forever:

```c
if (angle > 2 * PI)   angle = 0;
if (angle < 0)        angle = 2 * PI;
```

Angles that differ by a full circle are identical — the direction
never changes, but precision is preserved indefinitely.

---
