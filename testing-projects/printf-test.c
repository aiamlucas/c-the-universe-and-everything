#include <stdio.h>
#include <limits.h>
#include "ft_printf.h"

int main(void)
{
    int ft;
    int pf;
    int x = 42;

    /* TEST 1: Plain text */
    ft = ft_printf("[TEST_1]_Hello_World\n");
    pf = printf("[TEST_1]_Hello_World\n");
    printf("return -> ft:%d | printf:%d\n\n", ft, pf);

    /* TEST 2: Character */
    ft = ft_printf("[TEST_2]_Char:[%c]\n", 'A');
    pf = printf("[TEST_2]_Char:[%c]\n", 'A');
    printf("return -> ft:%d | printf:%d\n\n", ft, pf);

    /* TEST 3: String */
    ft = ft_printf("[TEST_3]_String:[%s]\n", "Hello 42");
    pf = printf("[TEST_3]_String:[%s]\n", "Hello 42");
    printf("return -> ft:%d | printf:%d\n\n", ft, pf);

    /* TEST 4: NULL string */
    ft = ft_printf("[TEST_4]_NULL_string:[%s]\n", NULL);
    pf = printf("[TEST_4]_NULL_string:[%s]\n", NULL);
    printf("return -> ft:%d | printf:%d\n\n", ft, pf);

    /* TEST 5: Integer */
    ft = ft_printf("[TEST_5]_Int:[%d]\n", 42);
    pf = printf("[TEST_5]_Int:[%d]\n", 42);
    printf("return -> ft:%d | printf:%d\n\n", ft, pf);

    /* TEST 6: Negative integer */
    ft = ft_printf("[TEST_6]_Neg_Int:[%d]\n", -42);
    pf = printf("[TEST_6]_Neg_Int:[%d]\n", -42);
    printf("return -> ft:%d | printf:%d\n\n", ft, pf);

    /* TEST 7: INT limits */
    ft = ft_printf("[TEST_7]_INT_MIN:[%d]\n", INT_MIN);
    pf = printf("[TEST_7]_INT_MIN:[%d]\n", INT_MIN);
    printf("return -> ft:%d | printf:%d\n\n", ft, pf);

    ft = ft_printf("[TEST_7]_INT_MAX:[%d]\n", INT_MAX);
    pf = printf("[TEST_7]_INT_MAX:[%d]\n", INT_MAX);
    printf("return -> ft:%d | printf:%d\n\n", ft, pf);

    /* TEST 8: Unsigned */
    ft = ft_printf("[TEST_8]_Unsigned:[%u]\n", 42);
    pf = printf("[TEST_8]_Unsigned:[%u]\n", 42);
    printf("return -> ft:%d | printf:%d\n\n", ft, pf);

    /* TEST 9: UINT_MAX */
    ft = ft_printf("[TEST_9]_UINT_MAX:[%u]\n", UINT_MAX);
    pf = printf("[TEST_9]_UINT_MAX:[%u]\n", UINT_MAX);
    printf("return -> ft:%d | printf:%d\n\n", ft, pf);

    /* TEST 10: Hex lowercase */
    ft = ft_printf("[TEST_10]_Hex_x:[%x]\n", 42);
    pf = printf("[TEST_10]_Hex_x:[%x]\n", 42);
    printf("return -> ft:%d | printf:%d\n\n", ft, pf);

    /* TEST 11: Hex uppercase */
    ft = ft_printf("[TEST_11]_Hex_X:[%X]\n", 42);
    pf = printf("[TEST_11]_Hex_X:[%X]\n", 42);
    printf("return -> ft:%d | printf:%d\n\n", ft, pf);

    /* TEST 12: Pointer */
    ft = ft_printf("[TEST_12]_Pointer:[%p]\n", &x);
    pf = printf("[TEST_12]_Pointer:[%p]\n", &x);
    printf("return -> ft:%d | printf:%d\n\n", ft, pf);

    /* TEST 13: NULL pointer */
    ft = ft_printf("[TEST_13]_NULL_ptr:[%p]\n", NULL);
    pf = printf("[TEST_13]_NULL_ptr:[%p]\n", NULL);
    printf("return -> ft:%d | printf:%d\n\n", ft, pf);

    /* TEST 14: Percent */
    ft = ft_printf("[TEST_14]_Percent:[%%]\n");
    pf = printf("[TEST_14]_Percent:[%%]\n");
    printf("return -> ft:%d | printf:%d\n\n", ft, pf);

    /* TEST 15: Mixed */
    ft = ft_printf("[TEST_15]_Mix:[%c]_[%s]_[%d]_[%u]_[%x]_[%X]_[%%]\n",
            'Z', "Hi", -42, 42, 42, 42);
    pf = printf("[TEST_15]_Mix:[%c]_[%s]_[%d]_[%u]_[%x]_[%X]_[%%]\n",
            'Z', "Hi", -42, 42, 42, 42);
    printf("return -> ft:%d | printf:%d\n\n", ft, pf);

    /* ========================= */
    /* EDGE CASES                */
    /* ========================= */

    /* TEST 16: Empty string */
    ft = ft_printf("[]\n");
    pf = printf("[]\n");
    printf("return -> ft:%d | printf:%d\n\n", ft, pf);

    /* TEST 17: Only newline */
    ft = ft_printf("\n");
    pf = printf("\n");
    printf("return -> ft:%d | printf:%d\n\n", ft, pf);

    /* TEST 18: Zero integer */
    ft = ft_printf("[Zero_d:[%d]]\n", 0);
    pf = printf("[Zero_d:[%d]]\n", 0);
    printf("return -> ft:%d | printf:%d\n\n", ft, pf);

    /* TEST 19: Zero unsigned */
    ft = ft_printf("[Zero_u:[%u]]\n", 0);
    pf = printf("[Zero_u:[%u]]\n", 0);
    printf("return -> ft:%d | printf:%d\n\n", ft, pf);

    /* TEST 20: Zero hex */
    ft = ft_printf("[Zero_x:[%x]]_[Zero_X:[%X]]\n", 0, 0);
    pf = printf("[Zero_x:[%x]]_[Zero_X:[%X]]\n", 0, 0);
    printf("return -> ft:%d | printf:%d\n\n", ft, pf);

    /* TEST 21: Pointer (0) */
    ft = ft_printf("[Ptr_0:[%p]]\n", (void *)0);
    pf = printf("[Ptr_0:[%p]]\n", (void *)0);
    printf("return -> ft:%d | printf:%d\n\n", ft, pf);

    /* TEST 22: Arbitrary pointer */
    ft = ft_printf("[Ptr_val:[%p]]\n", (void *)0x1234);
    pf = printf("[Ptr_val:[%p]]\n", (void *)0x1234);
    printf("return -> ft:%d | printf:%d\n\n", ft, pf);

    /* TEST 23: Non-printable char */
    ft = ft_printf("[Char_NUL:[%c]]\n", '\0');
    pf = printf("[Char_NUL:[%c]]\n", '\0');
    printf("return -> ft:%d | printf:%d\n\n", ft, pf);

    /* TEST 24: Consecutive specifiers */
    ft = ft_printf("[%d%d%d]\n", 1, 2, 3);
    pf = printf("[%d%d%d]\n", 1, 2, 3);
    printf("return -> ft:%d | printf:%d\n\n", ft, pf);

    /* TEST 25: Multiple percent */
    ft = ft_printf("[%%%%]\n");
    pf = printf("[%%%%]\n");
    printf("return -> ft:%d | printf:%d\n\n", ft, pf);

    /* TEST 26: Percent in text */
    ft = ft_printf("[100%%_sure]\n");
    pf = printf("[100%%_sure]\n");
    printf("return -> ft:%d | printf:%d\n\n", ft, pf);

    /* TEST 27: Long string */
    ft = ft_printf("[Long:[%s]]\n",
        "Lorem_ipsum_dolor_sit_amet_consectetur_adipiscing_elit");
    pf = printf("[Long:[%s]]\n",
        "Lorem_ipsum_dolor_sit_amet_consectetur_adipiscing_elit");
    printf("return -> ft:%d | printf:%d\n\n", ft, pf);

    return (0);
}
