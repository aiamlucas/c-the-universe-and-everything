# Two-Dimensional Arrays

### What is a Two-Dimensional Array?

A two-dimensional array is an array of arrays. It's a data structure used to store a table of values, with rows and columns. For example, a matrix of size ``` m x n ``` is a two-dimensional array that has ``` m ``` rows and ``` n ``` columns.

### Visual Representation

If you imagine a table of data:

```
1  2  3
4  5  6
7  8  9
```

This can be represented by a two-dimensional array in C as:

```
int array[3][3] = {
    {1, 2, 3},
    {4, 5, 6},
    {7, 8, 9}
};
```

### Why Use Two-Dimensional Arrays?

- To handle matrices and tabular data.
- Useful in applications involving graphs, game boards, image processing, etc.
- Allows easy traversal using nested loops.

### Syntax Overview

```
data_type array_name[row_size][column_size];
```

- ``` data_type ```: The type of data stored (e.g., ``` int ```, ``` float ```, ``` char ```).
- ``` array_name ```: The name of the array.
- ``` row_size ```: Number of rows.
- ``` column_size ```: Number of columns.

### Example Declaration

```
int matrix[5][4]; // Declares a 2D array with 5 rows and 4 columns
```

### Memory Representation

In memory, a two-dimensional array is stored in **row-major order** by default in C. That means all elements of a row are stored consecutively.

For example, consider:

```
int arr[2][3] = {
    {1, 2, 3},
    {4, 5, 6}
};
```

The memory layout will be:

```
1, 2, 3, 4, 5, 6
```

### Important Points to Note

- The size of a two-dimensional array must be specified during declaration.
- The total memory required is calculated as:

```
total_memory = row_size * column_size * size_of(data_type)
```

- Each element is accessed by specifying both row and column indices.

In the next chapter, we'll dive into **how to declare, initialize, and access elements of two-dimensional arrays**.

### Declaration of Two-Dimensional Arrays

```
data_type array_name[row_size][column_size];
```

**Example:**

```
int matrix[3][4];  // Declares a 2D array with 3 rows and 4 columns
```

### Initialization of Two-Dimensional Arrays

You can initialize a two-dimensional array in several ways:

#### 1. Compile-Time Initialization (Static Initialization)

You provide the values directly within curly braces ```{}```.

**Example:**

```
int matrix[2][3] = {
    {1, 2, 3},
    {4, 5, 6}
};
```

The above array is represented as:

```
1  2  3
4  5  6
```

#### 2. Partial Initialization

If you provide fewer elements than required, the remaining elements are automatically initialized to ``` 0 ```.

**Example:**

```
int matrix[2][3] = {
    {1, 2},
    {4}
};
```

The array will be stored as:

```
1  2  0
4  0  0
```

#### 3. Initialization Without Specifying Row Size

C allows you to omit the row size, but **not** the column size.

**Example:**

```
int matrix[][3] = {
    {1, 2, 3},
    {4, 5, 6}
};
```

Here, the compiler determines the number of rows automatically (2 rows in this case).

### Accessing Elements of Two-Dimensional Arrays

To access an element, you use:

```
array_name[row_index][column_index]
```

- ``` row_index ```: Index of the row (starting from ``` 0 ```).
- ``` column_index ```: Index of the column (starting from ``` 0 ```).

**Example:**

```
int matrix[2][3] = {
    {1, 2, 3},
    {4, 5, 6}
};

printf("%d", matrix[1][2]);  // Output: 6 (Second row, third column)
```

### Nested Loop Traversal

Since 2D arrays are stored row-wise in memory, traversing them using nested loops is straightforward.

**Example (using `while` loops):**

```
#include <stdio.h>

int main() {
    int matrix[3][3] = {
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 9}
    };

    int i = 0;
    while(i < 3) {
        int j = 0;
        while(j < 3) {
            printf("%d ", matrix[i][j]);
            j++;
        }
        printf("\n");
        i++;
    }
    return 0;
}
```

**Output:**

```
1 2 3
4 5 6
7 8 9
```

### Important Points to Note

- Indices start at ``` 0 ``` for both rows and columns.
- Accessing an element out of bounds results in **undefined behavior**.
- When using nested loops, the outer loop iterates over rows and the inner loop iterates over columns.

---

## Operations and Applications

Two-dimensional arrays can be used to perform various operations such as traversing, searching, sorting, arithmetic operations, etc.

---

### 1. Traversing a Two-Dimensional Array

Traversing means visiting each element of the array exactly once. This is usually done using nested loops.

**Example (using `while` loops):**

```
#include <stdio.h>

int main() {
    int matrix[2][3] = {{1, 2, 3}, {4, 5, 6}};

    int i = 0;
    while(i < 2) {
        int j = 0;
        while(j < 3) {
            printf("%d ", matrix[i][j]);
            j++;
        }
        printf("\n");
        i++;
    }

    return 0;
}
```

**Output:**

```
1 2 3
4 5 6
```

---

### 2. Searching an Element

Searching for an element involves traversing the array and comparing each element to the target value.

**Example (using `while` loops):**

```
#include <stdio.h>

int main() {
    int matrix[3][3] = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    int target = 5;
    int found = 0;

    int i = 0;
    while(i < 3 && !found) {
        int j = 0;
        while(j < 3) {
            if(matrix[i][j] == target) {
                printf("Element %d found at position [%d][%d]\n", target, i, j);
                found = 1;
                break;
            }
            j++;
        }
        i++;
    }

    if (!found) {
        printf("Element %d not found in the matrix.\n", target);
    }

    return 0;
}
```

---

### 3. Matrix Addition

Two matrices can be added if they have the same dimensions.

**Example (using `while` loops):**

```
#include <stdio.h>

int main() {
    int A[2][2] = {{1, 2}, {3, 4}};
    int B[2][2] = {{5, 6}, {7, 8}};
    int C[2][2];

    int i = 0;
    while(i < 2) {
        int j = 0;
        while(j < 2) {
            C[i][j] = A[i][j] + B[i][j];
            j++;
        }
        i++;
    }

    printf("Resultant Matrix:\n");
    i = 0;
    while(i < 2) {
        int j = 0;
        while(j < 2) {
            printf("%d ", C[i][j]);
            j++;
        }
        printf("\n");
        i++;
    }

    return 0;
}
```

**Output:**

```
6 8
10 12
```

---

### 4. Matrix Multiplication

Matrix multiplication is valid only when the number of columns of the first matrix equals the number of rows of the second matrix.

**Example (using `while` loops):**

```
#include <stdio.h>

int main() {
    int A[2][3] = {{1, 2, 3}, {4, 5, 6}};
    int B[3][2] = {{7, 8}, {9, 10}, {11, 12}};
    int C[2][2] = {0};

    int i = 0;
    while(i < 2) {
        int j = 0;
        while(j < 2) {
            int k = 0;
            while(k < 3) {
                C[i][j] += A[i][k] * B[k][j];
                k++;
            }
            j++;
        }
        i++;
    }

    printf("Resultant Matrix:\n");
    i = 0;
    while(i < 2) {
        int j = 0;
        while(j < 2) {
            printf("%d ", C[i][j]);
            j++;
        }
        printf("\n");
        i++;
    }

    return 0;
}
```

**Output:**

```
58 64
139 154
```

---

### 5. Applications of Two-Dimensional Arrays

- **Matrices:** Representing and performing operations on matrices.
- **Image Processing:** Storing pixel values of images (grayscale, RGB).
- **Game Development:** Board games like Tic-Tac-Toe, Chess, Sudoku.
- **Graphs:** Adjacency matrix representation.
- **Table Manipulation:** Creating tables for data storage and retrieval.

---

## Extra: Drawing Basic Graphs for Games in Terminal

By using characters like ``` - ```, ``` | ```, ``` / ```, ``` \\ ```, ``` + ```, ``` * ```, etc, you can draw basic game boards, graphs, and shapes directly in the terminal. This is useful for creating text-based games or visualizing data without relying on graphical libraries.

---

### 1. Drawing Grids for Games

#### Tic-Tac-Toe Board

A classic **Tic-Tac-Toe** board is a 3x3 grid.

**Code Example:**

```
#include <stdio.h>

void draw_tic_tac_toe() {
    int i = 0;
    while(i < 5) {
        int j = 0;
        while(j < 5) {
            if(i % 2 == 0) {
                if(j % 2 == 0)
                    printf("+");
                else
                    printf("-");
            } else {
                if(j % 2 == 0)
                    printf("|");
                else
                    printf(" ");
            }
            j++;
        }
        printf("\n");
        i++;
    }
}

int main() {
    draw_tic_tac_toe();
    return 0;
}
```

**Output:**

```
+ - + - +
|   |   |
+ - + - +
|   |   |
+ - + - +
```

---

### 2. Drawing a Maze

Mazes can be represented using ``` # ``` for walls and spaces for paths.

**Code Example:**

```
#include <stdio.h>

#define ROWS 5
#define COLS 5

void draw_maze(char maze[ROWS][COLS]) {
    int i = 0;
    while(i < ROWS) {
        int j = 0;
        while(j < COLS) {
            printf("%c ", maze[i][j]);
            j++;
        }
        printf("\n");
        i++;
    }
}

int main() {
    char maze[ROWS][COLS] = {
        {'#', '#', '#', '#', '#'},
        {'#', ' ', ' ', '#', '#'},
        {'#', ' ', '#', '#', '#'},
        {'#', ' ', ' ', ' ', '#'},
        {'#', '#', '#', '#', '#'}
    };

    draw_maze(maze);
    return 0;
}
```

**Output:**

```
# # # # #
#     # #
#   # # #
#       #
# # # # #
```

---

### 3. Drawing Graphs Using Characters

You can draw simple graphs or charts using ``` * ``` or ``` | ```.

#### Line Graph Example

```
#include <stdio.h>

void draw_line_graph() {
    int values[10] = {1, 3, 7, 2, 5, 9, 4, 6, 8, 2};
    int max_value = 10;

    int i = max_value;
    while(i >= 0) {
        int j = 0;
        while(j < 10) {
            if(values[j] >= i)
                printf("* ");
            else
                printf("  ");
            j++;
        }
        printf("\n");
        i--;
    }
}

int main() {
    draw_line_graph();
    return 0;
}
```

**Output:**

```
          * 
          *     * 
        * * *   * 
        * * *   * * 
      * * * * * * * 
    * * * * * * * * 
    * * * * * * * * * 
  * * * * * * * * * * 
* * * * * * * * * * * 
```

---

### 4. Creating a Chess Board

A simple representation of an 8x8 chess board.

**Code Example:**

```
#include <stdio.h>

#define SIZE 8

void draw_chess_board() {
    int i = 0;
    while(i < SIZE) {
        int j = 0;
        while(j < SIZE) {
            if((i + j) % 2 == 0)
                printf("##");
            else
                printf("  ");
            j++;
        }
        printf("\n");
        i++;
    }
}

int main() {
    draw_chess_board();
    return 0;
}
```

**Output:**

```
##    ##    ##    ##
  ##    ##    ##  
##    ##    ##    ##
  ##    ##    ##  
##    ##    ##    ##
  ##    ##    ##  
##    ##    ##    ##
  ##    ##    ##  
```

---

### 5. Drawing a Pyramid

Drawing a pyramid using ``` * ```.

**Code Example:**

```
#include <stdio.h>

void draw_pyramid(int height) {
    int i = 1;
    while(i <= height) {
        int j = i;
        while(j < height) {
            printf(" ");
            j++;
        }

        j = 1;
        while(j <= (2 * i - 1)) {
            printf("*");
            j++;
        }

        printf("\n");
        i++;
    }
}

int main() {
    draw_pyramid(5);
    return 0;
}
```

**Output:**

```
    *
   ***
  *****
 *******
*********
```

---

### Summary

- Two-dimensional arrays store data in a grid (rows and columns).
- Useful for matrices, game boards, image data, and tabular formats.
- Declared with two indices: `array[row][column]`.
- Stored in row-major order in memory.
- Accessed and traversed using nested loops.
- Support common operations like searching, addition, and multiplication.
- Enable drawing terminal-based visuals like graphs and board games.
