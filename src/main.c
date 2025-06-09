#include <eadk.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

const char eadk_app_name[] __attribute__((section(".rodata.eadk_app_name"))) = "MD Viewer";
const uint32_t eadk_api_level __attribute__((section(".rodata.eadk_api_level"))) = 0;

// === Données Markdown ===
// (Remplace ici par ton propre contenu Markdown)

const char *fichiers[] = {
  "Fichier1.md", "Guide.md", "Notes.md", "Intro.md", "Cours1.md", "Projet.md"
};

const char *contenus[] = {
  "## Fichier 1\nCeci est le contenu du fichier 1.\nAvec plusieurs lignes.\n...\n",
  "# Guide\nVoici un guide d’utilisation.\nLigne 2.\nLigne 3.\n",
  "# Notes diverses\n- Point 1\n- Point 2\n",
  "# Intro\nBienvenue dans ce Markdown.",
  "# Cours 1\nChapitre 1...\nChapitre 2...",
  "# Projet\nInstructions...\nÉtapes...\nFin."
};

#define NB_FICHIERS (sizeof(fichiers)/sizeof(fichiers[0]))
#define NB_PAR_LIGNE 3
#define LIGNES_MAX 100
#define LIGNES_VISIBLE 20

char *lignes[LIGNES_MAX];
size_t total_lignes = 0;

void parse_markdown(const char *data) {
  total_lignes = 0;
  const char *debut = data;

  while (*debut && total_lignes < LIGNES_MAX) {
    const char *fin = strchr(debut, '\n');
    if (!fin) fin = debut + strlen(debut);

    size_t taille = fin - debut;
    lignes[total_lignes] = malloc(taille + 1);
    if (lignes[total_lignes]) {
      strncpy(lignes[total_lignes], debut, taille);
      lignes[total_lignes][taille] = '\0';
      total_lignes++;
    }

    debut = (*fin) ? fin + 1 : fin;
  }
}

void liberer_lignes() {
  for (size_t i = 0; i < total_lignes; i++) {
    free(lignes[i]);
  }
  total_lignes = 0;
}

void afficher_lignes(size_t offset) {
  eadk_display_push_rect_uniform(eadk_screen_rect, eadk_color_white);

  for (size_t i = 0; i < LIGNES_VISIBLE; i++) {
    size_t idx = offset + i;
    if (idx >= total_lignes) break;

    eadk_display_draw_string(
      lignes[idx],
      (eadk_point_t){2, (uint16_t)(i * 12)},
      false,
      eadk_color_black,
      eadk_color_white
    );
  }
}

// === Affichage du contenu d’un fichier ===
void afficher_fichier(size_t index) {
  parse_markdown(contenus[index]);

  size_t offset = 0;
  while (1) {
    afficher_lignes(offset);
    eadk_keyboard_state_t keys = eadk_keyboard_scan();
    eadk_timing_msleep(100);

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

// === Affichage du menu wrap (gauche à droite) ===
void afficher_menu(size_t selection) {
  eadk_display_push_rect_uniform(eadk_screen_rect, eadk_color_white);

  for (size_t i = 0; i < NB_FICHIERS; i++) {
    int ligne = i / NB_PAR_LIGNE;
    int col = i % NB_PAR_LIGNE;

    eadk_point_t pos = {10 + col * 100, (uint16_t)(10 + ligne * 20)};
    eadk_color_t texte = (i == selection) ? eadk_color_white : eadk_color_black;
    eadk_color_t fond = (i == selection) ? eadk_color_blue : eadk_color_white;

    eadk_display_draw_string(fichiers[i], pos, false, texte, fond);
  }
}

// === Main principal ===
int main(int argc, char *argv[]) {
  size_t selection = 0;

  while (1) {
    afficher_menu(selection);
    eadk_keyboard_state_t keys = eadk_keyboard_scan();
    eadk_timing_msleep(100);

    if (eadk_keyboard_key_down(keys, eadk_key_left)) {
      if (selection > 0) selection--;
    }
    if (eadk_keyboard_key_down(keys, eadk_key_right)) {
      if (selection < NB_FICHIERS - 1) selection++;
    }
    if (eadk_keyboard_key_down(keys, eadk_key_down)) {
      if (selection + NB_PAR_LIGNE < NB_FICHIERS) selection += NB_PAR_LIGNE;
    }
    if (eadk_keyboard_key_down(keys, eadk_key_up)) {
      if (selection >= NB_PAR_LIGNE) selection -= NB_PAR_LIGNE;
    }

    if (eadk_keyboard_key_down(keys, eadk_key_ok)) {
      afficher_fichier(selection);
    }
  }

  return 0;
}
