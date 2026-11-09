#include <stdio.h>
#include <getopt.h>
#include <limits.h>
#include "opt.h"

//-- MACROS  -------------------------------------------------------------------

//  HANDLE_ERROR : Affiche un message d'erreur pour le programme progname
//    concernant l'option optname non reconnue ou invalide, avec le message msg,
//    puis termine le programme avec EXIT_FAILURE.
#define HANDLE_ERROR(progname, msg, optname)                                   \
  fprintf(stderr, "%s: %s '%s'.\n", (progname), (msg), (optname));             \
  fprintf(stderr, "Try '%s --help' for more information.\n", (progname));      \
  exit(EXIT_FAILURE)

//  BAD_OPTION : Renvoie le nom de l'option mal formée, en vérifiant
//    si l'option est une option longue ou une option courte.
#define BAD_OPTION(argv, argc, optind)                                         \
  ((argv)[(optind) - 1][0] == '-' && (argv)[(optind) - 1][1] == '-')           \
  ? (argv)[(optind) - 1]                                                       \
  : (((optind) < (argc) && (argv)[(optind)][0] == '-' &&                       \
  (argv)[(optind)][1] != '-')                                                  \
  ? (argv)[(optind)]                                                           \
  : (argv)[(optind) - 1])

//------------------------------------------------------------------------------

//  AUTHOR_NAME, AUTHOR_EMAIL : Des informations sur le nom et l'adresse
//    électronique de l'auteur.
#define AUTHOR_NAME  "BAHI Jugurta"
#define AUTHOR_EMAIL "jugurta.bahi@univ-rouen.fr"

//  print_author : Affiche les informations de l'auteur du programme.
static void print_author(void);

//  print_credits : Affiche les informations de crédits du projet.
//    Affiche le cadre du projet ainsi que les noms et titres des instructeurs
//    responsables.
static void print_credits(void);

void opt_processing(int argc, char *argv[], bool *graph, bool *ponct,
    size_t *initial_value) {
  opterr = 0;
  int opt;
  //  struct option, long_options : type et nom de type pour définir et gérer
  //    les options longues et courtes passées via la ligne de commande.
  //   avec :
  //    "--help"    ou  "-h" :  Affiche l'aide du programme.
  //    "--graph"   ou  "-g" :  Active le mode graphique.
  //    "--initial" ou  "-i" :  Définit la longueur maximale des mots.
  ///   "--punctuation-like-space" ou "-p" : Supprime les caractères de
  //                                         ponctuation du traitement.
  //    "--author"  ou  "-a" :  Affiche l'auteur.
  //    "--credits" ou  "-c" :  Afficher les credits.
  static struct option long_options[] = {
    { "help", no_argument, nullptr, 'h' },
    { "usage", no_argument, nullptr, 'u' },
    { "graph", no_argument, nullptr, 'g' },
    { "initial", required_argument, nullptr, 'i' },
    { "punctuation-like-space", no_argument, nullptr, 'p' },
    { "author", no_argument, nullptr, 'a' },
    { "credits", no_argument, nullptr, 'c' },
    { 0, 0, 0, 0 }
  };
  while ((opt = getopt_long(argc, argv, ":guhi:pac", long_options,
        nullptr)) != -1) {
    switch (opt) {
      case 'g':
        {
          *graph = 1;
          break;
        }
      case 'i':
        {
          char *endptr;
          long int val = strtol(optarg, &endptr, 10);
          if (*endptr != '\0' || val < 0 || val > LONG_MAX) {
            HANDLE_ERROR(argv[0] + 2, "Invalid argument for option", "-i");
          }
          *initial_value = (size_t) val;
          break;
        }
      case 'p':
        {
          *ponct = 1;
          break;
        }
      case 'h':
        {
          opt_print_help(argv[0]);
          exit(EXIT_SUCCESS);
        }
      case 'u':
        {
          opt_print_usage(argv[0]);
          exit(EXIT_SUCCESS);
        }
      case 'a':
        {
          print_author();
          exit(EXIT_SUCCESS);
        }
      case 'c':
        {
          print_credits();
          exit(EXIT_SUCCESS);
        }
      case ':':
        {
          const char *badopt = BAD_OPTION(argv, argc, optind);
          HANDLE_ERROR(argv[0] + 2, "Missing argument for option", badopt);
          break;
        }
      case '?':
        {
          const char *badopt = BAD_OPTION(argv, argc, optind);
          HANDLE_ERROR(argv[0] + 2, "Unrecognized option", badopt);
        }
      default:
        break;
    }
  }
}

void opt_print_usage(const char *s) {
  printf("Usage: %s [OPTION]... FILE1 FILE2 [FILE]...\n", s);
}

void opt_print_help(const char *s) {
  opt_print_usage(s);
  printf("\n");
  printf(
      "Jaccard distance. Computes the dissimilarity between two texts\n"
      "based on the words they contain.\n\n");
  printf(
      "The Jaccard index is calculated as: |A ∩ B| ÷ |A ∪ B|,\n"
      "where A and B are sets of words from each file.\n\n"
      "The Jaccard distance is then: 1 - Jaccard index.\n\n");
  printf(
      "A word is defined as the longest sequence of characters \n"
      "that do not belong to the white-space characters set.\n\n");
  printf("Results are displayed in columns on the standard output. \n"
      "Columns are separated by the tab character. Lines are terminated \n"
      "by the end-of-line character.\n\n");
  printf("For each pair of files, the Jaccard distance is displayed in the\n"
      "first column, followed by pair of the file names in the second and\n"
      "third columns. Each pair of file names is listed once without\n"
      "repetition.\n\n");
  printf("The Jaccard distance results are rounded to 4 decimal places.\n\n");
  printf("Read the standard input when a FILE is specified as \"-\". \n"
      "In such case, \"\" is displayed in the column associated with \n"
      "the FILE on the header line.\n\n");
  printf("Mandatory arguments to long options are mandatory for short \n"
      "options too.\n\n");
  printf("White-space and punctuation characters conform to the standard.\n\n"
      "At most 64 FILEs are supported.\n\n");
  printf("Program Information \n");
  printf("\t-h, --help\n");
  printf("\t\tPrint this help message and exit.\n");
  printf("\t-u, --usage\n");
  printf("\t\tPrint a short usage message and exit.\n");
  printf("\t-a, --author\n");
  printf("\t\tPrint the author's information and exit.\n");
  printf("\t-c, --credits\n");
  printf("\t\tPrint instructor information and exit.\n");
  printf("\n");
  printf("Input Control \n");
  printf("\t-i, --initial=VALUE \n");
  printf("\t\tSet the maximal number of significant\n"
      "\t\tinitial letters of words to VALUE. 0 means \n"
      "\t\twithout limitation. Default is 0.\n");
  printf("\t-p, --punctuation-like-space \n");
  printf("\t\tMake the punctuation characters play the same\n"
      "\t\trole as white-space characters in the meaning\n"
      "\t\tof words.\n");
  printf("\n");
  printf("Output Control \n");
  printf("\t-g, --graph \n");
  printf("\t\tDisplay results in graphical form. \n"
      "\t\tThe corresponding file will be marked with 'x';\n"
      "\t\totherwise, it will be marked with '-'.\n");
  printf(
      "\t\tThe results are sorted lexicographically based on\n"
      "\t\tthe current locale settings\n"
      "\t\t(typically determined by LC_COLLATE or LC_ALL),\n"
      "\t\twhich ensures correct order for language-specific\n"
      "\t\tsorting rules.\n");
}

void print_author(void) {
  printf("Author : %s\n", AUTHOR_NAME);
  printf("Email  : %s\n", AUTHOR_EMAIL);
}

void print_credits(void) {
  printf("Ce projet a été réalisé dans le cadre du cours d'Algorithmique 3.\n");
  printf("Sous la direction de :\n");
  printf("\t - M. HANCART Christophe : Professeur des universités.\n");
  printf("\t - M. GROULT Richard     : Maître de conférences,\n"
      "\t\t\t\t   encadrant des TP.\n");
}
