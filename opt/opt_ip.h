//  opt_ip.h : Précisions sur l'implantation du module opt.

//  Ce module repose principalement sur la fonction getopt_long() fournie par
//    la bibliothèque <getopt.h> pour le traitement des options de la ligne de
//    commande.

//  opt_processing : temps quasi-linéaire en (argc × m), où argc est le
//    nombre total d'arguments passés en ligne de commande et m le nombre
//    d'options définies dans le tableau getopt_long. espace constant.
//  print_help : temps constant, espace constant.
//  print_usage : temps constant, espace constant.
