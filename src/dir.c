/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info for license.
 */

#include <dirent.h>
#include <stdlib.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include "main.h"
#include "dir.h"
#include "string_util.h"
#include "ramdisk.h"

/*
 * Does the requested dir exist?
 */
uint8_t dir_exists (const char *indir)
{
    struct stat buf;
    char *dir;
    uint8_t rc;

    /*
     * msys is crap and unless you remove the / from the end of the name
     * will not think it is a dir.
     */
    dir = dupstr(indir, __FUNCTION__);
    strchopc(dir, DCHAR);

    rc = 0;
    if (stat(dir, &buf) >= 0) {
        rc = S_ISDIR(buf. st_mode);
    }

    myfree(dir);

    return (rc);
}

tree_root *dirlist (const char *dir,
                    const char *include_suffix,
                    const char *exclude_suffix,
                    uint8_t include_dirs,
                    uint8_t include_ramdisk)
{
    tree_file_node *node;
    struct dirent * e;
    tree_root *root;
    char *tmp;
    DIR * d;

    d = opendir(dir);
    if (!d) {
        MSG_BOX("Cannot list dir %s", dir);
        return (0);
    }

    root = tree_alloc(TREE_KEY_STRING, "TREE ROOT: dirlist");

    while ((e = readdir(d))) {
        char *dir_and_file;
        const char *name;
        struct stat s;

        name = e->d_name;

        if (!strcasecmp(name, ".")) {
            name = ".";
            dir_and_file = dupstr(name, "dirlist .");
        } else if (!strcasecmp(name, "..")) {
            name = "..";
            dir_and_file = dupstr(name, "dirlist ..");
        } else if (*name == '.') {
            continue;
        } else {
            char *tmp1;
            char *tmp2;

            tmp1 = strappend(dir, DSEP);
            tmp2 = strappend(tmp1, name);
            myfree(tmp1);

            dir_and_file = tmp2;
        }

        if (stat(dir_and_file, &s) < 0) {
            myfree(dir_and_file);
            continue;
        }

        if (!include_dirs) {
            if (exclude_suffix) {
                if (strstr(name, exclude_suffix)) {
                    myfree(dir_and_file);
                    continue;
                }
            }

            if (include_suffix) {
                if (!strstr(name, include_suffix)) {
                    myfree(dir_and_file);
                    continue;
                }
            }
        }

        /*
         * Get rid of any // from the path
         */
        tmp = strsub(dir_and_file, DSEP DSEP, DSEP);
        myfree(dir_and_file);
        dir_and_file = tmp;

        node = (typeof(node)) myzalloc(sizeof(*node), "TREE NODE: dirlist");

        node->is_file = !S_ISDIR(s.st_mode);

        /*
         * Static key so tree removals are easy.
         */
        node->tree.key = mybasename(dir_and_file, "TREE KEY: dirlist2");

        if (!tree_insert(root, &node->tree.node)) {
            ERR("insert dir %s", dir_and_file);
        }

        myfree(dir_and_file);
    }

    if (include_ramdisk) {
        extern ramdisk_t ramdisk_data[];

        ramdisk_t *ramfile;

        ramfile = ramdisk_data;

        while (ramfile->filename) {
            char *dir_and_file = dupstr(ramfile->filename, "ramdisk name");

            if (include_suffix) {
                if (!strstr(dir_and_file, include_suffix)) {
                    myfree(dir_and_file);
                    ramfile++;
                    continue;
                }
            }

            node = (typeof(node)) myzalloc(sizeof(*node), "TREE NODE: dirlist");
            node->is_file = true;

            /*
            * Static key so tree removals are easy.
            */
            node->tree.key = mybasename(dir_and_file, "TREE KEY: dirlist2");

            tree_insert(root, &node->tree.node);
            ramfile++;

            myfree(dir_and_file);
        }
    }

    closedir(d);

    return (root);
}

void dirlist_free (tree_root **root)
{
    tree_destroy(root, (tree_destroy_func)0);
}

/*
 * Given a directory, find out what ./ is.
 */
char *dir_dot (void)
{
    static char buf[MAXPATHLEN];

    dospath2unix(getcwd(buf, sizeof(buf)));

    return (buf);
}

/*
 * Remove \ and change to /
 *
 * Given a directory, find out what ../ is.
 */
char *dir_dotdot (char *s)
{
    uint32_t size;
    char *end;

    size = (uint32_t)strlen(s);
    if (size <= 1) {
        return (s);
    }

    /*
     * Skip trailing /
     */
    end = s + size - 2;
    while ((end >= s) && (*end != '/') && (*end != '\\')) {
        end--;
    }

    *(end + 1) = '\0';

    return (dospath2unix(s));
}

/*
 * Remove \ and change to /
 */
char *dospath2unix (char *in)
{
    char *s = in;

    while (*s) {
        if (*s == '\\') {
            *s = '/';
        }

        s++;
    }

    return (in);
}
