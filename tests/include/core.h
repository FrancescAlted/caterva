/*
 * Created by Aleix Alcacer
 * Part of this source code is based on https://github.com/bvdberg/ctest/
 */

#ifndef LWTEST_CORE_H
#define LWTEST_CORE_H

#ifdef __GNUC__
#define LWTEST_IMPL_FORMAT_PRINTF(a, b) __attribute__ ((format(printf, a, b)))
#else
#define LWTEST_IMPL_FORMAT_PRINTF(a, b)
#endif

typedef void (*lwtest_setup_func)(void *);

typedef void (*lwtest_teardown_func)(void *);

// Define the test struct
struct lwtest {
    const char *ssname;  // suite name
    const char *ttname;  // test name
    void (*run)();

    void *data;
    lwtest_setup_func setup;
    lwtest_teardown_func teardown;
    int skip;
    unsigned int magic;
};

//Define test name
#define _LWTEST_NAME(name) lwtest_##name

// Define test struct name */
#define _LWTEST_SNAME(sname, tname) _LWTEST_NAME(sname##_##tname)

#define _LWTEST_MAGIC (0xdeadbccf)

#if defined(_MSC_VER)
#pragma data_seg(push)
#pragma data_seg(".ctest$u")
#pragma data_seg(pop)
#define LWTEST_IMPL_SECTION __declspec(allocate(".lwtest$u")) __declspec(align(1))
#elif defined(__APPLE__)
#define LWTEST_IMPL_SECTION __attribute__ ((used, section ("__DATA, .lwtest"), aligned(1)))
#else
#define LWTEST_IMPL_SECTION __attribute__ ((used, section (".lwtest"), aligned(1)))
#endif

//#ifdef __APPLE__
//#define LWTEST_IMPL_SECTION __attribute__ ((used, section ("__DATA, .lwtest"), aligned(1)))
//#else
//#define LWTEST_IMPL_SECTION __attribute__ ((used, section (".lwtest"), aligned(1)))
//#endif

// Define struct
#define _LWTEST_STRUCT(sname, tname, tskip, tdata, tsetup, tteardown) \
    static struct lwtest LWTEST_IMPL_SECTION _LWTEST_NAME(sname##_##tname) = { \
        .ssname=#sname, \
        .ttname=#tname, \
        .run = _LWTEST_NAME(sname##_##tname##_run), \
        .data = tdata, \
        .setup = (lwtest_setup_func) tsetup, \
        .teardown = (lwtest_teardown_func) tteardown, \
        .skip = tskip, \
        .magic = _LWTEST_MAGIC }

#define LWTEST_DATA(sname) struct sname##_data

#define LWTEST_SETUP(sname) void sname##_setup(struct sname##_data* data)

#define LWTEST_TEARDOWN(sname) void sname##_teardown(struct sname##_data *data)


#define _LWTEST_TEST(sname, tname, tskip) \
    static void _LWTEST_NAME(sname##_##tname##_run)(void); \
    _LWTEST_STRUCT(sname, tname, tskip, NULL, NULL, NULL); \
    static void _LWTEST_NAME(sname##_##tname##_run)(void)

#define _LWTEST_FIXTURE(sname, tname, tskip) \
    LWTEST_DATA(sname) _LWTEST_NAME(sname##_data); \
    LWTEST_SETUP(sname); \
    LWTEST_TEARDOWN(sname); \
    static void _LWTEST_NAME(sname##_##tname##_run)(struct  sname##_data *data); \
    _LWTEST_STRUCT(sname, tname, tskip, &_LWTEST_NAME(sname##_data), &sname##_setup, &sname##_teardown); \
    static void _LWTEST_NAME(sname##_##tname##_run)(struct sname##_data* data)

void LWTEST_ERR(const char *fmt, ...) LWTEST_IMPL_FORMAT_PRINTF(1, 2);

#define LWTEST_TEST(sname, tname) _LWTEST_TEST(sname, tname, 0)
#define LWTEST_TEST_SKIP(sname, tname) _LWTEST_TEST(sname, tname, 1)

#define LWTEST_FIXTURE(sname, tname) _LWTEST_FIXTURE(sname, tname, 0)
#define LWTEST_FIXTURE_SKIP(sname, tname) _LWTEST_FIXTURE(sname, tname, 1)

#endif //LWTEST_CORE_H
