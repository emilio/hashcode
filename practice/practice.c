#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>

enum command_type {
    PAINT_LINE,
    PAINT_SQUARE
};

// We only generate PAINT_LINEs so...
struct command {
    enum command_type type;
    union {
        struct {
            size_t x1;
            size_t y1;
            size_t x2;
            size_t y2;
        } line;
        struct {
            size_t x;
            size_t y;
            size_t s;
        } square;
    } data;
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

void command_list_add(struct command_list* l, struct command* non_owned) {
    struct command* c = malloc(sizeof(struct command));
    assert(c);

    *c = *non_owned;
    c->next = NULL;

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

size_t line_to_right_len(uint8_t* grid,
                         uint8_t* already_painted,
                         size_t w, size_t h,
                         size_t x, size_t y,
                         size_t* wasted) {
    size_t size = 1;

    *wasted = 0;

    assert(x < w);
    assert(y < h);
    assert(grid[y * w + x]);
    assert(!already_painted[y * w + x]);

    while (x + size < w) {
       if (!grid[y * w + x + size])
           break;

       if (already_painted[y * w + x + size])
           *wasted += 1;

       size++;
    }

    return size;
}

size_t line_to_bottom_len(uint8_t* grid,
                          uint8_t* already_painted,
                          size_t w, size_t h,
                          size_t x, size_t y,
                          size_t* wasted) {
    size_t size = 1;

    *wasted = 0;

    assert(x < w);
    assert(y < h);
    assert(grid[y * w + x]);
    assert(!already_painted[y * w + x]);

    while (y + size < h) {
       if (!grid[(y + size) * w + x])
           break;

       if (already_painted[(y + size) * w + x])
           *wasted += 1;

       size++;
    }

    return size;
}

size_t square_half_side_len(uint8_t* grid,
                            uint8_t* already_painted,
                            size_t w, size_t h,
                            size_t x, size_t y,
                            size_t* wasted) {
    // TODO: This function could just use min(line_to_right, line_to_bottom)
    // and check the inner part of the square then.
    assert(x < w);
    assert(y < h);
    assert(grid[y * w + x]);
    assert(!already_painted[y * w + x]);

    size_t best_side = 0;
    size_t best_wasted = 0;
    // We start looking from size one, to the biggest possible square, and fall
    // back to size 0 if even one is too bad.
    size_t side = 1;
    while (true) {
        size_t dim = 2 * side + 1;
        size_t current_wasted = 0;

        // The best square we've got is the best we can do without going off
        // the figure.
        if (y + dim > h || x + dim > w)
            break;

        for (size_t i = 0; i < dim; ++i) {
            for (size_t j = 0; j < dim; ++j) {
                if (!grid[(y + j) * w + x + i])
                    goto end; // Here be dragons

                if (already_painted[(y + j) * w + x + i])
                    current_wasted++;
            }
        }

        // If we reached here, we've got a valid square that *might* be better
        // than the previous.
        size_t best_dim = 2 * best_side + 1;
        if (dim * dim - current_wasted > best_dim * best_dim - best_wasted) {
            best_side = side;
            best_wasted = current_wasted;
        }
        side++;
    }

end:
    *wasted = best_wasted;
    return best_side;
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
            struct command command;
            size_t r_wasted, b_wasted, square_wasted;
            size_t rlen = line_to_right_len(painting, grid, width, height, i, j, &r_wasted);
            size_t blen = line_to_bottom_len(painting, grid, width, height, i, j, &b_wasted);
            size_t square_half_side = square_half_side_len(painting, grid, width, height, i, j, &square_wasted);
            size_t square_dim = 2 * square_half_side + 1;
            size_t square_filled = square_dim * square_dim;

            // Min line len is 1
            assert(rlen);
            assert(blen);

            fprintf(stderr, "(%zu, %zu): rline(%zu - %zu), bline(%zu - %zu), square(%zu - %zu)\n", i, j,
                                                                                                   rlen, r_wasted,
                                                                                                   blen, b_wasted,
                                                                                                   square_filled, square_wasted);

            // There must always be a benefit
            assert(rlen > r_wasted);
            assert(blen > b_wasted);
            assert(square_filled > square_wasted);

            // We just do the best we can do right now, we don't look ahead
            if (rlen - r_wasted > blen - b_wasted && rlen - r_wasted > square_filled - square_wasted) {
                command.type = PAINT_LINE;
                command.data.line.x1 = i;
                command.data.line.y1 = j;
                command.data.line.x2 = i + rlen - 1;
                command.data.line.y2 = j;

                while (rlen--) {
                    assert(PAINTING(i + rlen, j));
                    GRID(i + rlen, j) = 1;
                }
            } else if (blen - b_wasted > square_filled - square_wasted) {
                command.type = PAINT_LINE;
                command.data.line.x1 = i;
                command.data.line.y1 = j;
                command.data.line.x2 = i;
                command.data.line.y2 = j + blen -1;

                while (blen--) {
                    assert(PAINTING(i, j + blen));
                    GRID(i, j + blen) = 1;
                }
            } else {
                command.type = PAINT_SQUARE;
                command.data.square.s = square_half_side;
                command.data.square.x = i + square_half_side;
                command.data.square.y = j + square_half_side;
                size_t dim = 2 * square_half_side + 1;
                for (size_t ii = 0; ii < dim; ++ii) {
                    for (size_t jj = 0; jj < dim; ++jj) {
                        assert(PAINTING(i + ii, j + jj));
                        GRID(i + ii, j + jj) = 1;
                    }
                }
            }

            command_list_add(&list, &command);
        }
    }

#undef GRID
#undef PAINTING

    ensure_grid_eq(painting, grid, width, height);

    printf("%zu\n", list.len);
    struct command* current = list.head;
    while (current) {
        switch (current->type) {
            case PAINT_LINE:
                assert(current->data.line.x1 < width);
                assert(current->data.line.x2 < width);
                assert(current->data.line.y1 < height);
                assert(current->data.line.y2 < height);
                // printf("PAINT_LINE %zu %zu %zu %zu\n", current->x1, current->y1, current->x2, current->y2);
                // row, col, row, col
                printf("PAINT_LINE %zu %zu %zu %zu\n", current->data.line.y1,
                                                       current->data.line.x1,
                                                       current->data.line.y2,
                                                       current->data.line.x2);
                break;
            case PAINT_SQUARE:
                assert(current->data.square.x >= current->data.square.s);
                assert(current->data.square.y >= current->data.square.s);
                assert(current->data.square.x + current->data.square.s < width);
                assert(current->data.square.y + current->data.square.s < height);
                printf("PAINT_SQUARE %zu %zu %zu\n", current->data.square.y,
                                                     current->data.square.x,
                                                     current->data.square.s);
                break;
            default:
                assert(0 && "Invalid type");
        }
        current = current->next;
    }

    command_list_free(&list);
    free(painting);
    free(grid);

    return 0;
}
