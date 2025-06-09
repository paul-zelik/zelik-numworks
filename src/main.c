#include <eadk.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

const char eadk_app_name[] __attribute__((section(".rodata.eadk_app_name"))) = "Markdown";
const uint32_t eadk_api_level __attribute__((section(".rodata.eadk_api_level"))) = 0;

#define MAX_LIGNES 100
#define LIGNES_VISIBLE 20

char *lignes[MAX_LIGNES];
size_t total_lignes = 0;


void parse_markdown(const char *data) {
  total_lignes = 0;
  const char *debut = data;

  while (*debut && total_lignes < MAX_LIGNES) {
    const char *fin = strchr(debut, '\n');
    if (!fin) fin = debut + strlen(debut);

    size_t taille = fin - debut;
    char *ligne_brute = (char *)malloc(taille + 1);
    if (!ligne_brute) break;

    strncpy(ligne_brute, debut, taille);
    ligne_brute[taille] = '\0';

    // Ligne transformée
    char *ligne_parsee = (char *)malloc(256);
    if (!ligne_parsee) {
      free(ligne_brute);
      break;
    }
    ligne_parsee[0] = '\0';

    // # Titre ou ## Sous-titre
    if (strncmp(ligne_brute, "# ", 2) == 0) {
      strcat(ligne_parsee, "## ");
      for (size_t i = 2; i < strlen(ligne_brute); i++)
        ligne_parsee[strlen(ligne_parsee)] = toupper(ligne_brute[i]);
      ligne_parsee[strlen(ligne_parsee)] = '\0';
    } else if (strncmp(ligne_brute, "## ", 3) == 0) {
      strcat(ligne_parsee, "# ");
      for (size_t i = 3; i < strlen(ligne_brute); i++)
        ligne_parsee[strlen(ligne_parsee)] = toupper(ligne_brute[i]);
      ligne_parsee[strlen(ligne_parsee)] = '\0';
    }

    // > Citation
    else if (strncmp(ligne_brute, "> ", 2) == 0) {
      snprintf(ligne_parsee, 255, "│ %s", ligne_brute + 2);
    }

    // - Liste
    else if (strncmp(ligne_brute, "- ", 2) == 0) {
      snprintf(ligne_parsee, 255, "➤ %s", ligne_brute + 2);
    }

    // Ligne classique + mise en forme dans le texte
    else {
      char *src = ligne_brute;
      while (*src && strlen(ligne_parsee) < 240) {
        // Gras : **texte**
        if (strncmp(src, "**", 2) == 0) {
          src += 2;
          while (*src && strncmp(src, "**", 2) != 0) {
            ligne_parsee[strlen(ligne_parsee)] = toupper(*src);
            src++;
          }
          src += 2;
        }

        // Italique : *texte*
        else if (*src == '*') {
          src++;
          strcat(ligne_parsee, "/");
          while (*src && *src != '*') {
            strncat(ligne_parsee, (char[]){*src, '\0'}, 1);
            src++;
          }
          strcat(ligne_parsee, "/");
          if (*src == '*') src++;
        }

        // Code inline : `texte`
        else if (*src == '`') {
          src++;
          strcat(ligne_parsee, "[");
          while (*src && *src != '`') {
            strncat(ligne_parsee, (char[]){*src, '\0'}, 1);
            src++;
          }
          strcat(ligne_parsee, "]");
          if (*src == '`') src++;
        }

        // Lien [texte](url)
        else if (*src == '[') {
          src++;
          char texte[64] = "", url[64] = "";
          size_t i = 0;
          while (*src && *src != ']' && i < 63) texte[i++] = *src++;
          texte[i] = '\0';
          if (*src == ']') src++;
          if (*src == '(') {
            src++;
            i = 0;
            while (*src && *src != ')' && i < 63) url[i++] = *src++;
            url[i] = '\0';
            if (*src == ')') src++;
          }
          snprintf(ligne_parsee + strlen(ligne_parsee), 255 - strlen(ligne_parsee), "%s (%s)", texte, url);
        }

        // Normal
        else {
          ligne_parsee[strlen(ligne_parsee)] = *src;
          src++;
        }
      }
      ligne_parsee[strlen(ligne_parsee)] = '\0';
    }

    lignes[total_lignes++] = ligne_parsee;
    free(ligne_brute);
    debut = (*fin) ? fin + 1 : fin;
  }
}

void liberer_lignes() {
  for (size_t i = 0; i < total_lignes; i++) {
    free(lignes[i]);
  }
  total_lignes = 0;
}

void afficher_lignes(size_t debut) {
  eadk_display_push_rect_uniform(eadk_screen_rect, eadk_color_white);

  for (size_t i = 0; i < LIGNES_VISIBLE; i++) {
    size_t ligne_index = debut + i;
    if (ligne_index >= total_lignes) break;

    eadk_display_draw_string(
      lignes[ligne_index],
      (eadk_point_t){2, (uint16_t)(i * 12)},
      false,
      eadk_color_black,
      eadk_color_white
    );
  }
}

void afficher_fichier() {
  parse_markdown(eadk_external_data);

  size_t offset = 0;

  while (1) {
    afficher_lignes(offset);
    eadk_timing_msleep(100);

    eadk_keyboard_state_t keys = eadk_keyboard_scan();

    if (eadk_keyboard_key_down(keys, eadk_key_down)) {
      if (offset + LIGNES_VISIBLE < total_lignes) offset++;
    }
    if (eadk_keyboard_key_down(keys, eadk_key_up)) {
      if (offset > 0) offset--;
    }
    if (eadk_keyboard_key_down(keys, eadk_key_back)) {
      break;
    }
  }

  liberer_lignes();
}




int main(int argc, char *argv[]) {
  afficher_fichier();
  return 0;
}
