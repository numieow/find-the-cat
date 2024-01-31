#ifndef __FTC__
#define __FTC__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <regex.h>
#include "MegaMimes.h"

#define FLAGS_WITH_PARAM "-name", "-size", "-date", "-mime", "-ctc", "-dir", "-threads", "-perm"
#define FLAGS_WITHOUT_PARAM "-color", "-link", "-ou", "-test", "-dir"
#define nb_flags_with_param 8
#define nb_flags_without_param 5

//Utilisé pour contenir tous les flags de la ligne de commande.
//Les flags sans valeur sont sauvegardés avec comme valeur le nom du flag.
typedef struct flag_t {
    char* flag;
    char* valeur;
    int flag_type;
} flag;

typedef struct nb_Flags {
    int nb_name ;
    int nb_dir_param ;
    int nb_dir_no_param ;
    int nb_ou ;
    int nb_color;
} nb_Flags;

int is_flag(char* str) ;

int is_size_param(char* str) ;

int is_date_param(char* param) ;

int is_mime_param(char* param) ;

int is_perm_param(char* param) ;

void test(char* argv[]) ;

flag* parse(int argc, char* argv[]);

void print_paths(char* path, flag* flags, nb_Flags* nb_flags);

void print_paths_none(char* path);

void test_flags(char* chemin, flag* flags, nb_Flags* nb_flags);

int check_name(char* chemin, char* nom_a_tester);

int is_regex(char* exp);

int check_size(char* chemin, char* size_a_tester);

char* read_file(char* path);

int check_ctc(char* chemin, char* mot_a_tester);

unsigned long param_to_secondes(char* param) ;

int check_date(char* chemin, char* param) ;

int check_mime(char* chemin, char* mime);

int check_perm(char* chemin, char* param) ;



//Définition des fonctions de gestion de la structure flag

int init_flag(flag* flag, char* flag_name, char* value, int type);

void print_flag(flag* flag);

void free_flag(flag* flags);


//Suite des opérations : 
/*
On crée une structure qui contient le nombre de fois que chaque flag est contenu (ici, on prend -name, -dir, -ou, -color) DONE
On crée le structure pour ranger les chemins qu'on parcourt. La structure contient (au moins) le chemin et le nombre de tests passés (initialisés à 0) DONE
Pour gérer et/ou : si on a les 2 flags, erreur
On check tous les chemins et on leur fait passer tous les tests (avec une liste qui contient l'information sur la présence de flag)
Si le flag présent est et, on regarde si le nombre de tests passés est égal au nombre de flag avec argument qu'on a en ligne de commande pour savoir si on print
Si le flag présent est ou, on regarde seulement si le nombre est supérieur strict à 0
*/

#endif
