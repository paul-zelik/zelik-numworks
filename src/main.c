#include <eadk.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

const char eadk_app_name[] __attribute__((section(".rodata.eadk_app_name"))) = "Markdown Viewer";
const uint32_t eadk_api_level  __attribute__((section(".rodata.eadk_api_level"))) = 0;

#define MAX_LINE_LENGTH 80

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
      // Titre : en bleu foncé
      fg = eadk_color_blue;
      x = 10;
      memmove(buffer, buffer + 1, strlen(buffer));
    } else if (buffer[0] == '-') {
      // Liste : préfixe avec "•"
      fg = eadk_color_darkGray;
      buffer[0] = '•';
    }

    // Gestion très simple du gras (**texte**) => on met en rouge
    char *boldStart = strstr(buffer, "**");
    if (boldStart) {
      *boldStart = '\0';
      char *boldEnd = strstr(boldStart + 2, "**");
      if (boldEnd) {
        *boldEnd = '\0';
        // Affiche avant le texte gras
        eadk_display_draw_string(buffer, (eadk_point_t){x, y}, true, fg, bg);
        x += 6 * strlen(buffer);
        // Affiche le texte en gras (rouge)
        eadk_display_draw_string(boldStart + 2, (eadk_point_t){x, y}, true, eadk_color_red, bg);
        x += 6 * strlen(boldStart + 2);
        // Affiche après le gras
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

// Extrait simulé de markdown
const char *markdown_text =
  "# Titre Principal\n"
  "- Élément 1\n"
  "- Élément 2\n"
  "Texte normal\n"
  "**Texte important** dans une phrase.\n"
  "# Deuxième Titre\n"
  "Encore du contenu\n";

int main(int argc, char * argv[]) {
  render_markdown(markdown_text);

  // Boucle jusqu’à appui sur "Back"
  while (true) {
    eadk_keyboard_state_t keyboard = eadk_keyboard_scan();
    if (eadk_keyboard_key_down(keyboard, eadk_key_back)) {
      break;
    }
    eadk_timing_msleep(100);
  }

  return 0;
}
