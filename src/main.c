/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include <libgen.h>
#include <unistd.h>
#include <SDL.h>
#include <SDL_net.h>
#include "slre.h"

#include "main.h"
#include "gl.h"
#include "wid_console.h"
#include "wid_editor.h"
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
#include "map.h"
#include "net.h"
#include "client.h"
#include "term.h"
#include "server.h"

static char **ARGV;
char *EXEC_FULL_PATH_AND_NAME;
char *EXEC_DIR;
char *DATA_PATH;
char *LEVELS_PATH;
char *TTF_PATH;
char *GFX_PATH;

FILE *LOG_STDOUT;
FILE *LOG_STDERR;

boolean quitting;

void quit (void)
{
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

    net_fini();
    server_fini();
    client_fini();
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

#ifdef ENABLE_LEAKCHECK
    if (!croaked) {
        ptrcheck_fini();
    }
#endif

    term_fini();
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

    term_fini();

    fprintf(MY_STDERR, "exit(1) error\n");

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
    DBG("Exec name   : \"%s\"", exec_name);
    DBG("Parent dir  : \"%s\"", parent_dir);
    DBG("Curr dir    : \"%s\"", curr_dir);
    DBG("Full name   : \"%s\"", exec_expanded_name);

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
    DBG("Level path  : \"%s\"", LEVELS_PATH);
    DBG("Gfx path    : \"%s\"", GFX_PATH);
    DBG("Font path   : \"%s\"", TTF_PATH);
}

static void usage (void)
{
    fprintf(MY_STDERR, "Gorynlich, options:\n");
    fprintf(MY_STDERR, "\n");
    fprintf(MY_STDERR, "        --server   run as a server\n");
    fprintf(MY_STDERR, "        -server\n");
    fprintf(MY_STDERR, "        -s\n");
    fprintf(MY_STDERR, "\n");
    fprintf(MY_STDERR, "        --client   run as a client\n");
    fprintf(MY_STDERR, "        -client\n");
    fprintf(MY_STDERR, "        -c\n");
    fprintf(MY_STDERR, "\n");
    fprintf(MY_STDERR, "        --host     hostname or ip of server\n");
    fprintf(MY_STDERR, "        -host\n");
    fprintf(MY_STDERR, "        -h\n");
    fprintf(MY_STDERR, "\n");
    fprintf(MY_STDERR, "        --port     port of server, default %d\n",
            SERVER_DEFAULT_PORT);
    fprintf(MY_STDERR, "        -port\n");
    fprintf(MY_STDERR, "        -p\n");
    fprintf(MY_STDERR, "\n");
    fprintf(MY_STDERR, "Written by Neil McGill, goblinhack@gmail.com\n");
}

static void parse_args (int32_t argc, char *argv[])
{
    const char *host = SERVER_DEFAULT_HOST;
    uint16_t port = SERVER_DEFAULT_PORT;
    int32_t i;

    LOG("Hello");

    is_server = false;
    is_client = false;

    /*
     * Parse format args
     */
    for (i = 1; i < argc; i++) {
        /*
         * -server
         */
        if (!strcmp(argv[i], "--server") ||
            !strcmp(argv[i], "-server") ||
            !strcmp(argv[i], "-s")) {

            is_server = true;
            continue;
        }

        /*
         * -connect
         */
        if (!strcmp(argv[i], "--connect") ||
            !strcmp(argv[i], "-connect") ||
            !strcmp(argv[i], "-c")) {

            is_client = true;
            continue;
        }

        /*
         * -host
         */
        if (!strcmp(argv[i], "--host") ||
            !strcmp(argv[i], "-host") ||
            !strcmp(argv[i], "-s")) {

            host = argv[i + 1];
            i++;
            continue;
        }

        /*
         * -port
         */
        if (!strcmp(argv[i], "--port") ||
            !strcmp(argv[i], "-port") ||
            !strcmp(argv[i], "-s")) {

            port = atoi(argv[i + 1]);
            i++;
            continue;
        }

        /*
         * Bad argument.
         */
        if (argv[i][0] == '-') {
            usage();
            DIE("unknown format argument, %s", argv[i]);
        }

        usage();
        DIE("unknown format argument, %s", argv[i]);
    }

    if (!is_server && !is_client) {
        is_client = true;
    }

    LOG("Trying to resolve %s:%u", host, port);

    if (SDLNet_ResolveHost(&server_address, host, port)) {
        DIE("cannot resolve host %s port %u", host, port);
    }
}

int32_t main (int32_t argc, char *argv[])
{
    LOG_STDOUT = fopen("stdout.txt", "w+");
    LOG_STDERR = fopen("stderr.txt", "w+");

#ifdef ENABLE_LOGFILE
#ifndef __IPHONE_OS_VERSION_MIN_REQUIRED
    if (!freopen("stdout.txt", "w", MY_STDOUT)) {
	WARN("Failed to open MY_STDOUT.txt");
    }

    if (!freopen("stderr.txt", "w", MY_STDERR)) {
	WARN("Failed to open MY_STDERR.txt");
    }
#endif
#endif

    ramdisk_init();

    ARGV = argv;

    dospath2unix(ARGV[0]);

    term_init();

    parse_args(argc, argv);

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

#ifdef MAP_TEST
    extern int32_t map_jigsaw_test(int32_t argc, char **argv);
    map_jigsaw_test(argc, argv);
#endif

#ifdef TERM_TEST
    extern int term_test(int32_t argc, char *argv[]);
    term_test(argc, argv);
    quit();
    exit(0);
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

#ifdef ENABLE_LEAKCHECK
    ptrcheck_leak_snapshot();
#endif

#ifndef WID_TEST
    if (!wid_console_init()) {
	DIE("wid_console init");
    }

    if (!command_init()) {
	DIE("command init");
    }
#endif

#ifdef ENABLE_LEAKCHECK
    if (!ptrcheck_init()) {
	DIE("ptrcheck init");
    }
#endif

    if (!resource_init()) {
	DIE("resource init");
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

    action_init_fn_create(&init_fns,
                          (action_init_fn_callback)wid_game_init,
                          0, "wid_game_init");

    action_init_fn_create(&init_fns,
                          (action_init_fn_callback)wid_editor_init,
                          0, "wid_editor_init");

#ifdef THING_TEST
    extern int thing_test(int32_t argc, char *argv[]);
    thing_test(argc, argv);
    exit(0);
#endif

#ifdef WID_TEST
    extern void wid_test_wid_create(void);
    wid_test_wid_create();
#else
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

    net_init();
    server_init();
    client_init();

    gl_enter_2d_mode();
    sdl_loop();
    gl_leave_2d_mode();

    quit();

    fprintf(MY_STDOUT, "Exited\n");

    return (0);
}
