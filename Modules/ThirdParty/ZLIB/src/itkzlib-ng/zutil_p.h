/* zutil_p.h -- Private inline functions used internally in zlib-ng
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#ifndef ZUTIL_P_H
#define ZUTIL_P_H

#if defined(__APPLE__) || defined(HAVE_POSIX_MEMALIGN) || defined(HAVE_ALIGNED_ALLOC)
#  include <stdlib.h>
#elif defined(__FreeBSD__)
#  include <stdlib.h>
#  include <malloc_np.h>
#else
#  include <malloc.h>
#endif

/* Function to allocate 16 or 64-byte aligned memory */
static inline void *zng_alloc(size_t size) {
#ifdef HAVE_ALIGNED_ALLOC
    /* Size must be a multiple of alignment */
    size = (size + (64 - 1)) & ~(64 - 1);
    return (void *)aligned_alloc(64, size);  /* Defined in C11 */
#elif defined(HAVE_POSIX_MEMALIGN)
    void *ptr;
    return posix_memalign(&ptr, 64, size) ? NULL : ptr;
#elif defined(_WIN32)
    return (void *)_aligned_malloc(size, 64);
#elif defined(__APPLE__)
    /* Fallback for when posix_memalign and aligned_alloc are not available.
     * On macOS, it always aligns to 16 bytes. */
    return (void *)malloc(size);
#else
    return (void *)memalign(64, size);
#endif
}

/* Function that can free aligned memory */
static inline void zng_free(void *ptr) {
#if defined(_WIN32)
    _aligned_free(ptr);
#else
    free(ptr);
#endif
}

#endif
