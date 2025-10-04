#include "main.h"
#include "../print/print.h"
#include "../string/string.h"
#include "../graphics/colors/stdclrs.h"

void mem_test(void)
{
    print("Memory Test\n", TITLE_COLOR);

    // test basic allocation
    char *test1 = (char *)kalloc(64);
    if (test1 == NULL) {
        print("FAIL: kalloc test1\n", FAIL_COLOR);
        return;
    }
    print("PASS: kalloc test1\n", PASS_COLOR);

    // test memset
    memset(test1, 0xAA, 64);
    int ok = 1;
    for (int i = 0; i < 64; i++) {
        if ((u8)test1[i] != 0xAA) {
            ok = 0;
            break;
        }
    }
    if (ok) {
        print("PASS: memset\n", PASS_COLOR);
    } else {
        print("FAIL: memset\n", FAIL_COLOR);
    }

    // test memcpy
    char *test2 = (char *)kalloc(64);
    if (test2 == NULL) {
        print("FAIL: kalloc test2\n", FAIL_COLOR);
        kfree(test1);
        return;
    }

    memset(test2, 0x55, 64);
    memcpy(test1, test2, 64);

    ok = 1;
    for (int i = 0; i < 64; i++) {
        if ((u8)test1[i] != 0x55) {
            ok = 0;
            break;
        }
    }
    if (ok) {
        print("PASS: memcpy\n", PASS_COLOR);
    } else {
        print("FAIL: memcpy\n", FAIL_COLOR);
    }

    // test memcmp
    if (memcmp(test1, test2, 64) == 0) {
        print("PASS: memcmp\n", PASS_COLOR);
    } else {
        print("FAIL: memcmp\n", FAIL_COLOR);
    }

    // test memmove
    char *test3 = (char *)kalloc(128);
    if (test3 == NULL) {
        print("FAIL: kalloc test3\n", FAIL_COLOR);
        kfree(test1);
        kfree(test2);
        return;
    }

    for (int i = 0; i < 128; i++) {
        test3[i] = i & 0xFF;
    }

    memmove(test3 + 10, test3, 64);

    ok = 1;
    for (int i = 0; i < 64; i++) {
        if ((u8)test3[i + 10] != (i & 0xFF)) {
            ok = 0;
            break;
        }
    }
    if (ok) {
        print("PASS: memmove\n", PASS_COLOR);
    } else {
        print("FAIL: memmove\n", FAIL_COLOR);
    }

    // test free
    size_t before_free = mem_get_free();
    kfree(test1);
    kfree(test2);
    kfree(test3);

    size_t after_free = mem_get_free();

    if (after_free > before_free) {
        print("PASS: kfree\n", PASS_COLOR);
    } else {
        print("FAIL: kfree\n", FAIL_COLOR);
    }

    // test realloc
    char *test4 = (char *)kalloc(32);
    if (test4 == NULL) {
        print("FAIL: kalloc test4\n", FAIL_COLOR);
        return;
    }

    memset(test4, 0x42, 32);
    test4 = (char *)krealloc(test4, 128);

    if (test4 == NULL) {
        print("FAIL: krealloc\n", FAIL_COLOR);
        return;
    }

    ok = 1;
    for (int i = 0; i < 32; i++) {
        if ((u8)test4[i] != 0x42) {
            ok = 0;
            break;
        }
    }

    if (ok) {
        print("PASS: krealloc\n", PASS_COLOR);
    } else {
        print("FAIL: krealloc\n", FAIL_COLOR);
    }

    kfree(test4);

    // print memory statistics
    char buf[128];

    str_copy(buf, "\nFree: ");
    str_append_uint(buf, (u32)mem_get_free());
    str_append(buf, " bytes");
    print(buf, TEXT_COLOR);

    str_copy(buf, "\nUsed: ");
    str_append_uint(buf, (u32)mem_get_used());
    str_append(buf, " bytes\n");
    print(buf, TEXT_COLOR);
}
