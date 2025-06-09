#include <eadk.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#define MAX_LINE_LENGTH 64
#define MAX_FILE_SIZE 2048
#define MAX_FILES 32
#define COLOR_LIGHT_GRAY ((eadk_color_t)0xC618) // RGB565 pour gris clair

// Dépendances externes Omega/Upsilon
extern int extapp_fileList(const char ** filenames, int maxRecords, const char * extension);
extern int extapp_fileRead(const char * filename, void * buffer, int maxLength);

// Buffer de lecture de fichier
char file_buffer[MAX_FILE_SIZE];

// Liste les fichiers Markdown
int list_markdown_files(const char ** filenames) {
  return extapp_fileList(filenames, MAX_FILES, "md");
}

// Charge un fichier texte
bool load_file(const char * filename) {
  int bytesRead = extapp_fileRead(filename, file_buffer, MAX_FILE_SIZE - 1);
  if (bytesRead <= 0) return false;
  file_buffer[bytesRead] = '\0'; // Null-terminate
  return true;
}

// Affiche le Markdown
void render_markdown(const char * text) {
  eadk_display_push_rect_uniform(eadk_screen_rect, eadk_color_white);
  uint16_t y = 0;
  char buffer[MAX_LINE_LENGTH + 1];

  const char *p = text;
  while (*p && y < EADK_SCREEN_HEIGHT - 14) {
    int i = 0;
    while (*p != '\n' && *p && i < MAX_LINE_LENGTH) {
      buffer[i++] = *p++;
    }
    buffer[i] = '\0';
    if (*p == '\n') p++;

    eadk_color_t fg = eadk_color_black;
    eadk_color_t bg = eadk_color_white;
    int x = 0;

    if (buffer[0] == '#') {
      fg = eadk_color_blue;
      x = 10;
      memmove(buffer, buffer + 1, strlen(buffer));
    } else if (buffer[0] == '-') {
      fg = COLOR_LIGHT_GRAY;

      buffer[0] = '*';            // caractère ASCII standard
    }

    char *boldStart = strstr(buffer, "**");
    if (boldStart) {
      *boldStart = '\0';
      char *boldEnd = strstr(boldStart + 2, "**");
      if (boldEnd) {
        *boldEnd = '\0';
        eadk_display_draw_string(buffer, (eadk_point_t){x, y}, true, fg, bg);
        x += 6 * strlen(buffer);
        eadk_display_draw_string(boldStart + 2, (eadk_point_t){x, y}, true, eadk_color_red, bg);
        x += 6 * strlen(boldStart + 2);
        eadk_display_draw_string(boldEnd + 2, (eadk_point_t){x, y}, true, fg, bg);
      } else {
        eadk_display_draw_string(buffer, (eadk_point_t){x, y}, true, fg, bg);
      }
    } else {
      eadk_display_draw_string(buffer, (eadk_point_t){x, y}, true, fg, bg);
    }

    y += 14;
  }
}

// Menu de sélection de fichier
int select_markdown_file(const char ** filenames, int count) {
  int selected = 0;
  while (1) {
    eadk_display_push_rect_uniform(eadk_screen_rect, eadk_color_white);

    for (int i = 0; i < count; i++) {
      eadk_color_t fg = (i == selected) ? eadk_color_white : eadk_color_black;
      eadk_color_t bg = (i == selected) ? eadk_color_blue : eadk_color_white;
      eadk_display_draw_string(filenames[i], (eadk_point_t){10, 10 + 14 * i}, true, fg, bg);
    }

    eadk_keyboard_state_t keyboard = eadk_keyboard_scan();
    if (eadk_keyboard_key_down(keyboard, eadk_key_down) && selected < count - 1) {
      selected++;
    } else if (eadk_keyboard_key_down(keyboard, eadk_key_up) && selected > 0) {
      selected--;
    } else if (eadk_keyboard_key_down(keyboard, eadk_key_ok)) {
      return selected;
    } else if (eadk_keyboard_key_down(keyboard, eadk_key_back)) {
      return -1;
    }

    eadk_timing_msleep(100);
  }
}

int main(int argc, char * argv[]) {
  const char * filenames[MAX_FILES];
  int fileCount = list_markdown_files(filenames);

  if (fileCount <= 0) {
    eadk_display_draw_string("Aucun fichier .md", (eadk_point_t){10, 10}, true, eadk_color_red, eadk_color_white);
    eadk_timing_msleep(3000);
    return 1;
  }

  int selected = select_markdown_file(filenames, fileCount);
  if (selected < 0) return 0;

  if (load_file(filenames[selected])) {
    render_markdown(file_buffer);
  } else {
    eadk_display_draw_string("Erreur de lecture", (eadk_point_t){10, 10}, true, eadk_color_red, eadk_color_white);
    eadk_timing_msleep(3000);
    return 1;
  }

  // Attente touche retour
  while (1) {
    eadk_keyboard_state_t keyboard = eadk_keyboard_scan();
    if (eadk_keyboard_key_down(keyboard, eadk_key_back)) break;
    eadk_timing_msleep(100);
  }

  return 0;
}
