#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "leak_detector_c.h"

#define MAX_NAME_LENGTH 50 //Maximum of length of all names is 50 characters

/*Definition of Structures*/

typedef struct monster
{
    char *name;
    char *element;
    int population;
} monster;

typedef struct region
{
    char *name;
    int nmonsters;
    int total_population;
    monster **monsters;
} region;

typedef struct itinerary
{
    int nregions;
    region **regions;
    int captures;
} itinerary;

typedef struct trainer
{
    char *name;
    itinerary *visits;
} trainer;

/*Function Prototypes*/

monster *createMonster(char *name, char *element, int population);
monster **readMonsters(FILE *infile, int *monsterCount);
region **readRegions(FILE *infile, int *countRegions, monster **monsterList, int monsterCount);
trainer *readTrainers(FILE *infile, int *trainerCount, region **regionList, int countRegions);
void process_inputs(monster **monsterList, int monsterCount, region **regionList, int regionCount, trainer *trainerList, int trainerCount);
void release_memory(monster **monsterList, int monsterCount, region **regionList, int regionCount, trainer *trainerList, int trainerCount);

/*Main Function*/

int main(void)
{
    atexit(report_mem_leak); //Memory leak check

    FILE *fp;
    int mcount, rcount, tcount;
    monster **M_arr;
    region **R_arr;
    trainer *T_arr;

    fp = fopen("in.txt", "r"); //Opening input file

    if (fp == NULL)
    {
        printf("ERROR: Input file not found.\n");
        exit(-2); //Exiting with returning "-2", indicating file handling error
    }

    /*Initializing Monster, Region and Trainer Arrays*/

    M_arr = readMonsters(fp, &mcount);
    R_arr = readRegions(fp, &rcount, M_arr, mcount);
    T_arr = readTrainers(fp, &tcount, R_arr, rcount);

    fclose(fp);

    process_inputs(M_arr, mcount, R_arr, rcount, T_arr, tcount);
    release_memory(M_arr, mcount, R_arr, rcount, T_arr, tcount);

    return 0;
}

monster *createMonster(char *name, char *element, int population)
{
    monster *M;

    M = (monster *)malloc(sizeof(monster)); //Dynamically allocating memory for monster struct

    M->name = name;
    M->element = element;
    M->population = population;

    return M;
}

monster **readMonsters(FILE *infile, int *monsterCount)
{
    int i;
    int population;
    char ignore[50];
    char *N, *E; //Name(N) and Element(E) strings to be dynamically allocated
    monster **M_arr;
    

    fscanf(infile, "%d %s", monsterCount, ignore); //Taking input from file. (ignoring string)

    M_arr = (monster **)malloc(sizeof(monster *) * (*monsterCount)); //Dynamically allocating array of (monster*) pointers
    if (M_arr == NULL)
    {
        printf("ERROR: Out of Memory.\n");
        exit(-1); //Exiting with returning "-1", indicating memory allocation error
    }

    for (i = 0; i < (*monsterCount); i++)
    {
        N = (char *)malloc(sizeof(char) * MAX_NAME_LENGTH);
        E = (char *)malloc(sizeof(char) * MAX_NAME_LENGTH);
        fscanf(infile, "%s %s %d", N, E, &population);
        M_arr[i] = createMonster(N, E, population);
    }

    return M_arr;
}

region **readRegions(FILE *infile, int *countRegions, monster **monsterList, int monsterCount)
{
    int i, j, k;
    int rmcount, totalp;
    char temp[MAX_NAME_LENGTH]; //Temporary string
    char *N;
	char ignore[50];
    region *R;
    region **R_arr;
    monster **RM_arr;

    fscanf(infile, "%d", countRegions);
    fscanf(infile, "%s", ignore);

    R_arr = (region **)malloc(sizeof(region *) * (*countRegions)); //Dynamically allocating array of (region*) pointers
    if (R_arr == NULL)
    {
        printf("ERROR: Out of Memory.\n");
        exit(-1);
    }

    for (k = 0; k < (*countRegions); k++)
    {
        totalp = 0;

        R_arr[k] = (region *)malloc(sizeof(struct region)); //Dynamically allocating memory for region struct
        if (R_arr[k] == NULL)
        {
            printf("ERROR: Out of Memory.\n");
            exit(-1);
        }

        N = (char *)malloc(sizeof(char) * MAX_NAME_LENGTH);
        if (N == NULL)
        {
            printf("ERROR: Out of Memory.\n");
            exit(-1);
        }

        fscanf(infile, "%s", N);
        R_arr[k]->name = N;

        fscanf(infile, "%d %s", &rmcount, ignore);
        R_arr[k]->nmonsters = rmcount;

        RM_arr = (monster **)malloc(sizeof(monster *) * (rmcount)); //Array for monsters specific to a region
        if (RM_arr == NULL)
        {
            printf("ERROR: Out of Memory.\n");
            exit(-1);
        }

        /*Extracting region specfic monster structs and storing them in new array "RM_arr"*/

        for (i = 0; i < rmcount; i++)
        {
            fscanf(infile, "%s", temp);

            for (j = 0; j < monsterCount; j++)
            {
                if (strcmp(monsterList[j]->name, temp) == 0)
                {
                    RM_arr[i] = monsterList[j];
                    totalp += monsterList[j]->population; //Calculating total population of monsters in specific region
                }
            }
        }

        R_arr[k]->monsters = RM_arr; //Linking new array "RM_arr" with monsters field of region struct
        R_arr[k]->total_population = totalp;
    }

    return R_arr;
}

trainer *readTrainers(FILE *infile, int *trainerCount, region **regionList, int countRegions)
{
    int i, j, k;
    int captures, trcount;
    char temp[MAX_NAME_LENGTH];
    char *N;
	char ignore[50];
    trainer *T;
    itinerary *IT;
    region **TR_arr;

    fscanf(infile, "%d", trainerCount);
    fscanf(infile, "%s", ignore);

    T = (trainer *)malloc(sizeof(struct trainer) * (*trainerCount)); //Dynamically allocating array of trainer structs
    if (T == NULL)
    {
        printf("ERROR: Out of Memory.\n");
        exit(-1);
    }

    for (k = 0; k < (*trainerCount); k++)
    {

        N = (char *)malloc(sizeof(char) * MAX_NAME_LENGTH);
        if (N == NULL)
        {
            printf("ERROR: Out of Memory.\n");
            exit(-1);
        }
        fscanf(infile, "%s", N);
        T[k].name = N; //Storing name in trainer struct

        IT = (itinerary *)malloc(sizeof(struct itinerary)); //Dynamically allocating itinerary
        if (IT == NULL)
        {
            printf("ERROR: Out of Memory.\n");
            exit(-1);
        }

        fscanf(infile, "%d", &captures);
        fscanf(infile, "%s", ignore);
        IT->captures = captures;

        fscanf(infile, "%d %s", &trcount, ignore);
        IT->nregions = trcount;

        TR_arr = (region **)malloc(sizeof(region *) * (trcount)); //Array for regions specific to a Trainer
        if (TR_arr == NULL)
        {
            printf("ERROR: Out of Memory.\n");
            exit(-1);
        }

        /*Extracting trainer specfic region structs and storing them in new array "TR_arr"*/

        for (i = 0; i < trcount; i++)
        {
            fscanf(infile, "%s", temp);

            for (j = 0; j < countRegions; j++)
            {
                if (strcmp(regionList[j]->name, temp) == 0)
                {
                    TR_arr[i] = regionList[j];
                }
            }
        }

        IT->regions = TR_arr; //Linking new array "TR_arr" with regions field of itinerary struct
        T[k].visits = IT;     //Finally, linking filled itinerary with trainer struct
    }

    return T;
}

void process_inputs(monster **monsterList, int monsterCount, region **regionList, int regionCount, trainer *trainerList, int trainerCount)
{
    FILE *fp;
    int i, j, k;
    int monstersCaptured;
    float capturesperRegion;
    float relativePopulation;
    float totalrelPopulation;

    fp = fopen("out.txt", "w");

    if (fp == NULL)
    {
        printf("ERROR: Output file cannot be created.\n");
        exit(-2); //File handling error -> "-2"
    }

    for (i = 0; i < trainerCount; i++)
    {
        printf("%s\n", trainerList[i].name);  //Writing to console
       fprintf(fp, "%s\n", trainerList[i].name);  //Writing to file

        for (j = 0; j < ((trainerList[i].visits)->nregions); j++)
        {
            printf("%s\n", trainerList[i].visits->regions[j]->name);
            fprintf(fp, "%s\n", trainerList[i].visits->regions[j]->name);

            for (k = 0; k < (trainerList[i].visits->regions[j]->nmonsters); k++)
            {
                //Calculating total monsters (name and number) captured  by Trainer in specific Region
                capturesperRegion = (trainerList[i].visits)->captures;
                relativePopulation = (trainerList[i].visits->regions[j]->monsters[k])->population;
                totalrelPopulation = (trainerList[i].visits->regions[j])->total_population;
                monstersCaptured = round((relativePopulation / totalrelPopulation) * capturesperRegion); //Rounding

                if (monstersCaptured == 0) //Skipping output if monsterCaptured is Zero i.e., no monster captured
                    continue;

                printf("%d ", monstersCaptured);
                fprintf(fp, "%d ", monstersCaptured); //Number of monsters captured

                printf("%s\n", ((trainerList[i].visits->regions[j]->monsters[k])->name));
                fprintf(fp, "%s\n", ((trainerList[i].visits->regions[j]->monsters[k])->name)); //Name of monsters captured
            }
        }
        printf("\n");
        fprintf(fp, "\n");
    }

    fclose(fp);
}

void release_memory(monster **monsterList, int monsterCount, region **regionList, int regionCount, trainer *trainerList, int trainerCount)
{
    int i;

    /*Freeing all  dynamically allocated stuff*/

    for (i = 0; i < monsterCount; i++)
    {
        free(monsterList[i]->name);
        free(monsterList[i]->element);
        free(monsterList[i]);
    }
    free(monsterList);

    for (i = 0; i < regionCount; i++)
    {
        free(regionList[i]->name);
        free(regionList[i]->monsters);
        free(regionList[i]);
    }
    free(regionList);

    for (i = 0; i < trainerCount; i++)
    {
        free(trainerList[i].name);
        free(trainerList[i].visits->regions);
        free(trainerList[i].visits);
    }
    free(trainerList);
}
