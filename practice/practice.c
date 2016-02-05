#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>

// We only generate PAINT_LINEs so...
struct command {
    size_t x1;
    size_t y1;
    size_t x2;
    size_t y2;
    struct command* next;
};

struct command_list {
    struct command* head;
    struct command* tail;
    size_t len;
};

#define COMMAND_LIST_INITIALIZER {NULL, NULL, 0}

void command_list_add_owned(struct command_list* l, struct command* c) {
    // This assumes c has no linked nodes
    assert(!c->next);

    if (l->head == NULL) {
        assert(!l->tail);
        l->head = l->tail = c;
    } else {
        l->tail->next = c;
        l->tail = c;
    }
    l->len++;
}

void command_list_add(struct command_list* l, size_t x1, size_t y1, size_t x2, size_t y2) {
    struct command* c = malloc(sizeof(struct command));
    assert(c);

    c->next = NULL;
    c->x1 = x1;
    c->y1 = y1;
    c->x2 = x2;
    c->y2 = y2;

    command_list_add_owned(l, c);
}

void command_list_free(struct command_list* l) {
    struct command* c = l->head;
    struct command* prev;
    while (c) {
        prev = c;
        c = c->next;

        free(prev);
    }
}

size_t line_to_right_len(uint8_t* grid, size_t w, size_t h, size_t x, size_t y) {
    size_t size = 1;

    assert(x < w);
    assert(y < h);
    assert(grid[y * w + x]);

    while (x + size < w) {
       if (grid[y * w + x + size]) {
           size++;
       } else {
           break;
       }
    }

    return size;
}

size_t line_to_bottom_len(uint8_t* grid, size_t w, size_t h, size_t x, size_t y) {
    size_t size = 1;

    assert(x < w);
    assert(y < h);
    assert(grid[y * w + x]);

    while (y + size < h) {
       if (grid[(y + size) * w + x]) {
           size++;
       } else {
           break;
       }
    }


    return size;
}

void ensure_grid_eq(uint8_t* a, uint8_t* b, size_t w, size_t h) {
    size_t total = w * h;

    for (size_t i = 0; i < total; ++i)
        assert(a[i] == b[i]);
}

int main(int argc, char** argv) {
    assert(argc > 1);

    FILE* f = fopen(argv[1], "r");
    assert(f);

    unsigned int width = 0, height = 0;
    fscanf(f, "%u %u\n", &height, &width);

    assert(width > 0 && width < 1000);
    assert(height > 0 && height < 1000);

    fprintf(stderr, "w: %u, h: %u\n", width, height);

    // + 1 => extra \0 added by fgets
    uint8_t* painting = calloc(width * height + 1, 1);
    uint8_t* grid = calloc(width * height, 1);
    struct command_list list = COMMAND_LIST_INITIALIZER;

    assert(painting);
    assert(grid);

    unsigned int to_read = height;
    while (to_read--) {
        unsigned int offset = (height - to_read - 1) * width;
        size_t read = fread(painting + offset, 1, width, f);
        assert(read == width);
        (void)fgetc(f); // Discard newline
    }

    size_t total = width * height;
    for (size_t i = 0; i < total; ++i) {
        switch (painting[i]) {
            case '.':
                painting[i] = 0;
                break;
            case '#':
                painting[i] = 1;
                break;
            default:
                assert(0 && "Unexpected character found");
        }
    }

#define GRID(x, y) grid[((y) * width) + x]
#define PAINTING(x, y) painting[((y) * width) + x]

    for (size_t i = 0; i < width; ++i) {
        for (size_t j = 0; j < height; ++j) {
            if (!PAINTING(i, j) || GRID(i, j))
                continue;

            size_t rlen = line_to_right_len(painting, width, height, i, j);
            size_t blen = line_to_bottom_len(painting, width, height, i, j);

            // Min line len is 1
            assert(rlen);
            assert(blen);

            if (rlen > blen) {
                command_list_add(&list, i, j, i + rlen - 1, j);
                while (rlen--) {
                    assert(PAINTING(i + rlen, j));
                    GRID(i + rlen, j) = 1;
                }
            } else {
                command_list_add(&list, i, j, i, j + blen - 1);
                while (blen--) {
                    assert(PAINTING(i, j + blen));
                    GRID(i, j + blen) = 1;
                }
            }
        }
    }

#undef GRID
#undef PAINTING

    ensure_grid_eq(painting, grid, width, height);

    printf("%zu\n", list.len);
    struct command* current = list.head;
    while (current) {
        assert(current->x1 < width);
        assert(current->x2 < width);
        assert(current->y1 < height);
        assert(current->y2 < height);
        // printf("PAINT_LINE %zu %zu %zu %zu\n", current->x1, current->y1, current->x2, current->y2);
        // row, col, row, col
        printf("PAINT_LINE %zu %zu %zu %zu\n", current->y1, current->x1, current->y2, current->x2);
        current = current->next;
    }

    command_list_free(&list);
    free(painting);
    free(grid);

    return 0;
}
