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

/*****************************************************************
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

	// TODO
	c->size = 0;
	c->capacity = cap;
	c->obj = (struct obj_t *) malloc(sizeof(struct obj_t) * cap);
}

/*
 Odstraneni vsech objektu shluku a inicializace na prazdny shluk.
 */
void clear_cluster(struct cluster_t *c)
{
	if (c->obj)
		free(c->obj);
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
	// TODO
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

	// TODO
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
	assert(idx < narr);
	assert(narr > 0);

	// TODO
	for (int clusterIndex = idx; clusterIndex < narr - 1; clusterIndex++) {
		clear_cluster(&(carr[clusterIndex]));
		init_cluster(&carr[clusterIndex], 1);
		merge_clusters(&carr[clusterIndex], &carr[clusterIndex + 1]);
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
//Pythagorean theorem
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

	float minDistance = obj_distance(&c1->obj[0], &c2->obj[0]);
	float currentDistance;
	for (int firstClusterIterator = 1; firstClusterIterator < c1->size; firstClusterIterator++) {
		for (int secondClusterIterator = 0; secondClusterIterator < c2->size; ++secondClusterIterator) {
			currentDistance = obj_distance(&c1->obj[firstClusterIterator], &c2->obj[secondClusterIterator]);
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

	// TODO
	float minClusterDistance = cluster_distance(&carr[0], &carr[1]);
	for (int firstClusterIndex = 0; firstClusterIndex < narr - 1; firstClusterIndex++) {
		for (int secondClusterIndex = firstClusterIndex + 1; secondClusterIndex < narr; secondClusterIndex++) {
			dint(firstClusterIndex);
			dint(secondClusterIndex);
			float clusterDistance = cluster_distance(&carr[firstClusterIndex], &carr[secondClusterIndex]);
			if (minClusterDistance > clusterDistance) {
				minClusterDistance = clusterDistance;
				*c1 = firstClusterIndex;
				*c2 = secondClusterIndex;
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

	// TODO
	FILE *input_file = fopen(filename, "r");
	if (input_file == NULL) {
		arr = NULL;
		errno = ENOFILE;
		fprintf(stderr, "%s", strerror(errno));
		return 0;
	}

	int clusterCount;
	fscanf(input_file, "count=%d", &clusterCount);
	*arr = (struct cluster_t *) malloc(sizeof(struct cluster_t) * clusterCount);
	for (int objectIndex = 0; objectIndex < clusterCount; objectIndex++) {
		init_cluster(&(*arr)[objectIndex], 1);
		fscanf(input_file, "%d%f%f", &(*arr)[objectIndex].obj->id, &(*arr)[objectIndex].obj->x,
		       &(*arr)[objectIndex].obj->y);
		(*arr)[objectIndex].size = 1;
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

int parseArgs(int argc, char **argv, parsedArgs_t *parsedArgs)
{
	parsedArgs->clusterCount = 1;
	switch (argc) {
		case 3:
			//atoi returns 0 on error
			parsedArgs->clusterCount = (int) strtol(argv[2], NULL, 10);
			if (errno) {
				break;
			}
		case 2:
			parsedArgs->filename = argv[1];
			break;
		default:
			errno = E2BIG;
			break;
	}
	if (errno)
		fprintf(stderr, "%s\n", strerror(errno));
	return errno;
}

int clean_clusters(struct cluster_t **carr, int *narr)
{
	for (; *narr > 0; --(*narr)) {
		remove_cluster(*carr, *narr, *narr - 1);
	}
	free(*carr);
	return 0;
}

int main(int argc, char *argv[])
{
	parsedArgs_t args;
	if (parseArgs(argc, argv, &args))
		return errno;
	struct cluster_t *clusters;
	int clusterCount = load_clusters(args.filename, &clusters);
	int c1, c2;

	while (clusterCount > args.clusterCount) {
		find_neighbours(clusters, clusterCount, &c1, &c2);
		merge_clusters(&clusters[c1], &clusters[c2]);
		remove_cluster(clusters, clusterCount, c2);
		clusterCount--;
	}
	print_clusters(clusters, clusterCount);
	clean_clusters(&clusters, &clusterCount);
}