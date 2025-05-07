# Arrays

## Introduction to Arrays

### What is an Array?

An **array** is a collection of elements of the same data type, stored in **contiguous memory locations** and accessed using a single index. It's essentially a list of values that can be individually accessed by their index positions.

### Visual Representation

Consider an array of integers:

```int arr[5] = {10, 20, 30, 40, 50};```

This array is visually represented as:

```
Index:   0   1   2   3   4
Values: 10  20  30  40  50
```

### Syntax Overview

```data_type array_name[array_size];```

- **data_type**: The type of data stored (e.g., ```int```, ```float```, ```char```).
- **array_name**: The name of the array.
- **array_size**: The number of elements the array can hold.

### Example Declaration

```int scores[10]; // Declares an integer array with 10 elements```

### Memory Representation

In memory, an array is stored as a **continuous block** of elements.

```int arr[4] = {1, 2, 3, 4};```

Memory layout:

```
Address    Value
1000       1
1004       2
1008       3
1012       4
```

(Assuming each ```int``` occupies 4 bytes.)

### Important Points to Note

- The index of the first element is always ```0```.
- The last element index is ```array_size - 1```.
- Accessing an index outside the defined range results in **undefined behavior**.

## Declaration, Initialization, and Accessing Elements

### Declaration

To declare an array in C, you specify the data type, name of the array and its size.

```int scores[5]; // Declares an array of integers with 5 elements.```

### Initialization

#### 1. Compile-Time Initialization

```int numbers[5] = {10, 20, 30, 40, 50};```

Result:

```
Index:   0   1   2   3   4
Values: 10  20  30  40  50
```

#### 2. Partial Initialization

```int numbers[5] = {1, 2};```

Result:

```
Index:   0  1  2  3  4
Values:  1  2  0  0  0
```

#### 3. Without Specifying Size

```int numbers[] = {1, 2, 3, 4, 5}; // Size is automatically determined as 5```

### Accessing Elements

Use the index operator ```[]```.

```
int numbers[5] = {10, 20, 30, 40, 50};
printf("%d", numbers[2]);  // Output: 30
```

## Traversing Arrays

```
#include <stdio.h>

int main() {
    int numbers[5] = {10, 20, 30, 40, 50};
    int i = 0;

    while (i < 5) {
        printf("Element at index %d: %d\n", i, numbers[i]);
        i++;
    }

    return 0;
}
```

## Operations and Applications

### 1. Searching (Linear Search)

```
#include <stdio.h>

int linear_search(int arr[], int size, int target) {
    int i = 0;
    while (i < size) {
        if (arr[i] == target) {
            return i;
        }
        i++;
    }
    return -1;
}

int main() {
    int arr[] = {10, 20, 30, 40, 50};
    int target = 30;
    int index = linear_search(arr, 5, target);

    if(index != -1)
        printf("Element %d found at index %d\n", target, index);
    else
        printf("Element not found\n");

    return 0;
}
```

### 2. Sorting (Bubble Sort)

```
#include <stdio.h>

void bubble_sort(int arr[], int size) {
    int i = 0;
    while (i < size - 1) {
        int j = 0;
        while (j < size - i - 1) {
            if (arr[j] > arr[j + 1]) {
                int temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
            j++;
        }
        i++;
    }
}

int main() {
    int arr[] = {50, 20, 40, 10, 30};
    int size = 5;

    bubble_sort(arr, size);

    printf("Sorted array: ");
    int i = 0;
    while (i < size) {
        printf("%d ", arr[i]);
        i++;
    }

    return 0;
}
```

### 3. Summation

```
#include <stdio.h>

int main() {
    int arr[] = {10, 20, 30, 40, 50};
    int size = 5, sum = 0, i = 0;

    while (i < size) {
        sum += arr[i];
        i++;
    }

    printf("Sum of array elements: %d\n", sum);

    return 0;
}
```

### 4. Finding Maximum and Minimum

```
#include <stdio.h>

int main() {
    int arr[] = {10, 50, 20, 80, 30};
    int size = 5, i = 1;
    int max = arr[0];
    int min = arr[0];

    while (i < size) {
        if (arr[i] > max)
            max = arr[i];
        if (arr[i] < min)
            min = arr[i];
        i++;
    }

    printf("Maximum: %d\n", max);
    printf("Minimum: %d\n", min);

    return 0;
}
```

### 5. Reversing an Array

```
#include <stdio.h>

int main() {
    int arr[] = {10, 20, 30, 40, 50};
    int size = 5;
    int i = size - 1;

    printf("Reversed array: ");
    while (i >= 0) {
        printf("%d ", arr[i]);
        i--;
    }

    return 0;
}
```

## Strings as Character Arrays

### Declaration and Initialization

```char name[] = "Alice";```

Equivalent to:

```char name[] = {'A', 'l', 'i', 'c', 'e', '\0'};```

### Accessing Characters

```
#include <stdio.h>

int main() {
    char name[] = "Alice";
    int i = 0;

    while (name[i] != '\0') {
        printf("Character at index %d: %c\n", i, name[i]);
        i++;
    }

    return 0;
}
```

## Additional Techniques

### Inputting Values at Runtime

```
#include <stdio.h>

int main() {
    int arr[5], i = 0;

    printf("Enter 5 numbers:\n");
    while (i < 5) {
        scanf("%d", &arr[i]);
        i++;
    }

    printf("You entered: ");
    i = 0;
    while (i < 5) {
        printf("%d ", arr[i]);
        i++;
    }

    return 0;
}
```

### Copying Arrays

```
#include <stdio.h>

int main() {
    int source[] = {1, 2, 3, 4, 5};
    int destination[5], i = 0;

    while (i < 5) {
        destination[i] = source[i];
        i++;
    }

    printf("Copied array: ");
    i = 0;
    while (i < 5) {
        printf("%d ", destination[i]);
        i++;
    }

    return 0;
}
```

### Swapping Elements

```
#include <stdio.h>

int main() {
    int a[] = {1, 2, 3, 4, 5};
    int temp = a[0];
    a[0] = a[4];
    a[4] = temp;

    printf("Swapped array: ");
    int i = 0;
    while (i < 5) {
        printf("%d ", a[i]);
        i++;
    }

    return 0;
}
```

## Extra Notes/Examples

### Out-of-Bounds Access

Accessing an array element outside its valid index range leads to **undefined behavior**, which may result in crashes or unpredictable output.

```
#include <stdio.h>

int main() {
    int arr[3] = {1, 2, 3};

    // Valid access
    printf("%d\n", arr[2]);

    // Invalid access
    printf("%d\n", arr[5]); // ⚠️ Undefined behavior
    return 0;
}
```

Always ensure that your loop or access logic respects the bounds: from `0` to `size - 1`.

---

### Constant Arrays

Using `const` makes an array read-only, preventing accidental modification of its values.

```
#include <stdio.h>

int main() {
    const int data[3] = {10, 20, 30};

    // data[0] = 100; // Compile-time error

    printf("First: %d\n", data[0]);
    return 0;
}
```

## Summary

- Arrays store sequences of values in contiguous memory.
- Useful for data storage, access, and manipulation.
- Arrays are foundational for more complex structures like matrices, strings and graphs.
