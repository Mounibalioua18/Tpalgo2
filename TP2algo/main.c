#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#define MAX_N 2000000
#define MAX_SIZES 1000
int tab[MAX_N];

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

//TRI
int cmp(const void *a, const void *b) { return (*(int*)a - *(int*)b); }
void fill_random(int n) { for (int i = 0; i < n; i++) tab[i] = rand(); }
void fill_sorted(int n) { fill_random(n); qsort(tab, n, sizeof(int), cmp); }


void MaxEtMinA(int n, int *max, int *min, int *comp_count) {
    *max = tab[0]; *min = tab[0]; *comp_count = 0;
    for (int i = 1; i < n; i++) {
        (*comp_count)++;
        if (tab[i] > *max) *max = tab[i];
        (*comp_count)++;
        if (tab[i] < *min) *min = tab[i];
    }
}

void MaxEtMinB(int n, int *max, int *min, int *comp_count) {
    *comp_count = 0;
    int i = 0, big, small;
    if (n % 2 == 1) { big = small = tab[0]; i = 1; }
    else {
        (*comp_count)++;
        if (tab[0] > tab[1]) { big = tab[0]; small = tab[1]; }
        else { big = tab[1]; small = tab[0]; }
        i = 2;
    }
    for (; i < n; i += 2) {
        (*comp_count)++;
        if (tab[i] > tab[i+1]) {
            if (tab[i] > big) big = tab[i];
            if (tab[i+1] < small) small = tab[i+1];
        } else {
            if (tab[i+1] > big) big = tab[i+1];
            if (tab[i] < small) small = tab[i];
        }
        (*comp_count) += 2;
    }
    *max = big; *min = small;
}


int main() {
    srand(time(NULL));


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

    printf("Lu %d tailles depuis tp2algo.txt\n", num_sizes);

    // === FICHIER 1 : resultats.txt (tableau du TP, dynamique) ===
    FILE *f = fopen("resultats.txt", "w");

    // En-tête
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

    double results[6][MAX_SIZES];

    for (int s = 0; s < num_sizes; s++) {
        int n = sizes[s];
        printf("Testing n = %7d ... ", n); fflush(stdout);

        fill_random(n);
        results[0][s] = measure_time_linear(rechElets_TabNonTries, n, tab[0]);     // NonTri MC
        results[1][s] = measure_time_linear(rechElets_TabNonTries, n, -1);         // NonTri PC

        fill_sorted(n);
        results[2][s] = measure_time_linear(rechElets_TabTries, n, tab[0]);        // Tri MC
        results[3][s] = measure_time_linear(rechElets_TabTries, n, 2000000000);   // Tri PC
        results[4][s] = measure_time_dicho(rechElets_Dicho, n, tab[n/2]);         // Dicho MC
        results[5][s] = measure_time_dicho(rechElets_Dicho, n, -1);               // Dicho PC

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


    FILE *m = fopen("maxmin.txt", "w");
    fprintf(m, "n         Naif Comps  Opti Comps  Naif Time  Opti Time\n");

    int maxmin_count = (num_sizes < 5) ? num_sizes : 5;  // max 5
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

    printf("\n The files are genrated :\n");
    printf("resultats.txt \n");
    printf(" maxmin.txt \n");

    return 0;
}
