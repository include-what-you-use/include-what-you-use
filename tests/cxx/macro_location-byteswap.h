#define bswap(x) ({ int __x = (x); bswap2(__x); })
#define bswap2(x) (x)
