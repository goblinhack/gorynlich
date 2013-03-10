/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include <libgen.h>
#include <unistd.h>
#include <SDL.h>
#include "slre.h"

#include "main.h"
#include "gl.h"
#include "config.h"
#include "wid_console.h"
#include "command.h"
#include "color.h"
#include "ttf.h"
#include "font.h"
#include "math.h"
#include "tex.h"
#include "music.h"
#include "sound.h"
#include "tile.h"
#include "thing.h"
#include "thing_template.h"
#include "wid.h"
#include "sdl.h"
#include "wid_editor.h"
#include "wid_game_map.h"
#include "wid_intro_about.h"
#include "wid_intro_help.h"
#include "wid_intro_guide.h"
#include "wid_game_over.h"
#include "wid_intro_settings.h"
#include "wid_hiscore.h"
#include "wid_intro.h"
#include "string.h"
#include "dir.h"
#include "file.h"
#include "player.h"
#include "level.h"
#include "resource.h"
#include "init_fn.h"
#include "ramdisk.h"

static char **ARGV;
char *EXEC_FULL_PATH_AND_NAME;
char *EXEC_DIR;
char *DATA_PATH;
char *LEVELS_PATH;
char *TTF_PATH;
char *GFX_PATH;

void quit (void)
{
    static boolean quitting;

    if (quitting) {
        return;
    }

    quitting = true;

    FINI_LOG("%s", __FUNCTION__);

    sdl_exit();

    wid_editor_fini();
    wid_game_fini();

    level_fini();
    player_fini();
    thing_template_fini();
    thing_fini();

    wid_console_fini();
    wid_intro_fini();
    wid_intro_about_fini();
    wid_intro_help_fini();
    wid_intro_guide_fini();
    wid_game_over_fini();
    wid_intro_settings_fini();
    wid_hiscore_fini();

    command_fini();

    wid_fini();
    ttf_fini();
    font_fini();
    tex_fini();
    music_fini();
    sound_fini();
    tile_fini();
    sdl_fini();
    config_fini();
    enum_fmt_destroy();

    if (EXEC_FULL_PATH_AND_NAME) {
        myfree(EXEC_FULL_PATH_AND_NAME);
        EXEC_FULL_PATH_AND_NAME = 0;
    }

    if (DATA_PATH) {
        myfree(DATA_PATH);
        DATA_PATH = 0;
    }

    if (LEVELS_PATH) {
        myfree(LEVELS_PATH);
        LEVELS_PATH = 0;
    }

    if (TTF_PATH) {
        myfree(TTF_PATH);
        TTF_PATH = 0;
    }

    if (GFX_PATH) {
        myfree(GFX_PATH);
        GFX_PATH = 0;
    }

    if (!croaked) {
        ptrcheck_fini();
    }
}

void restart (void)
{
    char *args[] = { 0, 0 };
    char *executable = ARGV[0];

    LOG("Run %s", executable);

    args[0] = executable;

    execve(executable, args, 0);
}

void die (void)
{
    quit();

    fprintf(stderr, "exit(1) error\n");

    exit(1);
}

/*
 * Find the binary we are running.
 */
static void find_executable (void)
{
    char *parent_dir = 0;
    char *curr_dir = 0;
    char *exec_name = 0;
    char *exec_expanded_name = 0;
    char *path = 0;
    char *tmp;

    exec_name = mybasename(ARGV[0], __FUNCTION__);

    /*
     * Get the current directory, ending in a single /
     */
    curr_dir = dynprintf("%s" DSEP, dir_dot());
    tmp = strsub(curr_dir, DSEP DSEP, DSEP);
    myfree(curr_dir);
    curr_dir = tmp;

    /*
     * Get the parent directory, ending in a single /
     */
    parent_dir = dynprintf("%s" DSEP, dir_dotdot(dir_dot()));
    tmp = strsub(parent_dir, DSEP DSEP, DSEP);
    myfree(parent_dir);
    parent_dir = tmp;

    /*
     * Get rid of ../ from the program name and replace with the path.
     */
    exec_expanded_name = dupstr(ARGV[0], __FUNCTION__);
    if (*exec_expanded_name == '.') {
        tmp = strsub(exec_expanded_name, ".." DSEP, parent_dir);
        myfree(exec_expanded_name);
        exec_expanded_name = tmp;
    }

    /*
     * Get rid of ./ from the program name.
     */
    if (*exec_expanded_name == '.') {
        tmp = strsub(exec_expanded_name, "." DSEP, "");
        myfree(exec_expanded_name);
        exec_expanded_name = tmp;
    }

    /*
     * Get rid of any // from th path
     */
    tmp = strsub(exec_expanded_name, DSEP DSEP, DSEP);
    myfree(exec_expanded_name);
    exec_expanded_name = tmp;

    /*
     * Look in the simplest case first.
     */
    EXEC_FULL_PATH_AND_NAME = dynprintf("%s%s", curr_dir, exec_name);
    if (file_exists(EXEC_FULL_PATH_AND_NAME)) {
        EXEC_DIR = dupstr(curr_dir, "exec dir 1");
        goto cleanup;
    }

    myfree(EXEC_FULL_PATH_AND_NAME);

    /*
     * Try the parent dir.
     */
    EXEC_FULL_PATH_AND_NAME = dynprintf("%s%s", parent_dir, exec_name);
    if (file_exists(EXEC_FULL_PATH_AND_NAME)) {
        EXEC_DIR = dupstr(parent_dir, "exec dir 2");
        goto cleanup;
    }

    myfree(EXEC_FULL_PATH_AND_NAME);

    /*
     * Try the PATH.
     */
    path = getenv("PATH");
    if (path) {
        char *dir = 0;

        path = dupstr(path, "path");

        for (dir = strtok(path, ":"); dir; dir = strtok(0, ":")) {
            EXEC_FULL_PATH_AND_NAME = dynprintf("%s" DSEP "%s", dir, exec_name);
            if (file_exists(EXEC_FULL_PATH_AND_NAME)) {
                EXEC_DIR = dynprintf("%s" DSEP, dir);
                goto cleanup;
            }

            myfree(EXEC_FULL_PATH_AND_NAME);
        }

        myfree(path);
        path = 0;
    }

    EXEC_FULL_PATH_AND_NAME = dupstr(exec_expanded_name, "full path");
    EXEC_DIR = dupstr(dirname(exec_expanded_name), "exec dir");

cleanup:
    INIT_LOG("Exec name   : \"%s\"", exec_name);
    INIT_LOG("Parent dir  : \"%s\"", parent_dir);
    INIT_LOG("Curr dir    : \"%s\"", curr_dir);
    INIT_LOG("Full name   : \"%s\"", exec_expanded_name);

    if (path) {
        myfree(path);
    }

    if (exec_name) {
        myfree(exec_name);
    }

    if (exec_expanded_name) {
        myfree(exec_expanded_name);
    }

    if (parent_dir) {
        myfree(parent_dir);
    }
}

/*
 * Find all installed file locations.
 */
static void find_exec_dir (void)
{
    char *tmp;

    find_executable();

    /*
     * Make sure the exec dir ends in a /
     */
    tmp = dynprintf("%s" DSEP, EXEC_DIR);
    EXEC_DIR = strsub(tmp, DSEP DSEP, DSEP);
    myfree(tmp);
}

/*
 * Hunt down the data/ dir.
 */
static void find_data_dir (void)
{
    DATA_PATH = dynprintf("%sdata" DSEP, EXEC_DIR);
    if (dir_exists(DATA_PATH)) {
        return;
    }

    myfree(DATA_PATH);

    DATA_PATH = dupstr(EXEC_DIR, __FUNCTION__);
}

/*
 * Hunt down the level/ dir.
 */
static void find_level_dir (void)
{
    LEVELS_PATH = dynprintf("%sdata" DSEP "levels" DSEP, EXEC_DIR);
    if (dir_exists(LEVELS_PATH)) {
        return;
    }

    myfree(LEVELS_PATH);

    LEVELS_PATH = dupstr(EXEC_DIR, __FUNCTION__);
}

/*
 * Hunt down the ttf/ dir.
 */
static void find_ttf_dir (void)
{
    TTF_PATH = dynprintf("%sdata" DSEP "ttf" DSEP, EXEC_DIR);
    if (dir_exists(TTF_PATH)) {
        return;
    }

    myfree(TTF_PATH);

    TTF_PATH = dupstr(EXEC_DIR, __FUNCTION__);
}

/*
 * Hunt down the gfx/ dir.
 */
static void find_gfx_dir (void)
{
    GFX_PATH = dynprintf("%sdata" DSEP "gfx" DSEP, EXEC_DIR);
    if (dir_exists(GFX_PATH)) {
        return;
    }

    myfree(GFX_PATH);

    GFX_PATH = dupstr(EXEC_DIR, __FUNCTION__);
}

/*
 * Find all installed file locations.
 */
static void find_file_locations (void)
{
    find_exec_dir();
    find_data_dir();
    find_level_dir();
    find_ttf_dir();
    find_gfx_dir();

    INIT_LOG("Exec dir    : \"%s\"", EXEC_DIR);
    INIT_LOG("Executable  : \"%s\"", EXEC_FULL_PATH_AND_NAME);
    INIT_LOG("Data path   : \"%s\"", DATA_PATH);
    INIT_LOG("Level path  : \"%s\"", LEVELS_PATH);
    INIT_LOG("Gfx path    : \"%s\"", GFX_PATH);
    INIT_LOG("Font path   : \"%s\"", TTF_PATH);
}

int32_t main (int32_t argc, char *argv[])
{
    ramdisk_init();

    ARGV = argv;

    dospath2unix(ARGV[0]);

#ifdef PTRCHECK_TEST
    extern int ptrcheck_test(int32_t argc, char *argv[]);
    ptrcheck_test(argc, argv);
    quit();
    exit(0);
#endif

#ifdef SLRE_TEST
    extern void slre_test(void);
    slre_test();
    exit(0);
#endif

#ifdef TREE_TEST
    extern void tree_debug_test(void);
    tree_debug_test();
    exit(0);
#endif

#ifdef MINIZ_TEST
    extern int miniz_test(int32_t argc, char *argv[]);
    miniz_test(argc, argv);
    exit(0);
#endif

#ifdef ZIP_FILE_TEST
    extern int mzip_file_test(int32_t argc, char *argv[]);
    mzip_file_test(argc, argv);
    exit(0);
#endif

#ifdef DIR_TEST
    extern int dir_test(int32_t argc, char *argv[]);
    dir_test(argc, argv);
    exit(0);
#endif

#ifdef STRING_TEST
    extern int string_test(int32_t argc, char *argv[]);
    string_test(argc, argv);
    exit(0);
#endif

#ifdef MARSHAL_TEST
    extern int marshal_test(int32_t argc, char *argv[]);
    marshal_test(argc, argv);
    exit(0);
#endif

#ifdef ENUM_TEST
    extern int enum_test(int32_t argc, char *argv[]);
    enum_test(argc, argv);
    exit(0);
#endif

#if 0
#ifndef __IPHONE_OS_VERSION_MIN_REQUIRED
    if (!freopen("stdout.txt", "w", stdout)) {
	ERR("Failed to open stdout.txt");
    }

    if (!freopen("stderr.txt", "w", stderr)) {
	ERR("Failed to open stderr.txt");
    }
#endif
#endif

    math_init();

    color_init();

    find_file_locations();

    if (!config_init()) {
	DIE("Config init");
    }

    if (!config_load()) {
	DIE("Config read");
    }

    if (!sdl_init()) {
	DIE("SDL init");
    }

    if (!tex_init()) {
	DIE("tex init");
    }

    if (!music_init()) {
	DIE("music init");
    }

    if (!sound_init()) {
	DIE("sound init");
    }

    if (!tile_init()) {
	DIE("tile init");
    }

    if (!ttf_init()) {
	DIE("TTF init");
    }

    if (!font_init()) {
	DIE("Font init");
    }

    if (!wid_init()) {
	DIE("wid init");
    }

    if (!wid_hiscore_init()) {
	DIE("hiscore init");
    }

    ptrcheck_leak_snapshot();

#ifndef WID_TEST
    if (!wid_console_init()) {
	DIE("wid_console init");
    }

    if (!command_init()) {
	DIE("command init");
    }
#endif

    if (!ptrcheck_init()) {
	DIE("ptrcheck init");
    }

    action_init_fn_create(&init_fns,
                          (action_init_fn_callback)level_init,
                          0, "level_init");

    action_init_fn_create(&init_fns,
                          (action_init_fn_callback)player_init,
                          0, "player_init");

    action_init_fn_create(&init_fns,
                          (action_init_fn_callback)thing_template_init,
                          0, "thing_template_init");

    action_init_fn_create(&init_fns,
                          (action_init_fn_callback)thing_init,
                          0, "thing_init");

    if (!resource_init()) {
	DIE("resource init");
    }

    action_init_fn_create(&init_fns,
                          (action_init_fn_callback)wid_editor_init,
                          0, "wid_editor_init");

    action_init_fn_create(&init_fns,
                          (action_init_fn_callback)wid_game_init,
                          0, "wid_game_init");

#ifdef THING_TEST
    extern int thing_test(int32_t argc, char *argv[]);
    thing_test(argc, argv);
    exit(0);
#endif

#ifdef WID_TEST
    extern void wid_test_wid_create(void);
    wid_test_wid_create();
#else
#if 1
    action_init_fn_create(&init_fns,
                          (action_init_fn_callback)wid_intro_about_init,
                          0, "wid_intro_about_init");

    action_init_fn_create(&init_fns,
                          (action_init_fn_callback)wid_intro_init,
                          0, "wid_intro_init");

    action_init_fn_create(&init_fns,
                          (action_init_fn_callback)wid_game_over_init,
                          0, "wid_intro_guide_init");

    action_init_fn_create(&init_fns,
                          (action_init_fn_callback)wid_intro_guide_init,
                          0, "wid_intro_guide_init");

    action_init_fn_create(&init_fns,
                          (action_init_fn_callback)wid_intro_settings_init,
                          0, "wid_intro_settings_init");

    action_init_fn_create(&init_fns,
                          (action_init_fn_callback)wid_intro_help_init,
                          0, "wid_intro_help_init");
#endif
#endif

    gl_enter_2d_mode();
    sdl_loop();
    gl_leave_2d_mode();

    quit();

    fprintf(stderr, "exit(0) ok\n");

    return (0);
}

