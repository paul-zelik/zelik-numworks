#include <eadk.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

const char eadk_app_name[] __attribute__((section(".rodata.eadk_app_name"))) = "Maze";
const uint32_t eadk_api_level __attribute__((section(".rodata.eadk_api_level"))) = 0;

#define WIDTH 21
#define HEIGHT 17
#define CELL_W (EADK_SCREEN_WIDTH / WIDTH)
#define CELL_H (222 / HEIGHT)

uint16_t map[WIDTH][HEIGHT];
eadk_color_t colors[WIDTH][HEIGHT]; // Add here

// Fills the entire screen with a color
void fill_background(eadk_color_t color) {
  eadk_display_push_rect_uniform((eadk_rect_t){0, 0, EADK_SCREEN_WIDTH, EADK_SCREEN_HEIGHT}, color);
}

// Replaces all occurrences of n1 with n2 in the map
void replace_all(uint16_t n1, uint16_t n2) {
  for (int x = 0; x < WIDTH; x++) {
    for (int y = 0; y < HEIGHT; y++) {
      if (map[x][y] == n1) {
        map[x][y] = n2;
      }
    }
  }
}

// Returns the values of left/right or top/bottom neighbors depending on y
void get_neighbors(int x, int y, uint16_t *a, uint16_t *b) {
  if (y % 2) { // Horizontal wall
    *a = map[x - 1][y];
    *b = map[x + 1][y];
  } else { // Vertical wall
    *a = map[x][y - 1];
    *b = map[x][y + 1];
  }
}

void draw_maze() {
  for (int x = 0; x < WIDTH; x++) {
    for (int y = 0; y < HEIGHT; y++) {
      eadk_rect_t rect = {
        .x = x * CELL_W,
        .y = y * CELL_H,
        .width = CELL_W,
        .height = CELL_H
      };
      // Use the pre-calculated color from the 'colors' array
      eadk_display_push_rect_uniform(rect, colors[x][y]);
    }
  }
}

void generate_maze() {
  // Initialization of the map and fixed colors
  for (int x = 0; x < WIDTH; x++) {
    for (int y = 0; y < HEIGHT; y++) {
      if (x % 2 == 1 && y % 2 == 1) { // Only for cells, not walls
        map[x][y] = (x / 2) + (y / 2) * (WIDTH / 2) + 1;
        colors[x][y] = (eadk_color_t)(eadk_random() & 0xFFFF); // assign a random color to each distinct region
      } else {
        map[x][y] = 0;
        colors[x][y] = eadk_color_black; // walls in black
      }
    }
  }

  // Generates the list of possible positions
  int max_poss = (WIDTH / 2) * (HEIGHT - 2);
  int poss_x[max_poss], poss_y[max_poss], poss_len = 0;

  for (int y = 1; y < HEIGHT - 1; y++) {
    for (int x = 1; x < WIDTH - 1; x++) { // Iterate through all potential wall positions
        if ((x % 2 == 0 && y % 2 == 1) || (x % 2 == 1 && y % 2 == 0)) { // Only walls that separate two cells
            poss_x[poss_len] = x;
            poss_y[poss_len] = y;
            poss_len++;
        }
    }
  }

  // Random merge
  while (poss_len > 0) {
    int i = eadk_random() % poss_len;
    int x = poss_x[i];
    int y = poss_y[i];

    uint16_t a, b;
    get_neighbors(x, y, &a, &b);

    if (a != b && a != 0 && b != 0) {
      uint16_t smaller_id = (a < b) ? a : b;
      uint16_t larger_id = (a > b) ? a : b;

      replace_all(larger_id, smaller_id);
      map[x][y] = smaller_id;

      // Propagate the color of the smaller_id region to the current wall position
      // Find a cell belonging to the smaller_id region and get its color
      for (int cx = 0; cx < WIDTH; cx++) {
          for (int cy = 0; cy < HEIGHT; cy++) {
              if (map[cx][cy] == smaller_id && (cx % 2 == 1 && cy % 2 == 1)) {
                  colors[x][y] = colors[cx][cy];
                  goto found_color; // Break out of nested loops once color is found
              }
          }
      }
      found_color:; // Label for goto

      draw_maze();
      eadk_display_wait_for_vblank(); // Force display update
      eadk_timing_msleep(30);
    }

    // Remove the processed element
    poss_x[i] = poss_x[poss_len - 1];
    poss_y[i] = poss_y[poss_len - 1];
    poss_len--;
  }
}

int main(int argc, char * argv[]) {
  while (1) {
    fill_background(eadk_color_white);
    generate_maze();
    draw_maze();

    // Wait until the user presses OK or BACK
    while (1) {
      eadk_keyboard_state_t state = eadk_keyboard_scan();

      if (eadk_keyboard_key_down(state, eadk_key_ok)) {
        break; // Restart generation
      }
      if (eadk_keyboard_key_down(state, eadk_key_back)) {
        return 0; // Exit
      }
      eadk_timing_msleep(50);
    }
  }
}
