// SPDX-License-Identifier: GPL-2.0-only
/*  
 *  hello.c - The simplest kernel module.
 */
#include    <linux/init.h>      /* Needed for __init/__exit */
#include    <linux/module.h>    /* Needed for module_init/module_exit/MODULE_* */

static int __init hello_init(void)
{
    pr_info("Hello world\n");

    return 0; // A non 0 return value means init_module failed; module can't be loaded.
}

static void __exit hello_exit(void)
{
    pr_info("Goodbye world\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_AUTHOR("Your Name <your.email@domain.com>");
MODULE_DESCRIPTION("A simple hello / goodbye driver");
MODULE_LICENSE("GPL");