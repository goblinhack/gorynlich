/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info for license.
 */

#define ENUM_TESTVAL(list_macro)                                \
    list_macro(ENUM_TESTVAL_STEELBLUE = 0,    "red"),                 \
    list_macro(ENUM_TESTVAL_GREEN,      "green"),               \
    list_macro(ENUM_TESTVAL_BLUE,       "blue"),                \

ENUM_DEF_H(ENUM_TESTVAL, enum_testval)
ENUM_DEF_C(ENUM_TESTVAL, enum_testval)

uint8_t enum_test (int32_t argc, char *argv[])
{
    printf("ENUM_TESTVAL_STEELBLUE   = %d\n", ENUM_TESTVAL_STEELBLUE);
    printf("ENUM_TESTVAL_GREEN = %d\n", ENUM_TESTVAL_GREEN);
    printf("ENUM_TESTVAL_BLUE  = %d\n", ENUM_TESTVAL_BLUE);

    printf("red   = %d\n", enum_testval_str2val("red"));
    printf("green = %d\n", enum_testval_str2val("green"));
    printf("blue  = %d\n", enum_testval_str2val("blue"));

    printf("[%d]  = %s\n", ENUM_TESTVAL_STEELBLUE,
           enum_testval_val2str(ENUM_TESTVAL_STEELBLUE));
    printf("[%d]  = %s\n", ENUM_TESTVAL_GREEN,
           enum_testval_val2str(ENUM_TESTVAL_GREEN));
    printf("[%d]  = %s\n", ENUM_TESTVAL_BLUE,
           enum_testval_val2str(ENUM_TESTVAL_BLUE));

    return (0);
}
