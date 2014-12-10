/*
 * =====================================================================================
 *
 *       Filename:  types.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  11/20/2014 02:21:09 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Changqing
 *        Company:  
 *
 * =====================================================================================
 */
#ifndef __linux_user__
#include <linux/module.h>
#define printf printk

static int __init __types_init(void)
#else
#include <stdio.h>

int main(int argc, char **argv)
#endif
{
#define TE(a)   { #a , sizeof(a) }
    struct {
        const char *str;
        int size;
    }types[] = {
        TE(char),
        TE(short int),
        TE(int),
        TE(long),
        TE(long long),
        TE(void *),
    };
    int i = 0;

	printf("Type-Size of %d-bits OS :\n", sizeof(char*)*8);
    for( ; i < sizeof(types) / sizeof(types[0]); i++)
    {
        printf("%10s : %d\n", types[i].str, types[i].size);
    }

    return 0;
}

#ifndef __linux_user__
module_init(__types_init);
static void __exit __types_exit(void){}
module_exit(__types_exit);
#endif
