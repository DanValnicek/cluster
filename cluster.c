/**
 * Kostra programu pro 2. projekt IZP 2022/23
 *
 * Jednoducha shlukova analyza: 2D nejblizsi soused.
 * Single linkage
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h> // sqrtf
#include <limits.h> // INT_MAX
#include <errno.h>
#include <string.h>
#include <ctype.h>

#define ERRNO_RETURN { \
if(errno)              \
return errno;          \
}

/**
 * If errno is non-zero print error message from parameter to stderr with errno code
 */
#define PRINT_ERRNO(errMes) {if(errno){fprintf(stderr,"%s: %s\n",errMes,strerror(errno)); \
}}
/**
 * If errno is non-zero print error message from parameter to stderr with errno code and returns 0
 */
#define RETURN_FALSE_IF_ERRNO(errMes) {if(errno){fprintf(stderr,"%s: %s\n",errMes,strerror(errno)); \
return 0; \
}}
/**
 * Evaluates condition and
 * Sets errno to EINVAL
 * Prints error massage from argument to stderr
 */
#define EINVAL_IF(condition, errMes) if(condition){ \
errno = EINVAL; \
fprintf(stderr,"%s: %s\n",errMes,strerror(errno)); \
return 1; \
}

/*****************************************************************
 *
 * Ladici makra. Vypnout jejich efekt lze definici makra
 * NDEBUG, napr.:
 *   a) pri prekladu argumentem prekladaci -DNDEBUG
 *   b) v souboru (na radek pred #include <assert.h>
 *      #define NDEBUG
 */
#ifdef NDEBUG
#define debug(s)
#define dfmt(s, ...)
#define dint(i)
#define dfloat(f)
#else

// vypise ladici retezec
#define debug(s) printf("- %s\n", s)

// vypise formatovany ladici vystup - pouziti podobne jako printf
#define dfmt(s, ...) printf(" - "__FILE__":%u: "s"\n",__LINE__,__VA_ARGS__)

// vypise ladici informaci o promenne - pouziti dint(identifikator_promenne)
#define dint(i) printf(" - " __FILE__ ":%u: " #i " = %d\n", __LINE__, i)

// vypise ladici informaci o promenne typu float - pouziti
// dfloat(identifikator_promenne)
#define dfloat(f) printf(" - " __FILE__ ":%u: " #f " = %g\n", __LINE__, f)

#endif

/*****************************************************************
 * Deklarace potrebnych datovych typu:
 *
 * TYTO DEKLARACE NEMENTE
 *
 *   struct obj_t - struktura objektu: identifikator a souradnice
 *   struct cluster_t - shluk objektu:
 *      pocet objektu ve shluku,
 *      kapacita shluku (pocet objektu, pro ktere je rezervovano
 *          misto v poli),
 *      ukazatel na pole shluku.
 */

struct obj_t {
    int id;
    float x;
    float y;
};

struct cluster_t {
    int size;
    int capacity;
    struct obj_t *obj;
};

/*****************************************************************
 * Deklarace potrebnych funkci.
 *
 * PROTOTYPY FUNKCI NEMENTE
 *
 * IMPLEMENTUJTE POUZE FUNKCE NA MISTECH OZNACENYCH 'TODO'
 *
 */

/*
 Inicializace shluku 'c'. Alokuje pamet pro cap objektu (kapacitu).
 Ukazatel NULL u pole objektu znamena kapacitu 0.
*/
void init_cluster(struct cluster_t *c, int cap)
{
    assert(c != NULL);
    assert(cap >= 0);

    c->size = 0;
    c->capacity = cap;
    c->obj = (struct obj_t *) malloc(sizeof(struct obj_t) * cap);
}

/*
 Odstraneni vsech objektu shluku a inicializace na prazdny shluk.
 */
void clear_cluster(struct cluster_t *c)
{
    free(c->obj);
    c->obj = NULL;
    c->size = c->capacity = 0;
}

/// Chunk of cluster objects. Value recommended for reallocation.
const int CLUSTER_CHUNK = 10;

/*
 Zmena kapacity shluku 'c' na kapacitu 'new_cap'.
 */
struct cluster_t *resize_cluster(struct cluster_t *c, int new_cap)
{
    // TUTO FUNKCI NEMENTE
    assert(c);
    assert(c->capacity >= 0);
    assert(new_cap >= 0);

    if (c->capacity >= new_cap)
        return c;

    size_t size = sizeof(struct obj_t) * new_cap;

    void *arr = realloc(c->obj, size);
    if (arr == NULL)
        return NULL;

    c->obj = (struct obj_t *) arr;
    c->capacity = new_cap;
    return c;
}

/*
 Prida objekt 'obj' na konec shluku 'c'. Rozsiri shluk, pokud se do nej objekt
 nevejde.
 */
void append_cluster(struct cluster_t *c, struct obj_t obj)
{
    c = resize_cluster(c, c->size);
    if (c == NULL) {
        fprintf(stderr, "%s\n", strerror(errno));
        return;
    }
    c->obj[c->size] = obj;
    c->size++;
}

/*
 Seradi objekty ve shluku 'c' vzestupne podle jejich identifikacniho cisla.
 */
void sort_cluster(struct cluster_t *c);

/*
 Do shluku 'c1' prida objekty 'c2'. Shluk 'c1' bude v pripade nutnosti rozsiren.
 Objekty ve shluku 'c1' budou serazeny vzestupne podle identifikacniho cisla.
 Shluk 'c2' bude nezmenen.
 */
void merge_clusters(struct cluster_t *c1, struct cluster_t *c2)
{
    assert(c1 != NULL);
    assert(c2 != NULL);

    c1 = resize_cluster(c1, c1->size + c2->size);
    if (c1 == NULL) {
        fprintf(stderr, "%s\n", strerror(errno));
        return;
    }
    for (int i = 0; i < c2->size; i++) {
        c1->obj[c1->size + i] = c2->obj[i];
    }
    c1->size = c1->size + c2->size;
    sort_cluster(c1);
}

/**********************************************************************/
/* Prace s polem shluku */

/*
 Odstrani shluk z pole shluku 'carr'. Pole shluku obsahuje 'narr' polozek
 (shluku). Shluk pro odstraneni se nachazi na indexu 'idx'. Funkce vraci novy
 pocet shluku v poli.
*/
int remove_cluster(struct cluster_t *carr, int narr, int idx)
{
    dint(idx);
    dint(narr);
    assert(idx < narr);
    assert(narr > 0);

    for (int i = idx; i < narr - 1; i++) {
        clear_cluster(&(carr[i]));
        init_cluster(&carr[i], 1);
        merge_clusters(&carr[i], &carr[i + 1]);
    }
    clear_cluster(&carr[narr - 1]);
    return narr - 1;
}

/*
 Pocita Euklidovskou vzdalenost mezi dvema objekty.
 */
float obj_distance(struct obj_t *o1, struct obj_t *o2)
{
    assert(o1 != NULL);
    assert(o2 != NULL);

    return sqrtf(powf((o1->x - o2->x), 2) + powf((o1->y - o2->y), 2));
}

/*
 Pocita vzdalenost dvou shluku.
*/
float cluster_distance(struct cluster_t *c1, struct cluster_t *c2)
{
    assert(c1 != NULL);
    assert(c1->size > 0);
    assert(c2 != NULL);
    assert(c2->size > 0);

    float minDistance = INFINITY;
    float currentDistance;

    for (int i_c1 = 0; i_c1 < c1->size; i_c1++) {
        for (int i_c2 = 0; i_c2 < c2->size; i_c2++) {
            currentDistance = obj_distance(&c1->obj[i_c1], &c2->obj[i_c2]);
            dfloat(currentDistance);
            if (currentDistance < minDistance) minDistance = currentDistance;
        }
    }
    return minDistance;
}

/*
 Funkce najde dva nejblizsi shluky. V poli shluku 'carr' o velikosti 'narr'
 hleda dva nejblizsi shluky. Nalezene shluky identifikuje jejich indexy v poli
 'carr'. Funkce nalezene shluky (indexy do pole 'carr') uklada do pameti na
 adresu 'c1' resp. 'c2'.
*/
void find_neighbours(struct cluster_t *carr, int narr, int *c1, int *c2)
{
    assert(narr > 0);

    float minClusterDistance = cluster_distance(&carr[0], &carr[1]);
    for (int i_c1 = 0; i_c1 < narr - 1; i_c1++) {
        for (int i_c2 = i_c1 + 1; i_c2 < narr; i_c2++) {
            dint(i_c1);
            dint(i_c2);
            float clusterDistance = cluster_distance(&carr[i_c1], &carr[i_c2]);
            if (minClusterDistance >= clusterDistance) {
                minClusterDistance = clusterDistance;
                *c1 = i_c1;
                *c2 = i_c2;
            }
        }
    }
}

// pomocna funkce pro razeni shluku
static int obj_sort_compar(const void *a, const void *b)
{
    // TUTO FUNKCI NEMENTE
    const struct obj_t *o1 = (const struct obj_t *) a;
    const struct obj_t *o2 = (const struct obj_t *) b;
    if (o1->id < o2->id) return -1;
    if (o1->id > o2->id) return 1;
    return 0;
}

/*
 Razeni objektu ve shluku vzestupne podle jejich identifikatoru.
*/
void sort_cluster(struct cluster_t *c)
{
    // TUTO FUNKCI NEMENTE
    qsort(c->obj, c->size, sizeof(struct obj_t), &obj_sort_compar);
}

/*
 Tisk shluku 'c' na stdout.
*/
void print_cluster(struct cluster_t *c)
{
    // TUTO FUNKCI NEMENTE
    for (int i = 0; i < c->size; i++) {
        if (i) putchar(' ');
        printf("%d[%g,%g]", c->obj[i].id, c->obj[i].x, c->obj[i].y);
    }
    putchar('\n');
}

/**
 * Checks whether object id is unique amongst clusters
 * @param carr cluster arrray
 * @param idx id of cluster to check
 * @return true when unique false otherwise
 */
int not_unique_ID(struct cluster_t *carr, int idx)
{
    for (int i = idx - 1; i >= 0; i--) {
        EINVAL_IF(carr[idx].obj->id == carr[i].obj->id, "Object ID is not unique!");
    }
    return 0;
}


/**
 * Important constants for cluster loading and error handling
 */
#define MAX_FIRST_LINE_LENGTH 20
//Number of objects to load from fscanf()
#define OBJECT_INPUT_COUNT 3
#define MAX_COORDINATE 1000
#define MIN_COORDINATE 0

/**
 * Loads object values from file into cluster and checks for invalid input
 * @param carr cluster array
 * @param idx id of cluster to insert
 * @param input_file file pointer to read from
 * @return 0 if load was successful 1 in case of error
 */
int load_object(struct cluster_t *carr, int idx, FILE *input_file)
{
    int matchedInputs = -1;
    init_cluster(&carr[idx], 1);
    matchedInputs = fscanf(input_file, "%d %f %f", &carr[idx].obj->id, &carr[idx].obj->x, &carr[idx].obj->y);
    carr[idx].size = 1;
    dfloat(carr[idx].obj->x);

    EINVAL_IF(isprint(fgetc(input_file)), "Too many characters in object definition!");
    EINVAL_IF(matchedInputs != OBJECT_INPUT_COUNT, "Invalid object format\\value!");

    EINVAL_IF((carr)[idx].obj->y < MIN_COORDINATE, "Y coordinate must be in interval [0,1000]!");
    EINVAL_IF((carr)[idx].obj->x < MIN_COORDINATE, "X coordinate must be in interval [0,1000]!");
    EINVAL_IF((carr)[idx].obj->y > MAX_COORDINATE, "Y coordinate must be in interval [0,1000]!");
    EINVAL_IF((carr)[idx].obj->x > MAX_COORDINATE, "X coordinate must be in interval [0,1000]!");

    EINVAL_IF(remainderf((carr)[idx].obj->y, 1) != 0, "Y coordinate must be an integer!");
    EINVAL_IF(remainderf((carr)[idx].obj->x, 1) != 0, "X coordinate must be an integer!");

    EINVAL_IF(errno, "Object parse error");

    return 0;
}

/**
 * Scans first line of file to for "count" of clusters to be read from file. Checks for invalid input and provides memory cleanup.
 * @param input_file file pointer
 * @return number of clusters to be read, NULL in case of error
 */
int get_cluster_count(FILE *input_file)
{
    int clusterCount = -1;
    char firstLine[MAX_FIRST_LINE_LENGTH];
    char charAfterMatch = 0;
    int parsedVars = -1;

    fgets(firstLine, MAX_FIRST_LINE_LENGTH, input_file);
    parsedVars = sscanf(firstLine, "count=%d%c", &clusterCount, &charAfterMatch);
    dint(clusterCount);
    if (isprint((unsigned char) charAfterMatch) || parsedVars != 2 || errno) {
        errno = EINVAL;
    }

    return clusterCount;
}

void clean_clusters(struct cluster_t **carr, int narr);

/*
 Ze souboru 'filename' nacte objekty. Pro kazdy objekt vytvori shluk a ulozi
 jej do pole shluku. Alokuje prostor pro pole vsech shluku a ukazatel na prvni
 polozku pole (ukalazatel na prvni shluk v alokovanem poli) ulozi do pameti,
 kam se odkazuje parametr 'arr'. Funkce vraci pocet nactenych objektu (shluku).
 V pripade nejake chyby uklada do pameti, kam se odkazuje 'arr', hodnotu NULL.
*/
int load_clusters(char *filename, struct cluster_t **arr)
{
    assert(arr != NULL);

    FILE *input_file = fopen(filename, "r");
    RETURN_FALSE_IF_ERRNO("Invalid file");

    int clusterCount = get_cluster_count(input_file);
    PRINT_ERRNO("Invalid count format in input file");

    if (!errno) {
        *arr = (struct cluster_t *) malloc(sizeof(struct cluster_t) * clusterCount);
        PRINT_ERRNO("Cluster memory allocation fail!");
    }

    if (!errno) {
        for (int i = 0; i < clusterCount; i++) {
            if (load_object(*arr, i, input_file) || not_unique_ID(*arr, i)) {
                clean_clusters(arr, i + 1);
                errno = EINVAL;
                break;
            }
        }
    }

    fclose(input_file);
    return clusterCount;
}

/*
 Tisk pole shluku. Parametr 'carr' je ukazatel na prvni polozku (shluk).
 Tiskne se prvnich 'narr' shluku.
*/
void print_clusters(struct cluster_t *carr, int narr)
{
    printf("Clusters:\n");
    for (int i = 0; i < narr; i++) {
        printf("cluster %d: ", i);
        print_cluster(&carr[i]);
    }
}

typedef struct {
    int clusterCount;
    char *filename;
} parsedArgs_t;

/**
 * Parses arguments into parsedArgs_t structure
 * @param argc
 * @param argv
 * @param parsedArgs structure with values parsed from argv
 * @return 1 if success 0 otherwise
 */
int parseArgs(int argc, char **argv, parsedArgs_t *parsedArgs)
{
    char *formatCheck = NULL;
    parsedArgs->clusterCount = 1;
    switch (argc) {
        case 3:
            parsedArgs->clusterCount = (int) strtol(argv[2], &formatCheck, 10);
            if (parsedArgs->clusterCount <= 0 || !iscntrl(formatCheck[0]))
                errno = EINVAL;
            RETURN_FALSE_IF_ERRNO("Cluster count must be a positive integer!!!");
            // fall through
        case 2:
            parsedArgs->filename = argv[1];
            break;
        default:
            errno = E2BIG;
            RETURN_FALSE_IF_ERRNO("Invalid argumets! Usage cluster.c <path> [N]")
            break;
    }
    return 1;
}

/**
 * Frees space in memory allocated for cluster array
 * @param carr cluster array
 * @param narr number of clusters
 */
void clean_clusters(struct cluster_t **carr, int narr)
{
    while (narr > 0) {
        narr = remove_cluster(*carr, narr, narr - 1);
    }
    if (*carr != NULL) {
        free(*carr);
        *carr = NULL;
    }
}

/**
 * Uses method of nearest neighbours to find clusters of set amount and merge them.
 * @param clusters cluster array to use
 * @param clusterCount number of clusters in cluster array
 * @param expectedClusterCount
 */
void nearest_neighbour_algorithm(struct cluster_t *clusters, int *clusterCount, int expectedClusterCount)
{
    int clusterToJoin1, clusterToJoin2;

    while (*clusterCount > expectedClusterCount) {
        find_neighbours(clusters, *clusterCount, &clusterToJoin1, &clusterToJoin2);
        merge_clusters(&clusters[clusterToJoin1], &clusters[clusterToJoin2]);
        *clusterCount = remove_cluster(clusters, *clusterCount, clusterToJoin2);
    }
}

int main(int argc, char *argv[])
{
    parsedArgs_t args;
    parseArgs(argc, argv, &args);
    ERRNO_RETURN;

    struct cluster_t *clusters;
    int clusterCount;

    clusterCount = load_clusters(args.filename, &clusters);
    ERRNO_RETURN;
    EINVAL_IF(args.clusterCount > clusterCount, "Specified target count is higher than number of objects!");
    ERRNO_RETURN;

    nearest_neighbour_algorithm(clusters, &clusterCount, args.clusterCount);
    print_clusters(clusters, clusterCount);
    clean_clusters(&clusters, clusterCount);
    return 0;
}