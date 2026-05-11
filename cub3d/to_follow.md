## After Parsing

After parsing the `.cub` file, the map is stored as an array of strings — one string per row:

```
map → [ ptr0 | ptr1 | ptr2 | ptr3 | ptr4 | NULL ]
         ↓      ↓      ↓      ↓      ↓
      "11111" "10001" "100N1" "10001" "11111"
     (each string is a separate heap allocation)
```

## Initialization

### MLX and textures
First we initialize MLX (the graphics library) and load the four XPM wall textures.

### Player position — pixel space vs grid space

The spawn point in the map is a grid cell (integer coordinates). We need to
place the player in the **center** of that cell, not at its corner:

```c
game->player.x = (game->config.player_x + 0.5) * BLOCK_SIZE;
game->player.y = (game->config.player_y + 0.5) * BLOCK_SIZE;
```

`+ 0.5` centers inside the cell. `× BLOCK_SIZE` converts from grid space to
pixel space. This is a good moment to explain the two coordinate systems:

```
PIXEL SPACE — where the player lives:

  0    64   128   192   256   320   384
  │    │     │     │     │     │     │
  ─────┼─────┼─────┼─────┼─────┼─────┼────
                                  ↑ pixel 360

GRID SPACE (÷ 64) — where the map lives and DDA operates:

+──────+──────+──────+──────+──────+──────+──────+
│  0   │  1   │  2   │  3   │  4   │  5   │  6   │
│      │      │      │      │      │  ↑   │      │
+──────+──────+──────+──────+──────+──────+──────+
                                    pos_in_grid_x = 5.625
                                    grid_cell_x   = 5  ← (int)5.625
```

The bridge: `pixel / BLOCK_SIZE = grid`,  `grid × BLOCK_SIZE = pixel`.

### Angle and direction vectors

The parsed spawn direction character is mapped to an angle in radians:

```
'N' → 3π/2    'S' → π/2    'E' → 0    'W' → π
```

Y grows **downward** (screen convention). One full circle = 2π ≈ 6.28 rad:

```
              3π/2  
               ↑
               │
   π  ─────────P─────────  0
 (West)        │         (East)
               ↓
              π/2  (South = down on screen)
```

---

Then two vectors are computed from the angle — these are used in every
frame of the rendering:

```c
dir_x    = cos(angle);           // unit vector pointing where player faces
dir_y    = sin(angle);
cplane_x = -sin(angle) * 0.66;  // perpendicular to dir, scaled by FOV factor
cplane_y =  cos(angle) * 0.66;
```

`dir` points straight ahead. `cplane` represents the width of the screen
projected into world space — always perpendicular to `dir`. Both are
recomputed from `angle` every frame so they never drift.

Finally all key booleans are set to `false`.

---


## draw_3d — Overview

```c
draw_floor_ceiling(game);   // fills entire buffer: top = ceiling, bottom = floor
                            // also acts as the frame clear

while (x < WIN_WIDTH)       // loop over all 1920 screen columns
{
    cast_ray(game, &ray, x);          // compute ray direction for this column
    while (!step_ray(game, &ray));    // DDA: march until wall hit
    draw_wall_column(game, &ray, x);  // draw textured wall strip for this column
    x++;
}
```

The 3D illusion is 1920 vertical strips drawn side by side. Each strip has
a different height depending on how far the wall is — that's the entire trick.


---


## cast_ray — Ray Direction per Column


`cast_ray` computes a **ray direction vector** `(ray_dir_x, ray_dir_y)` for
each screen column — two components saying where the ray points in 2D.

First, `camera_x` maps each column to `[-1.0, +1.0]` — how far from center:

```
cplane:  ◄────────────────────────────────►
             -1      0      +1
              \      │      /   rays fan out
               \     │     /
                \    │    /
                 \   │   /
                     P
```

```
x / WIN_WIDTH → [0.0,  1.0]  normalize
× 2.0         → [0.0,  2.0]  stretch
- 1.0         → [-1.0, +1.0] shift

col 0    col 960   col 1919
  \         │         /
-1.0       0.0       +1.0   ← camera_x
            P
```

```
dir_x/y            -> forward component
cplane_offset_x/y  -> sideways component 
ray_dir = dir + cplane × camera_x
```

```
facing East (angle=0):
  dir = (1.0, 0.0)   cplane = (0.0, 0.66)

  camera_x =  0.0 → ray_dir = (1.0,  0.00)  →  (right, no vertical)
  camera_x = +1.0 → ray_dir = (1.0,  0.66)  ↘  (right, down)
  camera_x = -1.0 → ray_dir = (1.0, -0.66)  ↗  (right, up)

facing South (angle=π/2):
  dir = (0.0, 1.0)   cplane = (-0.66, 0.0)

  camera_x =  0.0 → ray_dir = ( 0.00, 1.0)  ↓  (no horizontal, down)
  camera_x = +1.0 → ray_dir = (-0.66, 1.0)  ↙  (left, down)
  camera_x = -1.0 → ray_dir = ( 0.66, 1.0)  ↘  (right, down)

facing West (angle=π):
  dir = (-1.0, 0.0)  cplane = (0.0, -0.66)

  camera_x =  0.0 → ray_dir = (-1.0,  0.00)  ←  (left, no vertical)
  camera_x = +1.0 → ray_dir = (-1.0, -0.66)  ↖  (left, up)
  camera_x = -1.0 → ray_dir = (-1.0,  0.66)  ↙  (left, down)

facing North (angle=3π/2):
  dir = (0.0, -1.0)  cplane = (0.66, 0.0)

  camera_x =  0.0 → ray_dir = ( 0.00, -1.0)  ↑  (no horizontal, up)
  camera_x = +1.0 → ray_dir = ( 0.66, -1.0)  ↗  (right, up)
  camera_x = -1.0 → ray_dir = (-0.66, -1.0)  ↖  (left, up)
```

---

## init_ray — Setting Up the Ray State

With the ray direction computed, `init_ray` fills the `t_ray` struct with
everything DDA needs before it can start marching.

**Player position in grid space:**
**Which cell the player is in** — just the integer part:
**Store the ray direction** for this column:
**`delta_dist_x/y`** — how far the ray travels to cross one full cell on
each axis. Derived from trigonometry (same formula for both axes):

## init_ray_steps - Determines march direction and distance to first gridline


```
               3π/2 (North)
             sin < 0 ↑ dir_y < 0
                     │
  cos   < 0          │               cos   > 0
  dir_x < 0   π ─────P─────────  0   dir_x > 0
  step_x=-1   (West) │         (East) step_x=+1
                     │
             sin > 0 ↓ dir_y > 0
                 π/2 (South)
```
 
```
dir_x < 0 → step_x = -1  (marching left)
dir_x > 0 → step_x = +1  (marching right)
dir_y < 0 → step_y = -1  (marching up)
dir_y > 0 → step_y = +1  (marching down)
 
ray_dist_x = cell_fraction_x × delta_dist_x
```

first gridline is special, player is inside a cell, not at a gridline.
 
```
going RIGHT (step_x = +1):
 
  cell 5              cell 6
  +────────────────────+────────
  │    P               │
  │    5.625           │
  │    │◄── 0.375 ────►│  ← first gridline
  +────────────────────+────────
 
  cell_fraction = grid_cell + 1.0 - pos_in_grid
                = 6.0 - 5.625 = 0.375
 
going LEFT (step_x = -1):
 
  cell_fraction = pos_in_grid - grid_cell
                = 5.625 - 5.0 = 0.625
```
 
```
ray_dist = cell_fraction × delta_dist   ← only once
 
init:   ray_dist = 0.375 × delta_dist   ← special first step
step 1: ray_dist += delta_dist          ← uniform from here
step 2: ray_dist += delta_dist
step N: ray_dist += delta_dist → WALL
```
 
---

**DDA**
 
```
Brute force:
P ············································► [WALL]
  ↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑
  check at every tiny step
 
DDA:
P ──────┬────┬────► [WALL]
        │    │
        │    └── cross Y gridline
        └── cross X gridline
  3 checks total. Exact. Never skips.
```

---

## 8. draw_wall_column
 
Four steps to draw one textured vertical strip:

**1. `get_line_height`** — strip height from perpendicular distance:
 
```
after DDA, ray_dist is one step past the wall:
 
  ─────●────────●────────● ← ray_dist (one past wall)
                WALL      ↑ subtract delta_dist to land on wall
 
side == 0 → perp_dist = ray_dist_x - delta_dist_x  (crossed X gridline)
side == 1 → perp_dist = ray_dist_y - delta_dist_y  (crossed Y gridline)
```
 
`perp_dist <= 0` guard — player exactly on wall boundary → avoid division by zero.
 
```
line_height = WIN_HEIGHT / perp_dist
 
close wall → small perp_dist → tall strip
far wall   → large perp_dist → short strip
```

```
perp_dist = 1.0  → line_height = 1080 / 1.0  = 1080px  (wall 1 unit away  → fills screen)
perp_dist = 2.0  → line_height = 1080 / 2.0  = 540px   (wall 2 units away → half screen)
perp_dist = 4.0  → line_height = 1080 / 4.0  = 270px   (wall 4 units away → quarter)
perp_dist = 10.0 → line_height = 1080 / 10.0 = 108px   (wall far away     → small strip)
```

> raw length:   edge rays are LONGER than center ray  ← fisheye
> perp_dist:    all rays measured to camera plane line ← correct

---


**2. `set_col_geometry`** where on screen (centered on horizon):
 
```
draw_start = WIN_HEIGHT/2 - line_height/2
draw_end   = WIN_HEIGHT/2 + line_height/2
clamped to [0, WIN_HEIGHT-1]
```

Example 1: medium distance wall (line_height = 540, WIN_HEIGHT = 1080):

```
y = 0    ─┤
          │  ceiling
y = 270  ─┼──────────┐  ← draw_start (center - half => 540 - 270 = 270)
          │  W A L L │
y = 810  ─┼──────────┘  ← draw_end   (center + half => 540 + 270 = 810)
          │  floor
y = 1079 ─┤
```

Example 2: Very close wall (perp_dist = 0.3, line_height = 3600):
```

  without clamp:
  draw_start = 540 - 1800 = -1260 (out of bonds)
  draw_end   = 540 + 1800 =  2340 (out of bonds)

  with clamp:
  draw_start → clamped to 0
  draw_end   → clamped to 1079

y = 0          ─┤──────────┐  ← draw_start
                │          │
                │          │  ← you only see the middle part
                │  W A L L │     of the texture (top and bottom cut off)
                │          │
                │          │
y = 1079       ─┤──────────┘  ← draw_end
```

---


# Recap - draw_3d — How the 3D Render Works

The 3D scene is an illusion made of 1920 vertical strips — one per screen column.
Each strip represents one ray cast from the player into the world.
The height of each strip depends on how far the wall is — close walls are tall, far walls are short.

---

## The idea

The player has a direction (`dir`) and a camera plane (`cplane`) — a vector perpendicular
to `dir` that represents the width of the screen in world space:

```
cplane:  ◄────────────────────────────────►
             -1      0      +1
              \      │      /   rays fan out
               \     │     /
                \    │    /
                 \   │   /
                     P
                     │
                    dir
```

For each of the 1920 columns, a ray is cast in a slightly different direction —
from straight left (`camera_x = -1`) through center (`camera_x = 0`) to straight right (`camera_x = +1`).
The ray direction is `dir + cplane × camera_x` — forward plus a sideways offset.

---

## Step 1 — fill floor and ceiling

Before any rays are cast, the entire buffer is filled with two flat colors —
top half ceiling, bottom half floor. This also clears the previous frame.

RGB values from the `.cub` file are packed into one 32-bit int:
```
color = (R << 16) | (G << 8) | B
```
The buffer is a flat array — `WIN_WIDTH × WIN_HEIGHT` pixels total.
The halfway point is the horizon line.

---

## Step 2 — for each column, cast a ray

`camera_x` maps each column to `[-1.0, +1.0]`.
`ray_dir = dir + cplane × camera_x` — the direction this ray travels.

The ray is initialized with:
- its position in grid space (`pos_in_grid = player.x / 64`)
- `delta_dist` — how far it travels to cross one full cell (`1 / |ray_dir|`)
- `step` — which direction to march (`+1` or `-1` per axis)
- `ray_dist` — distance to the first gridline (`cell_fraction × delta_dist`)

---

## Step 3 — march with DDA until wall hit

DDA jumps gridline by gridline — no tiny steps, no missed cells.
Each iteration picks the closer gridline (X or Y), steps one cell, checks the map.

The number of steps tells you how far the wall is — few steps = close, many = far.
After the loop, `ray_dist - delta_dist` gives the exact perpendicular distance to the wall.

---

## Step 4 — draw the strip

`perp_dist` (perpendicular distance, not raw ray length) gives the strip height:
```
line_height = WIN_HEIGHT / perp_dist
```

The strip is centered on the horizon — `draw_start` and `draw_end` are clamped
so very close walls don't write outside the buffer.

The texture face is picked from `side` + `step_x/y`.
`wall_x` traces the ray to find where along the wall it hit — that gives `tex_x`.
`tex_y` advances each screen pixel, walking down the texture column.

Each pixel is sampled from the texture buffer and written to the screen buffer
using the same address formula: `row × line_length + col × 4`.

The texture coordinates:
- `tex_x` — which column of the texture, from where along the wall the ray hit
- `tex_y` — advances each screen pixel walking down that texture column
- `& (height-1)` wraps `tex_y` back into range on very close walls

Each pixel: read color from texture at `(tex_x, tex_y)` → write to screen buffer at `(x, y)`.

---

## Why perpendicular distance?

Raw ray length causes fisheye — flat walls appear to bow outward because
rays at the screen edges travel further than center rays to reach the same wall.
Perpendicular distance (measured at right angle to the camera plane) is the same
for all columns hitting the same flat wall — no distortion.

```
raw ray length:     varies per column  → fisheye
perp_dist:          same per flat wall → correct perspective
```

---


# Texture Pipeline

For each screen column `x`, we draw one vertical strip with the correct texture.

---

## About Textures

#### XPM (X PixMap)

It is a plain text image format (not MLX specific, it's a standard X11 format).

```
"64 64 4 1"
 ↑  ↑  ↑ ↑
 │  │  │ └─── chars per pixel (1 char = 1 pixel)
 │  │  └───── number of colors in palette
 │  └──────── height (64 rows)
 └─────────── width (64 columns)

- image is 64×64 pixels
- 4 colors total
- each pixel is represented by exactly 1 character 
```

#### Color palette — maps one character to one hex color:

```
"  c #000000"   ← space = black
". c #020818"   ← dot   = very dark blue
"+ c #030D22"   ← plus  = dark blue
"M c #FF00FF"   ← M     = magenta
```

#### Pixels — each row is a string, each character is one pixel:

```
"++++++++++"      ← row of dark blue pixels
"    ++    "      ← spaces = black, ++ = dark blue
"MMMMMMMMMM"      ← row of magenta pixels
```

Reading the file top to bottom gives you the image row by row. 
MLX reads it, converts each character to its RGB value using the palette, and stores it in a pixel buffer

#### From XPM file to screen pixel

**From XPM file to texture buffer**
```
XPM file (plain text)
↓
mlx_xpm_file_to_image()   ← MLX parses chars → RGB, stores in tex->addr
↓
tex->addr                 ← flat pixel buffer (same layout as game->img.addr)
```
**From texture pixel to screen**
```
get_texture_pixel(tex, tex_x, tex_y)   ← read one color from texture buffer
↓
img_put_pixel(game, x, y, color)       ← write to screen buffer
↓
mlx_put_image_to_window()              ← flush entire screen buffer once per frame 
```

```
TEXTURE BUFFER (read only)          SCREEN BUFFER (write)
──────────────────────────          ─────────────────────

XPM file (plain text)               game->img.addr
      ↓                             (flat array of 32-bit pixels)
mlx_xpm_file_to_image()
      ↓                             ┌──────────────────┐
tex->addr                           │     ceiling      │
(flat array of 32-bit pixels)       ├──────────────────┤
                                    │  │               │
┌───┬───┬───┬[47]┬───┐              │  │  wall strip   │
│   │   │   │ ██ │   │  row 0       │  │  (col x)      │
│   │   │   │ ██ │   │  row 1  →    │  │               │
│   │   │   │ ██ │   │  row 2       ├──────────────────┤
│   │   │   │ ██ │   │  ...         │      floor       │
└───┴───┴───┴────┴───┘              └──────────────────┘
     tex_x=47 fixed                      ↓
          ↓                    mlx_put_image_to_window()
  get_texture_pixel()                    ↓
          ↓                           window
     color (32-bit)
          ↓
     img_put_pixel(x, y)
```

```
TEXTURE (64×64)              SCREEN (1920×1080)

col: =0    47   63           col:  0      800     1919
      │     │    │                 │        │         │
row 0 ├─────█────┤           y=0   ├────────┼─────────┤
row 1 │     █    │                 │        │ ceiling │
row 2 │     █    │                 │        │         │
...   │     █    │           draw  ├────────█─────────┤
row 9 │     █    │           start │        █ ← tex_x=47
...   │     █    │                 │        █   walks
      │     █    │                 │        █   down
      ├─────█────┤           draw  ├────────█─────────┤
                             end   │        │ floor   │
                             y=1079├────────┴─────────┤

tex_x=47 → fixed column in texture    (where along the wall)
x=800    → fixed column on screen     (which ray we are drawing)
tex_y    → advances each screen row   (walks down texture column 47)
y        → advances each screen row   (walks down screen column 800)
```
---

# tex_x vs x — two independent coordinates

`tex_x` and `x` look similar but are completely unrelated:

```
tex_x  → which column of the TEXTURE (where along the wall the ray hit)
x      → which column of the SCREEN  (the draw_3d loop counter 0..1919)
```

They scale independently:

```
SCREEN (1920 columns)                    TEXTURE (64 columns)

0%        42%       74%    100%          0%        74%      100%
│          │         │        │          │          │          │
├──────────┼─────────┼────────┤          ├──────────┼──────────┤
col 0   col 800             col 1919    col 0     col 47    col 63

screen col 800 = 42% across screen
tex_x = 47     = 74% across texture   ← different percentages, no relation


wall_x  → where along the wall face the ray hit [0.0, 1.0]
          the bridge between x and tex_x: (Screen and Texture)
          x → camera_x → ray_dir → DDA → wall_x → tex_x

tex_x = wall_x × tex_width   (e.g. 0.74 × 64 = 47)
```

```
SCREEN (1920 cols)        WALL FACE             TEXTURE (64 cols)

0%      42%    100%       0%      74%   100%    0%      74%   100%
│        │        │       │        │       │    │        │       │
├────────┼────────┤       ├────────┼───────┤    ├────────┼───────┤
0      800     1919       0.0    0.74     1.0   0       47      63

x=800                     wall_x=0.74            tex_x=47
(loop counter)            (ray hit 74%            (0.74 × 64)
                           across face)

      x → camera_x → ray_dir → DDA ──→ wall_x ──→ tex_x
```

---

# set_tex_y — starting texture row and advance per pixel

```c
col->step = (double)tex->height / line_height;
dist_from_center = (double)col->draw_start - (double)WIN_HEIGHT / 2.0;
half_wall        = (double)line_height / 2.0;
tex_row_offset   = dist_from_center + half_wall;
col->text_pos    = tex_row_offset * col->step;
```

---

## step — how fast to advance through the texture

`step = tex_height / line_height` maps 64 texture rows onto `line_height` screen pixels:

```
line_height = 32   step=2.0  → 1 screen pixel = 2 texture rows  (compressed)
line_height = 64   step=1.0  → 1 screen pixel = 1 texture row   (1:1)
line_height = 320  step=0.2  → 5 screen pixels = 1 texture row  (stretched)
line_height = 3000 step=0.02 → 50 screen pixels = 1 texture row (very stretched + tiles)
```

---

## tex_x vs tex_y

```
tex_x   fixed for the whole strip — which column of the texture (where ray hit the wall)
tex_y   advances each screen pixel — walks down that texture column
```

```
SCREEN column 800              TEXTURE column 47

draw_start        → tex_y = 0   ← starts at row 0 (normal wall)
y = draw_start+1  → tex_y = 0   (step=0.2, same row for 5 pixels)
y = draw_start+5  → tex_y = 1   ← new texture row
...
draw_end          → tex_y = 63  ← last row
```

---

## text_pos vs tex_y

`text_pos` grows forever. `tex_y = (int)text_pos & 63` wraps it into `[0, 63]`:

```
text_pos = 62.0  → tex_y = 62
text_pos = 63.0  → tex_y = 63
text_pos = 64.0  → tex_y = 0   ← wraps
text_pos = 65.0  → tex_y = 1
text_pos = 128.0 → tex_y = 0   ← wraps again
```

Tiling only happens when `line_height > 64` (very close wall):

```
y=0    ┌──────┐
       │ row 0│  ← text_pos=20 (wall top offscreen, starts mid-texture)
       │  ... │
       │ row63│  ← text_pos reaches 64
       │ row 0│  ← wraps (& 63), texture tiles
       │  ... │
y=1079 └──────┘
```

---

## text_pos — where to start in the texture

Normally `text_pos = 0` — the texture always starts at row 0.

Only non-zero when `draw_start` is clamped (very close wall, `line_height > WIN_HEIGHT`):

```
normal wall:   tex_row_offset = -half_wall + half_wall = 0   → text_pos = 0
clamped wall:  draw_start forced to 0 → offset > 0          → text_pos > 0 (skip rows)
```

Example — close wall (`line_height = 3000`):
```
dist_from_center =    0 - 540 = -540
half_wall        = 3000 / 2   = 1500
tex_row_offset   = -540 + 1500 =  960
text_pos         = 960 × 0.021 = 20   ← skip first 20 rows (they were offscreen)
```














## tex_y tiling on very close walls

`tex_x` is fixed for the whole strip — always column 47 (same wall column all the way down).
`tex_y`  advances each screen pixel (walks down the texture)
        --> advances and wraps with `& 63` — tiles vertically on close walls:

```
SCREEN column 800              TEXTURE column 47

draw_start = 0  → tex_y = 32   ← starts mid-texture (wall top offscreen)
y = 100         → tex_y = 40
y = 200         → tex_y = 50
y = 300         → tex_y = 63
y = 301         → tex_y = 0    ← wraps here
y = 302         → tex_y = 1
y = 303         → tex_y = 2
y = 304         → tex_y = 3
...
y = 400         → tex_y = 10
draw_end = 1079 → tex_y = 45   ← tiled twice across the strip
```

**How does one texture column fill the entire strip at any distance?**
	`step = tex_height / line_height` does the mapping:

```
wall far away  (line_height = 64):
  step = 64/64 = 1.0 → each screen pixel = 1 texture row
  tex_y: 0,1,2,3...63 → texture fits exactly

wall medium (line_height = 320):
  step = 64/320 = 0.2 → every 5 screen pixels = 1 texture row
  tex_y: 0,0,0,0,0,1,1,1,1,1,2... → texture stretched

wall very close (line_height = 3000):
  step = 64/3000 = 0.02 → every 50 screen pixels = 1 texture row
  tex_y: 0×50 rows, 1×50 rows... → very stretched
  AND wraps: after row 63 → back to 0 → tiles ✓
```

```
line_height = 64  (wall at exact distance where texture fits 1:1):
  step = 64/64 = 1.0 → each screen pixel = 1 texture row
  tex_y: 0,1,2,3,...,63 → fits exactly, no wrap

line_height = 320 (wall medium distance, stretched):
  step = 64/320 = 0.2 → every 5 screen pixels = 1 texture row
  tex_y: 0,0,0,0,0,1,1,1,1,1,2,2,2,2,2,... → stretched

line_height = 3000 (wall very close, stretched + tiles):
  step = 64/3000 = 0.02 → every 50 screen pixels = 1 texture row
  tex_y: 0×50, 1×50, 2×50,... → very stretched
  after row 63 → wraps to 0 → tiles again

line_height = 32 (wall very far, compressed):
  step = 64/32 = 2.0 → each screen pixel = 2 texture rows skipped
  tex_y: 0,2,4,6,8,...,62 → only even rows sampled
  texture compressed, fits in 32 screen pixels
```

```
very close wall strip on screen:

y=0    ┌──────┐
       │ row 0│  ← texture starts at row 20 (text_pos=20)
       │ row 1│
       │  ... │
       │ row63│  ← bottom of texture
       │ row 0│  ← wraps back to top (& 63)
       │ row 1│  ← texture repeats
       │  ... │
       │ row63│  ← wraps again
       │ row 0│
       │  ... │
y=1079 └──────┘
```


---

# Back to the code!!

## 1. get_texture — which texture?

After DDA hits a wall, `side` and `step_x/y` identify the face:

```
side=0, step_x > 0  →  West   (game->we)
side=0, step_x < 0  →  East   (game->ea)
side=1, step_y > 0  →  North  (game->no)
side=1, step_y < 0  →  South  (game->so)
```

The texture is stored in `col.tex` — a pointer to the `t_texture` struct
that holds `addr`, `width`, `height`, etc.

---

## 2. set_wall_x — where along the wall? → wall_x [0.0, 1.0]

```
wall face:
 ← one full grid cell = 64px  →
├──────────────────────────────┤
0.0          0.5             1.0
                  ↑
              wall_x = 0.6   (ray hit 60% across the face)

```

Traces the ray from player: `pos_in_grid + perp_dist × dir` = hit coordinate.
`floor()` strips the cell number → keeps only the fraction:


```
Example:

side == 0 (vertical wall), hit varies in Y:
pos_in_grid_y = 4.2
perp_dist     = 1.8
dir_y         = 0.3

wall_x = 4.2 + 1.8 × 0.3 = 4.74
4.74 - floor(4.74) = 0.74  → ray hit 74% across the wall face
```

---

## 3. set_tex_x — which column of the texture?

```
tex_x = wall_x × tex_width
0.74 × 64 = 47   ← always sample column 47 for this strip

tex_x is FIXED for the entire strip — it never changes as we draw down.
```

East and North faces are mirrored → flip:
`tex_x = tex_width - tex_x - 1`
```
tex_width = 64,  wall_x = 0.74  →  tex_x = 47

without flip (West/South):   sample col 47  →  →  →  col 47
with flip    (East/North):   64 - 47 - 1 = 16  ←  ←  col 16
```

For the west texture at column 47:
```
/* XPM */
"64 64 4 1"
"  c #000000"
"+ c #030D22"

column 47 of each row:
row 0:  '+'  → 0x030D22
row 1:  '+'  → 0x030D22
...
row 8:  ' '  → 0x000000
row 9:  '+'  → 0x030D22
```

---

# set_tex_y — starting texture row and advance per pixel

```c
col->step = (double)tex->height / line_height;
dist_from_center = (double)col->draw_start - (double)WIN_HEIGHT / 2.0;
half_wall        = (double)line_height / 2.0;
tex_row_offset   = dist_from_center + half_wall;
col->text_pos    = tex_row_offset * col->step;
```

---

## step — how fast to advance through the texture

`step = tex_height / line_height`

The texture is 64px tall. The wall strip is `line_height` pixels on screen.
We need to map 64 texture rows onto `line_height` screen pixels:

```
line_height = 640  → step = 64/640 = 0.10  → 10 screen pixels = 1 texture row  (stretched)
line_height = 320  → step = 64/320 = 0.20  → 5  screen pixels = 1 texture row
line_height = 64   → step = 64/64  = 1.00  → 1  screen pixel  = 1 texture row  (1:1)
line_height = 32   → step = 64/32  = 2.00  → 1  screen pixel  = 2 texture rows (compressed)
```

```
far wall   → small line_height → large step  → texture compressed
close wall → large line_height → small step  → texture stretched
```

Each iteration of the pixel loop: `text_pos += step` — advances through the texture.

---

## text_pos — where to start in the texture

### Normal wall (draw_start not clamped)

`line_height = 540`, `WIN_HEIGHT = 1080`, `draw_start = 270`:

```
dist_from_center = draw_start - WIN_HEIGHT/2 = 270 - 540 = -270
half_wall        = line_height / 2           = 540 / 2   =  270
tex_row_offset   = -270 + 270                            =    0
text_pos         = 0 × step                              =    0   ← start at texture row 0
```

The strip starts exactly at the top of the texture — correct.

---

### Very close wall (draw_start clamped to 0)

`line_height = 3000`, `WIN_HEIGHT = 1080`:

Without clamping: `draw_start = 540 - 1500 = -960` (offscreen).
With clamping: `draw_start = 0`.

```
dist_from_center = 0 - 540    = -540
half_wall        = 3000 / 2   = 1500
tex_row_offset   = -540 + 1500 =  960   ← 960px down the wall
text_pos         = 960 × step           ← start partway through the texture ✓
```

Why does this work? `tex_row_offset` measures how far down the **full unclamped wall**
`draw_start` actually is. The real top of the wall is at `y = -960` (offscreen).
`draw_start = 0` is 960px below that. So we skip the first 960px of the texture
and start where the visible strip begins.

```
real wall top  y = -960  ─┤  (offscreen)
                          │  ← 960px of texture skipped
              y =    0   ─┼──────────┐  ← draw_start (clamped)
                          │  W A L L │  ← text_pos starts here
              y = 1079   ─┼──────────┘
```

---

## In the pixel loop

```c
tex_y = (int)col.text_pos & (col.tex->height - 1);  // wrap [0, 63]
col.text_pos += col.step;                            // advance
```

`text_pos` starts at `tex_row_offset × step` and advances by `step` each pixel.
`& (height - 1)` wraps it back into `[0, 63]` if it exceeds the texture height.















---

## 4. set_tex_y — starting row and step per pixel

```
step = tex_height / line_height   (texture rows per screen pixel)
```

Example — strip 320px tall, texture 64px:
```
step = 64 / 320 = 0.2   → every 5 screen pixels = 1 texture row
```

`text_pos` starts at the correct row (adjusted for clamped `draw_start` on close walls)
and advances by `step` each screen pixel.

---

## 5. The pixel loop

```c
while (y <= col.draw_end)
{
    tex_y = (int)col.text_pos & (col.tex->height - 1);  // wrap [0, 63]
    col.text_pos += col.step;                            // advance texture row
    img_put_pixel(game, x, y, get_texture_pixel(col.tex, col.tex_x, tex_y));
    y++;
}
```

`tex_x` = 47 (fixed)
`tex_y` advances each screen pixel:

```
screen y=270  text_pos=0.0 → tex_y=0  → tex[47][0]  → 0x030D22 → pixel (x, 270)
screen y=271  text_pos=0.2 → tex_y=0  → tex[47][0]  → 0x030D22 → pixel (x, 271)
screen y=272  text_pos=0.4 → tex_y=0  → tex[47][0]  → 0x030D22 → pixel (x, 272)
screen y=273  text_pos=0.6 → tex_y=0  → tex[47][0]  → 0x030D22 → pixel (x, 273)
screen y=274  text_pos=0.8 → tex_y=0  → tex[47][0]  → 0x030D22 → pixel (x, 274)
screen y=275  text_pos=1.0 → tex_y=1  → tex[47][1]  → 0x030D22 → pixel (x, 275)
...
screen y=319  text_pos=9.8 → tex_y=9  → tex[47][9]  → 0x000000 → pixel (x, 319)
```

---

## 6. get_texture_pixel — read from texture buffer

```c
pixel = tex->addr + (tex_y * tex->line_length + tex_x * (tex->bits_per_pixel / 8));
return (*(int *)pixel);
```

Same address formula as `img_put_pixel` — the texture buffer is a flat array
just like the render buffer. Jump to row `tex_y`, then column `tex_x`, read 4 bytes.

---

## 7. The bitmask wrap

On very close walls, `line_height > tex_height` — `text_pos` would exceed 63.
Without wrap: row 64 doesn't exist → crash or garbage pixels.

```
& (height - 1) = & 63 = 0b00111111   ← keeps only lower 6 bits

tex_y = 64 & 63 = 0   ← tiles back to row 0
tex_y = 65 & 63 = 1   ← row 1
tex_y = 26 & 63 = 26  ← no change, already in range
```

The texture tiles seamlessly — when it reaches the bottom it starts again from the top.

---

# Minimap

The minimap is drawn on top of the 3D scene into the same buffer.
Toggled with `M`.
`draw_minimap_live` has four layers drawn in order:

```
1. draw_minimap_map()     ← wall cells
2. draw_minimap_border()  ← border outline
3. draw_square()          ← player dot
4. 15 rays                ← FOV visualization
```

---

## 1. draw_minimap_map — wall cells

Loops through every row and column of `config.map`. When it finds a `'1'`,
it draws a box at that position scaled down by `MINIMAP_SCALE = 0.25`:

```c
cell_size = BLOCK_SIZE * MINIMAP_SCALE   // 64 × 0.25 = 16px per cell
x         = MINIMAP_OFFSET + col * cell_size
y         = MINIMAP_OFFSET + row * cell_size
```

`MINIMAP_OFFSET = 70` shifts the minimap away from the top-left corner
so it doesn't start at pixel `(0, 0)`.

`draw_box` draws only the **four edges** of each wall cell — a square, not filled:

```
x      x+cell_size
●────────●  ← top edge (y)
│        │
│        │
│        │
●────────●  ← bottom edge (y+cell_size)
```

Only cells with `'1'` get a box. For a map row `"11001"`:

```
col 0  col 1  col 2  col 3  col 4
●────● ●────●               ●────●
│    │ │    │               │    │
●────● ●────●               ●────●
 '1'    '1'    '0'    '0'    '1'
(box)  (box)  (skip) (skip) (box)
```

Result: a wireframe grid showing the wall layout.

---

## 2. draw_minimap_border — outline

Draws a gray rectangle around the full map area using `draw_box`
the same function used for individual wall cells, just with different
dimensions and color:

```c
width        = map_width  × (BLOCK_SIZE × MINIMAP_SCALE)
height       = map_height × (BLOCK_SIZE × MINIMAP_SCALE)
border_start = MINIMAP_OFFSET - 1   // 1px outside the map cells
draw_box(border_start, border_start, width, height, 0x4C4C4C, game);
```

`-1` places the border just outside the map cells so it doesn't overlap them.

---

## 3. draw_square — player dot

Draws a small filled green square at the player's scaled position:

```c
x = MINIMAP_OFFSET + (int)(player.x * MINIMAP_SCALE)
y = MINIMAP_OFFSET + (int)(player.y * MINIMAP_SCALE)
```

`player.x` is in pixel space — multiplying by `MINIMAP_SCALE` converts it
to minimap space. The result is a 4×4 green square that moves with the player.

`draw_square` fills every pixel inside (unlike `draw_box` which only draws edges).

---

## 4. Minimap Rays — FOV visualization

15 rays are cast using the **same DDA** as the 3D render — `init_ray` +
`step_ray`. The difference is that instead of drawing a wall strip, we
draw a line from the player dot to the hit point.

### draw_minimap_live — cast 15 rays

`camera_x` maps 15 rays across the camera plane. Same `[-1.0, +1.0]`
formula as `draw_3d`, but divides by `14` instead of `WIN_WIDTH`:

```c
camera_x = 2.0 * ray_idx / (double)(14) - 1.0

ray_idx =  0 → camera_x = -1.0  (far left)
ray_idx =  7 → camera_x =  0.0  (center)
ray_idx = 14 → camera_x = +1.0  (far right)
```

Then exactly like `draw_3d`, compute ray direction and cast it:

```c
ray_dir_x = dir_x + cplane_x * camera_x;
ray_dir_y = dir_y + cplane_y * camera_x;
draw_minimap_ray(game, ray_dir_x, ray_dir_y);
```

---

### draw_minimap_ray — march one ray, find hit point

Uses the same `init_ray` + `step_ray` DDA loop as the 3D render:

```c
init_ray(game, &ray, ray_dir_x, ray_dir_y);
while (!step_ray(game, &ray))
    ;
```

After the loop, `perp_dist` rolls back one step to the exact wall hit:

```c
if (ray.side == 0)
    perp_dist = ray.ray_dist_x - ray.delta_dist_x;
else
    perp_dist = ray.ray_dist_y - ray.delta_dist_y;
```

Then compute the hit point in **grid space**:

```c
ray_hit_x = ray.pos_in_grid_x + perp_dist * ray_dir_x;
ray_hit_y = ray.pos_in_grid_y + perp_dist * ray_dir_y;
```

Start at player, walk `perp_dist` units along `ray_dir`, land on the wall face.

---

### draw_minimap_ray_line — convert to screen and draw

Converts both endpoints to minimap screen coordinates:

```c
line.x_player = MINIMAP_OFFSET + (int)(player.x   * MINIMAP_SCALE)
line.y_player = MINIMAP_OFFSET + (int)(player.y   * MINIMAP_SCALE)
line.x_wall   = MINIMAP_OFFSET + (int)(ray_hit_x  * BLOCK_SIZE * MINIMAP_SCALE)
line.y_wall   = MINIMAP_OFFSET + (int)(ray_hit_y  * BLOCK_SIZE * MINIMAP_SCALE)
```

Why `× BLOCK_SIZE` for the hit point but not for the player?
- `player.x` is in **pixel space** (e.g. `368px`) → just scale down
- `ray_hit_x` is in **grid space** (e.g. `5.75`) → convert to pixels first, then scale down

---

### draw_line_pixels — pixel by pixel line

Draws a straight line between two points by plotting one pixel at a time,
stepping from `(x_player, y_player)` to `(x_wall, y_wall)`.

```
distance_x = x_wall - x_player   ← how far horizontally (can be negative)
distance_y = y_wall - y_player   ← how far vertically   (can be negative)
```

```
steps = (int)fmax(fabs(distance_x), fabs(distance_y))

  fmax(a, b) → larger of two:    fmax(4.0, 2.0) = 4.0
  fabs(x)    → absolute value:   fabs(-4.0)     = 4.0

  - fabs: distance can be negative (going left/up) → need absolute length
  - fmax: pick the longest axis → that axis needs the most pixels

  why longest axis?
  line from (10,10) to (14,12): distance_x=4, distance_y=2

  steps = 2 (wrong — too few):     steps = 4 (correct):
  ·  ·  ·  ·  ·                    ·  ·  ·  ·  ·
  ·  ·  ·  ·  ·                    ·  ●  ·  ·  ·
  ·  ●  ·  ●  ·   ← GAP!           ·  ●  ●  ·  ·
  ·  ·  ·  ·  ·                    ·  ·  ●  ●  ·
                                   ·  ·  ·  ·  ·
```

Once we have `steps`, divide both distances by it to get the increment
per pixel — how much to move in X and Y each step:

```
distance_x /= steps   e.g. 4 / 4 = 1.0  → move 1px right per step
distance_y /= steps   e.g. 2 / 4 = 0.5  → move 0.5px down per step
```

Each iteration plots one pixel by advancing `i` steps from the start:

```
pixel_x = x_player + distance_x × i
pixel_y = y_player + distance_y × i
```

---

Example — player at `(10, 10)`, wall hit at `(14, 12)`:

```
distance_x = 14-10 = 4
distance_y = 12-10 = 2
steps      = max(4, 2) = 4
increment_x = 4/4 = 1.0
increment_y = 2/4 = 0.5

i=0 → (10,   10  )  ← player
i=1 → (11,   10.5)
i=2 → (12,   11  )
i=3 → (13,   11.5)
i=4 → (14,   12  )  ← wall
```

```
10   11   12   13   14
 ●                       y=10
      ●                  y=10.5
           ●             y=11
                ●        y=11.5
                     ●   y=12
```

---

### Result

```
┌──────────────────────────────┐
│ ▓▓▓▓▓▓▓▓▓▓▓                  │
│ ▓           ▓                 │
│ ▓   \│/     ▓    3D view      │
│ ▓    P      ▓                 │
│ ▓           ▓                 │
│ ▓▓▓▓▓▓▓▓▓▓▓                  │
└──────────────────────────────┘
▓ = walls   P = player dot   \│/ = 15 magenta rays
```

---

## 5. Screen Rays — first person FOV visualization (toggle R)

A separate visual effect drawn on top of the 3D scene. `SCREEN_RAYS` green
lines fan out from the bottom center of the screen to where each ray hits
the wall. Toggled with `R`.

```
draw_loop:
  if (game->player.show_screen_rays)
      draw_screen_rays(game);
```

---

### draw_screen_rays — cast SCREEN_RAYS evenly across FOV

Same `camera_x` formula as `draw_3d` and the minimap, but divides by
`SCREEN_RAYS - 1` to space rays evenly across the full `[-1.0, +1.0]` range:

```c
camera_x = 2.0 * i / (double)(SCREEN_RAYS - 1) - 1.0
```

For each ray, compute direction and cast:

```c
ray_dir_x = dir_x + cplane_x * camera_x;
ray_dir_y = dir_y + cplane_y * camera_x;
draw_screen_ray(game, ray_dir_x, ray_dir_y,
    (int)(i * WIN_WIDTH / (double)(SCREEN_RAYS - 1)));
```

The third argument maps ray index `i` to a screen X column — evenly
distributed across `WIN_WIDTH`.

---

### draw_screen_ray — march one ray, find screen hit point

Same DDA loop as `draw_minimap_ray`:

```c
init_ray(game, &ray, ray_dir_x, ray_dir_y);
while (!step_ray(game, &ray))
    ;
perp_dist = ray.ray_dist_x - ray.delta_dist_x;  // side == 0
```

Then compute the screen Y of the wall strip top:

```c
ray_screen_x = col_x;
ray_screen_y = WIN_HEIGHT / 2 - (int)(WIN_HEIGHT / perp_dist) / 2;
```

`ray_screen_y` is the same `draw_start` formula as `set_col_geometry`
the top of the wall strip for this ray.

---

### draw_screen_ray_line — draw from bottom center to wall top

```c
line.x_player = WIN_WIDTH / 2;    // center bottom — fixed origin
line.y_player = WIN_HEIGHT - 1;
line.x_wall   = ray_screen_x;     // column for this ray
line.y_wall   = ray_screen_y;     // top of wall strip
```

All lines converge at `(WIN_WIDTH/2, WIN_HEIGHT-1)`. The center bottom
of the screen. 

---

# Player Movement

Called every frame from `draw_loop` after rendering:

```c
void	draw_loop(t_game *game)
{
    draw_3d(game);
    draw_minimap_live(game);
    move_player(game);        // ← applies all held keys
    mlx_put_image_to_window(...);
}
```

`move_player` has four steps:

```
1. rotate_player    → update angle, recompute dir and cplane
2. forward/back     → move along dir vector
3. strafe           → move perpendicular to dir vector
4. is_wall check    → collision detection per axis
```

---

## is_wall — when is there a wall?

Converts pixel position to grid space and checks the map:

```c
map_x = (int)(x / BLOCK_SIZE)
map_y = (int)(y / BLOCK_SIZE)
```

Returns `true` (wall) in these cases:

```
map_x < 0              → past LEFT   edge of map
map_x >= map_width     → past RIGHT  edge of map
map_y < 0              → past TOP    edge of map
map_y >= map_height    → past BOTTOM edge of map
map[map_y][map_x]=='1' → cell is a wall
```

Without bounds checks, `map[map_y][map_x]` would be an out-of-bounds

---

## rotate_player — update angle and recompute vectors

`ROTATION_SPEED = 0.03` radians per frame. One full circle = `2π ≈ 6.28` radians
so `0.03` is about `1.7°` per frame

```c
angle -= ROTATION_SPEED;   // left arrow
angle += ROTATION_SPEED;   // right arrow
```

Wrap to `[0, 2π]` to keep the number small:

```c
if (angle > 2 * PI)  angle = 0;
if (angle < 0)       angle = 2 * PI;
```

Then recompute both vectors from scratch:

```c
dir_x    = cos(angle);
dir_y    = sin(angle);
cplane_x = -sin(angle) × FOV_FACTOR;
cplane_y =  cos(angle) × FOV_FACTOR;
```

Never accumulate rotation, recomputing from `angle` every frame keeps
`dir` and `cplane` exactly perpendicular with no floating-point drift.

---
 
## Forward / backward — dir_x and dir_y explained
 
```
dir_x = cos(angle)
dir_y = sin(angle)
```
 
The unit vector pointing where the player faces. "Unit vector" means
its length is always exactly 1.
 
From the right triangle — when hypotenuse = 1:
 
```
        /|
       / |
   1  /  | dir_y = sin(angle)
     /   |
    / θ  |
   /─────┘
   dir_x = cos(angle)
```
 
`dir_x` = how much movement goes in X.
`dir_y` = how much goes in Y.
Their values depend on the angle:
 
```
angle = 0      → dir = ( 1.0,  0.0)   facing East  →
angle = π/2    → dir = ( 0.0,  1.0)   facing South ↓
angle = π      → dir = (-1.0,  0.0)   facing West  ←
angle = 3π/2   → dir = ( 0.0, -1.0)   facing North ↑
```
 
The forward movement formula:
 
```
player.x += dir_x × MOVEMENT_SPEED
player.y += dir_y × MOVEMENT_SPEED
```
 
Examples:
 
```
facing East (angle=0):
  dir_x = 1.0,  dir_y = 0.0
  player.x += 1.0 × 3 = +3px   → moves 3px right
  player.y += 0.0 × 3 =  0px   → no vertical movement
 
facing West (angle=π):
  dir_x = -1.0, dir_y = 0.0
  player.x += -1.0 × 3 = -3px  → moves 3px left
  player.y +=  0.0 × 3 =  0px  → no vertical movement
 
facing South (angle=π/2):
  dir_x = 0.0,  dir_y = 1.0
  player.x += 0.0 × 3 =  0px   → no horizontal movement
  player.y += 1.0 × 3 = +3px   → moves 3px down
```
 
Backward = negate both components: `move_direction(game, -dir_x, -dir_y)`.
 
---

## Strafe — dir rotated 90°

```
dir_x    = cos(angle)           forward/backward direction
dir_y    = sin(angle)

strafe_x = cos(angle - π/2)     = sin(angle)   sideways direction
strafe_y = sin(angle - π/2)     = -cos(angle)
```

Subtracting `π/2` rotates the vector 90° clockwise. 
The result is always
perpendicular to `dir` pointing left relative to the player:


```
player facing East (angle=0):
 
         ↑ strafe (North)
         │
         │
P ───────┼──────────► dir (East)
 
  dir    = (cos(0),    sin(0))    = ( 1.0,  0.0)
  strafe = (cos(-π/2), sin(-π/2)) = ( 0.0, -1.0)
```
 
**Strafe is always 90° clockwise from dir.**
 
```
player facing South (angle=π/2):
 
P ───────────────────► strafe (East)
│
│
▼ dir (South)
 
  dir    = (cos(π/2), sin(π/2)) = (0.0,  1.0)
  strafe = (cos(0),   sin(0))   = (1.0,  0.0)
```
 
`A` key  → move along  strafe_x,  strafe_y   (left)
`D` key → move along -strafe_x, -strafe_y   (right)

Right strafe = negate left strafe. No extra trig needed.

```
spawn   angle    dir              strafe
East    0        ( 1.0,  0.0) →   ( 0.0, -1.0) ↑ North
South   π/2      ( 0.0,  1.0) ↓   ( 1.0,  0.0) → East
West    π        (-1.0,  0.0) ←   ( 0.0,  1.0) ↓ South
North   3π/2     ( 0.0, -1.0) ↑   (-1.0,  0.0) ← West
```

---
 
## move_direction — wall sliding collision
 
Before moving, compute where the player would be a few pixels ahead
the projected position. If no wall there, move:
 
```c
projected_x = player.x + dx × (MOVEMENT_SPEED + 5);
projected_y = player.y + dy × (MOVEMENT_SPEED + 5);
 
if (!is_wall(projected_x, player.y))
	player.x += dx × MOVEMENT_SPEED;
if (!is_wall(player.x, projected_y))
	player.y += dy × MOVEMENT_SPEED;
```
 
**Why `+ 5`?** Without the margin, the player could clip inside a wall
the check passes on frame N, but the movement on frame N puts the player
inside the wall before the next check fires. The `+ 5` stops the player
a few pixels before the wall, preventing clipping.
 
**Why check axes independently?** Because hitting a wall in X doesn't mean
Y is blocked. Independent checks allow wall sliding, the player continues
moving along the wall instead of freezing completely:
 
```
player moving diagonally into a wall on the right:
 
  projected_x → WALL   → X blocked, stays
  projected_y → empty  → Y moves freely
  result: player slides down along the wall
```
 
Without independent checks, any wall contact would freeze all movement.

---







# set_col_geometry vs set_tex_y

Both functions work together — `set_col_geometry` decides **where on screen** to draw,
`set_tex_y` decides **where in the texture** to start.

---

## set_col_geometry — where on screen?

```c
center     = WIN_HEIGHT / 2             // 540
draw_start = center - line_height / 2  // top of strip
draw_end   = center + line_height / 2  // bottom of strip
// clamped to [0, WIN_HEIGHT-1]
```

```
normal wall (line_height=540):        close wall (line_height=3600):

y=0    ─┤                             y=0    ─┼──────────┐ ← clamped (real=-1260)
        │  ceiling                            │          │
y=270  ─┼──────────┐ ← draw_start            │  W A L L │   only middle part visible
        │  W A L L │                          │          │
y=810  ─┼──────────┘ ← draw_end      y=1079  ─┼──────────┘ ← clamped (real=2340)
        │  floor
y=1079 ─┤
```

---

## set_tex_y — where in the texture?

```c
step             = tex_height / line_height   // advance per screen pixel
dist_from_center = draw_start - WIN_HEIGHT/2  // how far draw_start is from horizon
half_wall        = line_height / 2            // half the full wall height
tex_row_offset   = dist_from_center + half_wall
text_pos         = tex_row_offset * step      // starting texture row
```

### Why dist_from_center + half_wall?

Think of it as: "how far down the FULL wall does draw_start fall?"

```
full wall (unclamped):

real top  ─┤  ← row 0 of texture
           │
           │  ← tex_row_offset = distance from real top to draw_start
           │
draw_start ─┼──────────┐  ← start drawing here
           │          │
draw_end   ─┼──────────┘
           │
real bottom─┤  ← row 63 of texture
```

`dist_from_center + half_wall` measures that distance in screen pixels.
`× step` converts it to texture rows.

---

## Side by side — normal vs clamped

```
NORMAL wall (line_height=540)          CLOSE wall (line_height=3600, clamped)

draw_start = 270  (not clamped)        draw_start = 0  (clamped from -1260)

dist_from_center = 270-540  = -270     dist_from_center = 0-540    = -540
half_wall        = 540/2    =  270     half_wall        = 3600/2   = 1800
tex_row_offset   = -270+270 =    0     tex_row_offset   = -540+1800 = 1260
step             = 64/540   =  0.118   step             = 64/3600  =  0.017
text_pos         = 0                   text_pos         = 1260×0.017 = 21.4

→ starts at texture row 0             → skips first 21 rows (they were offscreen)
```

---

## Does the bitmask always fire?

`tex_y = (int)text_pos & 63` — wraps `tex_y` into `[0, 63]`.

Wrapping happens when `text_pos` exceeds 63 during the pixel loop.
For Full HD (`WIN_HEIGHT=1080`), this happens when `line_height > 64`:

```
perp_dist < 1080/64 = 16.875 grid units → line_height > 64 → bitmask wraps
```

Since most walls in a normal map are within 17 cells, **wrapping happens every frame**
for most wall strips. The bitmask is not a rare safety net — it's always working.

```
line_height = 32    step=2.0   text_pos: 0→62        no wrap  (tex fits, rows skipped)
line_height = 64    step=1.0   text_pos: 0→63        no wrap  (exact 1:1)
line_height = 320   step=0.2   text_pos: 0→63.8      wraps    (just barely)
line_height = 3600  step=0.017 text_pos: 0→63→0→...  wraps    (tiles many times)
```









