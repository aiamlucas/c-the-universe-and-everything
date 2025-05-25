# Libft Bonus: Deep Dive (Linked Lists & Structs)

This document explores the **theoretical foundations** and **function-by-function documentation** needed for the Libft **bonus part**.  
It focuses on dynamic memory, data structures and how to build and manage **linked lists** in C.

```
[███████████████████████████████████████████░] 99% — compiling bonus...
```

## Table of Contents

- [Introduction to Data Structures](#introduction-to-data-structures)
- [ft_lstnew()](#ft_lstnew)
- [ft_lstadd_front()]() - TO DO
- [ft_lstsize()]() - TO DO
- [ft_lstlast()]() - TO DO
- [ft_lstadd_back()]()	- TO DO
- [ft_lstdelone()]()	- TO DO
- [ft_lstclear()]() - TO DO
- [ft_lstiter()]() - TO DO
- [ft_lstmap()]() - TO DO

---

# Introduction to Data Structures

Before exploring the bonus part of Libft, it's important to understand the broader context: what **data structures** are, how they work and where **linked lists** fit in.

---

## What is a Data Structure?

A **data structure** is a way to organize and store data efficiently so it can be used effectively.

There are many kinds of data structures in computer science. Each has strengths and weaknesses depending on the use case.

A **data structure** is a way of organizing and storing data in memory so that it can be accessed and modified efficiently.

Different types of structures serve different needs: some are great for searching, others for inserting, some for managing order, others for key-value access.

Here’s a breakdown of the most important ones, including their logic and memory characteristics.


## Common Types:

- **Primitive Types**: basic types like integers, floats, characters
- **Arrays**: contiguous blocks of memory, fast access by index, fixed size
- **Linked Lists**: dynamic memory, efficient insertion/deletion, slow access
- **Stacks**: LIFO (last-in, first-out), useful for undo operations or recursion
- **Queues**: FIFO (first-in, first-out), used in scheduling and messaging
- **Trees**: hierarchical structure, fast search and sorted storage (like binary trees)
- **Hash Tables**: key-value pairs, very fast access if hashing is well-distributed
- **Graphs**: general networks of connected nodes, used for routes, dependencies and relationships

Understanding these structures helps you make smart decisions about how to manage memory and performance.

---

### 1. Primitive Types

- **What**: Built-in types like ```int```, ```char```, ```float```, etc.
- **Memory**: Stored directly in stack memory (for local variables), or heap if dynamically allocated.
- **Usage**: The atomic units of data (you’ll use them everywhere).
- **Visualization**:

  ```
  int a = 42;   // [42]
  char c = 'A'; // [0x41]
  ```

---

### 2. Arrays

- **What**: A fixed-size sequence of elements of the same type.
- **Memory**: Contiguous block in memory.
- **Access**: Constant time ```O(1)``` with ```arr[i]```.
- **Limitations**: Fixed size; inserting or deleting in the middle requires shifting elements.
- **Visualization**:

  ```
  int arr[4] = {10, 20, 30, 40};
  // Memory    [10][20][30][40]
  ```

---

### 3. Linked Lists

- **What**: A dynamic sequence of nodes where each node points to the next.
- **Memory**: Nodes are scattered in memory, connected via pointers.
- **Flexibility**: Can grow/shrink at runtime; no preallocation.
- **Performance**:
  - Access: ```O(n)``` (must walk node-by-node)
  - Insert/Delete: ```O(1)``` if you have a pointer to the right spot
- **Visualization**:

  ```
  [data] → [data] → [data] → NULL
  ```
Behavior:
- Each node holds data and a pointer to the next node
- Nodes are not stored contiguously in memory
- Traversal starts at the head and follows each ```next``` pointer
- Insertion/removal is fast if you're already at the right position
- Slower random access: to get the 5th element, you must pass through the first 4

---

### 4. Stacks

- **What**: LIFO (last-in, first-out) structure.
- **Operations**:
  - ```push()```: add element to top
  - ```pop()```: remove top element
  - ```peek()```: view top without removing
- **Memory**: Can be implemented via array or linked list.
- **Use Cases**: Function call management, undo systems, parsers, recursion handling.
- **Visualization**:

```
Empty stack:
    [ ]
    
Push 1:
    [1]

Push 2:
    [2]
    [1]

Pop:
    [1]
    (2 removed)
```

Behavior:
- New items go on top
- Items removed from the top
- Think: a stack of trays in a cafeteria

Operations:
- push(x): add item to the top
- pop(): remove item from the top

---

### 5. Queues

- **What**: FIFO (first-in, first-out) structure.
- **Operations**:
  - ```enqueue()```: add to rear
  - ```dequeue()```: remove from front
- **Memory**: Array-based or list-based.
- **Use Cases**: Task scheduling, buffer pipelines, printers.
- **Visualization**:

```
  Enqueue "D":
  Front → [A] → [B] → [C] → [D] → NULL ← Rear

  Dequeue:
  Front → [B] → [C] → [D] → NULL ← Rear
  (A removed)
```

Behavior:
- New items are added at the back (rear)
- Items are removed from the front
- Think: a line of people waiting for coffee
    - First person in line gets served first
    - New people join at the end of the line

Operations:
- enqueue(x): add item to the rear
- dequeue(): remove item from the front

---

### 6. Trees

- **What**: A hierarchical structure with a root node and children.
- **Types**:
  - Binary Tree: each node has at most two children
  - Binary Search Tree (BST): left < root < right
  - Balanced Trees: like AVL, Red-Black Trees
- **Memory**: Nodes linked with multiple pointers (left/right).
- **Use Cases**: Sorting, searching, compilers, file systems.
- **Visualization**:

```
// Example of a Binary Search Tree (BST):

        [8]
       /   \
     [4]   [12]
    /  \   /   \
  [2] [6] [10] [14]
 /          \
[1]         [11]
```

Breakdown:
- Root node: 8
- Left subtree:
  - Node 4 has children 2 and 6
  - Node 2 has a left child 1
- Right subtree:
  - Node 12 has children 10 and 14
  - Node 10 has a right child 11

This is a binary search tree (BST) where:
- Left child < parent < right child
- It allows fast lookup, ordered traversal, and dynamic insertion

---

### 7. Hash Tables

- **What**: Store data as key-value pairs, using a hash function to map keys to array indices.
- **Memory**: Backed by an array, may include linked lists for collisions (chaining).
- **Performance**: Average ```O(1)``` for lookup/insert/delete.
- **Use Cases**: Dictionaries, symbol tables, caches, indexing.
- **Visualization**:
  ```
  Hash:  key("space") → index 3
  Table: [0] [1] [2] [ "space" : 42 ] ...
  ```

Behavior:
- Keys are passed to a hash function which returns an index
- That index determines where the data is stored in the array
- If two keys hash to the same index, they are stored together (collision handling via chaining or probing)
- Lookup, insert, and delete operations use the key to jump straight to the correct index
- Fast access is only guaranteed if the hash function distributes keys evenly

---
### 8. Graphs

- **What**: A flexible structure of connected nodes (called vertices) and edges.
- **Types**:
  - Directed / Undirected
  - Weighted / Unweighted
  - Cyclic / Acyclic
- **Memory**: Commonly stored as an adjacency list or adjacency matrix.
- **Use Cases**: Maps, networks, dependency resolution, pathfinding.
- **Visualization**:

```
 // Example of an undirected graph:

     A —— B —— C
    / \    \    \
   D   E —— F —— G
    \       /
     H ——— I
```
  
- Each letter is a node (vertex).
- Edges (lines) represent connections between nodes.
- The graph is **undirected** — connections go both ways.
- Multiple cycles exist, like: A–D–H–I–F–E–A.
- Useful for scenarios where nodes can connect in any direction and loops are allowed.
- You can traverse using algorithms like Depth-first search ```BFS``` or Breath-first search ```DFS```, to explore or search in the network.

Breakdown:
- Nodes are points (A, B, etc.), and lines are connections (edges).
- You can traverse using algorithms like Depth-first search (DFS) or Breadth-first search (BFS).
- Unlike trees, graphs may have cycles and multiple connections.

---

### When To Use What?

| Structure    | Quick Access | Growable | Insert/Delete  | Keeps Order | Use Case                         |
|--------------|--------------|----------|----------------|-------------|----------------------------------|
| Array        | ✓ (fast)     | ✗        | ✗ (shifts)     | ✓           | Data buffers, static strings     |
| Linked List  | ✗            | ✓        | ✓              | ✓           | Stack, queue, dynamic containers |
| Stack        | ✗ (only top) | ✓        | ✓ (top only)   | ✓ (LIFO)    | Backtracking, undo               |
| Queue        | ✗ (only ends)| ✓        | ✓ (ends only)  | ✓ (FIFO)    | Message passing, jobs            |
| Tree         | ✓ (sorted)   | ✓        | ✓              | ✓ (if BST)  | Search engines, expression trees |
| Hash Table   | ✓ (avg)      | ✓        | ✓              | ✗           | Key-value lookup, fast access    |
| Graph        | ✗            | ✓        | ✓              | ✗           | Routing, relationships, analysis |

---

> Understanding data structures is critical for working in C where manual memory management is involved.

---

### Data Structure Comparison Table

| Structure    | Access Time | Insert/Delete  | Dynamic Size  | Keeps Order | Use Cases                          |
|--------------|-------------|----------------|---------------|-------------|------------------------------------|
| Array        | O(1)        | O(n)           | No            | Yes         | Static data, buffers, strings      |
| Linked List  | O(n)        | O(1)*          | Yes           | Yes         | Queues, stacks, dynamic buffers    |
| Stack        | O(1) top    | O(1)           | Yes           | LIFO        | Undo, backtracking, parsers        |
| Queue        | O(1) ends   | O(1)           | Yes           | FIFO        | Job schedulers, event systems      |
| Tree (BST)   | O(log n)**  | O(log n)**     | Yes           | Yes         | Sorting, search, expression trees  |
| Hash Table   | O(1) avg    | O(1) avg       | Yes           | No          | Dictionaries, caches, lookups      |
| Graph        | Varies      | Varies         | Yes           | No          | Maps, networks, dependencies       |

* O(1) if pointer to node is known  
** Balanced trees only — unbalanced BSTs may degrade to O(n)

---

### Focus on Structs and Linked Lists

In the bonus part of **Libft**, you’ll start working directly with more complex data models, specifically: **linked lists**.

To build them, you need to understand two essential concepts in C:
- ```struct```: to define custom data types
- Linked Lists: to manage sequences of data using dynamic memory and pointers

While the mandatory part of Libft emphasizes low-level operations with **primitives** and **arrays**, the bonus introduces dynamic, pointer-driven structures that give you much more flexibility.

Understanding structs and linked lists is key for:
- Creating dynamic lists of data (e.g., command history, buffers, arguments)
- Implementing core library features in future projects
- Managing memory manually and safely in real applications

---

### What is a Structure in C?

A structure (declared with ```struct```) is a user-defined data type in C that allows grouping variables of different types under one single name.

It’s useful when you want to represent a complex data entity. For example, a point in 2D space or a node in a list.

#### Example:

```
typedef struct s_point {
    int x;
    int y;
} t_point;
```

This lets you define:

```
t_point p1;
p1.x = 10;
p1.y = 20;
```

You can access and modify each field using dot notation.

---

### What is a Linked List?

A **linked list** is a linear data structure where each element (called a **node**) contains:
- A piece of **data**
- A **pointer** to the next node

Each node is dynamically allocated and connected in sequence.

There are different types of linked lists:
- **Singly linked list**: each node points to the next node only
- **Doubly linked list**: each node points to both the next and the previous
- **Circular list**: the last node points back to the head

In Libft, we implement a singly linked list.

---

### The Node Structure Used in Libft

Libft defines its list nodes as:

```
typedef struct s_list {
    void *content;
    struct t_list *next;
} s_list;
```

#### Field Breakdown:

- ```content```: a ```void *``` pointer to hold any type of data. This makes the structure flexible — it can store a string, int, struct, or anything else.
- ```next```: a pointer to the next node in the list, or ```NULL``` if it’s the last node.

This layout allows you to construct chains of nodes.

---

### Visualizing a Linked List

Imagine three strings in a list: "Sun", "Ra", "Space"

```
+---------+      +--------+      +-----------+
| "Sun"   |  →   | "Ra"   |  →   | "Space"   | → NULL
+---------+      +--------+      +-----------+
```

Each node stores a value (via ```content```) and a pointer to the next.

---

### Key Terms

- **Head**: the first node in the list.
- **Tail**: the last node (its ```next``` is ```NULL```).
- **Traversal**: visiting all the nodes in sequence, usually via a loop.
- **Insertion**: adding a node (at the front or end).
- **Deletion**: removing one or more nodes and freeing memory.

---

### Operations You'll Implement

In the bonus part of Libft, you’ll write functions to:
- Create a node with ```ft_lstnew()```
- Add nodes at the front (```ft_lstadd_front()```) or end (```ft_lstadd_back()```)
- Count nodes (```ft_lstsize()```)
- Retrieve the last node (```ft_lstlast()```)
- Iterate through the list and apply a function (```ft_lstiter()```)
- Delete nodes or clear the whole list (```ft_lstdelone()```, ```ft_lstclear()```)
- Create a transformed copy of a list (```ft_lstmap()```)

---

### Why Use a Linked List?

Arrays are fixed in size. If you want to add or remove elements, it’s often inefficient.

Linked lists, by contrast:
- Allow dynamic size
- Make insertions and deletions fast (no shifting)
- Are useful for systems with unpredictable memory needs (like parsers, file readers, etc.)

---

### Linked List vs Array

| Feature                | Array                         | Linked List                      |
|------------------------|-------------------------------|----------------------------------|
| Memory layout          | Contiguous                    | Scattered (nodes on heap)        |
| Access (by index)      | Fast: ```arr[i]```            | Slow: must traverse nodes        |
| Insertion (middle)     | Expensive (must shift)        | Efficient (just change pointers) |
| Deletion (middle)      | Expensive (must shift)        | Efficient (pointer update)       |
| Size flexibility       | Fixed (unless reallocated)    | Dynamic (can grow/shrink)        |
| Cache friendliness     | High                          | Low                              |

---

# ft_lstnew()

## Overview

```ft_lstnew()``` creates a new node for a singly linked list.  
It allocates memory and stores the provided ```content``` pointer inside the node.

This is the starting point for building linked lists in the Libft bonus part.  
The ```next``` pointer is always initialized to ```NULL```, meaning the node doesn’t yet point to any other node.

---

## Prototype

```t_list *ft_lstnew(void *content);```

---

## How to Visualize It

You pass in some data, and ```ft_lstnew()``` wraps it into a node:

Before:
```
content: "Space Odyssey"
list:    (nothing yet)
```

After calling:  
```ft_lstnew("Space Odyssey")```

You get:
```
[ "Space Odyssey" | NULL ]
```

- The node stores the string pointer.
- It’s not linked to anything yet (next is NULL).

---

## Key Concepts

- Allocates memory for a new list node.
- The ```content``` pointer is stored directly, no copy is made.
- The ```next``` field is always set to ```NULL```.
- Returns ```NULL``` if allocation fails.

---

## Analogy

Think of placing a message into a sealed capsule and launching it into space:

- The message is your ```content```.
- The capsule is the new node.
- It's floating alone, until it's linked with others.

At this point, the capsule is ready, but it's not part of a chain yet.

---

## When To Use It?

- To create the first node of a linked list.
- To prepare a new node before inserting it at the front or back.
- While transforming or duplicating list content using functions like ```ft_lstmap()```.

---

## Downsides of ft_lstnew()

- Only stores the pointer, not a deep copy:
  - If the original data is freed or changed elsewhere, the node’s content is affected.
- You must manage memory: failure to ```free()``` leads to leaks.
- The node is standalone, linking it into a list is up to you.

---

## Example

```// Prototype: t_list *ft_lstnew(void *content);```

```
#include "libft.h"
#include <stdio.h>

int main(void)
{
    t_list *node = ft_lstnew("The answer is 42");

    if (node)
    {
        printf("Node content: %s\n", (char *)node->content);
    }

    return 0;
}
```

Output:
```
Node content: The answer is 42
```
