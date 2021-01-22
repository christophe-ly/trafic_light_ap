#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#define alpha 3

typedef struct voiture{
    int tps_arriv;
    int tps_attente;
    int tps_passage;
    int position;
    struct voiture *suiv;
    struct voiture *prec;
}Voiture;


typedef struct
{
    Voiture *tete;
}ListeVoiture;


typedef struct{
    int Tv, Tr, Tj;
    int position; //0 principal(commence au vert), 1 secondaire (commence au rouge)
    ListeVoiture l;
}Feu;

ListeVoiture *init_liste(Feu *feu);
void init_feu(Feu *feu);
void ajout_voiture(ListeVoiture *liste, Feu *feu, int intervalle_tps);
void affiche_liste(ListeVoiture *liste);
void affiche_voiture(Voiture *voiture);
void complete_liste(float lambda, Feu *feu, ListeVoiture *liste, int T);
void create_liste(ListeVoiture *liste, Feu *feu);

int tpsatt_vide(Feu *feu, int tps_arriv);
int calcul_tpsatt(ListeVoiture *liste, Feu *feu, Voiture *courant);
int calcul_tpspassage(Voiture *courant);
int calcul_position(ListeVoiture *liste, Feu *feu, Voiture *courant);

void creation_fichier(ListeVoiture *liste);
void affiche_fichier(void);
void taille_file_max(void);
void taille_moyenne(void);
void tps_attente_moyen(void);

void menu();
void submenu();


void init_feu(Feu *feu)
{
    int tv,tj,tr,pos;

    printf("Saisir respectivement la durée d'un feu vert, jaune, rouge et la position du feu \n");
    scanf("%d %d %d %d", &tv, &tj, &tr, &pos);

    feu->Tv=tv;
    feu->Tr=tr;
    feu->Tj=tj;
    feu->position=pos;
}

ListeVoiture *init_liste(Feu *feu){
    ListeVoiture *liste = malloc(sizeof(*liste));
    Voiture *voiture = malloc(sizeof(*voiture));

    voiture->tps_arriv = rand()%11;

    voiture->tps_attente=tpsatt_vide(feu, voiture->tps_arriv);
    voiture->tps_passage = calcul_tpspassage(voiture);
    voiture->suiv = NULL;
    voiture->prec=NULL;
    voiture->position=1;
    liste->tete = voiture;

    return liste;
}

void ajout_voiture(ListeVoiture *liste, Feu *feu, int intervalle_tps){
    Voiture *nouvelle, *courant;
    int tp_att, tp_pass;

    nouvelle = (Voiture *) malloc(sizeof(*nouvelle));
    courant = liste->tete;

    while(courant->suiv != NULL)
    {
        courant=courant->suiv;
    }

    nouvelle->tps_arriv = courant->tps_arriv + intervalle_tps;
    nouvelle->suiv = NULL;
    nouvelle->prec=courant;

    tp_att = calcul_tpsatt(liste, feu, nouvelle);
    nouvelle->tps_attente = tp_att;

    tp_pass=calcul_tpspassage(nouvelle);
    nouvelle->tps_passage = tp_pass;

    nouvelle->position=calcul_position(liste,feu,nouvelle);

    courant->suiv=nouvelle;


}

void affiche_liste(ListeVoiture *liste){
    Voiture *voiture = liste->tete;
    printf("\n");

    while (voiture != NULL)
    {
        affiche_voiture(voiture);
        voiture=voiture->suiv;
    }
}

void affiche_voiture(Voiture *voiture){
    printf("%d %d %d %d \n", voiture->tps_arriv, voiture->tps_passage, voiture->tps_attente, voiture->position);
}

void complete_liste(float lambda, Feu *feu, ListeVoiture *liste, int T){
    int tps_ecoule=liste->tete->tps_arriv;
    int intervalle_tps;
    float U; //nombre compris entre 0 et 1

    while(tps_ecoule<T)
    {
        U=(float)rand() / (float)RAND_MAX;
        intervalle_tps=1-log(1-U)/lambda;

        ajout_voiture(liste, feu, intervalle_tps);
        tps_ecoule=tps_ecoule+intervalle_tps;
    }

}

int tpsatt_vide(Feu *feu, int tps_arriv){
    //calcule le temps d'attente d'une voiture étant première dans la file (juste devant le feu)

    int tpsatt;
    int T = feu->Tv + feu->Tj + feu->Tr;

    if (feu->position == 0)
    {
        if(tps_arriv>=T)
        {
            while (tps_arriv >= T) {
                tps_arriv = tps_arriv - T;
            }
        }

        if(tps_arriv > feu->Tj + feu->Tv - alpha)
        {
            tpsatt=feu->Tr + alpha - (tps_arriv-(feu->Tj + feu->Tv-alpha));
        }

        else if(tps_arriv==0)
        {
            tpsatt=0;
        }

        else
        {
            tpsatt=0;
        }
    }

    else
    {
        while(tps_arriv>=T)
        {
            tps_arriv = tps_arriv-T;
        }


        if(tps_arriv >= feu->Tr && tps_arriv < feu->Tr + feu->Tj + feu->Tv -alpha)
        {
            tpsatt=0;
        }

        else if(tps_arriv==0)
        {
            tpsatt=feu->Tr;
        }

        else if(tps_arriv< feu->Tr)
        {
            tpsatt=feu->Tr - tps_arriv;
        }

        else
        {
            tpsatt= (feu->Tv +feu->Tj)-(tps_arriv-feu->Tr)+feu->Tr;
        }
    }
    return tpsatt;
}

int calcul_tpsatt(ListeVoiture *liste, Feu *feu, Voiture *courant)
{
    int temps, intervalle, tps_att;
    Voiture *precedent;

    temps=courant->tps_arriv;
    precedent=courant->prec;
    intervalle=temps - precedent->tps_arriv;

    if(precedent->tps_attente <= intervalle) //cela signifie que la voiture d'avant est déjà partie donc file vide
    {
        tps_att=tpsatt_vide(feu, temps);
    }

    else
    {
        //la voiture arrive devant le feu après le temps restant d'attente de la voiture d'avant et son tps de passage
        //elle doit attendre le temps restant d'attente de la voiture d'avant + le temps qu'elle passe une fois devant le feu (alpha) + son temps d'attente
        //à l'arrivée au feu
        tps_att= (precedent->tps_attente - intervalle) + alpha + tpsatt_vide(feu, temps + (precedent->tps_attente - intervalle) + alpha);
    }

    return tps_att;
}

int calcul_tpspassage(Voiture *courant)
{
    int tps_passage;

    tps_passage=courant->tps_arriv + courant->tps_attente;

    return tps_passage;
}

int calcul_position(ListeVoiture *liste, Feu *feu, Voiture *courant)
{
    int pos;
    Voiture *precedent;
    int compteur=0;

    precedent=courant->prec;

    if(courant->tps_arriv > precedent->tps_passage) //cela signifie que la voiture d'avant est déjà partie donc file vide
    {
        pos=1;
    }

    else
    {
        while(courant->tps_arriv < precedent->tps_passage)
        {
            precedent=precedent->prec;
            compteur++;
        }

        pos=compteur + 1;
    }
    return pos;
}

void creation_fichier(ListeVoiture *liste)
{
    FILE *fichier;
    Voiture *courant;
    courant=liste->tete;

    fichier=fopen("listevehicule.bin", "w");
    fclose(fichier);

    fichier=fopen("listevehicule.bin", "a");
    while (courant->suiv != NULL)
    {
        fwrite(courant, sizeof(Voiture), 1, fichier);
        courant=courant->suiv;
    }
    fclose(fichier);
}

void affiche_fichier(void)
{
    FILE* fichier;
    int i=0;
    int test=1;
    Voiture *courant = malloc(sizeof(Voiture));

    fichier=fopen("listevehicule.bin","r");
    while(fread(courant, sizeof(Voiture), 1, fichier) ==1)
    {
        affiche_voiture(courant);
    }

    fclose(fichier);

}

void taille_file_max(void){
    int taille_max=0;
    FILE* fichier;
    Voiture *courant = malloc(sizeof(Voiture));

    fichier=fopen("listevehicule.bin","r");
    while(fread(courant, sizeof(Voiture), 1, fichier) !=0)
    {
        if(courant->position>taille_max)
        {
            taille_max=courant->position;
        }
    }

    fclose(fichier);
    printf("La taille maximale d'une file est : %d \n", taille_max);

}

void taille_moyenne(void)
{
    float taille_moyenne=0;
    int compteur=0;
    FILE* fichier;
    Voiture *courant = malloc(sizeof(Voiture));

    fichier=fopen("listevehicule.bin","r");
    while(fread(courant, sizeof(Voiture), 1, fichier) !=0)
    {
        taille_moyenne=taille_moyenne + courant->position;
        compteur++;
    }

    fclose(fichier);
    taille_moyenne=taille_moyenne/compteur;
    printf("La taille moyenne d'une file est : %f \n", taille_moyenne);
}

void tps_attente_moyen(void)
{
    float tps_moyen=0;
    int compteur=0;
    FILE* fichier;
    Voiture *courant = malloc(sizeof(Voiture));

    fichier=fopen("listevehicule.bin","r");
    while(fread(courant, sizeof(Voiture), 1, fichier) !=0)
    {
        tps_moyen=tps_moyen + courant->tps_attente;
        compteur++;
    }
    fclose(fichier);
    tps_moyen=tps_moyen/compteur;
    printf("La temps moyen d'attente au feu est : %f \n", tps_moyen);
}



void create_liste(ListeVoiture *liste, Feu *feu)
{
    float lambda;
    int tps_etude;

    printf("Saisir la valeur de lambda :\n");
    scanf("%f", &lambda);

    printf("Saisir la durée de l'étude du feu\n");
    scanf("%d", &tps_etude);

    liste=init_liste(feu);
    complete_liste(lambda,feu, liste,tps_etude);
    creation_fichier(liste);
}

void submenu()
{
    bool quitter = false;
    int choix;

    while (!quitter)
    {
        printf("\n");
        printf("TESTS DE PERFOMRANCE \n \n");
        printf("1 Calcul de la taille maximale d'une file d'attente\n");
        printf("2 Calcul de la taille moyenne d'une file d'attente\n");
        printf("3 Calcul du temps de réponse moyen/ temps d'attente moyen\n");
        printf("4 Quitter\n");
        printf("Choix : ");
        scanf("%d", &choix);
        switch (choix)
        {
            case 1 :
                taille_file_max();;
                break;
            case 2 :
                taille_moyenne();
                break;
            case 3 :
                tps_attente_moyen();
                break;
            case 4 :
                quitter = true;
                break;
        }
    }
}

void menu()
{
    bool quitter = false;
    ListeVoiture *liste;
    Feu *feu=malloc(sizeof(Feu));
    int choix;

    while (!quitter)
    {
        printf("\n");
        printf("TABLEAU DE BORD PRINCIPAL \n \n");
        printf("1 Initialisation du feu\n");
        printf("2 Création de la liste de véhicules\n");
        printf("3 Afficher la liste de véhicules\n");
        printf("4 Tests de performance\n");
        printf("5 Quitter\n");
        printf("Choix : ");
        scanf("%d", &choix);
        switch (choix)
        {
            case 1 :
                init_feu(feu);;
                break;
            case 2 :
                create_liste(liste, feu);
                break;
            case 3 :
                affiche_fichier();
                break;
            case 4 :
                submenu();
                break;
            case 5 :
                quitter = true;
                break;
        }
    }
}

int main() {

    menu();

    return 0;
}