# Guide to Cub3d

## After Parsing

After parsing the `.cub` file, the map is stored as an array of strings — one string per row:

```
map → [ ptr0 | ptr1 | ptr2 | ptr3 | ptr4 | NULL ]
         ↓      ↓      ↓      ↓      ↓
      "11111" "10001" "100N1" "10001" "11111"
     (each string is a separate heap allocation)
```

## Initialization

### Player position — pixel space vs grid space

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

## Angle

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
---

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
---

## DDA
 
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

## Draw_wall_column
 
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

## Set_col_geometry()
 
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

---

## How tex_y walks down the texture column

`y` advances 1 screen pixel at a time (loop counter).
`tex_y` advances by `step` — how fast depends on the wall distance.

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

## Steps

```
step=2.0  (far wall):    tex_y: 0    2   4   6   8   10    (skips rows)
                         y:     508 509 510 511 512 513 

step=1.0  (exact):       tex_y: 0  1  2  3  4   5     (1:1)
                         y:     508 509 510 511 512 

step=0.2  (medium):      tex_y: 0  0  0  0  0   1   1    (repeats rows)
                         y:     270 271 272 273 274 275.

step=0.02 (close):       tex_y: 0  0  0  ... 0   1   1   (very stretched)
                         y:     0  1  2  ... 49  50  51
```

---

## set_tex_y
 
```c
col->step = (double)tex->height / line_height;
dist_from_center = (double)col->draw_start - (double)WIN_HEIGHT / 2.0;
half_wall        = (double)line_height / 2.0;
tex_row_offset   = dist_from_center + half_wall;
col->text_pos    = tex_row_offset * col->step;
```
 
**step** — how much to advance in the texture per screen pixel:
```
step=2.0  (far,   line_height=32)   → each screen pixel = 2 texture rows
step=1.0  (exact, line_height=64)   → each screen pixel = 1 texture row
step=0.2  (med,   line_height=320)  → every 5 screen pixels = 1 texture row
step=0.02 (close, line_height=3000) → every 50 screen pixels = 1 texture row
```
 
---
 
### 1) Normal wall — text_pos = 0
 
Wall fits on screen, `draw_start` not clamped. Example (`line_height=540`):
 
```
step             = 64 / 540   = 0.118
draw_start       = 540 - 270  = 270
dist_from_center = 270 - 540  = -270
half_wall        = 540 / 2    =  270
tex_row_offset   = -270 + 270 =    0
text_pos         = 0 × 0.118  =    0   ← start at texture row 0
```
 
`dist_from_center` and `half_wall` always cancel → `text_pos = 0` for any normal wall.
 
---
 
### 2) Clamped wall — text_pos > 0
 
Wall too tall for screen, `draw_start` clamped to `0`. Example (`line_height=3000`):
 
```
step             = 64 / 3000   = 0.021
real draw_start  = 540 - 1500  = -960  (offscreen → clamped to 0)
dist_from_center = 0 - 540     = -540
half_wall        = 3000 / 2    = 1500
tex_row_offset   = -540 + 1500 =  960  ← 960px down the full wall
text_pos         = 960 × 0.021 =   20  ← skip first 20 texture rows
```
 
```
real wall top  y = -960  ── row 0  (offscreen, skipped)
                          │
draw_start     y =    0  ─┼── text_pos = 20  ← start here
                          │
draw_end       y = 1079  ─┘
```
 
The first 20 texture rows were offscreen — `text_pos` skips them so the
texture aligns correctly with the visible part of the wall.

---

## Draw_wall_column — the pixel loop
 
```c
y = col.draw_start;
while (y <= col.draw_end)
{
    tex_y = (int)col.text_pos; // which texture row to sample (truncate floar to int)
    col.text_pos += col.step; // advance tex_pos by step (walk down the texture)
    img_put_pixel(game, x, y, get_texture_pixel(col.tex, col.tex_x, tex_y)); // write the sampled color to screen buffer at (x, y)
    y++;
}
```
 
### the two walkers
 
```
y          walks the SCREEN    — 1 pixel at a time, draw_start → draw_end
tex_y      walks the TEXTURE   — step pixels at a time, 0 → tex_height
```
 
They walk in parallel — for every screen pixel `y`, there is one texture pixel `tex_y`.
`step` controls how fast `tex_y` advances relative to `y`.
 
---
 
### concrete example (line_height=320, step=0.2)
 
```
y     tex_y   color              screen pixel
270   0       0x030D22 ░░░░░░    (x=800, y=270)
271   0       0x030D22 ░░░░░░    same row, same color
272   0       0x030D22 ░░░░░░
273   0       0x030D22 ░░░░░░
274   0       0x030D22 ░░░░░░
275   1       0x020818 ▒▒▒▒▒▒    ← new row, different color
276   1       0x020818 ▒▒▒▒▒▒
277   1       0x020818 ▒▒▒▒▒▒
278   1       0x020818 ▒▒▒▒▒▒
279   1       0x020818 ▒▒▒▒▒▒
280   2       0x000000 ██████    ← new row, different color
281   2       0x000000 ██████
...
810   63      0xFF00FF ▓▓▓▓▓▓    ← last row (magenta)
```
 
`tex_x=47` is fixed — same column of the texture all the way down.
`tex_y` changes only when `(int)text_pos` increments — every 5 pixels here.

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

`draw_square` fills every pixel inside (unlike `draw_box` which only draws edges).

---

## 4. Minimap Rays — FOV visualization

15 rays are cast using the **same DDA** as the 3D render
`init_ray` + `step_ray`. 

### draw_minimap_live — cast 15 rays

`camera_x` maps 15 rays across the camera plane. Same `[-1.0, +1.0]`
formula as `draw_3d`, but divides by `14` instead of `WIN_WIDTH`:

`camera_x = 2.0 * ray / (double)(MINIMAP_RAYS - 1) - 1.0;`

Then exactly like `draw_3d`, compute ray direction and cast it:
```
ray_dir_x = game->player.dir_x + game->player.cplane_x * camera_x;
ray_dir_y = game->player.dir_y + game->player.cplane_y * camera_x;
draw_minimap_ray(game, ray_dir_x, ray_dir_y);
```

#### draw_minimap_ray — cast one ray and draw a line to where it hits

```
1. init_ray + step_ray    → same DDA as draw_3d, march until wall hit
2. perp_dist              → exact distance to wall face (roll back one step)
3. ray_hit_x/y            → exact hit point in grid space:
                            pos_in_grid + perp_dist × ray_dir
                            same formula as set_wall_x but keeps
                            the full coordinate — not just where along the face
4. draw_minimap_ray_line  → convert to minimap screen coords and draw
```

#### draw_minimap_ray_line + draw_line_pixels

---

##### draw_minimap_ray_line

Converts both endpoints to minimap screen coordinates:

```c
line.x_player = MINIMAP_OFFSET + (int)(player.x * minimap_scale)
line.y_player = MINIMAP_OFFSET + (int)(player.y * minimap_scale)
line.x_wall   = MINIMAP_OFFSET + (int)(hit_x * BLOCK_SIZE * minimap_scale)
line.y_wall   = MINIMAP_OFFSET + (int)(hit_y * BLOCK_SIZE * minimap_scale)
```

Why `× BLOCK_SIZE` for the wall but not the player?

```
player.x  = 368px  → pixel space → just × minimap_scale
hit_x     = 5.75   → grid space  → × BLOCK_SIZE first → pixel → × minimap_scale
```

---

## draw_line_pixels

Draws a straight line between two points pixel by pixel:

```
distance_x = x_wall - x_player        total horizontal distance (can be negative)
distance_y = y_wall - y_player        total vertical distance   (can be negative)

steps = fmax(fabs(distance_x), fabs(distance_y))
```

`fmax` picks the longest axis — ensures no gaps in the line:

```
steps = 2 (wrong — too few):     steps = 4 (correct — longest axis):
·  ·  ·  ·  ·                    ·  ·  ·  ·  ·
·  ●  ·  ●  ·  ← GAP!            ·  ●  ·  ·  ·
·  ·  ·  ·  ·                    ·  ●  ●  ·  ·
                                  ·  ·  ●  ●  ·
                                  ·  ·  ·  ·  ·
```

Then divide by steps to get the increment per pixel:

```
distance_x /= steps   →  how much to move in X each iteration
distance_y /= steps   →  how much to move in Y each iteration

each i:  img_put_pixel(x_player + distance_x × i,  y_player + distance_y × i)
```

---

## Concrete example

Player at `(10, 10)`, wall hit at `(14, 12)`:

```
distance_x = 14 - 10 = 4
distance_y = 12 - 10 = 2
steps      = fmax(4, 2) = 4   ← longest axis

increment_x = 4 / 4 = 1.0
increment_y = 2 / 4 = 0.5

i=0 → (10,   10  )  ← player dot
i=1 → (11,   10.5)                
i=2 → (12,   11  )
i=3 → (13,   11.5)
i=4 → (14,   12  )  ← wall dot
```

On the minimap:

```
10   11   12   13   14
 ●                       y=10
      ●                  y=10.5 → rounded to 10
           ●             y=11
                ●        y=11.5 → rounded to 11
                     ●   y=12
```

Color: magenta `0xFF00FF`.

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

# draw_screen_rays vs draw_minimap_rays

Both use the exact same DDA logic — `init_ray` + `step_ray` + `perp_dist`.
The only difference is what they do with the result.

---

## What they share

```
init_ray + while (!step_ray())     ← same DDA, same wall hit
perp_dist = ray_dist - delta_dist  ← same distance computation
draw_line_pixels()                 ← same function to draw the line
```

---

## The difference — what to draw after the hit

```
minimap ray:                        screen ray:
──────────────────────────────      ──────────────────────────────
hit point in GRID space             hit point in SCREEN space

ray_hit_x = pos_in_grid_x           ray_screen_x = col_x
          + perp_dist × ray_dir_x   ray_screen_y = WIN_HEIGHT/2
ray_hit_y = pos_in_grid_y                        - (WIN_HEIGHT/perp_dist)/2
          + perp_dist × ray_dir_y
                                    ← draw_start of the wall strip
→ where in the world the ray hit    → where on screen the wall strip starts
```

```
minimap line:                       screen ray line:
  x_player = MINIMAP_OFFSET          x_player = WIN_WIDTH / 2
           + player.x × scale                   (center bottom)
  y_player = MINIMAP_OFFSET          y_player = WIN_HEIGHT - 1
           + player.y × scale
  x_wall   = MINIMAP_OFFSET          x_wall   = col_x
           + hit_x × BLOCK_SIZE      y_wall   = ray_screen_y
           × scale
  color    = 0xFF00FF (magenta)      color    = 0x00FF00 (green)
```

**All lines converge at `(WIN_WIDTH/2, WIN_HEIGHT-1)`. The center bottom
of the screen.**

---

## Visual

```
MINIMAP                              SCREEN

┌─────────────────┐                 ┌─────────────────────────────┐
│ ▓▓▓▓▓▓▓▓▓▓▓▓    │                 │                             │
│ ▓          ▓    │                 │                             │
│ ▓   \│/    ▓    │                 │    \  │  /                  │
│ ▓    P     ▓    │                 │     \ │ /  ← green lines    │
│ ▓          ▓    │                 │      \│/     from center    │
│ ▓▓▓▓▓▓▓▓▓▓▓▓    │                 │       P (WIN_WIDTH/2,       │
└─────────────────┘                 │         WIN_HEIGHT-1)       │
                                    └─────────────────────────────┘

origin = player dot                 origin = center bottom (fixed)
target = wall hit in world          target = top of wall strip on screen
color  = magenta                    color  = green
```

---

---

# Player Movement


`move_player` has four steps:

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

Never accumulate rotation!

---
 
## Forward / backward — dir_x and dir_y explained
 
```
dir_x = cos(angle)
dir_y = sin(angle)
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

`strafe` is `dir` rotated 90° clockwise — always perpendicular to `dir`:

```
dir_x    = cos(angle)           forward/backward direction
dir_y    = sin(angle)
-------------------------------------------------------------------

(-π/2 to rotate 90° clockwise)
strafe_x = cos(angle - π/2)     = sin(angle)   sideways direction
strafe_y = sin(angle - π/2)     = -cos(angle)
```

player facing East (angle=0):
```
         ↑ strafe (North)
         │
P ───────┼──────────► dir (East)
```

player facing South (angle=π/2):
```
P ───────────────────► strafe (East)
│
▼ dir (South)
```

```
spawn   angle    dir              strafe
East    0        ( 1.0,  0.0) →   ( 0.0, -1.0) ↑ North
South   π/2      ( 0.0,  1.0) ↓   ( 1.0,  0.0) → East
West    π        (-1.0,  0.0) ←   ( 0.0,  1.0) ↓ South
North   3π/2     ( 0.0, -1.0) ↑   (-1.0,  0.0) ← West
```

---

## move_direction — wall sliding collision

Check a few pixels ahead before moving — if no wall, move:

```c
projected_x = player.x + dx × (MOVEMENT_SPEED + WALL_MARGIN)
projected_y = player.y + dy × (MOVEMENT_SPEED + WALL_MARGIN)

if (!is_wall(projected_x, player.y))  player.x += dx × MOVEMENT_SPEED
if (!is_wall(player.x, projected_y))  player.y += dy × MOVEMENT_SPEED
```
`+ WALL_MARGIN` stops the player a few pixels before the wall — prevents clipping.

Each axis checked independently → wall sliding:

```
player moving ↘ into right wall:

  X: projected_x → WALL   → blocked
  Y: projected_y → empty  → moves

  result: player slides ↓ along the wall instead of freezing
```


dx and dy are the direction components passed to move_direction
theythey come from either dir or strafe depending on which key is pressed:

```
W → dx = dir_x,     dy = dir_y     (forward)
S → dx = -dir_x,    dy = -dir_y    (backward)
A → dx = strafe_x,  dy = strafe_y  (left)
D → dx = -strafe_x, dy = -strafe_y (right)
```
