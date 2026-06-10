#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define WIDTH 80
#define HEIGHT 35
#define MAX_SHAPES 50

// Shape types
typedef enum { CIRCLE, RECTANGLE, LINE, TRIANGLE } ShapeType;

// Structure to hold shape data
typedef struct {
    ShapeType type;
    int id;
    int active; // 1 if exists, 0 if deleted
    int x1, y1, x2, y2, x3, y3; // Coordinates
    int radius;                 // For circle
} Shape;

// Global Canvas and Shape Repository
char canvas[HEIGHT][WIDTH];
Shape shapes[MAX_SHAPES];
int shape_count = 0;
int next_id = 1;

// Helper to flush leftover characters from stdin
void flush_input() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// --- Canvas Functions ---

void clear_canvas() {
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            canvas[i][j] = '_';
        }
    }
}

void display_canvas() {
    // Print column headers (units digit) for scaling assistance
    printf("   ");
    for (int j = 0; j < WIDTH; j++) printf("%d", j % 10);
    printf("\n");

    for (int i = 0; i < HEIGHT; i++) {
        printf("%2d ", i); // Row header
        for (int j = 0; j < WIDTH; j++) {
            putchar(canvas[i][j]);
        }
        putchar('\n');
    }
}

void set_pixel(int x, int y) {
    if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
        canvas[y][x] = '*';
    }
}

// --- Primitive Drawing Algorithms ---

// Bresenham's Line Algorithm
void draw_line(int x0, int y0, int x1, int y1) {
    int dx = abs(x1 - x0);
    int sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0);
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;
    int e2;

    while (1) {
        set_pixel(x0, y0);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { 
            err += dy; 
            x0 += sx; 
        }
        if (e2 <= dx) { 
            err += dx; 
            y0 += sy; 
        }
    }
}

// Fixed Midpoint Circle Algorithm with 2:1 Font Scale Adjustment
void draw_circle(int xc, int yc, int r) {
    int x = 0;
    int y = r;
    int d = 3 - 2 * r;
    
    while (y >= x) {
        // Step x and step y must be adjusted horizontally by 2.1x 
        // to counter terminal text height-to-width distortion.
        set_pixel(xc + (int)(x * 2.1), yc + y);
        set_pixel(xc - (int)(x * 2.1), yc + y);
        set_pixel(xc + (int)(x * 2.1), yc - y);
        set_pixel(xc - (int)(x * 2.1), yc - y);
        set_pixel(xc + (int)(y * 2.1), yc + x);
        set_pixel(xc - (int)(y * 2.1), yc + x);
        set_pixel(xc + (int)(y * 2.1), yc - x);
        set_pixel(xc - (int)(y * 2.1), yc - x);
        
        x++;
        if (d > 0) {
            y--;
            d = d + 4 * (x - y) + 10;
        } else {
            d = d + 4 * x + 6;
        }
    }
}

void draw_rectangle(int x1, int y1, int x2, int y2) {
    draw_line(x1, y1, x2, y1); 
    draw_line(x2, y1, x2, y2); 
    draw_line(x2, y2, x1, y2); 
    draw_line(x1, y2, x1, y1); 
}

void draw_triangle(int x1, int y1, int x2, int y2, int x3, int y3) {
    draw_line(x1, y1, x2, y2);
    draw_line(x2, y2, x3, y3);
    draw_line(x3, y3, x1, y1);
}

// --- Engine Core ---

void render_all() {
    clear_canvas();
    for (int i = 0; i < shape_count; i++) {
        if (!shapes[i].active) continue;

        switch (shapes[i].type) {
            case CIRCLE:
                draw_circle(shapes[i].x1, shapes[i].y1, shapes[i].radius);
                break;
            case RECTANGLE:
                draw_rectangle(shapes[i].x1, shapes[i].y1, shapes[i].x2, shapes[i].y2);
                break;
            case LINE:
                draw_line(shapes[i].x1, shapes[i].y1, shapes[i].x2, shapes[i].y2);
                break;
            case TRIANGLE:
                draw_triangle(shapes[i].x1, shapes[i].y1, shapes[i].x2, shapes[i].y2, shapes[i].x3, shapes[i].y3);
                break;
        }
    }
}

int find_shape_index(int id) {
    for (int i = 0; i < shape_count; i++) {
        if (shapes[i].active && shapes[i].id == id) {
            return i;
        }
    }
    return -1;
}

void list_shapes() {
    printf("\n--- Active Shapes ---\n");
    int count = 0;
    for (int i = 0; i < shape_count; i++) {
        if (shapes[i].active) {
            count++;
            printf("ID: %d | ", shapes[i].id);
            switch (shapes[i].type) {
                case CIRCLE:    printf("Circle (Center: %d,%d, Radius: %d)\n", shapes[i].x1, shapes[i].y1, shapes[i].radius); break;
                case RECTANGLE: printf("Rectangle (Top-Left: %d,%d, Bottom-Right: %d,%d)\n", shapes[i].x1, shapes[i].y1, shapes[i].x2, shapes[i].y2); break;
                case LINE:      printf("Line (From: %d,%d to %d,%d)\n", shapes[i].x1, shapes[i].y1, shapes[i].x2, shapes[i].y2); break;
                case TRIANGLE:  printf("Triangle (P1: %d,%d, P2: %d,%d, P3: %d,%d)\n", shapes[i].x1, shapes[i].y1, shapes[i].x2, shapes[i].y2, shapes[i].x3, shapes[i].y3); break;
            }
        }
    }
    if (count == 0) printf("No shapes currently on canvas.\n");
    printf("---------------------\n");
}

// --- Menu Interaction Handlers ---

void add_shape_menu() {
    if (shape_count >= MAX_SHAPES) {
        printf("Error: Maximum shape limit reached!\n");
        return;
    }

    int choice;
    printf("\nSelect Shape to Add:\n1. Circle\n2. Rectangle\n3. Line\n4. Triangle\nChoice: ");
    if (scanf("%d", &choice) != 1) { flush_input(); return; }

    Shape s;
    s.id = next_id++;
    s.active = 1;

    switch (choice) {
        case 1:
            s.type = CIRCLE;
            printf("Enter Center X (0-79), Center Y (0-34), and Radius: ");
            scanf("%d %d %d", &s.x1, &s.y1, &s.radius);
            break;
        case 2:
            s.type = RECTANGLE;
            printf("Enter Top-Left X, Y and Bottom-Right X, Y: ");
            scanf("%d %d %d %d", &s.x1, &s.y1, &s.x2, &s.y2);
            break;
        case 3:
            s.type = LINE;
            printf("Enter X1, Y1 and X2, Y2: ");
            scanf("%d %d %d %d", &s.x1, &s.y1, &s.x2, &s.y2);
            break;
        case 4:
            s.type = TRIANGLE;
            printf("Enter X1,Y1, X2,Y2, and X3,Y3: ");
            scanf("%d %d %d %d %d %d", &s.x1, &s.y1, &s.x2, &s.y2, &s.x3, &s.y3);
            break;
        default:
            printf("Invalid choice!\n");
            return;
    }
    flush_input();

    shapes[shape_count++] = s;
    printf("Shape added successfully with ID: %d\n", s.id);
}

void delete_shape_menu() {
    list_shapes();
    int id;
    printf("Enter the ID of the shape to delete: ");
    if (scanf("%d", &id) != 1) { flush_input(); return; }
    flush_input();

    int index = find_shape_index(id);
    if (index != -1) {
        shapes[index].active = 0;
        printf("Shape ID %d deleted.\n", id);
    } else {
        printf("Shape ID not found.\n");
    }
}

void modify_shape_menu() {
    list_shapes();
    int id;
    printf("Enter the ID of the shape to modify: ");
    if (scanf("%d", &id) != 1) { flush_input(); return; }
    flush_input();

    int index = find_shape_index(id);
    if (index == -1) {
        printf("Shape ID not found.\n");
        return;
    }

    Shape *s = &shapes[index];
    printf("Modifying ");
    switch (s->type) {
        case CIRCLE:
            printf("Circle. Enter new Center X, Center Y, and Radius: ");
            scanf("%d %d %d", &s->x1, &s->y1, &s->radius);
            break;
        case RECTANGLE:
            printf("Rectangle. Enter new Top-Left X, Y and Bottom-Right X, Y: ");
            scanf("%d %d %d %d", &s->x1, &s->y1, &s->x2, &s->y2);
            break;
        case LINE:
            printf("Line. Enter new X1, Y1 and X2, Y2: ");
            scanf("%d %d %d %d", &s->x1, &s->y1, &s->x2, &s->y2);
            break;
        case TRIANGLE:
            printf("Triangle. Enter new X1,Y1, X2,Y2, and X3,Y3: ");
            scanf("%d %d %d %d %d %d", &s->x1, &s->y1, &s->x2, &s->y2, &s->x3, &s->y3);
            break;
    }
    flush_input();
    printf("Shape ID %d updated successfully.\n", id);
}

int main() {
    int choice = 0;
    clear_canvas();

    while (choice != 5) {
        render_all();
        printf("\n=================================================================================\n");
        printf("                               2D GRAPHICS EDITOR                                \n");
        printf("=================================================================================\n");
        display_canvas();
        
        printf("\n1. Add Object\n2. Delete Object\n3. Modify Object\n4. List Objects\n5. Exit\n");
        printf("Enter your choice: ");
        
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input type. Please enter a number.\n");
            flush_input();
            continue;
        }
        flush_input();

        switch (choice) {
            case 1: add_shape_menu(); break;
            case 2: delete_shape_menu(); break;
            case 3: modify_shape_menu(); break;
            case 4: list_shapes(); break;
            case 5: printf("Exiting Editor.\n"); break;
            default: printf("Invalid choice, try again.\n");
        }
    }
    return 0;
}