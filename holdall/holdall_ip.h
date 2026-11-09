//  holdall_ip.h : précisions sur l'implantation du module holdall.

//  L'implantation recoure à un tableau dynamique des références.

//  Lorsqu'ils ne sont pas constants, les couts sont exprimés en fonction du
//    nombre (n) d'insertions effectuées avec succès dans le fourretout
//    depuis sa création.

//  holdall_empty : temps constant ; espace constant.
//  holdall_dispose : temps constant ; espace constant.
//  holdall_put : temps constant ; espace constant.
//  holdall_count : temps constant ; espace constant.
//  holdall_apply, holdall_apply_context, holdall_apply_context2, sans compter
//    ni le temps ni l'espace nécessaire à l'exécution des fonctions fun, fun1
//    ou fun2 passées en paramètre : temps au plus linéaire ; espace constant.

#define HOLDALL_EXT

//  Dans la fonction holdall_sort, la fonction qsort() est utilisée. Selon
//   l'implémentation de la glibc, qsort() utilise généralement l'algorithme
//   quicksort, mais peut basculer vers heapsort si nécessaire, par exemple en
//   cas de problème de mémoire ou si la pile locale, utilisée pour la
//   récursivité, est pleine.

//  Cas quicksort :
//  holdall_sort : temps en moyenne (nlog(n)) ; espace logarithmique.

//  Cas heapsort :
//  holdall_sort : temps au plus (nlog(n)) ; espace constant.
