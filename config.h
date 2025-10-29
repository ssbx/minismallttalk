#ifndef _MINIST_CONFIG_H_
#define _MINIST_CONFIG_H_

#define M8_VMAJ   0
#define M8_VMIN   0
#define M8_VPATCH 1

/*
 * #define HACK_SUSPEND
 */
#define DEBUGGING_SUPPORT

#define _PROFILE(func) func
#undef WITH_PROFILE
#ifdef WITH_PROFILE
static Uint32   a;
static Uint32   b;
#define PROFILE(func) \
  a = SDL_GetTicks(); \
  func; \
  b = SDL_GetTicks(); \
  fprintf(stderr, "%s %i %s %i ms\n", __FILE__, __LINE__, #func, b - a)
#else
#define PROFILE(func) _PROFILE
#endif                          /* WITH_PROFILE */

/*
 * implement optional primitiveNext
 */
#define IMPLEMENT_PRIMITIVE_NEXT

/*
 * implement optional primitiveAtEnd
 */
#define IMPLEMENT_PRIMITIVE_AT_END

/*
 * implement optional primitiveNextPut
 */
#define IMPLEMENT_PRIMITIVE_NEXT_PUT

/*
 * implement optional primitiveScanCharacters
 */
#define IMPLEMENT_PRIMITIVE_SCANCHARS

/*
 *
 * // objmemory.c
 *
 * // The Smalltalk-80 VM generates a tremendous amount of circular references
 * as // it runs // -- primarily a MethodContext that references a BlockContext
 * (from a temp // field) that // has a back reference to that MethodContext
 * (the sender field). If a // reference counting only // scheme is used, then
 * free object table entries will eventually be consumed. // If, on the other
 * hand, // a GC only approach is used then memory will fill up with contexts
 * and GC // will happen fairly // frequently. Therefore, the hybrid reference
 * counting approach with full // garbage collection // when too much circular
 * garbage accumulates is recommended.
 *
 * // GM_MARK_SWEEP and GC_REF_COUNT are not mutually exclusive! // You can
 * define *BOTH* for a hybrid collector which ref counts until // memory is
 * exhausted (cyclical data) and then does a full GC
 *
 */

/*
 * Mark and sweep collection when memory full
 */
#define GC_MARK_SWEEP

/*
 * Ref counting
 */
#define GC_REF_COUNT

/*
 * // Define to use recursive marking for ref counting/GC // If undefined the
 * stack space efficient pointer reversal approach described // on page 678 of
 * G&R is used. Not recommended, and only included for // completeness. //
 * #define RECURSIVE_MARKING
 */

#ifdef RUNTIME_CHECKING
void
OBJMEM_runtime_check(bool condition, const char *errorMessage)
{
    if (!condition) {
        assert(0);
        fprintf(stderr, "%s\n", errorMessage);
        abort();
    }
}
#endif

/*
 * Perform range checks etc. at runtime
 */
#define RUNTIME_CHECKING

#ifdef RUNTIME_CHECKING
#define RUNTIME_CHECK(cond)  { \
  bool c = cond; \
  if (!c) { \
    fprintf(stderr, "RUNTIME ERROR:  file %s line %i\n", __FILE__, __LINE__); \
    abort(); } }
/*
 * // #define RUNTIME_CHECK2(c,f,l) OBJMEM_runtime_check(c, "RUNTIME ERROR: ("
 * #c // ") at: " f "(" #l ")") // #define RUNTIME_CHECK1(c,f,l)
 * RUNTIME_CHECK2(c,f,l) // #define RUNTIME_CHECK(cond) RUNTIME_CHECK1(cond,
 * __FILE__, __LINE__)
 */
#else
#define RUNTIME_CHECK(cond) ((void)0)
#endif

#endif                          /* _MINIST_CONFIG_H_ */
