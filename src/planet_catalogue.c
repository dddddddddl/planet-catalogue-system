#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "portable.h"

#define MAX_PLANETS 100

struct  planet  //details of the planet
{
    char name[50];//name of the planet
    float diameters;//diameters of the planet
    char mass[20];//mass of the planet
    float period;//rotation period
    float density;//the density of the planet
    float age;//age of the planet
    float distance;//distance between fixed star
    char timestr[20];//modified time
    int isDiscovered;     // Whether found, 1 is found, 0 is not found
    int isObserved;       // Whether it has been observed, 1 is observed, 0 is not observed
    char addedBy[50];     // Record the account of the researcher who added this planet
    struct planet* next;
};

struct account
{
    char acc[20];//account
    char password[20];//passwords
    int isAdmin;
    struct account* next;
};

struct history
{
    char time[20];//edit time
    char editor[50];//editor name
    char content[50];//added content
    struct history* next;
};

struct System {
    struct planet planets[MAX_PLANETS];  // planetary array
    int planetCount;                    // Add planet count
    int isdiscoveredcount;  //Number of discovered planets
    int isobservedcount;  //count of observed planets
    int researcherCount;  // Number of researchers registered in the system
};

// Global variables
struct planet* head = NULL;
struct account* accountHead = NULL;

// ... Add a function declaration at the beginning of the file ...
void mainmenu(void);
void loadPlanet(void);
void adminmenu(void);
void researchermenu(void);
void land(void);
void createacc(void);
void modifyAccount(void);
void increasepl(void);
void displayPlanets(void);
void freePlanetList(void);
void loadAccounts(void);
void freeAccountList(void);
void updateAccountFile(void);
void viewStatistics(void);
void findcontribution(void);
void modifyPlanet(void);
void deletePlanet(void);
void searchPlanet(void);

/* Shared record parsing and canonical headers keep edits/deletions consistent. */
static int parsePlanetLine(const char *line, struct planet *planet) {
    return sscanf(line,
        "%49[^\t]\t%f\t%19[^\t]\t%f\t%f\t%f\t%f\t%19[^\t]\t%d\t%d\t%49s",
        planet->name, &planet->diameters, planet->mass, &planet->period,
        &planet->density, &planet->age, &planet->distance, planet->timestr,
        &planet->isDiscovered, &planet->isObserved, planet->addedBy) == 11;
}

static char **readCatalogueLines(FILE *stream, int *count) {
    size_t capacity = 16;
    char **lines = malloc(capacity * sizeof(*lines));
    char line[512];
    *count = 0;
    if (lines == NULL) return NULL;
    while (fgets(line, sizeof(line), stream)) {
        if ((size_t)*count == capacity) {
            capacity *= 2;
            char **larger = realloc(lines, capacity * sizeof(*lines));
            if (larger == NULL) goto failure;
            lines = larger;
        }
        lines[*count] = malloc(strlen(line) + 1);
        if (lines[*count] == NULL) goto failure;
        strcpy(lines[*count], line);
        ++*count;
    }
    if (ferror(stream)) goto failure;
    return lines;
failure:
    for (int i = 0; i < *count; ++i) free(lines[i]);
    free(lines);
    return NULL;
}

static int writeCatalogueLines(char **lines, int count, int skip) {
    struct planet planet;
    int total = 0, discovered = 0, observed = 0;
    for (int i = 0; i < count; ++i) {
        if (i != skip && parsePlanetLine(lines[i], &planet)) {
            ++total;
            discovered += planet.isDiscovered != 0;
            observed += planet.isObserved != 0;
        }
    }
    FILE *stream = fopen("planetcat.txt", "w");
    if (stream == NULL) {
        printf("Failed to open catalogue for writing.\n");
        return 0;
    }
    fprintf(stream, "TOTAL COUNT: %d\nDiscovered Planets: %d\nObserved Planets: %d\n",
            total, discovered, observed);
    for (int i = 0; i < count; ++i) {
        if (i != skip && parsePlanetLine(lines[i], &planet)) {
            fputs(lines[i], stream);
            size_t length = strlen(lines[i]);
            if (length > 0 && lines[i][length - 1] != '\n') fputc('\n', stream);
        }
    }
    int failed = ferror(stream);
    if (fclose(stream) != 0) failed = 1;
    if (failed) printf("Catalogue write failed.\n");
    return !failed;
}

static int normalizeCatalogue(void) {
    FILE *stream = fopen("planetcat.txt", "r");
    if (stream == NULL) return 0;
    int count;
    char **lines = readCatalogueLines(stream, &count);
    fclose(stream);
    if (lines == NULL) return 0;
    int result = writeCatalogueLines(lines, count, -1);
    for (int i = 0; i < count; ++i) free(lines[i]);
    free(lines);
    return result;
}

static int initializeFiles(void) {
    const char *names[] = {"land.txt", "Researchers.txt", "planetcat.txt"};
    const char *initial[] = {"", "REGISTERED RESEARCHERS: 0\n",
        "TOTAL COUNT: 0\nDiscovered Planets: 0\nObserved Planets: 0\n"};
    for (size_t i = 0; i < sizeof(names) / sizeof(names[0]); ++i) {
        FILE *stream = fopen(names[i], "r");
        if (stream != NULL) {
            fclose(stream);
            continue;
        }
        if (errno != ENOENT) return 0;
        stream = fopen(names[i], "w");
        if (stream == NULL) {
            printf("Cannot create %s in the working directory.\n", names[i]);
            return 0;
        }
        if (fputs(initial[i], stream) == EOF) {
            fclose(stream);
            return 0;
        }
        if (fclose(stream) != 0) return 0;
    }
    return 1;
}

void mainmenu(void) //main menu
{
    int choose;
    printf("\n\n\n");
    printf("       --PLANET CATALOGUE SYSTEM--       \n");
    printf("********************************************\n");
    printf("*       1-------LOGIN                      *\n");
    printf("*       2-------CREATE NEW ACCOUNT         *\n");
    printf("*       3-------VIEW CONTRIBUTIONS         *\n");
    printf("*       4-------QUIT                       *\n");
    printf("********************************************\n");
    readInput("%d", &choose);
    switch (choose) {
    case 1:
        land(); //login function
        break;
    case 2:
        createacc(); //Register a new account function
        break;
    case 3:
        findcontribution();
        break;
    case 4:
        exit(0); //quit
    }
    return;
}


void adminmenu(void) //admin menu
{
    int choose;
    printf("\n\n\n");
    printf("       --PLANET CATALOGUE SYSTEM--       \n");
    printf("********************************************\n");
    printf("*     1-------ADD NEW PLANETS DATA         *\n");
    printf("*     2-------MODIFY PLANETS DATA          *\n");
    printf("*     3-------VIEW DATA STATISTICS         *\n");
    printf("*     4-------DELETE PLANETS               *\n");
    printf("*     5-------MODIFY RESEARCHER ACCOUNTS  *\n");
    printf("*     6-------RETURN                       *\n");
    printf("********************************************\n");
    readInput("%d", &choose);
    switch (choose) {
    case 1:
        increasepl();
        break;
    case 2:
        modifyPlanet();  // correct
        break;
    case 3:
        viewStatistics();//data statistics
        break;
    case 4:
        deletePlanet();
        break;
    case 5:
        modifyAccount();//Modify account
        break;
    case 6:
        return;
        break;
    }
}

void researchermenu(void) //researcher menu
{
    int choose;
    printf("\n\n\n");
    printf("       --PLANET CATALOGUE SYSTEM--       \n");
    printf("********************************************\n");
    printf("*     1-------SEARCH FOR PLANET            *\n");
    printf("*     2-------VIEW DATA STATISTICS         *\n");
    printf("*     3-------MODIFY RESEARCHER ACCOUNTS  *\n");
    printf("*     4-------RETURN                       *\n");
    printf("********************************************\n");
    readInput("%d", &choose);
    switch (choose) {
    case 1:
        searchPlanet();
        break;
    case 2:
        viewStatistics();//View statistics
        break;
    case 3:
        modifyAccount();//Modify account
        break;
    case 4:
        return;
        break;
    }
}

void increasepl(void) /* Planetary data entry*/
{
    FILE* fp;
    struct System state = {0};
    struct System* sys = &state;
    memset(sys, 0, sizeof(struct System));  // Initialize memory
    struct planet* newPlanet = (struct planet*)malloc(sizeof(struct planet));
    time_t t = time(NULL);
    struct tm timeinfo;
    catalogueLocalTime(&timeinfo, &t);  // correct

    if (newPlanet == NULL) {
        printf("MEMORY ALLOCATE FAIL  \n");
        return;
    }
    memset(newPlanet, 0, sizeof(struct planet));  // Initialize memory

    // Open the file in "a +" mode to append data
    if (catalogueOpen(&fp, "planetcat.txt", "a+") != 0) {
        printf("Unable to open file for writing!\n");
        free(newPlanet);
        newPlanet = NULL;
        return;
    }

    char existingLine[512];
    struct planet existingPlanet;
    rewind(fp);
    while (fgets(existingLine, sizeof(existingLine), fp)) {
        if (parsePlanetLine(existingLine, &existingPlanet)) {
            sys->planetCount++;
            if (existingPlanet.isDiscovered) sys->isdiscoveredcount++;
            if (existingPlanet.isObserved) sys->isobservedcount++;
        }
    }
    if (sys->planetCount >= MAX_PLANETS) {
        printf("PLANET ARRAY IS FULL, UNABLE TO ADD MORE PLANETS.\n");
        fclose(fp);
        free(newPlanet);
        return;
    }
    fseek(fp, 0, SEEK_END);

    // Clear input buffer
    discardInputLine();

    // Get the researcher account and record the additer.
    printf("Please enter your account name: ");
    char researcherAcc[50];
    readInputLine(researcherAcc, sizeof(researcherAcc));
    researcherAcc[strcspn(researcherAcc, "\n")] = 0;
    int err = catalogueCopy(newPlanet->addedBy, sizeof(newPlanet->addedBy), researcherAcc);
    if (err != 0) {
        printf("An error occurred while copying the account name  \n");
        fclose(fp);
        free(newPlanet);
        newPlanet = NULL;
        return;
    }

    printf("Please enter a planet name: ");
    readInputLine(newPlanet->name, sizeof(newPlanet->name));
    newPlanet->name[strcspn(newPlanet->name, "\n")] = 0;

    printf("Please enter the planet diameter(AU): ");
    readInput("%f", &newPlanet->diameters);
    discardInputLine();

    printf("Please enter the mass of the planet(kg, Scientific notation): ");
    readInputLine(newPlanet->mass, sizeof(newPlanet->mass));
    newPlanet->mass[strcspn(newPlanet->mass, "\n")] = 0;

    printf("Please enter the rotation period (hours): ");
    readInput("%f", &newPlanet->period);
    discardInputLine();

    printf("Please enter the density (kg/m^3): ");
    readInput("%f", &newPlanet->density);
    discardInputLine();

    printf("Please enter the planet age (years): ");
    readInput("%f", &newPlanet->age);
    discardInputLine();

    printf("Please enter the distance from the star (AU): ");
    readInput("%f", &newPlanet->distance);
    discardInputLine();

    // Add new input
    printf("Has the planet been discovered? (1-Yes, 0-No): ");
    readInput("%d", &newPlanet->isDiscovered);
    discardInputLine();

    printf("Has the planet been observed? (1-Yes, 0-No): ");
    readInput("%d", &newPlanet->isObserved);
    discardInputLine();

    // Get and format the current time
    strftime(newPlanet->timestr, sizeof(newPlanet->timestr), "%Y-%m-%d %H:%M:%S", &timeinfo);

    // Update the count of discovered and observed planets and write the count to a file//
    if (newPlanet->isDiscovered) {
        sys->isdiscoveredcount++;
        fprintf(fp, "Discovered Planets: %d\n", sys->isdiscoveredcount); //Write into file//
    }

    if (newPlanet->isObserved) {
        sys->isobservedcount++;
        fprintf(fp, "Observed Planets: %d\n", sys->isobservedcount); //Write into file//
    }

    // Modify the file write format and add new fields
    fprintf(fp, "%s\t%.2f\t%s\t%.2f\t%.2f\t%.2f\t%.2f\t%s\t%d\t%d\t%s\n",
        newPlanet->name,
        newPlanet->diameters,
        newPlanet->mass,
        newPlanet->period,
        newPlanet->density,
        newPlanet->age,
        newPlanet->distance,
        newPlanet->timestr,
        newPlanet->isDiscovered,
        newPlanet->isObserved,
        newPlanet->addedBy);

    // Insert a new planet into the planet array//
    if (sys->planetCount < MAX_PLANETS) {
        sys->planets[sys->planetCount] = *newPlanet;
        sys->planetCount++;
    }
    else {
        printf("PLANET ARRAY IS FULL, UNABLE TO ADD MORE PLANETS.\n");
    }

    fprintf(fp, "TOTAL COUNT: %d\n", sys->planetCount);

    // Insert a new node into the head of the linked list
    newPlanet->next = head;
    head = newPlanet;

    fclose(fp);
    if (normalizeCatalogue()) {
        printf("\nData saved successfully!\n");
    }
}

// Add a function to display all planet data
void displayPlanets(void)
{
    struct planet* current = head;
    int count = 0;

    printf("\nCurrent planetary data in memory  \n");
    printf("serial number\tname\tdiameters(AU)\tmass\tperiod(h)\tdensity(kg/m^3)\tage\tdistance(AU)\ttimestr\tisDiscovered\tisObserved\n");
    printf("-------------------------------------------------------------------------\n");

    while (current != NULL) {
        printf("%d\t%s\t%.2f\t%s\t%.2f\t%.2f\t%.2f\t%.2f\t%s\t%d\t%d\n",
            ++count,
            current->name,
            current->diameters,
            current->mass,
            current->period,
            current->density,
            current->age,
            current->distance,
            current->timestr,
            current->isDiscovered,
            current->isObserved);
        current = current->next;
    }

    if (count == 0) {
        printf("No planet data yet!\n");
    }
}

// Add a function to free the linked list memory
void freePlanetList(void) {
    struct planet* current = head;
    while (current != NULL) {
        struct planet* temp = current;
        current = current->next;
        free(temp);
    }
    head = NULL;
}


// Load account data from file to linked list
void loadAccounts(void) {
    FILE* fp;
    if (catalogueOpen(&fp, "land.txt", "r") != 0) {
        return;
    }

    char acc[20], password[20];
    int isAdmin;

    while (fscanf(fp, "%19s %19s %d", acc, password, &isAdmin) == 3) {
        struct account* newAccount = (struct account*)malloc(sizeof(struct account));
        if (newAccount == NULL) continue;

        catalogueCopy(newAccount->acc, sizeof(newAccount->acc), acc);
        catalogueCopy(newAccount->password, sizeof(newAccount->password), password);
        newAccount->isAdmin = isAdmin;

        // Insert linked list header
        newAccount->next = accountHead;
        accountHead = newAccount;
    }
    fclose(fp);
}

// Match account and password, return account type
int match(const char* a, const char* p) {
    struct account* current = accountHead;

    while (current != NULL) {
        if (strcmp(a, current->acc) == 0) {
            if (strcmp(p, current->password) == 0) {
                if (current->isAdmin) {
                    return 2;
                }
                else {
                    return 1;
                } // 2 means administrator, 1 means regular user
            }
            return -1; // Wrong password
        }
        current = current->next;
    }
    return 0; // Account does not exist
}

void land(void) {
    char acc[20] = { '\0' };
    char password[20] = { '\0' };

    // If the linked list is empty, load the data first
    if (accountHead == NULL) {
        loadAccounts();
    }

    printf("       --PLANET CATALOGUE SYSTEM--       \n");
    printf("********************************************\n");
    printf("ENTER YOUR ACCOUNT BELOW: \n");
    readInput("%19s", acc);
    printf("ENTER YOUR PASSWORD: \n");
    readInput("%19s", password);

    int result = match(acc, password);
    switch (result) {
    case 2:
        printf("ADMIN LOGIN\n");
        adminmenu();
        break;
    case 1:
        printf("RESEARCHER LOGIN\n");
        researchermenu();
        break;
    case -1:
        printf("PASSWORD WRONG!!\n");
        break;
    case 0:
        printf("ACCOUNT DOES NOT EXIST!!\n");
        break;
    }
}

// Free linked list memory
void freeAccountList(void) {
    struct account* current = accountHead;
    while (current != NULL) {
        struct account* temp = current;
        current = current->next;
        free(temp);
    }
    accountHead = NULL;
}

void createacc(void) //Register an account
{
    freeAccountList();
    loadAccounts();
    struct System state = {0};
    struct System* sys = &state;
    struct account* newAccount = (struct account*)malloc(sizeof(struct account));
    if (newAccount == NULL) {
        printf("MEMORY ALLOCATE FAIL!\n");
        return;
    }
    FILE* fp1;
    if (catalogueOpen(&fp1, "Researchers.txt", "a+") != 0) {
        printf("FILE OPEN FAIL!\n");
        free(newAccount);
        return;
    }
    memset(sys, 0, sizeof(struct System));
    FILE* fp;
    if (catalogueOpen(&fp, "land.txt", "a+") != 0) {
        printf("FILE OPEN FAIL!\n");
        free(newAccount);
        fclose(fp1);
        return;
    }
    memset(newAccount, 0, sizeof(struct account));
    fseek(fp1, 0, SEEK_END);
    // Use ftell to check the file size
    long file_size = ftell(fp1);


    if (file_size == 0) {
        sys->researcherCount = 0;
    }
    else {
        // The file is not empty, reposition the file pointer to the beginning
        fseek(fp1, 0, SEEK_SET);
        fscanf(fp1, "REGISTERED RESEARCHERS: %d", &sys->researcherCount);
    }


    printf("PLEASE ENTER YOUR ACCOUNT NAME: ");
    readInput("%19s", newAccount->acc);

    // Check if the username already exists
    struct account* current = accountHead;
    while (current != NULL) {
        if (strcmp(current->acc, newAccount->acc) == 0) {
            printf("NAME USED.CHOOSE ANOTHER NAME\n");
            free(newAccount);
            fclose(fp);
            fclose(fp1);
            return;
        }
        current = current->next;
    }

    printf("PLEASE ENTER YOUR PASSWORD: ");
    readInput("%19s", newAccount->password);

    printf("PLEASE ENTER YOUR PASSWORD AGAIN: ");
    char confirm[20];
    readInput("%19s", confirm);
    if (strcmp(newAccount->password, confirm) != 0) {
        printf("PASSWORD ARE NOT SAME.\n");
        free(newAccount);
        fclose(fp);
        fclose(fp1);
        return;
    }

    // Select account type
    int choice;
    do {
        printf("\nCHOOSE YOUR ACCOUNT TYPE  \n");
        printf("1.ADMIN\n");
        printf("2.RESEARCHERS\n");
        printf("ENTER 1 OR 2:");
        readInput("%d", &choice);

        // Clear input buffer
        discardInputLine();

        switch (choice) {
        case 1:
            newAccount->isAdmin = 1;  // administrator
            printf("ADMIN CHOSEN\n");
            break;
        case 2:
            newAccount->isAdmin = 0;  // researcher
            printf("RESEARCHER CHOSEN\n");
            sys->researcherCount++;  // Increase researcher count
            break;
        default:
            printf("INVALID CHOICE\n");
            continue;
        }
    } while (choice != 1 && choice != 2);

    fclose(fp1);
    if (catalogueOpen(&fp1, "Researchers.txt", "w") != 0) {
        printf("FILE OPEN FAIL!\n");
        free(newAccount);
        fclose(fp);
        return;
    }

    // Write file
    fprintf(fp, "%s %s %d\n", newAccount->acc, newAccount->password, newAccount->isAdmin);
    // Write updated researcherCount using fprintf
    fprintf(fp1, "REGISTERED RESEARCHERS: %d", sys->researcherCount);
    fclose(fp);
    fclose(fp1);

    // Publish the node only after the files have been opened successfully.
    newAccount->next = accountHead;
    accountHead = newAccount;

    printf("ACCOUNT CREATED SUCCESSFULLY!!\n");
    return;  // The main loop handles the next menu visit.
}
void modifyAccount(void) {  // Editing personal information
    char acc[20];
    printf("PLEASE ENTER THE ACCOUNT YOU WANT TO MODIFY: ");
    readInput("%19s", acc);

    struct account* current = accountHead;
    while (current != NULL) {
        if (strcmp(current->acc, acc) == 0) {
            // Find the account and start changing the password.
            printf("PLEASE ENTER THE NEW PASSWORD: ");
            readInput("%19s", current->password);
            current->password[strcspn(current->password, "\n")] = '\0';  // Remove newline characters
            updateAccountFile();  // Update account info to file
            printf("SUCCESSFULLY UPDATED!\n");
            return;  // After successful modification, exit the function
        }
        current = current->next;  // Continue to check the next account.
    }
    // If the corresponding account has not been found at the end of the loop, it means that the input is invalid
    printf("INVALID ACCOUNT!\n");
}

void updateAccountFile(void) { //Update account information in the file
    FILE* fp;
    if (catalogueOpen(&fp, "land.txt", "w") != 0) {
        printf("CANNOT OPEN FILE  \n");
        return;
    }

    struct account* current = accountHead;
    while (current != NULL) {
        fprintf(fp, "%s %s %d\n",
            current->acc, current->password, current->isAdmin);
        current = current->next;
    }
    fclose(fp);
}



void viewStatistics(void) {
    FILE* fp;
    FILE* fp1;
    struct System* sys = (struct System*)malloc(sizeof(struct System));
    if (sys == NULL) {
        printf("Memory allocation failed!\n");
        return;
    }

    memset(sys, 0, sizeof(struct System));

    if (catalogueOpen(&fp, "planetcat.txt", "r") != 0) {
        printf("1 Unable to open file for reading!\n");
        free(sys);
        return;
    }

    if (catalogueOpen(&fp1, "Researchers.txt", "r") != 0) {
        printf("2 Unable to open file for reading  \n");
        fclose(fp);
        free(sys);
        return;
    }

    char line[512];
    int totalPlanets = 0;
    sys->isdiscoveredcount = 0;
    sys->isobservedcount = 0;

    // Read the file and parse each line
    while (fgets(line, sizeof(line), fp)) {
        // Skip the statistics line
        if (strstr(line, "TOTAL COUNT:") ||
            strstr(line, "Discovered Planets:") ||
            strstr(line, "Observed Planets:")) {
            continue;
        }

        // Trying to parse planetary data
        char name[50], mass[20], timestr[20], addedBy[50];
        float diameters, period, density, age, distance;
        int isDiscovered, isObserved;

        int parsed = sscanf(line, "%49[^\t]\t%f\t%19[^\t]\t%f\t%f\t%f\t%f\t%19[^\t]\t%d\t%d\t%49s",
            name,
            &diameters,
            mass,
            &period,
            &density,
            &age,
            &distance,
            timestr,
            &isDiscovered,
            &isObserved,
            addedBy);

        if (parsed == 11) {  // Make sure all fields are parsed correctly
            totalPlanets++;
            if (isDiscovered) sys->isdiscoveredcount++;
            if (isObserved) sys->isobservedcount++;
        }
    }

    // number of researchers read
    rewind(fp1);  // Make sure to read from the beginning of the file
    while (fgets(line, sizeof(line), fp1)) {
        if (sscanf(line, "REGISTERED RESEARCHERS: %d", &sys->researcherCount) == 1) {
            break;
        }
    }

    printf("\nSYSTEM STATISTICS:\n");
    printf("TOTAL NUMBER OF CATALOGUED PLANETS: %d\n", totalPlanets);
    printf("TOTAL NUMBER OF DISCOVERED PLANETS: %d\n", sys->isdiscoveredcount);
    printf("TOTAL NUMBER OF OBSERVED PLANETS: %d\n", sys->isobservedcount);
    printf("TOTAL NUMBER OF REGISTERED RESEARCHERS: %d\n", sys->researcherCount);

    fclose(fp);
    fclose(fp1);
    free(sys);
}

void loadPlanet(void) {
    FILE* fp;
    if (catalogueOpen(&fp, "planetcat.txt", "r") != 0) {
        printf("Unable to open file for reading!\n");
        return;
    }

    char line[512];
    struct planet tempPlanet;
    int isDiscovered, isObserved;

    // Read the file and parse each line
    while (fgets(line, sizeof(line), fp)) {
        // Skip the statistics lines
        if (strstr(line, "TOTAL COUNT:") ||
            strstr(line, "Discovered Planets:") ||
            strstr(line, "Observed Planets:")) {
            continue;
        }

        // Parse planetary data
        if (sscanf(line, "%49[^\t]\t%f\t%19[^\t]\t%f\t%f\t%f\t%f\t%19[^\t]\t%d\t%d\t%49s",
            tempPlanet.name,
            &tempPlanet.diameters,
            tempPlanet.mass,
            &tempPlanet.period,
            &tempPlanet.density,
            &tempPlanet.age,
            &tempPlanet.distance,
            tempPlanet.timestr,
            &isDiscovered,
            &isObserved,
            tempPlanet.addedBy) == 11) {

            tempPlanet.isDiscovered = isDiscovered;
            tempPlanet.isObserved = isObserved;

            // Create a new planet node and copy the data
            struct planet* newPlanet = (struct planet*)malloc(sizeof(struct planet));
            if (newPlanet == NULL) {
                printf("Memory allocation failed!\n");
                continue;
            }
            *newPlanet = tempPlanet;

            // Insert the new planet into the linked list
            newPlanet->next = head;
            head = newPlanet;
        }
    }
    fclose(fp);
}
// View individual contributions
void findcontribution(void) {
    FILE* fp;
    struct planet* current = head;
    int contributions = 0;
    int found = 0;
    char inputAccount[50];
    if (catalogueOpen(&fp, "planetcat.txt", "r") != 0) {
        printf("Unable to open file for reading  \n");
        return;
    }
    discardInputLine();  // Clear any leftover characters from previous inputs

    printf("Please enter your account number: ");
    readInputLine(inputAccount, sizeof(inputAccount));
    inputAccount[strcspn(inputAccount, "\n")] = 0;  // Remove line breaks
    freePlanetList();
    loadPlanet();
    current = head;
    while (current != NULL) {
        if (strcmp(current->addedBy, inputAccount) == 0) {//Find the corresponding account and output the data of its contributing planet
            printf("Name: %s\n", current->name);
            printf("Last modified: %s\n", current->timestr);
            contributions++;
            found = 1;
        }
        current = current->next;  // Continue to find the next node
    }
    if (found) {
        printf("\nPlanets added by this account: %d.\n", contributions);
    }
    else {
        printf("INVALID ACCOUNT OR NO CONTRIBUTIONS");
    }
    fclose(fp);
}


void modifyPlanet(void) {
    // Open file for reading
    FILE* fp;
    if (catalogueOpen(&fp, "planetcat.txt", "r") != 0) {
        printf("Failed to open file!\n");
        return;
    }

    char planetName[50];
    discardInputLine();
    printf("Enter the name of the planet you want to modify: ");
    readInputLine(planetName, sizeof(planetName));
    planetName[strcspn(planetName, "\n")] = 0;

    int lineCount = 0;
    char** lines = readCatalogueLines(fp, &lineCount);
    fclose(fp);
    if (lines == NULL) {
        printf("Unable to read the catalogue into memory.\n");
        return;
    }

    // Initialize planet structure and counters
    int found = 0;
    struct planet* tempPlanet = (struct planet*)malloc(sizeof(struct planet));
    if (tempPlanet == NULL) {
        printf("Memory allocation failed!\n");
        for (int i = 0; i < lineCount; i++) {
            free(lines[i]);
        }
        free(lines);
        return;
    }
    // Process file content
    for (int i = 0; i < lineCount; i++) {
        if (strstr(lines[i], "TOTAL COUNT:")) {
            continue;
        }
        if (strstr(lines[i], "Discovered Planets:")) {
            continue;
        }
        if (strstr(lines[i], "Observed Planets:")) {
            continue;
        }

        // Parse planet data
        if (sscanf(lines[i], "%49[^\t]\t%f\t%19[^\t]\t%f\t%f\t%f\t%f\t%19[^\t]\t%d\t%d\t%49s",
            tempPlanet->name,
            &tempPlanet->diameters,
            tempPlanet->mass,
            &tempPlanet->period,
            &tempPlanet->density,
            &tempPlanet->age,
            &tempPlanet->distance,
            tempPlanet->timestr,
            &tempPlanet->isDiscovered,
            &tempPlanet->isObserved,
            tempPlanet->addedBy) == 11) {

            if (strcmp(tempPlanet->name, planetName) == 0) {
                found = 1;
                // Display current planet details
                printf("\nCurrent planet details:\n");
                printf("1. Name: %s\n", tempPlanet->name);
                printf("2. Diameter: %.2f AU\n", tempPlanet->diameters);
                printf("3. Mass: %s\n", tempPlanet->mass);
                printf("4. Rotation Period: %.2f hours\n", tempPlanet->period);
                printf("5. Density: %.2f kg/m^3\n", tempPlanet->density);
                printf("6. Age: %.2f years\n", tempPlanet->age);
                printf("7. Distance from Star: %.2f AU\n", tempPlanet->distance);
                printf("8. Discovered status: %s\n", tempPlanet->isDiscovered ? "Yes" : "No");
                printf("9. Observed status: %s\n", tempPlanet->isObserved ? "Yes" : "No");

                int choice;
                printf("\nSelect the attribute to modify (1-9): ");
                readInput("%d", &choice);
                discardInputLine();

                // Handle user modification choice
                switch (choice) {
                case 1:
                    printf("Enter new name: ");
                    readInputLine(tempPlanet->name, sizeof(tempPlanet->name));
                    tempPlanet->name[strcspn(tempPlanet->name, "\n")] = 0;
                    break;
                case 2:
                    printf("Enter new diameter (AU): ");
                    readInput("%f", &tempPlanet->diameters);
                    discardInputLine();
                    break;
                case 3:
                    printf("Enter new mass: ");
                    readInputLine(tempPlanet->mass, sizeof(tempPlanet->mass));
                    tempPlanet->mass[strcspn(tempPlanet->mass, "\n")] = 0;
                    break;
                case 4:
                    printf("Enter new rotation period (hours): ");
                    readInput("%f", &tempPlanet->period);
                    discardInputLine();
                    break;
                case 5:
                    printf("Enter new density (kg/m^3): ");
                    readInput("%f", &tempPlanet->density);
                    discardInputLine();
                    break;
                case 6:
                    printf("Enter new age (years): ");
                    readInput("%f", &tempPlanet->age);
                    discardInputLine();
                    break;
                case 7:
                    printf("Enter new distance from star (AU): ");
                    readInput("%f", &tempPlanet->distance);
                    discardInputLine();
                    break;
                case 8:
                    printf("Enter new discovered status (1 for Yes, 0 for No): ");
                    readInput("%d", &tempPlanet->isDiscovered);
                    discardInputLine();
                    break;
                case 9:
                    printf("Enter new observed status (1 for Yes, 0 for No): ");
                    readInput("%d", &tempPlanet->isObserved);
                    discardInputLine();
                    break;
                default:
                    printf("Invalid choice!\n");
                    goto cleanup;
                }

                // Update timestamp
                time_t t = time(NULL);
                struct tm tm;
                catalogueLocalTime(&tm, &t);
                strftime(tempPlanet->timestr, sizeof(tempPlanet->timestr), "%Y-%m-%d %H:%M:%S", &tm);

                // Create updated line
                char newLine[512];
                snprintf(newLine, sizeof(newLine), "%s\t%.2f\t%s\t%.2f\t%.2f\t%.2f\t%.2f\t%s\t%d\t%d\t%s\n",
                    tempPlanet->name,
                    tempPlanet->diameters,
                    tempPlanet->mass,
                    tempPlanet->period,
                    tempPlanet->density,
                    tempPlanet->age,
                    tempPlanet->distance,
                    tempPlanet->timestr,
                    tempPlanet->isDiscovered,
                    tempPlanet->isObserved,
                    tempPlanet->addedBy);

                // Update file content
                char *replacement = malloc(strlen(newLine) + 1);
                if (replacement == NULL) {
                    printf("Memory allocation failed!\n");
                    goto cleanup;
                }
                free(lines[i]);
                lines[i] = replacement;
                strcpy(lines[i], newLine);

                // Headers are recomputed from records when saving.
                break;
            }
        }
    }

    // Handle results and cleanup
    if (!found) {
        printf("Planet not found!\n");
    }
    else {
        if (writeCatalogueLines(lines, lineCount, -1)) {
            printf("Planet modified successfully!\n");
            freePlanetList();
        }
    }

cleanup:
    // Clean up resources
    free(tempPlanet);
    for (int i = 0; i < lineCount; i++) {
        free(lines[i]);
    }
    free(lines);
}



void deletePlanet(void) {
    // Open file for reading
    FILE* fp;
    if (catalogueOpen(&fp, "planetcat.txt", "r+") != 0) {
        printf("Failed to open file!\n");
        return;
    }

    // Get planet name to delete
    char planetName[50];
    discardInputLine();  // Clear input buffer
    printf("Enter the name of the planet you want to delete: ");
    readInputLine(planetName, sizeof(planetName));
    planetName[strcspn(planetName, "\n")] = 0;

    int lineCount = 0;
    char** lines = readCatalogueLines(fp, &lineCount);
    fclose(fp);
    if (lines == NULL) {
        printf("Unable to read the catalogue into memory.\n");
        return;
    }

    // Initialize counters and planet structure
    struct planet* tempPlanet = (struct planet*)malloc(sizeof(struct planet));
    if (tempPlanet == NULL) {
        printf("Memory allocation failed!\n");
        for (int i = 0; i < lineCount; i++) {
            free(lines[i]);
        }
        free(lines);
        return;
    }
    int deleteIndex = -1;
    int found = 0;

    // First pass: count current statistics and find planet to delete
    for (int i = 0; i < lineCount; i++) {
        // Skip statistics lines
        if (strstr(lines[i], "TOTAL COUNT:") ||
            strstr(lines[i], "Discovered Planets:") ||
            strstr(lines[i], "Observed Planets:")) {
            continue;
        }

        // Parse planet data
        if (sscanf(lines[i], "%49[^\t]\t%f\t%19[^\t]\t%f\t%f\t%f\t%f\t%19[^\t]\t%d\t%d\t%49s",
            tempPlanet->name,
            &tempPlanet->diameters,
            tempPlanet->mass,
            &tempPlanet->period,
            &tempPlanet->density,
            &tempPlanet->age,
            &tempPlanet->distance,
            tempPlanet->timestr,
            &tempPlanet->isDiscovered,
            &tempPlanet->isObserved,
            tempPlanet->addedBy) == 11) {


            // Check if this is the planet to delete
            if (strcmp(tempPlanet->name, planetName) == 0) {
                found = 1;
                deleteIndex = i;
                printf("\nPlanet found:\n");
                printf("Name: %s\n", tempPlanet->name);
                printf("Diameter: %.2f AU\n", tempPlanet->diameters);
                printf("Mass: %s\n", tempPlanet->mass);
                printf("Rotation Period: %.2f hours\n", tempPlanet->period);
                printf("Density: %.2f kg/m^3\n", tempPlanet->density);
                printf("Age: %.2f years\n", tempPlanet->age);
                printf("Distance from Star: %.2f AU\n", tempPlanet->distance);
                printf("Discovered: %s\n", tempPlanet->isDiscovered ? "Yes" : "No");
                printf("Observed: %s\n", tempPlanet->isObserved ? "Yes" : "No");
                printf("Added by: %s\n", tempPlanet->addedBy);
            }
        }
    }

    if (!found) {
        printf("Planet not found!\n");
        // Clean up and return
        free(tempPlanet);
        for (int i = 0; i < lineCount; i++) {
            free(lines[i]);
        }
        free(lines);
        return;
    }

    // Confirm deletion
    printf("\nDo you want to delete this planet? (1 for Yes, 0 for No): ");
    int confirm;
    readInput("%d", &confirm);

    if (!confirm) {
        printf("Deletion cancelled.\n");
        // Clean up and return
        free(tempPlanet);
        for (int i = 0; i < lineCount; i++) {
            free(lines[i]);
        }
        free(lines);
        return;
    }

    if (writeCatalogueLines(lines, lineCount, deleteIndex)) {
        printf("Planet deleted successfully!\n");
        freePlanetList();
    }

    // Clean up all allocated memory
    free(tempPlanet);
    for (int i = 0; i < lineCount; i++) {
        free(lines[i]);
    }
    free(lines);
}


void searchPlanet(void) {
    // Open file for reading
    FILE* fp;
    if (catalogueOpen(&fp, "planetcat.txt", "r") != 0) {
        printf("Failed to open file!\n");
        return;
    }

    int choice;
    char inputName[50];
    float inputDiameter, inputPeriod, inputDensity, inputAge, inputDistance;
    float inputMassValue;
    int found = 0;

    // Display search menu
    printf("\nSelect search criteria:\n");
    printf("1. Name\n");
    printf("2. Diameter (AU)\n");
    printf("3. Mass\n");
    printf("4. Rotation Period (hours)\n");
    printf("5. Density (kg/m^3)\n");
    printf("6. Age (years)\n");
    printf("7. Distance from Star (AU)\n");
    printf("Enter your choice (1-7): ");
    readInput("%d", &choice);
    discardInputLine();

    switch (choice) {
    case 1:
        printf("Enter planet name: ");
        readInputLine(inputName, sizeof(inputName));
        inputName[strcspn(inputName, "\n")] = 0;
        break;
    case 2:
        printf("Enter planet diameter (AU): ");
        readInput("%f", &inputDiameter);
        discardInputLine();
        break;
    case 3:
        printf("Enter planet mass: ");
        readInput("%f", &inputMassValue);
        discardInputLine();
        break;
    case 4:
        printf("Enter rotation period (hours): ");
        readInput("%f", &inputPeriod);
        discardInputLine();
        break;
    case 5:
        printf("Enter density (kg/m^3): ");
        readInput("%f", &inputDensity);
        discardInputLine();
        break;
    case 6:
        printf("Enter age (years): ");
        readInput("%f", &inputAge);
        discardInputLine();
        break;
    case 7:
        printf("Enter distance from star (AU): ");
        readInput("%f", &inputDistance);
        discardInputLine();
        break;
    default:
        printf("Invalid choice!\n");
        fclose(fp);
        return;
    }

    char line[512];
    struct planet* tempPlanet = (struct planet*)malloc(sizeof(struct planet));
    if (tempPlanet == NULL) {
        printf("Memory allocation failed!\n");
        fclose(fp);
        return;
    }
    printf("\nSearch Results:\n");

    while (fgets(line, sizeof(line), fp) != NULL) {
        if (strstr(line, "TOTAL COUNT:") ||
            strstr(line, "Discovered Planets:") ||
            strstr(line, "Observed Planets:")) {
            continue;
        }

        if (sscanf(line, "%49[^\t]\t%f\t%19[^\t]\t%f\t%f\t%f\t%f\t%19[^\t]\t%d\t%d\t%49s",
            tempPlanet->name,
            &tempPlanet->diameters,
            tempPlanet->mass,
            &tempPlanet->period,
            &tempPlanet->density,
            &tempPlanet->age,
            &tempPlanet->distance,
            tempPlanet->timestr,
            &tempPlanet->isDiscovered,
            &tempPlanet->isObserved,
            tempPlanet->addedBy) == 11) {

            int match = 0;
            switch (choice) {
            case 1:
                if (strcmp(tempPlanet->name, inputName) == 0) {
                    match = 1;
                }
                break;
            case 2:
                if (tempPlanet->diameters == inputDiameter) {
                    match = 1;
                }
                break;
            case 3:
            {
                float planetMassValue;
                if (sscanf(tempPlanet->mass, "%f", &planetMassValue) == 1) {
                    if (planetMassValue == inputMassValue) {
                        match = 1;
                    }
                }
            }
            break;
            case 4:
                if (tempPlanet->period == inputPeriod) {
                    match = 1;
                }
                break;
            case 5:
                if (tempPlanet->density == inputDensity) {
                    match = 1;
                }
                break;
            case 6:
                if (tempPlanet->age == inputAge) {
                    match = 1;
                }
                break;
            case 7:
                if (tempPlanet->distance == inputDistance) {
                    match = 1;
                }
                break;
            }

            if (match) {
                found = 1;
                printf("\nPlanet Details:\n");
                printf("Name: %s\n", tempPlanet->name);
                printf("Diameter: %.2f AU\n", tempPlanet->diameters);
                printf("Mass: %s\n", tempPlanet->mass);
                printf("Rotation Period: %.2f hours\n", tempPlanet->period);
                printf("Density: %.2f kg/m^3\n", tempPlanet->density);
                printf("Age: %.2f years\n", tempPlanet->age);
                printf("Distance from Star: %.2f AU\n", tempPlanet->distance);
                printf("Last Modified: %s\n", tempPlanet->timestr);
                printf("Discovered: %s\n", tempPlanet->isDiscovered ? "Yes" : "No");
                printf("Observed: %s\n", tempPlanet->isObserved ? "Yes" : "No");
                printf("Added by: %s\n", tempPlanet->addedBy);
                printf("----------------------------------------\n");
            }
        }
    }

    if (!found) {
        printf("No planets found matching the search criteria.\n");
    }

    free(tempPlanet);
    fclose(fp);
}


int main(void) {
    // Initialize global variables
    head = NULL;
    accountHead = NULL;

    // Create and initialize the system structure
    struct System* sys = (struct System*)malloc(sizeof(struct System));
    if (sys == NULL) {
        printf("System initialization failed  \n");
        return -1;
    }

    // Initialize system data
    sys->planetCount = 0;
    sys->isdiscoveredcount = 0;
    sys->isobservedcount = 0;
    sys->researcherCount = 0;

    if (!initializeFiles()) {
        free(sys);
        return EXIT_FAILURE;
    }

    // Load account data
    loadAccounts();

    // Show main menu
    while (1) {
        mainmenu();
        // Check whether to opt out
        char choice;
        printf("\nWhether to continue  (Y/N): ");
        readInput(" %c", &choice);
        if (choice == 'N' || choice == 'n') {
            break;
        }
    }

    // clean up resources
    freePlanetList();
    freeAccountList();
    free(sys);

    printf("\nThank you for using the planet catalog system  \n");
    return 0;
}
