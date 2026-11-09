#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <locale.h>
#include <string.h>
#include <getopt.h>
#include <stdint.h>
#include "hashtable.h"
#include "holdall.h"
#include "opt.h"

//--- DISPLAY ANSI  ------------------------------------------------------------

#define ANSI_BOLD   "\033[1m"                // Mettre le texte en gras.
#define ANSI_SG1    "\033[38;2;220;210;150m" // Couleur de texte (Jaune pâle).
#define ANSI_SG2    "\033[38;2;255;245;145m" // Couleur de texte (Jaune pastel).
#define ANSI_GD1    "\033[48;2;121;139;131m" // Couleur de fond (Gris olive).
#define ANSI_GD2    "\033[48;2;60;60;60m"    // Couleur de fond (Gris foncé).
#define ANSI_RESET  "\033[0m"                // Réinitialiser les styles.

//--- BITWISE  -----------------------------------------------------------------

#define INIT_BITS(p)     (*(p) = 0ULL) // Initialise tous les bits à zéro.
#define BIT_IS_SET(p, i) (((*(p)) >> (i)) & 1ULL) // Vérifie si le i-ème bit de
                                                  // p est à 1.
#define SET_BIT_1_(p, i) (*(p) |= (1ULL << (i))) // Met le i-ème bit de p à 1.

//--- DISTANCE DE JACCARD  -----------------------------------------------------

#define MAX_COMBINATIONS 2016  // Nombre total des Combinaisons, avec la formule
                               // C(64, 2) <==> 2 parmi 64.

//  struct context, context : type et nom de type regroupant les informations
//    nécessaires pour le traitement des fichiers ainsi que pour l'affichage
//    des résultats, incluant les distances de Jaccard entre les fichiers.
typedef struct context context;

struct context {
  size_t n_files;                // Nombre de fichiers traités.
  bool graph_flag;               // Indicateur pour l'affichage graphique.
  size_t intersection_array[MAX_COMBINATIONS]; // Tableau des intersections.
  size_t union_array[MAX_COMBINATIONS];        // Tableau des unions.
  double jaccard_dist[MAX_COMBINATIONS];       // Tableau des distances
};                                             // de Jaccard.

//  combinations_count : calcule le nombre total de combinaisons de 2 éléments
//    parmi n éléments.
static size_t combinations_count(size_t n);

//  combination_rank : calcule l'indice d'une combinaison (i, j) parmi toutes
//    les combinaisons possibles de taille 2 dans un ensemble de n éléments.
//    Les combinaisons sont générées dans un ordre croissant, et le rang
//    correspond à la position de la combinaison dans cet ordre.
static size_t combination_index(size_t i, size_t j, size_t n);

//  jaccard_dist : calcule la distance de Jaccard pour chaque paire d'ensembles.
//    La distance de Jaccard est définie comme : 1 - (|A ∩ B| / |A ∪ B|), où
//    |A ∩ B| est la taille de l'intersection des ensembles A et B, et
//    |A ∪ B| est la taille de leur union.
//    Les tailes des unions entre chaque paire d'ensembles sont pointées par
//    union_array, tandis que les tailles des intersections sont pointées par
//    inter_array. n représente le nombre total d'ensembles, utilisé pour
//    déterminer le nombre total de paires d'ensembles à traiter.
//    Modifie directement dist_array pour y stocker la distance de Jaccard pour
//    chaque paire d'ensembles.
static void jaccard_dist(size_t *inter_array, size_t *union_array, size_t n,
    double *dist_array);

//  scptr_display : Fonction d’AFFICHAGE pilotée par le drapeau graph_flag, qui
//    un champ de la structure pointée par ctx.
//    Si le drapeau est activé ==> MODE GRAPH, sinon MODE MÉTRIQUE.
//    MODE GRAPH    : affiche le mot pointé par s, et son appartenance aux
//                    fichiers selon le tableau des bits pointé par cptr séparés
//                    avec des tabulations.
//    MODE MÉTRIQUE : remplit uniquement les tableaux d’intersection/union de
//                    la structure pointée par ctx puis calcule la distance de
//                    Jaccard (aucune impression).
//    Renvoie toujours zéro, ce qui indique un déroulement normal sans erreur.
static int scptr_display(context *ctx, const char *s, const uint64_t *cptr);

//--- Text Processing  ---------------------------------------------------------

//  strtok_ : Extrait le mot suivant de la chaine de caractères pointée par *pp
//    en considérant les délimiteurs spécifiés dans la classe isspace() pour
//    les espaces et ispunct() pour la ponctuation selon le drapeau ponct.
//    Le mot formé sera pointé par *pstart.
//    Renvoie la longueur du mot extrait, sinon 0 si aucun mot n'a été formé.
static size_t strtok_(char **pp, char **pstart, bool ponct);

//  strcoll_compar : Wrapper autour de la fonction strcoll
//    (définie dans <string.h>) pour la rendre compatible avec des fonctions
//    comme qsort (définie dans <stdlib.h>) qui attendent une fonction de
//    comparaison prenant des pointeurs génériques (const void *).
static int strcoll_compar(const void *s1, const void *s2);

#define IS_STDIN(s)               (((s)[0] == '-' && (s)[1] == '\0'))

#define INITIAL_BUFFER_CAPACITY   4096
#define CAPACITY_MUL              2
#define MIN_FILES_REQUIRED        2
#define MAX_FILES_SUPPORTED       64
#define LOAD_FACTOR               0.75

//--- DIVERS  ------------------------------------------------------------------

//  Sauter au label spécifié si expr vaut vrai.
#define ON_ERROR_GOTO(expr, label) if (expr) { goto label; }

//  str_hashfun : l'une des fonctions de pré-hachage conseillées par Kernighan
//    et Pike pour les chaines de caractères.
static size_t str_hashfun(const char *s);

//  rfree : libère la zone mémoire occupée par le couple clé-valeur,
//    dont la clé est pointée par s et la valeur par cptr, dans la table de
//    hachage pointée par ht. On suppose que la ht, s et cptr ne valent pas
//    des pointeurs nulls. Renvoie zéro.
static int rfree(hashtable *ht, char *s, uint64_t *cptr);

//--- FONCTION PRINCIPALE  -----------------------------------------------------

int main(int argc, char *argv[]) {
  ON_ERROR_GOTO(setlocale(LC_COLLATE, "") == nullptr, error_locale);
  bool graph = 0;
  size_t initial_value = 0;
  bool ponct = 0;
  opt_processing(argc, argv, &graph, &ponct, &initial_value);
  size_t n = (size_t) (argc - optind);
  if (n < MIN_FILES_REQUIRED) {
    fprintf(stderr, "%s: Requiert %d fichiers au minimum !\n", argv[0] + 2,
        MIN_FILES_REQUIRED);
    exit(EXIT_FAILURE);
  }
  if (n > MAX_FILES_SUPPORTED) {
    fprintf(stderr, "***Erreur : The number of files exeeds 64.\n");
    exit(EXIT_FAILURE);
  }
  int r = EXIT_SUCCESS;
  hashtable *ht = hashtable_empty((int (*)(const void *, const void *)) strcmp,
        (size_t (*)(const void *)) str_hashfun, LOAD_FACTOR);
  holdall *has = holdall_empty();
  ON_ERROR_GOTO(ht == nullptr || has == nullptr, error_capacity);
  context ctx = { 0 };
  ctx.graph_flag = graph;
  ctx.n_files = n;
  size_t suivi = 0;
  FILE *f;
  char *w = nullptr;
  for (size_t i = (size_t) optind; i < (size_t) argc; i++) {
    suivi++;
    bool is_stdin = IS_STDIN(argv[i]);
    if (is_stdin) {
      f = stdin;
      printf("%s%s%s--- starts reading for #%zu FILE\n%s", ANSI_BOLD, ANSI_SG1,
          ANSI_GD1, suivi, ANSI_RESET);
    } else {
      f = fopen(argv[i], "rb");
      if (f == nullptr) {
        fprintf(stderr,
            "%s%s: reading error occurred with the file '%s'.\n", argv[0] + 2,
            ANSI_RESET, argv[i]);
        goto error;
      }
    }
    w = malloc(INITIAL_BUFFER_CAPACITY * sizeof *w);
    ON_ERROR_GOTO(w == nullptr, error_capacity);
    size_t buffer_capacity = INITIAL_BUFFER_CAPACITY;
    long int k = 0;
    size_t len = 0;
    while (fgets(w + len, (int) (buffer_capacity - len), f) != nullptr) {
      k++;
      size_t line_length = strlen(w);
      if (line_length > 0 && w[line_length - 1] != '\n') {
        len = line_length;
        ON_ERROR_GOTO(buffer_capacity > SIZE_MAX / CAPACITY_MUL / sizeof(char),
            error_capacity);
        char *w_ = realloc(w, buffer_capacity * CAPACITY_MUL * sizeof *w_);
        if (w_ == nullptr) {
          free(w);
          if (!is_stdin) {
            fclose(f);
          }
          goto error_capacity;
        }
        buffer_capacity *= CAPACITY_MUL;
        w = w_;
        continue;
      }
      char *p = w;
      char *t = nullptr;
      size_t n = 0;
      while ((n = strtok_(&p, &t, ponct)) > 0) {
        if (initial_value != 0 && n > initial_value) {
          fprintf(stderr, "%s: Word from %s at line %ld cut: '%.*s...'\n",
              argv[0] + 2, is_stdin ? "standard input" : argv[i], k,
              (int) initial_value, t);
          n = initial_value;
        }
        char saved = t[n];
        t[n] = '\0';
        uint64_t *cptr = hashtable_search(ht, t);
        t[n] = saved;
        if (cptr) {
          SET_BIT_1_(cptr, suivi - 1);
        } else {
          char *s = malloc(n + 1);
          ON_ERROR_GOTO(s == nullptr, error_capacity);
          memcpy(s, t, n);
          s[n] = '\0';
          cptr = malloc(sizeof *cptr);
          ON_ERROR_GOTO(cptr == nullptr, error_capacity);
          INIT_BITS(cptr);
          SET_BIT_1_(cptr, suivi - 1);
          ON_ERROR_GOTO(hashtable_add(ht, s, cptr) == nullptr, error_capacity);
          ON_ERROR_GOTO(holdall_put(has, s) != 0, error_capacity);
        }
      }
      len = 0;
    }
    free(w);
    ON_ERROR_GOTO((!feof(f) && ferror(f)) || (f != stdin && fclose(f) == EOF),
        error_read);
    if (f == stdin) {
      printf("%s%s%s--- ends reading for #%zu FILE\n%s", ANSI_BOLD, ANSI_GD2,
          ANSI_SG2, suivi, ANSI_RESET);
      rewind(f);
    }
  }
  if (graph == true) {
    holdall_sort(has, strcoll_compar);
    for (int i = optind; i < argc; i++) {
      printf("\t%s", IS_STDIN(argv[i]) ? "\"\"" : argv[i]);
    }
    printf("\n");
  }
  ON_ERROR_GOTO(
      holdall_apply_context2(has,
      ht,
      (void *(*)(void *, void *)) hashtable_search,
      &ctx,
      (int (*)(void *, void *, void *)) scptr_display)
      != 0, error_read);
  if (graph == false) {
    jaccard_dist(ctx.intersection_array, ctx.union_array, ctx.n_files,
        ctx.jaccard_dist);
    for (int i = optind; i < argc; i++) {
      for (int k = i + 1; k < argc; k++) {
        size_t index = combination_index((size_t) (i - optind),
              (size_t) (k - optind), ctx.n_files);
        printf("%.4f", ctx.jaccard_dist[index]);
        printf("\t%s\t%s\n", IS_STDIN(argv[i]) ? "\"\"" : argv[i],
            IS_STDIN(argv[k]) ? "\"\"" : argv[k]);
      }
    }
  }
  goto dispose;
error_capacity:
  fprintf(stderr, "*** Error: Not enough memory\n");
  goto error;
error_read:
  fprintf(stderr, "*** Error: A read error occurs \n");
  goto error;
error_locale:
  fprintf(stderr, "*** Error: Could not set the locale.\n");
  goto error;
error:
  r = EXIT_FAILURE;
  goto dispose;
dispose:
  if (has != nullptr) {
    holdall_apply_context2(has,
        ht,
        (void *(*)(void *, void *)) hashtable_search,
        ht,
        (int (*)(void *, void *, void *)) rfree);
  }
  holdall_dispose(&has);
  hashtable_dispose(&ht);
  return r;
}

size_t str_hashfun(const char *s) {
  size_t h = 0;
  for (const unsigned char *p = (const unsigned char *) s; *p != '\0'; ++p) {
    h = 31 * h + *p;
  }
  return h;
}

int rfree(hashtable *ht, char *s, uint64_t *cptr) {
  free(cptr);
  hashtable_remove(ht, s);
  free(s);
  return 0;
}

size_t strtok_(char **pp, char **pstart, bool ponct) {
  char *p = *pp;
  while (*p != '\0'
      && ((ponct && (ispunct(*p) || isspace(*p)))
      || (!ponct && isspace(*p)))) {
    p++;
  }
  if (*p == '\0') {
    *pp = p;
    return 0;
  }
  *pstart = p;
  while (*p != '\0'
      && !((ponct && (ispunct(*p) || isspace(*p)))
      || (!ponct && isspace(*p)))) {
    p++;
  }
  size_t len = (size_t) (p - *pstart);
  *pp = p;
  return len;
}

int scptr_display(context *ctx, const char *s, const uint64_t *cptr) {
  if (ctx->graph_flag) {
    printf("%s\t", s);
    for (size_t k = 0; k < ctx->n_files; k++) {
      if (BIT_IS_SET(cptr, k)) {
        printf("x");
      } else {
        printf("-");
      }
      if (k < ctx->n_files - 1) {
        printf("\t");
      }
    }
    printf("\n");
  } else {
    for (size_t i = 0; i < ctx->n_files - 1; i++) {
      size_t bit_i = BIT_IS_SET(cptr, i);
      for (size_t k = i + 1; k < ctx->n_files; k++) {
        size_t bit_k = BIT_IS_SET(cptr, k);
        size_t index = combination_index(i, k, ctx->n_files);
        ctx->intersection_array[index] += (bit_i & bit_k);
        ctx->union_array[index] += (bit_i | bit_k);
      }
    }
  }
  return 0;
}

size_t combination_index(size_t i, size_t j, size_t n) {
  return i * (2 * n - i - 1) / 2 + (j - i - 1);
}

size_t combinations_count(size_t n) {
  return (n < 2) ? 0 : (n * (n - 1) / 2);
}

void jaccard_dist(size_t *inter_array, size_t *union_array, size_t n,
    double *dist_array) {
  size_t nb_combs = combinations_count(n);
  for (size_t i = 0; i < nb_combs; i++) {
    size_t u = union_array[i];
    size_t inter = inter_array[i];
    dist_array[i] = (u == 0) ? 0.0 : 1.0 - ((double) inter / (double) u);
  }
}

int strcoll_compar(const void *s1, const void *s2) {
  return strcoll(*(const char **) s1, *(const char **) s2);
}
