#include "ftc.h"

int main(int argc, char *argv[]) {
    if(argc <= 1) {                                                         //Gestion d'erreur : pas assez d'arguments
        perror("Not enough arguments\n");
        return EXIT_FAILURE;
    } else if(argc >= 3) {                                                  //Si on a au moins ./ftc [chemin] [flag]
        if(strcmp(argv[2], "-test") == 0) {                                 //Si le premier flag est -test, on ne fait que test
            if(argc == 3) {
                perror("Aucun flag à tester");
            } else {
                test(argv + 3);
            }
            
        } else {
            flag* flags = parse(argc, argv);                                //On parse la ligne de commande
            nb_Flags* nb_flags = calloc(1, sizeof(nb_Flags)) ;              //On initialise la structure qui contient le nb d'occ de certains flags
            if(flags[0].flag_type == 0) {
                return EXIT_FAILURE;
            }
            int i = 0;                                                      //On compte le nombre de flags de certains types dans while
            while((flags[i].flag_type > -1) && (flags[i].flag != NULL)) {
                if (strcmp(flags[i].flag, "-name") == 0) {
                    nb_flags->nb_name++ ;
                }
                if (strcmp(flags[i].flag, "-dir") == 0) {
                    if (flags[i].flag_type == 1) {
                        nb_flags->nb_dir_param++ ;
                    }
                    else if (flags[i].flag_type == 2) {
                        nb_flags->nb_dir_no_param++ ;
                    }
                }
                if (strcmp(flags[i].flag, "-ou") == 0) {
                    nb_flags->nb_ou++ ;
                }
                if (strcmp(flags[i].flag, "-color") == 0) {
                    nb_flags->nb_color++ ;
                }
                i++;
            }
            //printf("-dir %d\n", nb_flags->nb_dir_param + nb_flags->nb_dir_no_param);
            //printf("On a un flag dir et pas d'ou %d\n", (nb_flags->nb_dir_no_param || nb_flags->nb_dir_param) && !nb_flags->nb_ou);
            i = 0;
            //Si on a un flag dir, on rend les flags -name inopérants
            if((nb_flags->nb_dir_no_param || nb_flags->nb_dir_param) && !nb_flags->nb_ou) {
                while((flags[i].flag_type > -1) && (flags[i].flag != NULL)) {
                    if((strcmp(flags[i].flag, "-name") == 0) || (strcmp(flags[i].flag, "-size") == 0) || (strcmp(flags[i].flag, "-date") == 0) || (strcmp(flags[i].flag, "-mime") == 0) || (strcmp(flags[i].flag, "-perm") == 0)) {
                        flags[i].flag_type = 0;
                        //printf("On rend inopérant le flag %s ; %d\n", flags[i].flag, flags[i].flag_type);
                    }
                    i++;
                }
            }
            //On parcourt tous les chemins et on fait les tests : 
            if(!(flags[0].flag_type == -2)) {
                test_flags(argv[1], flags, nb_flags);
                print_paths(argv[1], flags, nb_flags);
            }

            free(nb_flags) ;
            free_flag(flags);
            free(flags);
            return EXIT_SUCCESS;
        }
    }
    else {
        printf("%s\n", argv[1]);
        print_paths_none(argv[1]);
    }
    return EXIT_SUCCESS;
}
/**
 * @brief Fonction qui permet de tester si les flags sont corrects
 * 
 * @param str le paramètre à tester
 * @return 0 si le test est négatif, 1 si str est un flag avec paramètre, 2 si str est un flag sans paramètre
 */
int is_flag(char* str) {
    char* FLAGS_PARA[] = {FLAGS_WITH_PARAM} ;
    char* FLAGS_NOPARA[] = {FLAGS_WITHOUT_PARAM} ;
    for (int i = 0 ; i < nb_flags_with_param ; i++) {
        if (strcmp(str, FLAGS_PARA[i]) == 0) {
            return 1 ;
        }
    }
    for (int i = 0 ; i < nb_flags_without_param ; i++) {
        if (strcmp(str, FLAGS_NOPARA[i]) == 0) {
            return 2 ;
        }
    }
    return 0 ;
}

/**
 * @brief Fonction qui permet de savoir si le paramètre d'un flag -size est de la forme correcte. Une forme correcte implique de soit commencer par - ou + et un entier, soit commencer par un nombre entier. Le caractère final doit être un c, k, M ou G.
 * 
 * @param str le paramètre à tester
 * @return 0 si le test est négatif, 1 si le test est positif
 */
int is_size_param(char* str) {
    if ((str[0] == '-') || (str[0] == '+') || ((str[0] >= '1') && (str[0] <= '9'))) {
        if((str[0] == '-') || (str[0] == '+')) {
            str ++;                                                         //On avance d'un caractère si le premier est un + ou un -
        }
        if (str[0] == '0') {
            return 0 ;
        }
        while ((str[0] != '\0') && (str[0] >= '0') && (str[0] <= '9')) {
            str++ ;
        }
        if ((strlen(str) == 0) || (strlen(str) > 1)) {
            return 0 ;
        }
        else {
            if ((str[0] == 'c') || (str[0] == 'k') || (str[0] == 'M') || (str[0] == 'G')) {
                return 1 ;
            } else {
                return 0 ;
            }
        }
    } else {
        return 0 ;
    }
}

/**
 * @brief Fonction qui permet de tester si param est un format acceptable par le flag -date (les seules lettres acceptées sont m, h et j, avec une occurrence maximum par lettre, et elles doivent chacune être séparées par au moins un chiffre ; param peut commencer par un + comme un chiffre, doit terminer par une lettre)
 * 
 * @param param L'argument de l'option -date
 * @return 1 si param a le bon format, 0 sinon
 */
int is_date_param(char* param) {
    /*
    int n = strlen(param) ;
    int index = 0 ;
    int m_checked = 0 ;
    int h_checked = 0 ;
    int j_checked = 0 ;
    if (n <= 1) {
        return 0 ;
    }
    else {
        if ((param[0] == '+') || ((param[0] >= '0') && (param[0] <= '9'))) {
            if (param[0] == '+') {
                index++ ;
            }
            int shift ;
            while (index < n) {
                shift = 0 ;
                while ((index < n) && (param[index] >= '0') && (param[index] <= '9')) {
                    index++ ;
                    shift++;
                }
                if ((param[index] == 'm') || (param[index] == 'h') || (param[index] == 'j')) {
                    if (param[index] == 'm') {
                        if (m_checked == 0) {
                            m_checked = 1 ;
                        }
                        else {
                            return 0 ;
                        }
                    }
                    if (param[index] == 'h') {
                        if (h_checked == 0) {
                            h_checked = 1 ;
                        }
                        else {
                            return 0 ;
                        }
                    }
                    if (param[index] == 'j') {
                        if (j_checked == 0) {
                            j_checked = 1 ;
                        }
                        else {
                            return 0 ;
                        }
                    }
                    if (shift == 0) {
                        return 0 ;
                    }
                    index++ ;
                }
                else {
                    return 0 ;
                }
            }
            return 1 ;
        }
        else {
            return 0 ;
        }
    }
    */
    if ((param[0] == '+') || ((param[0] >= '0') && (param[0] <= '9'))) {
        if((param[0] == '+')) {
            param++;                                                         //On avance d'un caractère si le premier est un + ou un -
        }
        int i = 0 ;
        while ((param[0] != '\0') && (param[0] >= '0') && (param[0] <= '9')) {
            param++ ;
            i++ ;
        }
        if ((i > 0) && (strlen(param) == 1) && ((param[0] == 'm') || (param[0] == 'h') || (param[0] == 'j'))) {
            return 1 ;
        }
    }
    return 0 ;
}

/**
 * @brief Fonction qu permet de tester si param a le format adéquat pour le flag -mime, à savoir : type ou type/sous-type
 * 
 * @param param La valeur qui a été entrée pour le flag -mime
 * @return 1 si param a le format adéquat pour -mime, 0 sinon
 */
int is_mime_param(char* param) {
    const char* mime_const = (const char*) param ;
    char* mime_pos_slash = strrchr(mime_const, '/') ;
    int mime_index_slash = mime_pos_slash - mime_const ;
    int no_subtype = !mime_pos_slash ;

    int position = 0 ;
    for ( ; MegaMimeTypes[position][0] ; ++position) {
        if (no_subtype) {
            if (strncmp(mime_const, MegaMimeTypes[position][MIMETYPE_POS], strlen(mime_const)) == 0) {
                return 1 ;
            }
        }
        else {
            if (strcmp(mime_const, MegaMimeTypes[position][MIMETYPE_POS]) == 0) {
                return 1 ;
            }
        }
        
    }
    return 0 ;
}

/**
 * @brief Fonction qui permet de vérifier que le format de param est bien adéquat pour -perm : (0-7)(0-7)(0-7)
 * 
 * @param param Argument de l'option -perm
 * @return 1 si param a le bon format, 0 sinon
 */
int is_perm_param(char* param) {
    return ((strlen(param) == 3) && (param[0] >= '0') && (param[0] <= '7') && (param[1] >= '0') && (param[1] <= '7') && (param[2] >= '0') && (param[2] <= '7')) ;
}

/**
 * @brief Fonction qui est lancée si le premier flag est -test. L'affichage est sous la forme "La valeur du flag -xxxx est yyyy".
 * 
 * @param argv La ligne de commande d'entrée
 */
void test(char* argv[]) {
    if(argv == NULL) {
        perror("Not enough arguments for -test");
    }
    if(is_flag(argv[0]) == 0) {
        printf("Le flag %s n'est pas correct\n", argv[0]);
    } else if (is_flag(argv[0]) == 2) {
        if (argv[1] != NULL) {
            if (argv[1][0] == '-') {
                printf("La valeur du flag %s est vide\n", argv[0]);
            } else {
                printf("Le flag %s n'est pas correct\n", argv[0]);
            }
        } else {
            printf("Le flag %s n'est pas correct\n", argv[0]);
        }
    } else {
        if (argv[1] == NULL) {
            printf("Le flag %s n'est pas correct\n", argv[0]);
        } else {
            if (strcmp(argv[0], "-size") == 0) {
                if(is_size_param(argv[1])) {
                    printf("La valeur du flag %s est %s\n", argv[0], argv[1]);
                } else {
                    printf("La valeur du flag %s est %s\n", argv[0], argv[1]);
                }
            } else {
                if(argv[1][0] == '-') {
                    printf("La valeur du flag %s est %s\n", argv[0], argv[1]);
                } else {
                    printf("La valeur du flag %s est %s\n", argv[0], argv[1]);
                }
            } 
        }
    }
}

/**
 * @brief 
 * 
 * @param argc Le nombre de paramètres du main
 * @param argv La ligne de commande d'entrée
 * @return Une liste de strct flags correspondant aux flags de la ligne de commande. S'il y a une erreur en ligne de commande, le premier flags aura comme flag_type -2. Sinon, le dernier flag aura comme flag_type -1.
 */
flag* parse(int argc, char* argv[]) {
    //char* chemin = argv[1];
    argv = argv + 2;
    argc = argc - 2;
    flag* flags_parse = calloc(argc + 1, sizeof(flag));
    if(argv[0] == NULL) {
        perror("Aucun flag");
    } else {
        int index_comm = 0;
        int index_parse = 0;
        while(index_comm < argc) {
            if(index_parse >= argc) {
                perror("Trop de flags");
                init_flag(&flags_parse[0], "END", "END", -2);
                return flags_parse;
            } else if(is_flag(argv[index_comm]) == 0) {
                perror("Flag incorrect");
                init_flag(&flags_parse[0], "END", "END", -2);
                return flags_parse;
            } else if(strcmp(argv[index_comm], "-dir") == 0) {
                if(index_comm+1 >= argc) {
                    init_flag(&flags_parse[index_parse], argv[index_comm], argv[index_comm], 2);
                    index_comm++;
                    index_parse++;
                }
                else if(is_flag(argv[index_comm + 1])) {
                    init_flag(&flags_parse[index_parse], argv[index_comm], argv[index_comm], 2);
                    index_comm++; 
                    index_parse++;
                } else {
                    init_flag(&flags_parse[index_parse], argv[index_comm], argv[index_comm + 1], 1);
                    index_comm++; index_comm++;
                    index_parse++;
                }
            } else if(is_flag(argv[index_comm]) == 2) {
                init_flag(&flags_parse[index_parse], argv[index_comm], argv[index_comm], 2);
                index_comm++;
                index_parse++;
            } else {
                if(argv[index_comm+1] == NULL) {
                    perror("Valeur manquante");
                    init_flag(&flags_parse[0], "END", "END", -2);
                return flags_parse;
                } else if(strcmp(argv[index_comm], "-size") == 0 && (is_size_param(argv[index_comm+1]) == 0)) {
                    perror("Mauvais flag pour -size");
                    init_flag(&flags_parse[0], "END", "END", -2);
                return flags_parse;
                } else {
                    init_flag(&flags_parse[index_parse], argv[index_comm], argv[index_comm+1], 1);
                    index_comm++; index_comm++;
                    index_parse++;
                }
            }
        }
        if(index_parse<argc) {
            init_flag(&flags_parse[index_parse+1], "END", "END", -1);
        }
        return flags_parse;
    }
    return flags_parse;
}

/**
 * @brief Fonction qui permet de print tous les chemins respectant les flags passés en ligne de commande. Gère aussi le flag -ou.
 * 
 * @param path Le chemin du dossier à parcourir
 * @param flags La liste de flags de la ligne de commande
 * @param nb_flags La structure qui stocke le nombre de certains flags
 */
void print_paths(char* path, flag* flags, nb_Flags* nb_flags) {
    DIR* dir;
    struct dirent* ent;
    if((dir=opendir(path)) != NULL) {
        while((ent=readdir(dir)) != NULL) {
            if(ent->d_type == DT_DIR) {
                if(strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0) {
                    char* new_path = malloc(strlen(path) + strlen(ent->d_name) + 2);
                    strcpy(new_path, path);
                    strcat(new_path, "/");
                    strcat(new_path, ent->d_name);
                    test_flags(new_path, flags, nb_flags);
                    print_paths(new_path, flags, nb_flags);
                    //free(new_path);
                }
            } else {
                char* new_path = malloc(strlen(path) + strlen(ent->d_name) + 2);
                strcpy(new_path, path);
                strcat(new_path, "/");
                strcat(new_path, ent->d_name);
                test_flags(new_path, flags, nb_flags);
                //free(new_path);
            }
        }
        closedir(dir);
    } else {
        perror("Impossible d'ouvrir le dossier");
    }
}

/**
 * @brief Fait un simple ls récursif. Elle est appelée quand aucun flags est passé en ligne de commande.
 * 
 * @param path Le chemin du dossier à parcourir
 */
void print_paths_none(char* path) {
    DIR* dir;
    struct dirent* ent;
    if((dir=opendir(path)) != NULL) {
        while((ent=readdir(dir)) != NULL) {
            if(ent->d_type == DT_DIR) {
                if(strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0) {
                    char* new_path = malloc(strlen(path) + strlen(ent->d_name) + 2);
                    strcpy(new_path, path);
                    strcat(new_path, "/");
                    strcat(new_path, ent->d_name);
                    printf("%s\n", new_path);
                    print_paths_none(new_path);
                }
            } else {
                char* new_path = malloc(strlen(path) + strlen(ent->d_name) + 2);
                strcpy(new_path, path);
                strcat(new_path, "/");
                strcat(new_path, ent->d_name);
                printf("%s/%s\n", path, ent->d_name);
            }
        }
        closedir(dir);
    } else {
        perror("Impossible d'ouvrir le dossier");
    }
}

/**
 * @brief Fonction qui permet de tester les flags sur un chemin donné.
 * 
 * @param chemin Le chemin sur lequel on teste les flags
 * @param flags La liste de flags de la ligne de commande
 * @param nb_flags La structure qui contient le nombre de certains flags
 */
void test_flags(char* chemin, flag* flags, nb_Flags* nb_flags) {
    if (chemin[strlen(chemin)-1] == '/') {
        chemin[strlen(chemin)-1] = '\0' ;
    }
    int tests_a_valider = 0;
    int tests_valides = 0;
    int index_flag = 0;
    struct stat path;
    stat(chemin, &path); 
    //printf("\n\nTraitement d'un nouveau chemin : %s\n", chemin);
    while((flags[index_flag].flag_type > -1) && (flags[index_flag].flag != NULL)) {
        //On teste si le flag est -name
        if((strcmp(flags[index_flag].flag, "-name") == 0) && (flags[index_flag].flag_type > 0)) {                                                  
            if(!S_ISDIR(path.st_mode)) {
                //printf("\n\n\nChemin en train d'être testé : %s\n", chemin);
                tests_valides += check_name(chemin, flags[index_flag].valeur);
            }
            tests_a_valider++;
        } else if((strcmp(flags[index_flag].flag, "-size") == 0) && (flags[index_flag].flag_type > 0)) {
            if(!S_ISDIR(path.st_mode)) {
                tests_valides += check_size(chemin, flags[index_flag].valeur);
            }
            tests_a_valider++;
        } else if((strcmp(flags[index_flag].flag, "-ctc") == 0) && (flags[index_flag].flag_type > 0)) {
            if(!S_ISDIR(path.st_mode)) {
                //char* text = read_file(chemin);
                tests_valides += check_ctc(chemin, flags[index_flag].valeur);
                //free(text);
            }
            tests_a_valider++;
        } else if(strcmp(flags[index_flag].flag, "-dir") == 0 && flags[index_flag].flag_type == 2) {
            if(S_ISDIR(path.st_mode)) {
                tests_valides++;
            }
            tests_a_valider++;
        } else if(strcmp(flags[index_flag].flag, "-dir") == 0 && flags[index_flag].flag_type == 1) {
            if(S_ISDIR(path.st_mode)) {
                tests_valides += check_name(chemin, flags[index_flag].valeur);
            }
            tests_a_valider++;
        } else if ((strcmp(flags[index_flag].flag, "-date") == 0) && (flags[index_flag].flag_type > 0)) {
            if(!S_ISDIR(path.st_mode)) {
                if (flags[index_flag].valeur[0] == '+') {
                    tests_valides += 1 - check_date(chemin, flags[index_flag].valeur) ;
                }
                else {
                    tests_valides += check_date(chemin, flags[index_flag].valeur) ;
                }
            }
            tests_a_valider++;
        } else if ((strcmp(flags[index_flag].flag, "-mime") == 0) && (flags[index_flag].flag_type > 0)) {
            if(!S_ISDIR(path.st_mode)) {
                tests_valides += check_mime(chemin, flags[index_flag].valeur);
            }
            tests_a_valider++;
        } else if ((strcmp(flags[index_flag].flag, "-perm") == 0) && (flags[index_flag].flag_type > 0)) {
            if(!S_ISDIR(path.st_mode)) {
                tests_valides += check_perm(chemin, flags[index_flag].valeur);
            }
            tests_a_valider++;
        }
        index_flag++;
    }
    if(!nb_flags->nb_ou) {
        if(tests_a_valider == tests_valides) {
            if(nb_flags->nb_color && S_ISDIR(path.st_mode)) {
                printf("\033[0;32m");
                printf("%s\n", chemin);
                printf("\033[0m");
            } else {
                printf("%s\n", chemin);
            }
        }        
    } else {
        if(tests_valides >= 1 || tests_a_valider == 0) {
            if(nb_flags->nb_color && S_ISDIR(path.st_mode)) {
                printf("\033[0;32m");
                printf("%s\n", chemin);
                printf("\033[0m");
            } else {
                printf("%s\n", chemin);
            }
        }
    } 
}

/**
 * @brief Fonction qui permet de tester si le chemin passé en paramètre correspond au nom passé en paramètre. Applique donc le test sur -name.
 * 
 * @param chemin Le chemin sur lequel on teste le nom
 * @param nom_a_tester Le nom en paramètre de -name
 * @return 1 si le test est validé, 0 sinon
 */
int check_name(char* chemin, char* nom_a_tester) {
        char* pos_slash_ptr = strrchr(chemin, '/');                                          //Renvoie un pointeur vers la position du dernier slash
        int pos_slash = (pos_slash_ptr == NULL ? -1 : pos_slash_ptr - chemin);               //On calcule la position du dernier slash
        if(pos_slash == -1) {
            return 0;
        } else {
            char* name = chemin + pos_slash + 1;                                               //On récupère le nom du fichier
            if(is_regex(nom_a_tester)) {
                //printf("Regex : %s\n", nom_a_tester); 
                regex_t reegex;
                int value = regcomp(&reegex, nom_a_tester, 0);
                if(value) {
                    perror("Erreur de compilation de l'expression régulière");
                    exit(1);
                } else {
                    value = regexec(&reegex, name, 0, NULL, 0);
                    if(value == 0) {
                        regfree(&reegex);
                        return 1;
                    } else if (value == REG_NOMATCH) {
                        regfree(&reegex);
                        return 0;
                    }
                }
            } else {
                //printf("Pas regex : %s\n", nom_a_tester);
                if(strcmp(name, nom_a_tester) == 0) {
                    return 1;
                } else {
                    return 0;
                }
            }  
        }
    return 0;
}

/**
 * @brief Returns 1 if exp is a regex
 * 
 * @param exp A string to test
 * @return 1 if the test is positive
 */
int is_regex(char* exp) {
    for(int i = 0; i < (int) strlen(exp); i++) {
        //printf("%c\n", exp[i]);
        if(exp[i] == '*' || exp[i] == '?' || exp[i] == '[' || exp[i] == ']' || exp[i] == '{' || exp[i] == '}' || exp[i] == '(' || exp[i] == ')' || exp[i] == '|' || exp[i] == '^' || exp[i] == '$') {
            return 1;
        }
    }
    return 0;
}


/**
 * @brief Fonction qui permet de tester si le chemin passé en paramètre correspond à la taille passée en paramètre. Applique donc le test sur -size.
 * 
 * @param chemin Le chemin sur lequel on teste le nom
 * @param size_a_tester La taille en paramètre de -size
 * @return 1 si le test est validé, 0 sinon
 */
int check_size(char* chemin, char* size_a_tester) {
    if(!is_size_param(size_a_tester)) {
        perror("Mauvais flag pour -size");
        return 0;
    } else {
        char signe = '0';
        char* taille_string = malloc(sizeof(char)*(strlen(size_a_tester)-1));
        char multiplicateur = size_a_tester[strlen(size_a_tester)-1];
        if((size_a_tester[0] == '-') || (size_a_tester[0] == '+')) {
            signe = size_a_tester[0];
            strncpy(taille_string, size_a_tester+1, strlen(size_a_tester)-2);
            taille_string[strlen(size_a_tester)-2] = '\0';
        } else {
            strncpy(taille_string, size_a_tester, strlen(size_a_tester)-1);
            taille_string[strlen(size_a_tester)-1] = '\0';
        }
        long long int taille = atoi(taille_string);
        if(multiplicateur == 'k') {
            taille *= 1024;
        } else if(multiplicateur == 'M') {
            taille *= 1024*1024;
        } else if(multiplicateur == 'G') {
            taille *= 1024*1024*1024;
        }
        //printf("Taille : %lld\n", taille);
        if(signe == '+') {
            struct stat st;
            stat(chemin, &st);
            int taille_du_fichier = st.st_size;
            return (taille_du_fichier > taille);
        } else if(signe == '-') {
            struct stat st;
            stat(chemin, &st);
            int taille_du_fichier = st.st_size;
            return (taille_du_fichier < taille);
        } else {
            struct stat st;
            stat(chemin, &st);
            int taille_du_fichier = st.st_size;
            return (taille_du_fichier == taille);
        }
    }
}

/**
 * @brief Fonction qui renvoie le contenu du fichier de chemin path dans une chaîne de caractères
 * 
 * @param path Le chemin du fichier à lire
 * @return Le contenu du fichier
 */
char* read_file(char* path) {
    char* buffer = 0;
    long int length;
    FILE* file = fopen(path, "r");
    if(file) {
        fseek(file, 0, SEEK_END);
        length = ftell(file);
        fseek(file, 0, SEEK_SET);
        buffer = (char*) malloc((length+1) * sizeof(char));
        if(buffer) {
            fread(buffer, 1, length, file);
        }
        fclose(file);
    }
    buffer[length] = '\0';
    return buffer;
}

/**
 * @brief Fonction qui permet de tester si le mot_a_tester est présent dans le texte passé en paramètre. Applique le test sur -ctc.
 * 
 * @param texte Le texte que l'on a préalablement lu dans un fichier
 * @param mot_a_tester Le mot que l'on cherche dans le texte
 * @return 0 si le teste est négatif, 1 si il est positif
 */
/*
int check_ctc(char* texte, char* mot_a_tester) {
    char* pos_mot = strstr(texte, mot_a_tester);
    if(pos_mot == NULL) {
        return 0;
    } else {
        return 1;
    }
}
*/
int check_ctc(char* chemin, char* param)
{
    FILE* file = NULL ;
    file = fopen(chemin, "r+") ;
    if (file) {
        if (is_regex(param)) {
            regex_t reegex ;
            int value = regcomp(&reegex, param, 0);
            if(value) {
                perror("Erreur de compilation de l'expression régulière");
                exit(1);
            }
            else {
                char* previous_line = (char*) malloc(sizeof(char)*256) ;
                char current_line[256] = "" ;
                while (fgets(current_line, 256, file) != NULL) {
                    char* previous_current_line = (char*) malloc(sizeof(char)*512) ;
                    strncpy(previous_current_line, previous_line, strlen(current_line)) ;
                    strcat(previous_current_line, current_line) ;
                    value = regexec(&reegex, previous_current_line, 0, NULL, 0);
                    if (value == 0) {
                        regfree(&reegex) ;
                        free(previous_line) ;
                        free(previous_current_line) ;
                        fclose(file) ;
                        return 1 ;
                    }
                    free(previous_current_line) ;
                    memset(previous_line, 0, strlen(previous_line)) ;
                }
                free(previous_line) ;
                fclose(file) ;
            }
            regfree(&reegex);
        }
        else {
            char* previous_line = (char*) malloc(sizeof(char)*256) ;
            char current_line[256] = "" ;
            while (fgets(current_line, 256, file) != NULL) {
                char* previous_current_line = (char*) malloc(sizeof(char)*512) ;
                strncpy(previous_current_line, previous_line, strlen(current_line)) ;
                strcat(previous_current_line, current_line) ;
                char* pos_param = strstr(previous_current_line, param) ;
                if (pos_param != NULL) {
                    free(previous_line) ;
                    free(previous_current_line) ;
                    fclose(file) ;
                    return 1 ;
                }
                free(previous_current_line) ;
                memset(previous_line, 0, strlen(previous_line)) ;
            }
            free(previous_line) ;
            fclose(file) ;
        }
    }
    return 0 ;
}


/**
 * @brief Fonction qui permet de convertir la chaine de caractères du temps passé en paramètre de -date en nombre de secondes
 * 
 * @param param La valeur du flag -date
 * @return Le nombre de secondes (unsigned long) correspondant à la durée du paramètre
 */
unsigned long param_to_secondes(char* param) {
    unsigned long total = 0 ;

    int n = strlen(param) ;
    char* param_cpy = (char*) calloc(1, sizeof(char)*n) ;
    strcpy(param_cpy, param) ;

    int i = 0 ;
    while (i < (int) strlen(param_cpy)) {
        if ((param_cpy[i] != 'm') && (param_cpy[i] != 'h') && (param_cpy[i] != 'j')) {
            i++;
        }
        else {
            if (param_cpy[i] == 'j') {
                char* nb_days = (char*) calloc(1, sizeof(char)*i) ;
                strncpy(nb_days, param_cpy, i) ;
                total += (unsigned long) atoi(nb_days)*24*60*60 ;

                i++;
                if ((i >= (int) strlen(param_cpy)) && total > 0) {
                    //free(param_cpy) ;
                    total += 59 ;
                }
                else {
                    param_cpy = param_cpy + i ;
                    i = 0 ;
                }
                free(nb_days) ;
            }
            else if (param_cpy[i] == 'h') {
                char* nb_hours = (char*) calloc(1, sizeof(char)*i) ;
                strncpy(nb_hours, param_cpy, i) ;
                total += (unsigned long) atoi(nb_hours)*60*60 ;
                
                i++;
                if ((i >= (int) strlen(param_cpy)) && total > 0) {
                    //free(param_cpy) ;
                    total += 59 ;
                }
                else {
                    param_cpy = param_cpy + i ;
                    i = 0 ;
                }
                free(nb_hours) ;
            }
            else if (param_cpy[i] == 'm') {
                char* nb_minutes = (char*) calloc(1, sizeof(char)*i) ;
                strncpy(nb_minutes, param_cpy, i) ;
                total += (unsigned long) atoi(nb_minutes)*60 ;
                
                i++;
                if ((i >= (int) strlen(param_cpy)) && total > 0) {
                    //free(param_cpy) ;
                    total += 59 ;
                }
                else {
                    param_cpy = param_cpy + i ;
                    i = 0 ;
                }
                free(nb_minutes) ;
            }
        }
    }
    return total ;
}

/**
 * @brief Fonction qui permet de tester si fichier d'emplacement chemin a été modifié pour la dernière fois moins récemment que la durée précisée par param
 * 
 * @param chemin L'emplacement du fichier tester
 * @param param La valeur du flag -date
 * @return 1 si le fichier a été modifié il y a moins de temps que param, 0 sinon
 */
int check_date(char* chemin, char* param) {
    if(!is_date_param(param)) {
        perror("Mauvais flag pour -date");
        return 0;
    }
    else {
        unsigned long nb_secondes_param = (unsigned long) param_to_secondes(param) ;
        struct stat st;
        stat(chemin, &st);
        time_t last_access = st.st_mtime ;
        time_t timestamp = time(NULL) ;
        unsigned long nb_secondes = (unsigned long) difftime(timestamp, last_access) ;
        return nb_secondes_param >= nb_secondes ;
    }
}

/*
int check_mime(char* chemin, char* mime) {
    //printf("Nom %s\n", chemin);
    char* pos_point_ptr = strrchr(chemin, '.');                                          //Renvoie un pointeur vers la position du dernier slash
    int pos_point = (pos_point_ptr == NULL ? -1 : pos_point_ptr - chemin);
    if(!pos_point) {
        return 0;
    }
    char* point_fich = chemin + pos_point;
    //printf("Extension %s\n", point_fich);
    if(mime[strlen(mime)] != '*') {
        char* slash_pos_pointer = strrchr(mime, '/');
        int slash_pos = (slash_pos_pointer == NULL ? -1 : slash_pos_pointer - mime);
        if(slash_pos == -1) {
            strcat(mime, "/*");
        } else if (slash_pos == strlen(mime) - 1) {
            strcat(mime, "*");
        }
    }
    //printf("Mime %s\n", mime);
    char** extensions = getMegaMimeExtensions(mime);
    if(!extensions) {
        return 0;
    } else {
        int i = 0;
        while(extensions[i]) {
            //printf("Extension %s et point_fich %s\n", extensions[i] + 1, point_fich);
            if(!strcmp(extensions[i] + 1, point_fich)) {
                freeMegaStringArray(extensions);
                return 1;
            }
            i++;
        }
        freeMegaStringArray(extensions);
        return 0;
    }
}
*/

/**
 * @brief Fonction qui permet de tester si le fichier à l'emplacement chemin est de type ou de type/sous-type mime
 * 
 * @param chemin L'emplacement du fichier testé
 * @param mime La valeur du flag -mime
 * @return 1 si le fichier est bien du type ou du type/sous-type mime, 0 sinon
 */
int check_mime(char* chemin, char* mime) {
    if (!is_mime_param(mime)) {
        perror("Mauvais flag pour -mime");
        return 0;
    }
    else {
        const char* mime_const = (const char*) mime ;
        char* mime_pos_slash = strrchr(mime_const, '/') ;
        int not_searching_subtype = !mime_pos_slash ;

        const char* chemin_const = (const char*) chemin ;
        const char* chemin_mime = getMegaMimeType(chemin_const) ;

        if ((!chemin_mime)) {
            return 0 ;
        }
        else {
            if (not_searching_subtype) {
                int diff_types = strncmp(chemin_mime, mime_const, strlen(mime_const)) ;
                return (diff_types == 0) ;
            }
            else {
                const char** mime_extensions = getMegaMimeExtensions(mime_const) ;
                if (!mime_extensions) {
                    return 0 ;
                }
                int mime_index_slash = mime_pos_slash - mime_const ;
                int diff_types = strncmp(chemin_mime, mime_const, mime_index_slash) ;

                // On extrait l'extension du chemin
                char* chemin_pos_dot = strrchr(chemin_const, '.') ;
                int chemin_extention_len = strlen(chemin_pos_dot) ;
                char* chemin_extension = (char*) calloc(1, sizeof(char)*(chemin_extention_len+1)) ;
                chemin_extension[0] = '*' ;
                strcat(chemin_extension, chemin_pos_dot) ;
                int index = 0 ;
                while (mime_extensions[index] != NULL) {
                    int diff_subtypes = strcmp(mime_extensions[index], chemin_extension) ;
                    if (diff_subtypes == 0) {
                        free(chemin_extension) ;
                        return (diff_types == 0) ;
                    }
                    index++;
                }
                free(chemin_extension) ;
                return 0 ;
            }
        }
    }
}

/**
 * @brief Fonction qui permet de tester si un fichier d'emplacement chemin a les permissions précisées par param
 * 
 * @param chemin Emplacement du fichier testé
 * @param param Permissions que l'on veut tester
 * @return 1 si le fichier a les permissions de param, 0 sinon
 */
int check_perm(char* chemin, char* param) {
    if (!is_perm_param(param)) {
        perror("Mauvais flag pour -perm");
        return 0;
    }
    else {
        struct stat path ;
        stat(chemin, &path) ;
        int i = 0 ;
        // Les droits de l'utilisateur
        if ((param[0] == '7') && (path.st_mode & S_IRUSR) && (path.st_mode & S_IWUSR) && (path.st_mode & S_IXUSR)) {
            i++ ;
        }
        else if ((param[0] == '6') && (path.st_mode & S_IRUSR) && (path.st_mode & S_IWUSR) && !(path.st_mode & S_IXUSR)) {
            i++ ;
        }
        else if ((param[0] == '5') && (path.st_mode & S_IRUSR) && !(path.st_mode & S_IWUSR) && (path.st_mode & S_IXUSR)) {
            i++ ;
        }
        else if ((param[0] == '4') && (path.st_mode & S_IRUSR) && !(path.st_mode & S_IWUSR) && !(path.st_mode & S_IXUSR)) {
            i++ ;
        }
        else if ((param[0] == '3') && !(path.st_mode & S_IRUSR) && (path.st_mode & S_IWUSR) && (path.st_mode & S_IXUSR)) {
            i++ ;
        }
        else if ((param[0] == '2') && !(path.st_mode & S_IRUSR) && (path.st_mode & S_IWUSR) && !(path.st_mode & S_IXUSR)) {
            i++ ;
        }
        else if ((param[0] == '1') && !(path.st_mode & S_IRUSR) && !(path.st_mode & S_IWUSR) & (path.st_mode & S_IXUSR)) {
            i++ ;
        }
        else if ((param[0] == '0') && !(path.st_mode & S_IRUSR) && !(path.st_mode & S_IWUSR) && !(path.st_mode & S_IXUSR)) {
            i++ ;
        }

        // Les droits du groupe
        if ((param[1] == '7') && (path.st_mode & S_IRGRP) && (path.st_mode & S_IWGRP) && (path.st_mode & S_IXGRP)) {
            i++ ;
        }
        else if ((param[1] == '6') && (path.st_mode & S_IRGRP) && (path.st_mode & S_IWGRP) && !(path.st_mode & S_IXGRP)) {
            i++ ;
        }
        else if ((param[1] == '5') && (path.st_mode & S_IRGRP) && !(path.st_mode & S_IWGRP) && (path.st_mode & S_IXGRP)) {
            i++ ;
        }
        else if ((param[1] == '4') && (path.st_mode & S_IRGRP) && !(path.st_mode & S_IWGRP) && !(path.st_mode & S_IXGRP)) {
            i++ ;
        }
        else if ((param[1] == '3') && !(path.st_mode & S_IRGRP) && (path.st_mode & S_IWGRP) && (path.st_mode & S_IXGRP)) {
            i++ ;
        }
        else if ((param[1] == '2') && !(path.st_mode & S_IRGRP) && (path.st_mode & S_IWGRP) && !(path.st_mode & S_IXGRP)) {
            i++ ;
        }
        else if ((param[1] == '1') && !(path.st_mode & S_IRGRP) && !(path.st_mode & S_IWGRP) && (path.st_mode & S_IXGRP)) {
            i++ ;
        }
        else if ((param[1] == '0') && !(path.st_mode & S_IRGRP) && !(path.st_mode & S_IWGRP) && !(path.st_mode & S_IXGRP)) {
            i++ ;
        }

        // Les droits des autres
        if ((param[2] == '7') && (path.st_mode & S_IROTH) && (path.st_mode & S_IWOTH) && (path.st_mode & S_IXOTH)) {
            i++ ;
        }
        else if ((param[2] == '6') && (path.st_mode & S_IROTH) && (path.st_mode & S_IWOTH) && !(path.st_mode & S_IXOTH)) {
            i++ ;
        }
        else if ((param[2] == '5') && (path.st_mode & S_IROTH) && !(path.st_mode & S_IWOTH) && (path.st_mode & S_IXOTH)) {
            i++ ;
        }
        else if ((param[2] == '4') && (path.st_mode & S_IROTH) && !(path.st_mode & S_IWOTH) && !(path.st_mode & S_IXOTH)) {
            i++ ;
        }
        else if ((param[2] == '3') && !(path.st_mode & S_IROTH) && (path.st_mode & S_IWOTH) && (path.st_mode & S_IXOTH)) {
            i++ ;
        }
        else if ((param[2] == '2') && !(path.st_mode & S_IROTH) && (path.st_mode & S_IWOTH) && !(path.st_mode & S_IXOTH)) {
            i++ ;
        }
        else if ((param[2] == '1') && !(path.st_mode & S_IROTH) && !(path.st_mode & S_IWOTH) && (path.st_mode & S_IXOTH)) {
            i++ ;
        }
        else if ((param[2] == '0') && !(path.st_mode & S_IROTH) && !(path.st_mode & S_IWOTH) && !(path.st_mode & S_IXOTH)) {
            i++ ;
        }

        return (i == 3) ;
    }
}

////////////////////////////////////////////////////////////
//Définition des fonctions de gestion de la structure flag//
////////////////////////////////////////////////////////////

/**
 * @brief Fonction qui permet d'initialiser une structure flag
 * 
 * @param flag L'adresse du flag duquel on change les attributs
 * @param flag_name Le prochain nom du flag.
 * @param value La prochaine valeur du flag. Si le flag est sans paramètre, on y met le nom du flag. 
 * @param type Il sert à discriminer les flags. En règle général, c'est is_flag(flag_name). Il peut valoir -2 dans le cas où le parsing a échoué. Il peut valoir -1 pour la dernier flag de la liste parsée. Et on le met à 0 aussi par exemple pour les flags -name si on a en ligne de commande -dir avec un paramètre.
 * @return 0 si tout s'est bien passé, -1 sinon
 */
int init_flag(flag* flag, char* flag_name, char* value, int type) {
    if(flag==NULL) {
        return -1;
    }
    flag->flag = calloc(1, sizeof(char*));
    flag->flag = strdup(flag_name);
    flag->valeur = calloc(1, sizeof(char*));
    flag->valeur = strdup(value);
    flag->flag_type = type;

    return 0;
}

/**
 * @brief Fonction qui libère la mémoire allouée pour chaque flag dans la liste de flags. Ne libère pas la mémoire allouée pour la liste de flags.
 * 
 * @param flags La liste à libérer.
 */
void free_flag(flag* flags) {
    int i = 0;
    while(flags[i].flag != NULL) {
        free(flags[i].flag);
        free(flags[i].valeur);
        i++;
    }
}

/**
 * @brief Fonction qui permet de print un flag
 * 
 * @param flag Le flag à print
 */
void print_flag(flag* flag) {
    printf("Le flag est %s, a pour valeur %s et type %d\n", flag->flag, flag->valeur, flag->flag_type);
}
