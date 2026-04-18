#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <time.h>

// Board: 9 positions, each can hold 3 pieces (0=bottom, 2=top)
// Piece value: 0 (empty), 1-3 (Player sizes 0-2), 4-6 (CPU sizes 0-2)
unsigned char board[27]; 
unsigned char height[9];
unsigned char inven[6]; // [0-2] Player, [3-5] CPU
unsigned char turn = 0; // 0 = Player, 1 = CPU

// Fast lookups for winning lines (9 positions: 0-8)
const unsigned char wins[8][3] = {
    {0,1,2}, {3,4,5}, {6,7,8}, // Rows
    {0,3,6}, {1,4,7}, {2,5,8}, // Cols
    {0,4,8}, {2,4,6}           // Diags
};

void init_game() {
    unsigned char i;
    for(i=0; i<27; i++) board[i] = 0;
    for(i=0; i<9; i++) height[i] = 0;
    for(i=0; i<6; i++) inven[i] = 2;
    srand(time(NULL));
}

// Get piece at top of stack for board position 'pos'
// Returns 0 if empty, otherwise 1-6
unsigned char get_top(unsigned char pos) {
    if (height[pos] == 0) return 0;
    return board[(pos * 3) + height[pos] - 1];
}

void show_board() {
    unsigned char i, h, p;
    clrscr();
    cprintf("  0 1 2\r\n");
    for(i=0; i<9; i++) {
        if (i % 3 == 0) cprintf("%d ", i/3);
        p = get_top(i);
        if (p == 0) cputs("[.]");
        else {
            // Player: A,B,C | CPU: 0,1,2
            if (p < 4) cprintf("[%c]", (p-1) + 'A');
            else cprintf("[%c]", (p-4) + '0');
        }
        if (i % 3 == 2) cputs("\r\n\r\n");
    }
    cprintf("YOU: ");
    for(i=0; i<3; i++) for(h=0; h<inven[i]; h++) cputc(i+'A');
    cputs("\r\nCPU: ");
    for(i=3; i<6; i++) for(h=0; h<inven[i]; h++) cputc((i-3)+'0');
    cputs("\r\n");
}

signed char check_win() {
    unsigned char i, p1, p2, p3;
    for(i=0; i<8; i++) {
        p1 = get_top(wins[i][0]); if (p1 == 0) continue;
        p2 = get_top(wins[i][1]); if (p2 == 0) continue;
        p3 = get_top(wins[i][2]); if (p3 == 0) continue;
        
        // Check if all three top pieces belong to the same player
        // Player pieces are 1-3 ( < 4), CPU pieces are 4-6 ( > 3)
        if (p1 < 4 && p2 < 4 && p3 < 4) return 0;
        if (p1 > 3 && p2 > 3 && p3 > 3) return 1;
    }
    return -1;
}

// Logic: piece is 1-3 or 4-6. Size is (p-1)%3. 
unsigned char get_size(unsigned char p) {
    if (p > 3) return p - 4;
    return p - 1;
}

unsigned char try_new(unsigned char p_idx, unsigned char sz, unsigned char pos) {
    unsigned char top = get_top(pos);
    if (inven[p_idx * 3 + sz] == 0) return 0;
    if (top != 0 && get_size(top) >= sz) return 0;
    
    board[(pos * 3) + height[pos]] = (p_idx * 3) + sz + 1;
    height[pos]++;
    inven[p_idx * 3 + sz]--;
    return 1;
}

unsigned char try_move(unsigned char p_idx, unsigned char from, unsigned char to) {
    unsigned char p_val, target_sz;
    if (height[from] == 0) return 0;
    p_val = get_top(from);
    if ((p_idx == 0 && p_val > 3) || (p_idx == 1 && p_val < 4)) return 0;
    
    if (height[to] > 0) {
        if (get_size(get_top(to)) >= get_size(p_val)) return 0;
    }
    
    board[(to * 3) + height[to]++] = p_val;
    board[(from * 3) + --height[from]] = 0;
    return 1;
}

/* --- MINIMAX --- */

int evaluate() {
    signed char win = check_win();
    if (win == 1) return 100;
    if (win == 0) return -100;
    return 0; 
}

int minimax(unsigned char depth, int alpha, int beta, unsigned char is_max) {
    unsigned char i, j, sz, p_val;
    int val, best;
    signed char score = evaluate();

    if (depth == 0 || score != 0) return score;

    if (is_max) {
        best = -127;
        for (sz = 3; sz-- > 0; ) {
            if (inven[3 + sz] == 0) continue;
            for (i = 0; i < 9; i++) {
                if (try_new(1, sz, i)) {
                    val = minimax(depth - 1, alpha, beta, 0);
                    inven[3 + sz]++; height[i]--;
                    if (val > best) best = val;
                    if (best > alpha) alpha = best;
                    if (beta <= alpha) return best;
                }
            }
        }
        return best;
    } else {
        best = 127;
        for (sz = 3; sz-- > 0; ) {
            if (inven[sz] == 0) continue;
            for (i = 0; i < 9; i++) {
                if (try_new(0, sz, i)) {
                    val = minimax(depth - 1, alpha, beta, 1);
                    inven[sz]++; height[i]--;
                    if (val < best) best = val;
                    if (best < beta) beta = best;
                    if (beta <= alpha) return best;
                }
            }
        }
        return best;
    }
}

void cpu_play() {
    unsigned char i, sz, b_sz, b_pos;
    int v, b_v = -127;
    cprintf("CPU THINKING...");
    for (sz = 3; sz-- > 0; ) {
        if (inven[3 + sz] == 0) continue;
        for (i = 0; i < 9; i++) {
            if (try_new(1, sz, i)) {
                v = minimax(2, -127, 127, 0); // Depth 2 for speed
                inven[3 + sz]++; height[i]--;
                if (v > b_v) { b_v = v; b_sz = sz; b_pos = i; }
            }
        }
    }
    try_new(1, b_sz, b_pos);
}

void player_input() {
    char cmd;
    unsigned char sz, r, c, from, to;
    cprintf("\n(N)EW OR (M)OVE? ");
    cmd = cgetc();
    if (cmd == 'n' || cmd == 'N') {
        cprintf("\nSZ(0-2) POS(0-8): ");
        sz = cgetc() - '0';
        from = cgetc() - '0';
        if (try_new(0, sz, from)) turn = 1;
    } else if (cmd == 'm' || cmd == 'M') {
        cprintf("\nFROM(0-8) TO(0-8): ");
        from = cgetc() - '0';
        to = cgetc() - '0';
        if (try_move(0, from, to)) turn = 1;
    }
}

int main() {
    signed char win;
    init_game();
    while(1) {
        show_board();
        win = check_win();
        if (win != -1) {
            if (win == 0) cputs("\r\n*** YOU WIN! ***");
            else cputs("\r\n*** CPU WINS! ***");
            break;
        }
        if (turn == 0) player_input();
        else { cpu_play(); turn = 0; }
    }
    while(1); 
    return 0;
}