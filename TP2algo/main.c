#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#define MAX_N 2000000
#define MAX_SIZES 1000
int tab[MAX_N];

// === RECHERCHE ===
bool rechElets_TabNonTries(int n, int x) {
    for (int i = 0; i < n; i++) if (tab[i] == x) return true;
    return false;
}

bool rechElets_TabTries(int n, int x) {
    for (int i = 0; i < n; i++) {
        if (tab[i] == x) return true;
        if (tab[i] > x) return false;
    }
    return false;
}

bool rechElets_Dicho(int n, int x) {
    int low = 0, high = n - 1;
    while (low <= high) {
        int mid = low + (high - low) / 2;
        if (tab[mid] == x) return true;
        if (tab[mid] < x) low = mid + 1;
        else high = mid - 1;
    }
    return false;
}

// === MESURE TEMPS ===
double measure_time_linear(bool (*func)(int, int), int n, int x) {
    const int REPS = 1000;
    clock_t start = clock();
    for (int i = 0; i < REPS; i++) {
        volatile bool result = func(n, x);
        (void)result;
    }
    clock_t end = clock();
    return ((double)(end - start)) / CLOCKS_PER_SEC * 1000 / REPS;
}

double measure_time_dicho(bool (*func)(int, int), int n, int x) {
    const int REPS = 1000000;
    clock_t start = clock();
    for (int i = 0; i < REPS; i++) {
        volatile bool result = func(n, x);
        (void)result;
    }
    clock_t end = clock();
    return ((double)(end - start)) / CLOCKS_PER_SEC * 1000 / REPS;
}

// === TRI ===
int cmp(const void *a, const void *b) {
    int x = *(const int*)a;
    int y = *(const int*)b;
    return (x > y) - (x < y);  // safe
}
void fill_random(int n) { for (int i = 0; i < n; i++) tab[i] = rand(); }
void fill_sorted(int n) { fill_random(n); qsort(tab, n, sizeof(int), cmp); }

// === MAX & MIN NAIF ===
void MaxEtMinA(int n, int *max, int *min, int *comp_count) {
    *max = tab[0]; *min = tab[0]; *comp_count = 0;
    for (int i = 1; i < n; i++) {
        (*comp_count)++;
        if (tab[i] > *max) *max = tab[i];
        (*comp_count)++;
        if (tab[i] < *min) *min = tab[i];
    }
}

// === MAX & MIN OPTI (CORRIGÉ : DYNAMIQUE, PAS DE STACK OVERFLOW) ===
void MaxEtMinB(int n, int *max, int *min, int *comp_count) {
    *comp_count = 0;

    // Allocation dynamique
    int *grands = malloc((n/2 + 1) * sizeof(int));
    int *petits = malloc((n/2 + 1) * sizeof(int));
    if (!grands || !petits) {
        printf("Erreur allocation mémoire\n");
        exit(1);
    }

    int nb_grands = 0, nb_petits = 0;
    int i = 0;

    while (i < n - 1) {
        (*comp_count)++;
        if (tab[i] > tab[i + 1]) {
            grands[nb_grands++] = tab[i];
            petits[nb_petits++] = tab[i + 1];
        } else {
            grands[nb_grands++] = tab[i + 1];
            petits[nb_petits++] = tab[i];
        }
        i += 2;
    }
    if (i < n) {
        grands[nb_grands++] = tab[i];
    }

    // Max parmi grands
    *max = grands[0];
    for (int j = 1; j < nb_grands; j++) {
        (*comp_count)++;
        if (grands[j] > *max) *max = grands[j];
    }

    // Min parmi petits
    *min = petits[0];
    for (int j = 1; j < nb_petits; j++) {
        (*comp_count)++;
        if (petits[j] < *min) *min = petits[j];
    }

    free(grands);
    free(petits);
}

// === MAIN ===
int main() {
    srand(time(NULL));

    // Lire tp2algo.txt
    FILE *input = fopen("tp2algo.txt", "r");
    if (!input) {
        printf("Erreur : impossible d'ouvrir tp2algo.txt\n");
        return 1;
    }

    int *sizes = malloc(MAX_SIZES * sizeof(int));
    int num_sizes = 0;
    while (num_sizes < MAX_SIZES && fscanf(input, "%d", &sizes[num_sizes]) == 1) {
        num_sizes++;
    }
    fclose(input);

    if (num_sizes == 0) {
        printf("Erreur : tp2algo.txt est vide\n");
        free(sizes);
        return 1;
    }

    // === resultats.txt (CORRIGÉ : ALIGNEMENT PARFAIT) ===
    FILE *f = fopen("resultats.txt", "w");
    fprintf(f, "              n    ");
    for (int i = 0; i < num_sizes; i++) {
        fprintf(f, " %8d", sizes[i]);
    }
    fprintf(f, "\n");

    const char *labels[] = {
        "EletsNonTries  Meilleur cas", "               Pire cas    ",
        "EletsTries     Meilleur cas", "               Pire cas    ",
        "EletsTrisDicho Meilleur cas", "               Pire cas    "
    };

    double results[6][MAX_SIZES] = {0};

    for (int s = 0; s < num_sizes; s++) {
        int n = sizes[s];
        printf("Testing n = %7d ... ", n); fflush(stdout);

        fill_random(n);
        results[0][s] = measure_time_linear(rechElets_TabNonTries, n, tab[0]);
        results[1][s] = measure_time_linear(rechElets_TabNonTries, n, -1);

        fill_sorted(n);
        results[2][s] = measure_time_linear(rechElets_TabTries, n, tab[0]);
        results[3][s] = measure_time_linear(rechElets_TabTries, n, 2000000000);
        results[4][s] = measure_time_dicho(rechElets_Dicho, n, tab[n/2]);
        results[5][s] = measure_time_dicho(rechElets_Dicho, n, -1);

        printf("Done\n");
    }

    for (int i = 0; i < 6; i++) {
        fprintf(f, "%s ", labels[i]);
        for (int j = 0; j < num_sizes; j++) {
            fprintf(f, " %8.3f", results[i][j]);
        }
        fprintf(f, "\n");
    }
    fclose(f);

    // === maxmin.txt (CORRIGÉ : ESPACES) ===
    FILE *m = fopen("maxmin.txt", "w");
    fprintf(m, "n         Naif Comps  Opti Comps  Naif Time  Opti Time\n");

    int maxmin_count = (num_sizes < 5) ? num_sizes : 5;
    for (int s = 0; s < maxmin_count; s++) {
        int n = sizes[s];
        fill_random(n);

        int max1, min1, comp1 = 0;
        clock_t start = clock();
        MaxEtMinA(n, &max1, &min1, &comp1);
        double time1 = ((double)(clock() - start)) / CLOCKS_PER_SEC * 1000;

        int max2, min2, comp2 = 0;
        start = clock();
        MaxEtMinB(n, &max2, &min2, &comp2);
        double time2 = ((double)(clock() - start)) / CLOCKS_PER_SEC * 1000;

        fprintf(m, "%-10d %-11d %-11d %-10.3f %-10.3f\n", n, comp1, comp2, time1, time2);
    }
    fclose(m);

    free(sizes);

    printf("\nFICHIERS GENERES :\n");
    printf("  → resultats.txt  \n");
    printf("  → maxmin.txt     \n");

    return 0;
}
