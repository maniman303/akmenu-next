#define LIBNDS_STRINGIFY(x) LIBNDS_STRINGIFY_(x)
#define LIBNDS_STRINGIFY_(x) #x

#ifdef __INTELLISENSE__

#define ITCM_FUNC(x) x
#define ARM_CODE
#define LIBNDS_NOINLINE

#else // __INTELLISENSE__

/// Used to tell the compiler to compile a function as ARM code
#define ARM_CODE __attribute__((__target__("arm")))
/// Used to place a function in ITCM
#define ITCM_FUNC(x) __attribute__((__section__(".itcm.text." LIBNDS_STRINGIFY(x)))) x
/// Used to mark functions that mustn't be inlined.
#define LIBNDS_NOINLINE __attribute__((__noinline__))

#endif // LIBNDS_NDS_NDSTYPES_H__