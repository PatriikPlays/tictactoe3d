#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <termios.h>
#include <unistd.h>
#include <stdbool.h>

int index3Dto1D(int x, int y, int z, int size) {
    return z*size*size + y*size + x;
}

bool check_direction(const unsigned char* grid, int size, int winline, int startX, int startY, int startZ, int dx, int dy, int dz) {
    unsigned char current = grid[index3Dto1D(startX, startY, startZ, size)];
    if (current == 0) return false;

    int count = 1;
    int x = startX + dx;
    int y = startY + dy;
    int z = startZ + dz;

    while (x >= 0 && x < size && y >= 0 && y < size && z >= 0 && z < size) {
        if (grid[index3Dto1D(x, y, z, size)] == current) {
            count++;
            if (count == winline) return true;
        } else {
            break;
        }
        x += dx;
        y += dy;
        z += dz;
    }

    return false;
}

bool check_win(const unsigned char* grid, int size, int winline) {
    int directions[][3] = {
        {1, 0, 0}, {0, 1, 0}, {0, 0, 1},
        {1, 1, 0}, {1, 0, 1}, {0, 1, 1},
        {1, 1, 1}, {-1, 1, 1}, {1, -1, 1}, {1, 1, -1}
    };

    for (int z = 0; z < size; ++z) {
        for (int y = 0; y < size; ++y) {
            for (int x = 0; x < size; ++x) {
                for (unsigned dir = 0; dir < sizeof(directions) / sizeof(directions[0]); ++dir) {
                    if (check_direction(grid, size, winline, x, y, z, directions[dir][0], directions[dir][1], directions[dir][2])) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

void draw(int x, int y, int z, const unsigned char* grid, int size, size_t grid_size, int player) {
    printf("\033[2J\033[H");
    printf("z:  %02d\n", z);
    printf("plr: %d\n", player);
    
    for (int i = 0; i < size*2-1; i++) {
        printf("-");
    }
    printf("\n");

    for (int yl = 0; yl < size; yl++) {
        for (int xl = 0; xl < size; xl++) {
            char c = '-';

            if (grid[z*size*size+yl*size+xl] == 1) {
                c = 'X';
            } else if (grid[z*size*size+yl*size+xl] == 2) {
                c = 'O';
            }

            printf("%s%c\033[0m", (yl==y && xl==x ? "\033[41m" : ""), c);
            if (xl != size-1) printf(" ");
        }
        printf("\n");
    }
}

void enableRawMode(struct termios *origTermios) {
    struct termios raw;
    tcgetattr(STDIN_FILENO, origTermios);
    raw = *origTermios;
    raw.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void disableRawMode(struct termios *origTermios) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, origTermios);
}

char get_inp() {
    char buffer[3] = {0};

    struct termios origTermios;
    enableRawMode(&origTermios);
    size_t bytesRead = read(STDIN_FILENO, buffer, sizeof(buffer));
    disableRawMode(&origTermios);

    if (bytesRead == 1) {
        return buffer[0];
    } else if (bytesRead == 3 && buffer[0] == 27 && buffer[1] == 91) {
        // Arrow keys
        switch (buffer[2]) {
            case 'A':
                return -1;  // Up arrow
            case 'B':
                return -2;  // Down arrow
            case 'C':
                return -3;  // Right arrow
            case 'D':
                return -4;  // Left arrow
        }
    }

    return 0;  // Unrecognized input
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: %s <size> <winline>\n", argv[0]);
        return 1;
    }

    int size = atoi(argv[1]);
    int winline = atoi(argv[2]);

    if (size < 2 || size > 512) {
        printf("Size should be within [3;15], inclusive\n");
        return 1;
    }

    if (winline < 2 || winline > 512) {
        printf("Winline should be within [3;15], inclusive\n");
        return 1;
    }

    unsigned char* grid = malloc(pow(size, 3));

    int x = 0;
    int y = 0;
    int z = 0;

    unsigned char player = 0;

    while (1) {
        draw(x, y, z, grid, size, pow(size, 3), player);

        if (check_win(grid, size, winline)) {
            printf("\nPlayer %d won\n", (player + 1) % 2);
            return 0;
        }

        char ch = get_inp();   
        printf("%c", ch);
        if (ch == 'w') {
            z = z+1;
        } else if (ch == 's') {
            z = z-1;
        } else if (ch == -1) {
            y = y-1;
        } else if (ch == -2) {
            y = y+1;
        } else if (ch == -3) {
            x = x+1;
        } else if (ch == -4) {
            x = x-1;
        } else if (ch == 10 && grid[z*size*size+y*size+x] == 0) {
            grid[z*size*size+y*size+x] = player + 1;
            player = (player + 1) % 2;
        }

        x = fmin(fmaxl(x, 0),size-1);
        y = fmin(fmaxl(y, 0),size-1);
        z = fmin(fmaxl(z, 0),size-1);
    }

    return 0;
}