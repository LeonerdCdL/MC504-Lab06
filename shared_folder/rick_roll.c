// SPDX-License-Identifier: GPL-2.0-only
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/string.h>

static dev_t lkcamp_dev; // Holds the major and minor number for our driver
static struct cdev lkcamp_cdev; // Char device. Holds fops and device number

// Possible states for the driver
enum driver_state {
    STATUS0 = 0,
    STATUS1 = 1,
    STATUS2 = 2,
    STATUS3 = 3,
    STATUS4 = 4,
};

static enum driver_state status = STATUS0;
static const char *status_strings[] = {
    "tttftttttttttttttttttffftttttt11ttttt1111tt11111111tttt1ttt11tttt111111tttttttttttttttttttt1111111tt\nttfftttttttttttttttffLfftfffttffttttt111111111t11ttffffftttt1111111111111111ttffttfttttttttt1111tttt\ntttttttttttttttttffffftttffffffffffft1111tttt111tfffffffttttttttt11111tttttffffftfLLfttttttt1111ttt1\nttttttttttttttttffffftttffffffLfffttt1111ttttt1tffffffffffttttttt11111tfffffffLfttffffffttt1111ttttt\nttttttttttttfffftttftttffffffffttttft1i;:;;;;;i11tfffffftttttt111t111111ttffffLLffttfLLft111111ttttt\nttttttttttffLLLLftttttffLLfttttttfft1;:,,,,,,,,:1ttfffftttfffft11t111tt111tttffLLfttffft1ttt11tttttt\nttttttttfffLLLLLLffttfLLffttfffttti::,,,,,,,,,,;ttttftttffffffftt1111tftt11t11tfLLfttt11tffftttttttt\ntttttttfffLLLLLLLLLfttfttttfffffti,,,,::;;;iiiii1ffttttffffffffftt111tft1ttft11tffLfttttfffffftttttt\ntttttttfLLLLLLLLLLftttttfftfffftt1:,,,;iii1111111fftttfffffffffftt1111111ttftttttttttttfffffLLfffttt\ntttffttfffLLLLLLLLfttfttffftttttff:,,:;;;iiii1ii1tttttttttfffffftt1111ttt1ttt1ttfttt11tfffffLLLLfttt\nttfffftfffLLLLLfffttfffttttffftfLt;:,:;;;;iiiiii11tfffft11tfffft11111tffft1tt11tt1ttttfffLLLLLLLffff\ntffftfttttfLLffttttttffftttfLLftff1;;;;;iiiii;i11ttfffftt11tttt1ttt11tfffttffft1ttfft1tffLLLLLLLLffL\ntttttttttttffftttttttttttttfffLtfffii;;;;;ii;;iittffffffftt1ttt1111111ffftfffftttttttt1ttfLLLLffffff\nttffffffffftttfffffffffttft1tffftffti;;;;;iiiii1ttttffffttt1tffft11111tfttfffttt1tttttttttfLLfffffft\nttffffffffftttfffffffftttfttttfttttti;;;;;iiii1111tfftft11ttttffff1111tt1tffttffttfffffLLffffffLffLL\nttffffffffttttfffffffft1tt11111tft11i;;;;;;;iit11111tt111tffttttft11111tt1ttttfft1tffffLLLfttfLLLLLL\ntttffffffftttfffftfftt1111111111i:,;i;;;;;;ii1i;;;111111tfffft11t11111tfftttttttt1tfffffLfftttfLLLLf\nttttt1tttffttfttt11t111tttt1i;:,...:1i;ii;ii;1i:;;i;;i1tftfffft1111111tttt11ttfft11ttffffffftfLLLLLf\ntfttttttttt1tt1111111ttt1;:,,.......;ti;iii;it1;;;;,..,:i1fffft11t1111ttt11tt1tffftt1111ttfftfLffftt\nttttfffftt1111ttttt111;:,...........;tt11iii1t1::;;,,,,,.,1ft111tt1111ttt11tf1tffffftttttt1ttttttftt\n111tfffffft1ttttttt11,..............:i11iii111:,;;,,,,,,,.;t1111t11111tt111tft1tffffttfffftt11tfffLf\n111ttfffft111ttttttt;.................:;;;;;;:.,ii,....,,.:1tt11111111111t11tf1tfffttfffffffttfLfLLf\ntttt1tftt11111ttt111:.................,;;i;;;,.,1i,.......:t111111t1111111111tttffttttffffftt11fffLf\ntttt11ttttttt1111111,..................:;;;;;,,,;;,.......:11tttt11111111tttt11tttfLffttfft1tt1tffft\n1tttt11ttttttt11111i...................,;;;;:,..:,........:1tttttt111111tttffttttfLLLLfttttffffttttf\ntfftt11ttttttt11111:....................:i;;:,.,:,........,1ttttttt1111ttttffftttffffffftttttttffttf\nttfft11ttftttt111ti,.....................;;::,..:,........,i1tttt11111111ttffttttffLLfLfttfffffffttf\ntttft11ttttttt111t;......................,::,...:,.........;11t11111111111ttt1tttffLLLLfttfffffffttf\nttttt11ttttttt111t:......................,::,...:,...,,....it11111111111t111tttttfffLLLfttfffftffttf\ntttt11tttt11tt1111,.......................,,,...:,...,:;;;;itt11ttt1111ttt1ttftttffffffftttffftffttt\nttttt11t111111111i.........................,,...:,...,:;iiii11111t111111t111tttttttttffft1tftttffttt\nttttt11ttttttt111,.........................,,...:....,:;;iii11111111111111ttttt1ttfffttt11ttttttfttt\nttttt11ttttttt111;...................,,,,..,....,......,:;ii1ttt1111111111ttt1111tfttttt11ttttttfttt\n11111111t11t11111t;...............,:::::::..,...:.........,,it11tt1111111tttt1111ttt1tt1111ttttttt1t\n1111111111111111111,.............:::::;;i:......:.....     :1ttttt111111111111111111111111111111t111\n11111111111111111111;,.. ........,::::;;:.......,... ,i;;;i1tttt111111111111111111111111111111111111\n11111111111111111111t11i,.........,:::;;. ..,:,.:.....itttttttt1111111t111111111111111111111111t1111\n11111111111111111111111t;...........,,,.... ,:,,:......it1111ttt1111111111111111111111111111111t1111\n111111111111111111111111;..........    .... .:,,:,. ...:tttt11tt111111111111111111111111111111111111\n111111111111111111111111;....................::::..,,..,1tttttttt1tttt1111t1111111111111111111111111\n11111111111111111111111t;....................::::..,,...;t1tttttttttttt1t11ttttt111t111111tttttt1111\n11111111111111111111111ti....................:;::,......:11tttttttttttttttttttttt11t11t1111ttttt1111\n",
    "                      :                      \n                      :                      \n          %@          :      @@              \n          %@          :      @@     @@       \n          %@          :      @@     @@       \n          %@          :      @@     @@       \n          %@          :      @@     @@       \n          %@          :      @@     @@       \n          %@          :      @@     @@       \n                      :                      \n                      :                      \n----------------------+----------------------\n                      -                      \n                      -                      \n       @@    :@#      -     @@               \n       @@    :@#      -     @@               \n       @@    :@#      -     @@               \n       @@    :@#      -     @@               \n       @@    :@#      -     @@               \n       @@    :@#      -     @@               \n       @@    :@#      -     @@  @@@@@@@@@.   \n                      -                      \n                      -                      \n",
    "         :###########################=           +##############################################\n       .*#*.                        -##=         +#-                                         -##\n      ###.  ***********+*******#***-  +##:       +#- .***+*+*******************************+ :#+\n   .+##. .###########+*##############.  +##.     +#- .####*##############+***##############+ :*#\n  ++*:  *##########*#################++. .*##.   +#- .#**################+*+*##############= -*#\n-##+  :#################*################  :##*  +#- .*####################################+ -##\n##- :############:            +###########= .*#  +#- .######-                    =#########  -##\n##- ###########:  **##*#####*. .*########## .*#  +#- .######-  ***************- =####*####= .#*:\n##- ##########. *##-:::::::-###  ########## .*#  +#-           ##          ##* .##########  +## \n##- ##########. *#*          ##             .*#  +###############         *#*. ##########: +#*  \n##- ###*######. +#*          ##***+********+*##  .::.::::::::::::        :##. -*########= -##:  \n##- #*########. *#*          ***********++*****                          ##=  #########+  ##-   \n##- *#########. *#*                                                     +##  *#########. +*+    \n#+: *#########. *##+*++##############*                                 :#*. -#########: .##     \n##- ##########.                     :##+.                             .**+ :#########* .##=     \n##- ###########*###################=  =##=                            *#*  *#########  =##      \n##- #################################-  =##-                         =##: *#########= =##:      \n##- ###################################:  +##-                       ##- :#########+ .##=       \n##- ####################################*. .*#*                     ##+  #########*. *#+        \n##- ###*########*             -############ .*#                    :##  =######*##. -##.        \n##- ##########+  .####**#####:  +########## .*#                   .##- :#########=  ##:         \n**: ##########. *#*.        *##  ####*+*+## .*#                   +## .#*+***####  *#*          \n#+: *#########. *#*          ##  ####*++### .*#                  :##  +##*++####: :##           \n##- ##########. *#*          ##  ########## .*#                 :##- =#########+ :##=           \n##- ##########. *#*         -##  ########## .*#                 *#+  #########*  *#*            \n##- ##########=  -###########+  :########## .*#                +##. ##########: *##.            \n##- ############=             .*########### .*#                ##. :#########- .##:             \n###  .############*########*#############=  =##               ##+  #########*  ##=              \n :##+  -###############################*  :*#+               -##  +#########  =*#               \n   -**-  =###########################=  -##+                .##: =#########- :##.               \n     =##-  =#######****############*  :*#*                  +#* :#########* .*#*                \n       +##-                          +##:                  -#*              +#*                 \n        .#############################:                   :##*################:                 \n************************************************************************************************\n*++***+*+=++++++=***********************************************************==*++==++=++==+=++**\n************************************************************************************************\n",
    "                                    -@@@@@@@@@@@@@@@@@@@@-                                    \n                               +@@@@+                    +@@@@+                               \n                          ..%%%-                              -%%#..                          \n                        :-++                                      ++-:                        \n                      .%=.                                          .=%.                      \n                     @%                                                %@                     \n                     @%                                                %@                     \n                   @@                                                    @@                   \n                   @@                                                    @@                   \n                   @@     ...-@@@@@@@@@%..          ..%@@@@@@@@@-...     @@                   \n                   @@   :=%@@@@@@@@@@@@@@#          #@@@@@@@@@@@@@@%=:   @@                   \n                   @@   +@@@@@@@@...=@@@@#          #@@@@=...@@@@@@@@+   @@                   \n                   @@   +@@@@@@@@   -@@@@#          #@@@@-   @@@@@@@@+   @@                   \n                     @%   #@@@@@@@@@@@@%     %@@%     %@@@@@@@@@@@@#   %@                     \n                     @%      :@@@@@@@@@%     %@@%     %@@@@@@@@@:      %@                     \n                      .@=           -%-    +@@@@@@+    -%-           =@.                      \n                     -=+:    .-. +++-      +@@@@@@+      -+++ .-.    :+=-                     \n                     @%      :@#+          :------:           -@*+     %@                     \n                     @%   *%%%@=.%%%+                      #%%@@@@%#   %@                     \n                     @%      :@- @% -@@@@@@@@@@@@@@@@@@@@@@. @@@:      %@@@@@@:               \n               :@@@@@@@@=      +@@%     %#   %@     #@   +@. @+      =@@@     *@              \n              @*     @@@=        @@@+   %#   %@     #@   +@@@        =@.        @#            \n            #@        .%%@-.        -%%%@#...%@.....#@.-%%%.      .:%=     .=@%#..            \n          .-+*-----     -**+---.        ***************=      .---@#     --@@@=-*+-.          \n          .-+*@@@@@*****-   :=@#****=                    =****#@@@-:   +*@@@@@@@  :=*.        \n          -%@@@@@@@@@@@@%%:   .+@@@@%%%%%%%%%%%%%%%%%%%%%%@@@@*...   -%@@@@@@@@@%* ..#%       \n        .@+ #@@@@@@@@@@@@@@@@*    .@@@@%            #@@@@@@@@      :@@@@@@@ -@@@@@@- %@       \n       @% -@@@@@@@@  @@@@@@@@@@-      *@@#        -@@@@*      -@@@@@@@@@@  @@@@@@@@@@. @*     \n     %@ .@@@@@@@@@@  @@@@@@@@@@- @@@+  .%@@=      -@.      %@@@@@@@@@@@@@  @@@@@@@@@@.  :@:   \n   :@. @@@@@@@@@@@@  @@@@@@@@@@- @@@+ *@@@#######@@@@@ -@- %@@@@@@@@@@@##  @@@@@@@@@@@@:. *@  \n  =====@@@@@@@@@@@@  @@@@@@@@@@- ===- *@@#       =*@+= .=: %@@@@@@@@@#=.   @@@@@@@@@@@@@* -===\n  @* %@@@@@@@@@@@@@  --@+------. ***= *@@#        -@.  :*: %@@@@@@---. +*  @@@@@@@@@@@@@*   @@\n%%   %@@@@@@@@@@@@@     =%%%%%%%% .@@%-.%#        -@. %@@- %@@+    :%%%@@  @@@@@@@@@@@@@@%: @@\n@@   %@@@@@@@@@@@@@  @@@@@@@@@@@@ .@+   %#        -@.      %@ -@@@@@@@@@@   -@@@@@@@@@@@@@: @@\n@@   %@@@@@@@@@@@- @@@@@@@@@@@@@@   -@- %#        -@. %@@- %@ -@@@@@@@@@@@@ -@@@@@@@@@@@@@: @@\n  @*   @@@@@@@@@@- @@@@@@@@@@@@@@ .@@@- %#        -@. %@@- #@ -@@@@@@@@@@@@ -@@@@@@@@@@@@@: @@\n   :%:. .#%@@@@@@- @@@@@@@@@@@@@@.:#+.  %#        -@. .. +@. @@@@@@@@@@@@@@ -@@@@@@@@@@#+ ..#%\n     +*-- :+++@@@- @@@@@@@@@@@@@@@% -@- %#        -@. %* -+--@@@@@@@@@@@@@@ -@@@@@@@@*+ .-+*  \n       :-#=   @@@- @@@@@@@@@@@@@@@% -@- %@#########@. %* =#@@@@@@@@@@@@@@:: -@@@@@@+:  #+:.   \n         .-%: @@@- ...:@@@@@@@@@@@%  .  ............  .  +@@@@@@@@@@@@@:.   -@-....+%%%..     \n            #@@*        +@@@@@@@@@%        +@@@@@@@@@%                     @@@:  .@-          \n               :@@@               .@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@.   @+ *@@#            \n                   @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@                   \n                   ::**@@@#**#@@@@@@@@@@@@@@@@@@@@@@@@@@@@@.  -@@@@@@@@. @@                   \n                 -+=-++@@@:  :@@@@@@@@@@@@@@@@@@@@@@@@@@@@@.  -@@@@@@@@++-=+-                 \n                 +@  @@@+. .#%@@@@@@@@@@@@@@@:.@@@@@@@@@@@@.  -@@@@@@@@@@  @+                 \n                 +@  @@@=  .@@@@@@@@@@@@@@@- %@ .@@@@@@@@@@.  -@@@@@@@@@@  @+                 \n                 +@  @@@=  .@@@@@@@@@@@@@@@- %@ .@@@@@@@@@@.    *@@@@@@@@  @+                 \n                 +@  @@@=  .@@@@@@@@@@@@@@@- %@ .@@@@@@@@@@.    *@@@@@@@@@@ -@:               \n               :@- @@@%   #@@@@@@@@@@@@@@@@- %@@% -@@@@@@@@.    *@@@@@@@@@@ -@:               \n               :@- @@@%   #@@@@@@@@@@@@@@# ..*#@% -@@@@@@@@:.   *@@@@@@@@@@ -@:               \n               :@- @@@%   #@@@@@@@@@@@@@@# +@. @% -@@@@@@@@@@   *@@@@@@@@@@ -@:               \n               :@- @@@%   #@@@@@@@@@@@@@@# +@. @% -@@@@@@@@@@   *@@@@@@@@@@ -@:               \n               :@-        #@@@@@@@@@@@@@@# +@. @% -@@@@@@@@@@     @@@+      -@:               \n                 +@@@@@@@@:              .@-    .@+                  =@@@@@@+                 \n                          #@@@@@@@@@@@@@@#        -@@@@@@@@@@@@@@@@@@+                        \n              .:%%%%%%#...*%%%%%%..                    -@- #%%%%%%%%%=.....%%%%#...           \n            -==-      .=====     @%                   ===:         .=*@+===     =====.        \n          -#:.            ..#+   @%                   %*          #*...            ..#%       \n          -@:              .@@@@@@%                   %@@@@@@@@@@%                   %@       \n          -@:                :@-  .@+               #@          *%                 +@.        \n            #@@@@@@@@@@@@@@@@* +@@%                   %@@@@@@@@@: @@@@@@@@@@@@@@@@@-          \n",
    "@@@@@@@@@@  @@      @@  @@@@@@@@@@          @@@@@@      @@@@@@    @@      @@  @@@@@@@@@@  \n@@@@@@@@@@  @@      @@  @@@@@@@@@@          @@@@@@      @@@@@@    @@      @@  @@@@@@@@@@  \n    @@      @@      @@  @@                @@          @@      @@  @@@@  @@@@  @@          \n    @@      @@      @@  @@                @@          @@      @@  @@@@  @@@@  @@          \n    @@      @@@@@@@@@@  @@@@@@@@          @@    @@@@  @@@@@@@@@@  @@  @@  @@  @@@@@@@@    \n    @@      @@@@@@@@@@  @@@@@@@@          @@    @@@@  @@@@@@@@@@  @@  @@  @@  @@@@@@@@    \n    @@      @@      @@  @@                @@      @@  @@      @@  @@      @@  @@          \n    @@      @@      @@  @@                @@      @@  @@      @@  @@      @@  @@          \n    @@      @@      @@  @@@@@@@@@@          @@@@@@    @@      @@  @@      @@  @@@@@@@@@@  \n    @@      @@      @@  @@@@@@@@@@          @@@@@@    @@      @@  @@      @@  @@@@@@@@@@  \n"

};

static ssize_t lkcamp_read(struct file *file, char __user *buf, size_t size,
               loff_t *ppos)
{
    // Return the string corresponding to the current driver state
    return simple_read_from_buffer(buf, size, ppos, status_strings[status],
                       strlen(status_strings[status]));
}


static ssize_t lkcamp_write(struct file *file, const char __user *buf,
                size_t size, loff_t *ppos)
{
    char value;

    // Copy the first character written to this device to 'value'
    if (copy_from_user(&value, buf, 1))
        return -EFAULT; // Something went very wrong

    if (value == '1')
        status = STATUS1;
    else if (value == '2')
        status = STATUS2;
    else if (value == '3')
        status = STATUS3;
    else if (value == '4')
        status = STATUS4;
    else    
        status = STATUS0;

    return 1; // We only read one character from the written string
}

// Define the functions that implement our file operations
static struct file_operations lkcamp_fops =
{
    .read = lkcamp_read,
    .write = lkcamp_write,
};

static int __init lkcamp_init(void)
{
    int ret;

    // Allocate a major and a minor number
    ret = alloc_chrdev_region(&lkcamp_dev, 0, 1, "lkcamp");
    if (ret)
        pr_err("Failed to allocate device number\n");

    // Initialize our character device structure
    cdev_init(&lkcamp_cdev, &lkcamp_fops);

    // Register our character device to our device number
    ret = cdev_add(&lkcamp_cdev, lkcamp_dev, 1);
    if (ret)
        pr_err("Char device registration failed\n");

    pr_info("Escolha seu meme\n");

    return 0;
}

static void __exit lkcamp_exit(void)
{
    // Clean up our mess
    cdev_del(&lkcamp_cdev);
    unregister_chrdev_region(lkcamp_dev, 1);

    pr_info("Espero que tenha sido rickrollado\n");
}

module_init(lkcamp_init); // Register our functions so they get called when our
module_exit(lkcamp_exit); // module is loaded and unloaded

MODULE_AUTHOR("Irmoes");
MODULE_DESCRIPTION("Irmoes memes");
MODULE_LICENSE("GPL");
