#include <stdio.h>
#include <stdlib.h>
#include <time.h> 

/* Board: [row][col][stack] 
   Values: -1 (empty), 0-2 (Player pieces), 3-5 (CPU pieces) */
signed char board[3][3][3]; 
int height[3][3];
int inven[2][3]; // [player][size]
int pl = 0;      // 0 = Human, 1 = CPU

void flush() { int c; while((c=getchar())!='\n' && c!=EOF); }

void init() {
    int r, c, i;
    srand(time(NULL));
    for(r=0; r<3; r++) {
        for(c=0; c<3; c++) {
            height[r][c] = 0;
            for(i=0; i<3; i++) board[r][c][i] = -1;
        }
    }
    for(i=0; i<3; i++) {
        inven[0][i] = 2; // Human
        inven[1][i] = 2; // CPU
    }
}

void show() {
    int r, c;
    printf("\033[H\033[2J"); 
    printf("    0    1    2\n\n");
    for(r=0; r<3; r++) {
        printf("%d ", r);
        for(c=0; c<3; c++) {
            int h = height[r][c];
            if (h == 0) printf("[ . ]");
            else {
                char p = board[r][c][h-1];
                if (p < 3) printf("[ %c ]", p + 'A'); // Player A, B, C
                else printf("[ %c ]", (p-3) + '0');   // CPU 0, 1, 2
            }
        }
        printf("\n\n");
    }
    printf("YOU "); for(r=0;r<3;r++) for(c=0;c<inven[0][r];c++) putchar(r+'A'); putchar('\n');
    printf("CPU "); for(r=0;r<3;r++) for(c=0;c<inven[1][r];c++) putchar(r+'0'); putchar('\n');
}

int check_win() {
    int i, r, c, top[3][3];
    for(r=0; r<3; r++) {
        for(c=0; c<3; c++) {
            if (height[r][c] == 0) top[r][c] = -1;
            else top[r][c] = board[r][c][height[r][c]-1] / 3;
        }
    }
    for(i=0; i<3; i++) {
        if(top[i][0] != -1 && top[i][0] == top[i][1] && top[i][1] == top[i][2]) return top[i][0];
        if(top[0][i] != -1 && top[0][i] == top[1][i] && top[1][i] == top[2][i]) return top[0][i];
    }
    if(top[0][0] != -1 && top[0][0] == top[1][1] && top[1][1] == top[2][2]) return top[0][0];
    if(top[0][2] != -1 && top[0][2] == top[1][1] && top[1][1] == top[2][0]) return top[0][2];
    return -1;
}

int piece_new(int p, int sz, int r, int c) {
    if (sz < 0 || sz > 2 || r < 0 || r > 2 || c < 0 || c > 2) return 0;
    if (inven[p][sz] <= 0) return 0;
    if (height[r][c] > 0 && (board[r][c][height[r][c]-1] % 3) >= sz) return 0;
    board[r][c][height[r][c]++] = (p * 3) + sz;
    inven[p][sz]--;
    return 1;
}

int piece_move(int p, int r1, int c1, int r2, int c2) {
    int p_val, target_sz;
    if (r1<0 || r1>2 || c1<0 || c1>2 || r2<0 || r2>2 || c2<0 || c2>2) return 0;
    if (height[r1][c1] == 0) return 0;
    p_val = board[r1][c1][height[r1][c1]-1];
    if (p_val / 3 != p) return 0;
    if (height[r2][c2] > 0) {
        target_sz = board[r2][c2][height[r2][c2]-1] % 3;
        if ((p_val % 3) <= target_sz) return 0;
    }
    board[r2][c2][height[r2][c2]++] = p_val;
    board[r1][c1][--height[r1][c1]] = -1;
    return 1;
}

void cpu_play() {
    int r, c, sz;
    printf("CPU SEARCHING...\n");

    // 1. VICTORY CHECK
    for (sz = 2; sz >= 0; sz--) {
        if (inven[1][sz] <= 0) continue;
        for (r = 0; r < 3; r++) {
            for (c = 0; c < 3; c++) {
                if (piece_new(1, sz, r, c)) {
                    if (check_win() == 1) return; 
                    inven[1][sz]++; board[r][c][--height[r][c]] = -1; 
                }
            }
        }
    }

    // 2. DEFENSIVE GOBBLE-BLOCK (Identify Winning Threats)
    for (r = 0; r < 3; r++) {
        for (c = 0; c < 3; c++) {
            if (height[r][c] < 3) {
                // If the player placing a Large piece here wins...
                board[r][c][height[r][c]++] = 2; // Virtual player piece
                if (check_win() == 0) {
                    board[r][c][--height[r][c]] = -1; 
                    // Priority block: Try to GOBBLE if possible
                    for (sz = 2; sz >= 0; sz--) {
                        if (piece_new(1, sz, r, c)) return; 
                    }
                } else {
                    board[r][c][--height[r][c]] = -1; 
                }
            }
        }
    }

    // 3. TACTICAL PRESSURE (Cover player pieces)
    for (sz = 2; sz >= 1; sz--) {
        if (inven[1][sz] > 0) {
            for (r = 0; r < 3; r++) {
                for (c = 0; c < 3; c++) {
                    if (height[r][c] > 0 && (board[r][c][height[r][c]-1] / 3 == 0)) {
                        if (piece_new(1, sz, r, c)) return;
                    }
                }
            }
        }
    }

    // 4. RANDOM FALLBACK
    while (1) {
        sz = rand() % 3; r = rand() % 3; c = rand() % 3;
        if (piece_new(1, sz, r, c)) return;
    }
}

void input() {
    int t, sz, r, c, r2, c2;
    printf("\nYOU (1:NEW 2:MOV): ");
    fflush(stdout);
    if (scanf("%d", &t) != 1) { flush(); return; }
    flush();
    if(t == 1) {
        printf("SIZE(0-2) R C: ");
        fflush(stdout);
        if (scanf("%d %d %d", &sz, &r, &c) == 3) if(piece_new(0, sz, r, c)) pl = 1;
        flush();
    } else if (t == 2) {
        printf("FROM R C: ");
        fflush(stdout);
        if (scanf("%d %d", &r, &c) == 2) {
            printf("TO R C: ");
            fflush(stdout);
            if (scanf("%d %d", &r2, &c2) == 2) if(piece_move(0, r, c, r2, c2)) pl = 1;
        }
        flush();
    }
}

int main(void) {
    int win;
    init();
    while(1) {
        show();
        win = check_win();
        if (win != -1) {
            if (win == 0) printf("\n*** YOU WIN! ***\n");
            else printf("\n*** CPU WINS! ***\n");
            break;
        }
        if (pl == 0) input();
        else { cpu_play(); pl = 0; }
    }
    return 0;
}


