#ifndef INCLUDE_GLOBALS
#define INCLUDE_GLOBALS

#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define CLAMP(x, l, u) MAX((l), MIN((x), (u)))

#define isizeof(x) ((int)sizeof(x))

#define ALLOCATE(n, t) ((t*)malloc((size_t)(n) * sizeof(t)))

#define FORI(lo, hi) for(int i=(lo); i<(hi); i++)

__attribute__((const)) float mix(float a, float b, float x);

#endif
