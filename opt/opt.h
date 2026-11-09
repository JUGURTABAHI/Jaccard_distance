//  opt.h : Interface du module qui gère les options passées via la ligne
//    de commande.

#ifndef OPT_H
#define OPT_H

#include <stdlib.h>

// opt_processing : Traite les options de la ligne de commande fournies via argc
//    et argv. Gère diverses options comme le mode graphique, la gestion de la
//    ponctuation, la longueur maximale des mots, respectivement via les
//    pointeurs graph, ponct et initial_value. Gère ainsi les options
//    d'affichage d'informations sur l'aide, l'usage, l'auteur, et les crédits,
//    en quittant le programme avec EXIT_FAILURE en cas d'erreur.
extern void opt_processing(int argc, char *argv[], bool *graph, bool *ponct,
    size_t *initial_value);

//  print_help : affiche l'aide du programme.
extern void opt_print_help(const char *s);

//  print_usage : affiche l'usage du programme.
extern void opt_print_usage(const char *s);

#endif
