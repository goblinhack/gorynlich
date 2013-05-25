/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#include <SDL.h>

#include "main.h"
#include "tree.h"
#include "thing.h"
#include "item_private.h"
#include "item.h"
#include "thing_template.h"
#include "string.h"

const char *item_name (itemp p)
{
    return (thing_name(p->tree.thing));
}

const char *item_shortname (itemp p)
{
    return (thing_shortname(p->tree.thing));
}

thingp item_thing (itemp i)
{
    verify(i->tree.thing);

    return (i->tree.thing);
}

void items_destroy (tree_rootp *root)
{
    itemp p;

    if (!root) {
        DIE("no items to destroy");
    }

    TREE_WALK((*root), p) {
        item_destroy(root, p);
        if (!*root) {
            return;
        }
    }

    if (*root) {
        tree_destroy(root, 0);
    }
}

void item_destroy (tree_rootp *t, itemp p)
{
    if (!t) {
        return;
    }

    ITEM_LOG(p, "destroyed");

    if (!tree_remove(*t, &p->tree.node)) {
        DIE("item destroy name [%s] failed", item_name(p));
    }

    if (p->logname) {
        myfree(p->logname);
    }

    myfree(p);

    if (!tree_root_size(*t)) {
        tree_destroy(t, 0);
    }
}

static int32_t item_compare (const tree_node *a, const tree_node *b)
{
    itemp A = (typeof(A))a;
    itemp B = (typeof(B))b;
    thingp tA = A->tree.thing;
    thingp tB = B->tree.thing;
    thing_templatep pA = thing_get_template(tA);
    thing_templatep pB = thing_get_template(tB);

    verify(tA);
    verify(tB);

    if (pA > pB) {
        return (-1);
    }

    if (pA < pB) {
        return (1);
    }

    if (tA > tB) {
        return (-1);
    }

    if (tA < tB) {
        return (1);
    }

    return (0);
}

itemp item_find (tree_rootp root, thingp thing)
{
    item target;
    itemp p;

    memset(&target, 0, sizeof(target));
    target.tree.thing = thing;

    p = (typeof(p)) tree_find(root, &target.tree.node);

    return (p);
}

itemp item_find_template (tree_rootp root, thing_templatep thing_template)
{
    itemp p;

    TREE_WALK(root, p) {

        if (thing_get_template(item_thing(p)) == thing_template) {
            return (p);
        }
    }

    return (0);
}

itemp item_collect (tree_rootp *root, thingp thing, thingp owner)
{
    item *p;

    verify(thing);

    if (!*root) {
        *root = tree_alloc_custom(item_compare, "TREE ROOT: items");
    }

    p = item_find(*root, thing);
    if (p) {
        DIE("item [%s] exists", item_name(p));
        return (p);
    }

    p = (typeof(p)) myzalloc(sizeof(*p), "TREE NODE: item");
    p->tree.thing = thing;

    if (!tree_insert(*root, &p->tree.node)) {
        DIE("collect item [%s] failed", item_name(p));
    }

    p->logname = dynprintf("%s[%p] owner:%s", item_shortname(p), p,
                           thing_logname(owner));

    ITEM_LOG(p, "created");

    return (p);
}

boolean item_drop (tree_rootp *root, thingp thing)
{
    item *p;

    p = item_find(*root, thing);
    if (!p) {
        return (false);
    }

    item_destroy(root, p);

    return (true);
}

itemp item_get_first (tree_rootp root)
{
    itemp item;

    item = (typeof(item)) tree_root_first(root);

    if (item) {
        verify(item);
    }

    return (item);
}

itemp item_get_last (tree_rootp root)
{
    itemp item;

    item = (typeof(item)) tree_root_last(root);

    if (item) {
        verify(item);
    }

    return (item);
}

itemp item_get_next (tree_rootp root, itemp in)
{
    item cursor;
    itemp next;

    verify(in);

    memset(&cursor, 0, sizeof(cursor));
    cursor.tree.thing = in->tree.thing;

    next = (typeof(next)) tree_get_next(root,
                                        tree_root_top(root),
                                        &cursor.tree.node);
    if (next) {
        verify(next);
    }

    return (next);
}

itemp item_get_prev (tree_rootp root, itemp in)
{
    item cursor;
    itemp prev;

    verify(in);

    memset(&cursor, 0, sizeof(cursor));
    cursor.tree.thing = in->tree.thing;

    prev = (typeof(prev)) tree_get_prev(root,
                                        tree_root_top(root),
                                        &cursor.tree.node);
    if (prev) {
        verify(prev);
    }

    return (prev);
}

const char *item_logname (itemp t)
{
    verify(t);

    return (t->logname);
}

typedef boolean (*item_is_callback)(thing_templatep);

static boolean items_has_x (tree_rootp tree, item_is_callback func)
{
    thing_templatep thing_template;
    itemp item;

    item = item_get_first(tree);

    while (item) {
        thing_template = thing_get_template(item_thing(item));

        if ((*func)(thing_template)) {
            return (true);
        }

        item = item_get_next(tree, item);
    }

    return (false);
}

boolean items_has_player (tree_rootp root)
{
    return (items_has_x(root, thing_template_is_player));
}

boolean items_has_monst (tree_rootp root)
{
    return (items_has_x(root, thing_template_is_monst));
}

boolean items_has_snail (tree_rootp root)
{
    return (items_has_x(root, thing_template_is_snail));
}

boolean items_has_wall (tree_rootp root)
{
    return (items_has_x(root, thing_template_is_wall));
}

boolean items_has_star (tree_rootp root)
{
    return (items_has_x(root, thing_template_is_star_yellow));
}

boolean items_has_rock (tree_rootp root)
{
    return (items_has_x(root, thing_template_is_rock));
}

boolean items_has_xxx2 (tree_rootp root)
{
    return (items_has_x(root, thing_template_is_xxx2));
}

boolean items_has_xxx3 (tree_rootp root)
{
    return (items_has_x(root, thing_template_is_xxx3));
}

boolean items_has_xxx4 (tree_rootp root)
{
    return (items_has_x(root, thing_template_is_xxx4));
}

boolean items_has_xxx5 (tree_rootp root)
{
    return (items_has_x(root, thing_template_is_xxx5));
}

boolean items_has_xxx6 (tree_rootp root)
{
    return (items_has_x(root, thing_template_is_xxx6));
}

boolean items_has_xxx7 (tree_rootp root)
{
    return (items_has_x(root, thing_template_is_xxx7));
}

boolean items_has_xxx8 (tree_rootp root)
{
    return (items_has_x(root, thing_template_is_xxx8));
}

boolean items_has_is_star (tree_rootp root)
{
    return (items_has_x(root, thing_template_is_star));
}

boolean items_has_powerup_spam (tree_rootp root)
{
    return (items_has_x(root, thing_template_is_powerup_spam));
}

boolean items_has_rock1 (tree_rootp root)
{
    return (items_has_x(root, thing_template_is_rock1));
}

boolean items_has_car (tree_rootp root)
{
    return (items_has_x(root, thing_template_is_car));
}

boolean items_has_star_green (tree_rootp root)
{
    return (items_has_x(root, thing_template_is_star_green));
}

boolean items_has_star_cyan (tree_rootp root)
{
    return (items_has_x(root, thing_template_is_star_cyan));
}

boolean items_has_star_black (tree_rootp root)
{
    return (items_has_x(root, thing_template_is_star_black));
}

boolean items_has_star_purple (tree_rootp root)
{
    return (items_has_x(root, thing_template_is_star_purple));
}

boolean items_has_explosion (tree_rootp root)
{
    return (items_has_x(root, thing_template_is_explosion));
}

boolean items_has_spikes (tree_rootp root)
{
    return (items_has_x(root, thing_template_is_spikes));
}

boolean items_has_is_star_red (tree_rootp root)
{
    return (items_has_x(root, thing_template_is_star_red));
}

boolean items_has_is_star_pink (tree_rootp root)
{
    return (items_has_x(root, thing_template_is_star_pink));
}

boolean items_has_is_star_blue (tree_rootp root)
{
    return (items_has_x(root, thing_template_is_star_blue));
}

boolean items_has_seedpod (tree_rootp root)
{
    return (items_has_x(root, thing_template_is_seedpod));
}

boolean items_has_bomb (tree_rootp root)
{
    return (items_has_x(root, thing_template_is_bomb));
}

boolean items_has_spam (tree_rootp root)
{
    return (items_has_x(root, thing_template_is_spam));
}

boolean items_has_road (tree_rootp root)
{
    return (items_has_x(root, thing_template_is_road));
}

boolean items_has_pipe (tree_rootp root)
{
    return (items_has_x(root, thing_template_is_pipe));
}

boolean items_has_xxx27 (tree_rootp root)
{
    return (items_has_x(root, thing_template_is_item_removed_at_level_end));
}

boolean items_has_xxx28 (tree_rootp root)
{
    return (items_has_x(root, thing_template_is_scarable));
}

boolean items_has_is_shrunk_when_carried (tree_rootp root)
{
    return (items_has_x(root, thing_template_is_shrunk_when_carried));
}

boolean items_has_hidden_from_editor (tree_rootp root)
{
    return (items_has_x(root, thing_template_is_hidden_from_editor));
}

boolean items_has_rocket (tree_rootp root)
{
    return (items_has_x(root, thing_template_is_powerup_rocket));
}

boolean items_has_is_left_as_corpse_on_death (tree_rootp root)
{
    return (items_has_x(root, thing_template_is_left_as_corpse_on_death));
}

boolean items_has_item_perma (tree_rootp root)
{
    return (items_has_x(root, thing_template_is_item_perma));
}

boolean items_has_esnail (tree_rootp root)
{
    return (items_has_x(root, thing_template_is_esnail));
}

boolean items_has_is_item_hidden (tree_rootp root)
{
    return (items_has_x(root, thing_template_is_item_hidden));
}

boolean items_has_bonus_letter (tree_rootp root)
{
    return (items_has_x(root, thing_template_is_bonus_letter));
}

boolean items_has_is_thing (tree_rootp root)
{
    return (items_has_x(root, thing_template_is_thing));
}

boolean items_has_plant (tree_rootp root)
{
    return (items_has_x(root, thing_template_is_plant));
}

boolean items_has_food (tree_rootp root)
{
    return (items_has_x(root, thing_template_is_food));
}

boolean items_has_floor (tree_rootp root)
{
    return (items_has_x(root, thing_template_is_floor));
}

boolean items_has_exit (tree_rootp root)
{
    return (items_has_x(root, thing_template_is_exit));
}

boolean items_has_letter (tree_rootp root)
{
    return (items_has_x(root, thing_template_is_letter));
}

uint32_t items_count_is_thing_template (tree_rootp tree, thing_templatep in)
{
    thing_templatep thing_template;
    uint32_t count;
    itemp item;

    count = 0;
    item = item_get_first(tree);

    while (item) {
        thing_template = thing_get_template(item_thing(item));

        if (thing_template == in) {
            count++;
        }

        item = item_get_next(tree, item);
    }

    return (count);
}

static uint32_t items_count_is_x (tree_rootp tree, item_is_callback func)
{
    thing_templatep thing_template;
    itemp item;
    uint32_t count;

    count = 0;
    item = item_get_first(tree);

    while (item) {
        thing_template = thing_get_template(item_thing(item));

        if ((*func)(thing_template)) {
            count++;
        }

        item = item_get_next(tree, item);
    }

    return (count);
}

uint32_t items_count_is_player (tree_rootp root)
{
    return (items_count_is_x(root, thing_template_is_player));
}

uint32_t items_count_is_monst (tree_rootp root)
{
    return (items_count_is_x(root, thing_template_is_monst));
}

uint32_t items_count_is_snail (tree_rootp root)
{
    return (items_count_is_x(root, thing_template_is_snail));
}

uint32_t items_count_is_wall (tree_rootp root)
{
    return (items_count_is_x(root, thing_template_is_wall));
}

uint32_t items_count_is_star_yellow (tree_rootp root)
{
    return (items_count_is_x(root, thing_template_is_star_yellow));
}

uint32_t items_count_is_rock (tree_rootp root)
{
    return (items_count_is_x(root, thing_template_is_rock));
}

uint32_t items_count_is_xxx2 (tree_rootp root)
{
    return (items_count_is_x(root, thing_template_is_xxx2));
}

uint32_t items_count_is_xxx3 (tree_rootp root)
{
    return (items_count_is_x(root, thing_template_is_xxx3));
}

uint32_t items_count_is_xxx4 (tree_rootp root)
{
    return (items_count_is_x(root, thing_template_is_xxx4));
}

uint32_t items_count_is_xxx5 (tree_rootp root)
{
    return (items_count_is_x(root, thing_template_is_xxx5));
}

uint32_t items_count_is_xxx6 (tree_rootp root)
{
    return (items_count_is_x(root, thing_template_is_xxx6));
}

uint32_t items_count_is_xxx7 (tree_rootp root)
{
    return (items_count_is_x(root, thing_template_is_xxx7));
}

uint32_t items_count_is_xxx8 (tree_rootp root)
{
    return (items_count_is_x(root, thing_template_is_xxx8));
}

uint32_t items_count_is_star (tree_rootp root)
{
    return (items_count_is_x(root, thing_template_is_star));
}

uint32_t items_count_is_powerup_spam (tree_rootp root)
{
    return (items_count_is_x(root, thing_template_is_powerup_spam));
}

uint32_t items_count_is_rock1 (tree_rootp root)
{
    return (items_count_is_x(root, thing_template_is_rock1));
}

uint32_t items_count_is_car (tree_rootp root)
{
    return (items_count_is_x(root, thing_template_is_car));
}

uint32_t items_count_is_star_green (tree_rootp root)
{
    return (items_count_is_x(root, thing_template_is_star_green));
}

uint32_t items_count_is_star_cyan (tree_rootp root)
{
    return (items_count_is_x(root, thing_template_is_star_cyan));
}

uint32_t items_count_is_star_black (tree_rootp root)
{
    return (items_count_is_x(root, thing_template_is_star_black));
}

uint32_t items_count_is_star_purple (tree_rootp root)
{
    return (items_count_is_x(root, thing_template_is_star_purple));
}

uint32_t items_count_is_explosion (tree_rootp root)
{
    return (items_count_is_x(root, thing_template_is_explosion));
}

uint32_t items_count_is_spikes (tree_rootp root)
{
    return (items_count_is_x(root, thing_template_is_spikes));
}

uint32_t items_count_is_star_red (tree_rootp root)
{
    return (items_count_is_x(root, thing_template_is_star_red));
}

uint32_t items_count_is_star_pink (tree_rootp root)
{
    return (items_count_is_x(root, thing_template_is_star_pink));
}

uint32_t items_count_is_star_blue (tree_rootp root)
{
    return (items_count_is_x(root, thing_template_is_star_blue));
}

uint32_t items_count_is_seedpod (tree_rootp root)
{
    return (items_count_is_x(root, thing_template_is_seedpod));
}

uint32_t items_count_is_bomb (tree_rootp root)
{
    return (items_count_is_x(root, thing_template_is_bomb));
}

uint32_t items_count_is_spam (tree_rootp root)
{
    return (items_count_is_x(root, thing_template_is_spam));
}

uint32_t items_count_is_road (tree_rootp root)
{
    return (items_count_is_x(root, thing_template_is_road));
}

uint32_t items_count_is_pipe (tree_rootp root)
{
    return (items_count_is_x(root, thing_template_is_pipe));
}

uint32_t items_count_is_item_removed_at_level_end (tree_rootp root)
{
    return (items_count_is_x(root, thing_template_is_item_removed_at_level_end));
}

uint32_t items_count_is_scarable (tree_rootp root)
{
    return (items_count_is_x(root, thing_template_is_scarable));
}

uint32_t items_count_is_shrunk_when_carried (tree_rootp root)
{
    return (items_count_is_x(root, thing_template_is_shrunk_when_carried));
}

uint32_t items_count_is_hidden_from_editor (tree_rootp root)
{
    return (items_count_is_x(root, thing_template_is_hidden_from_editor));
}

uint32_t items_count_is_powerup_rocket (tree_rootp root)
{
    return (items_count_is_x(root, thing_template_is_powerup_rocket));
}

uint32_t items_count_is_left_as_corpse_on_death (tree_rootp root)
{
    return (items_count_is_x(root, thing_template_is_left_as_corpse_on_death));
}

uint32_t items_count_is_item_perma (tree_rootp root)
{
    return (items_count_is_x(root, thing_template_is_item_perma));
}

uint32_t items_count_is_esnail (tree_rootp root)
{
    return (items_count_is_x(root, thing_template_is_esnail));
}

uint32_t items_count_is_item_is_hidden (tree_rootp root)
{
    return (items_count_is_x(root, thing_template_is_item_hidden));
}

uint32_t items_count_is_bonus_letter (tree_rootp root)
{
    return (items_count_is_x(root, thing_template_is_bonus_letter));
}

uint32_t items_count_is_thing (tree_rootp root)
{
    return (items_count_is_x(root, thing_template_is_thing));
}

uint32_t items_count_is_plant (tree_rootp root)
{
    return (items_count_is_x(root, thing_template_is_plant));
}

uint32_t items_count_is_food (tree_rootp root)
{
    return (items_count_is_x(root, thing_template_is_food));
}

uint32_t items_count_is_floor (tree_rootp root)
{
    return (items_count_is_x(root, thing_template_is_floor));
}

uint32_t items_count_is_exit (tree_rootp root)
{
    return (items_count_is_x(root, thing_template_is_exit));
}

uint32_t items_count_is_letter (tree_rootp root)
{
    return (items_count_is_x(root, thing_template_is_letter));
}

static itemp items_first_is_x (tree_rootp tree, item_is_callback func)
{
    thing_templatep thing_template;
    itemp item;

    item = item_get_first(tree);

    while (item) {
        thing_template = thing_get_template(item_thing(item));

        if ((*func)(thing_template)) {
            return (item);
        }

        item = item_get_next(tree, item);
    }

    return (0);
}

itemp items_first_is_player (tree_rootp root)
{
    return (items_first_is_x(root, thing_template_is_player));
}

itemp items_first_is_monst (tree_rootp root)
{
    return (items_first_is_x(root, thing_template_is_monst));
}

itemp items_first_is_snail (tree_rootp root)
{
    return (items_first_is_x(root, thing_template_is_snail));
}

itemp items_first_is_wall (tree_rootp root)
{
    return (items_first_is_x(root, thing_template_is_wall));
}

itemp items_first_is_star_yellow (tree_rootp root)
{
    return (items_first_is_x(root, thing_template_is_star_yellow));
}

itemp items_first_is_rock (tree_rootp root)
{
    return (items_first_is_x(root, thing_template_is_rock));
}

itemp items_first_is_xxx2 (tree_rootp root)
{
    return (items_first_is_x(root, thing_template_is_xxx2));
}

itemp items_first_is_xxx3 (tree_rootp root)
{
    return (items_first_is_x(root, thing_template_is_xxx3));
}

itemp items_first_is_xxx4 (tree_rootp root)
{
    return (items_first_is_x(root, thing_template_is_xxx4));
}

itemp items_first_is_xxx5 (tree_rootp root)
{
    return (items_first_is_x(root, thing_template_is_xxx5));
}

itemp items_first_is_xxx6 (tree_rootp root)
{
    return (items_first_is_x(root, thing_template_is_xxx6));
}

itemp items_first_is_xxx7 (tree_rootp root)
{
    return (items_first_is_x(root, thing_template_is_xxx7));
}

itemp items_first_is_xxx8 (tree_rootp root)
{
    return (items_first_is_x(root, thing_template_is_xxx8));
}

itemp items_first_is_star (tree_rootp root)
{
    return (items_first_is_x(root, thing_template_is_star));
}

itemp items_first_is_powerup_spam (tree_rootp root)
{
    return (items_first_is_x(root, thing_template_is_powerup_spam));
}

itemp items_first_is_rock1 (tree_rootp root)
{
    return (items_first_is_x(root, thing_template_is_rock1));
}

itemp items_first_is_car (tree_rootp root)
{
    return (items_first_is_x(root, thing_template_is_car));
}

itemp items_first_is_star_green (tree_rootp root)
{
    return (items_first_is_x(root, thing_template_is_star_green));
}

itemp items_first_is_star_cyan (tree_rootp root)
{
    return (items_first_is_x(root, thing_template_is_star_cyan));
}

itemp items_first_is_star_black (tree_rootp root)
{
    return (items_first_is_x(root, thing_template_is_star_black));
}

itemp items_first_is_star_purple (tree_rootp root)
{
    return (items_first_is_x(root, thing_template_is_star_purple));
}

itemp items_first_is_explosion (tree_rootp root)
{
    return (items_first_is_x(root, thing_template_is_explosion));
}

itemp items_first_is_spikes (tree_rootp root)
{
    return (items_first_is_x(root, thing_template_is_spikes));
}

itemp items_first_is_star_red (tree_rootp root)
{
    return (items_first_is_x(root, thing_template_is_star_red));
}

itemp items_first_is_star_pink (tree_rootp root)
{
    return (items_first_is_x(root, thing_template_is_star_pink));
}

itemp items_first_is_star_blue (tree_rootp root)
{
    return (items_first_is_x(root, thing_template_is_star_blue));
}

itemp items_first_is_seedpod (tree_rootp root)
{
    return (items_first_is_x(root, thing_template_is_seedpod));
}

itemp items_first_is_bomb (tree_rootp root)
{
    return (items_first_is_x(root, thing_template_is_bomb));
}

itemp items_first_is_spam (tree_rootp root)
{
    return (items_first_is_x(root, thing_template_is_spam));
}

itemp items_first_is_road (tree_rootp root)
{
    return (items_first_is_x(root, thing_template_is_road));
}

itemp items_first_is_pipe (tree_rootp root)
{
    return (items_first_is_x(root, thing_template_is_pipe));
}

itemp items_first_is_item_removed_at_level_end (tree_rootp root)
{
    return (items_first_is_x(root, thing_template_is_item_removed_at_level_end));
}

itemp items_first_is_scarable (tree_rootp root)
{
    return (items_first_is_x(root, thing_template_is_scarable));
}

itemp items_first_is_shrunk_when_carried (tree_rootp root)
{
    return (items_first_is_x(root, thing_template_is_shrunk_when_carried));
}

itemp items_first_is_hidden_from_editor (tree_rootp root)
{
    return (items_first_is_x(root, thing_template_is_hidden_from_editor));
}

itemp items_first_is_powerup_rocket (tree_rootp root)
{
    return (items_first_is_x(root, thing_template_is_powerup_rocket));
}

itemp items_first_is_left_as_corpse_on_death (tree_rootp root)
{
    return (items_first_is_x(root, thing_template_is_left_as_corpse_on_death));
}

itemp items_first_is_item_perma (tree_rootp root)
{
    return (items_first_is_x(root, thing_template_is_item_perma));
}

itemp items_first_is_esnail (tree_rootp root)
{
    return (items_first_is_x(root, thing_template_is_esnail));
}

itemp items_first_is_item_is_hidden (tree_rootp root)
{
    return (items_first_is_x(root, thing_template_is_item_hidden));
}

itemp items_first_is_bonus_letter (tree_rootp root)
{
    return (items_first_is_x(root, thing_template_is_bonus_letter));
}

itemp items_first_is_thing (tree_rootp root)
{
    return (items_first_is_x(root, thing_template_is_thing));
}

itemp items_first_is_plant (tree_rootp root)
{
    return (items_first_is_x(root, thing_template_is_plant));
}

itemp items_first_is_food (tree_rootp root)
{
    return (items_first_is_x(root, thing_template_is_food));
}

itemp items_first_is_floor (tree_rootp root)
{
    return (items_first_is_x(root, thing_template_is_floor));
}

itemp items_first_is_exit (tree_rootp root)
{
    return (items_first_is_x(root, thing_template_is_exit));
}

itemp items_first_is_letter (tree_rootp root)
{
    return (items_first_is_x(root, thing_template_is_letter));
}

static tree_rootp items_find_all_is_x (tree_rootp tree, item_is_callback func)
{
    thing_templatep thing_template;
    tree_item_node *node;
    tree_rootp root;
    itemp item;

    root = 0;
    item = item_get_first(tree);

    while (item) {
        thing_template = thing_get_template(item_thing(item));

        if ((*func)(thing_template)) {
            if (!root) {
                root = tree_alloc(TREE_KEY_POINTER,
                                  "TREE ROOT: item find things");
            }

            node = (typeof(node))
                myzalloc(sizeof(*node), "TREE NODE: map find item");

            node->tree.key = (void*)item;

            if (!tree_insert(root, &node->tree.node)) {
                DIE("insert itemp %p", item);
            }
        }

        item = item_get_next(tree, item);
    }

    return (root);
}

tree_rootp items_find_all_is_player (tree_rootp root)
{
    return (items_find_all_is_x(root, thing_template_is_player));
}

tree_rootp items_find_all_is_monst (tree_rootp root)
{
    return (items_find_all_is_x(root, thing_template_is_monst));
}

tree_rootp items_find_all_is_snail (tree_rootp root)
{
    return (items_find_all_is_x(root, thing_template_is_snail));
}

tree_rootp items_find_all_is_wall (tree_rootp root)
{
    return (items_find_all_is_x(root, thing_template_is_wall));
}

tree_rootp items_find_all_is_star_yellow (tree_rootp root)
{
    return (items_find_all_is_x(root, thing_template_is_star_yellow));
}

tree_rootp items_find_all_is_rock (tree_rootp root)
{
    return (items_find_all_is_x(root, thing_template_is_rock));
}

tree_rootp items_find_all_is_xxx2 (tree_rootp root)
{
    return (items_find_all_is_x(root, thing_template_is_xxx2));
}

tree_rootp items_find_all_is_xxx3 (tree_rootp root)
{
    return (items_find_all_is_x(root, thing_template_is_xxx3));
}

tree_rootp items_find_all_is_xxx4 (tree_rootp root)
{
    return (items_find_all_is_x(root, thing_template_is_xxx4));
}

tree_rootp items_find_all_is_xxx5 (tree_rootp root)
{
    return (items_find_all_is_x(root, thing_template_is_xxx5));
}

tree_rootp items_find_all_is_xxx6 (tree_rootp root)
{
    return (items_find_all_is_x(root, thing_template_is_xxx6));
}

tree_rootp items_find_all_is_xxx7 (tree_rootp root)
{
    return (items_find_all_is_x(root, thing_template_is_xxx7));
}

tree_rootp items_find_all_is_xxx8 (tree_rootp root)
{
    return (items_find_all_is_x(root, thing_template_is_xxx8));
}

tree_rootp items_find_all_is_star (tree_rootp root)
{
    return (items_find_all_is_x(root, thing_template_is_star));
}

tree_rootp items_find_all_is_powerup_spam (tree_rootp root)
{
    return (items_find_all_is_x(root, thing_template_is_powerup_spam));
}

tree_rootp items_find_all_is_rock1 (tree_rootp root)
{
    return (items_find_all_is_x(root, thing_template_is_rock1));
}

tree_rootp items_find_all_is_car (tree_rootp root)
{
    return (items_find_all_is_x(root, thing_template_is_car));
}

tree_rootp items_find_all_is_star_green (tree_rootp root)
{
    return (items_find_all_is_x(root, thing_template_is_star_green));
}

tree_rootp items_find_all_is_star_cyan (tree_rootp root)
{
    return (items_find_all_is_x(root, thing_template_is_star_cyan));
}

tree_rootp items_find_all_is_star_black (tree_rootp root)
{
    return (items_find_all_is_x(root, thing_template_is_star_black));
}

tree_rootp items_find_all_is_star_purple (tree_rootp root)
{
    return (items_find_all_is_x(root, thing_template_is_star_purple));
}

tree_rootp items_find_all_is_explosion (tree_rootp root)
{
    return (items_find_all_is_x(root, thing_template_is_explosion));
}

tree_rootp items_find_all_is_spikes (tree_rootp root)
{
    return (items_find_all_is_x(root, thing_template_is_spikes));
}

tree_rootp items_find_all_is_star_red (tree_rootp root)
{
    return (items_find_all_is_x(root, thing_template_is_star_red));
}

tree_rootp items_find_all_is_star_pink (tree_rootp root)
{
    return (items_find_all_is_x(root, thing_template_is_star_pink));
}

tree_rootp items_find_all_is_star_blue (tree_rootp root)
{
    return (items_find_all_is_x(root, thing_template_is_star_blue));
}

tree_rootp items_find_all_is_seedpod (tree_rootp root)
{
    return (items_find_all_is_x(root, thing_template_is_seedpod));
}

tree_rootp items_find_all_is_bomb (tree_rootp root)
{
    return (items_find_all_is_x(root, thing_template_is_bomb));
}

tree_rootp items_find_all_is_spam (tree_rootp root)
{
    return (items_find_all_is_x(root, thing_template_is_spam));
}

tree_rootp items_find_all_is_road (tree_rootp root)
{
    return (items_find_all_is_x(root, thing_template_is_road));
}

tree_rootp items_find_all_is_pipe (tree_rootp root)
{
    return (items_find_all_is_x(root, thing_template_is_pipe));
}

tree_rootp items_find_all_is_item_removed_at_level_end (tree_rootp root)
{
    return (items_find_all_is_x(root, thing_template_is_item_removed_at_level_end));
}

tree_rootp items_find_all_is_scarable (tree_rootp root)
{
    return (items_find_all_is_x(root, thing_template_is_scarable));
}

tree_rootp items_find_all_is_shrunk_when_carried (tree_rootp root)
{
    return (items_find_all_is_x(root, thing_template_is_shrunk_when_carried));
}

tree_rootp items_find_all_is_hidden_from_editor (tree_rootp root)
{
    return (items_find_all_is_x(root, thing_template_is_hidden_from_editor));
}

tree_rootp items_find_all_is_powerup_rocket (tree_rootp root)
{
    return (items_find_all_is_x(root, thing_template_is_powerup_rocket));
}

tree_rootp items_find_all_is_left_as_corpse_on_death (tree_rootp root)
{
    return (items_find_all_is_x(root, thing_template_is_left_as_corpse_on_death));
}

tree_rootp items_find_all_is_item_perma (tree_rootp root)
{
    return (items_find_all_is_x(root, thing_template_is_item_perma));
}

tree_rootp items_find_all_is_esnail (tree_rootp root)
{
    return (items_find_all_is_x(root, thing_template_is_esnail));
}

tree_rootp items_find_all_is_item_is_hidden (tree_rootp root)
{
    return (items_find_all_is_x(root, thing_template_is_item_hidden));
}

tree_rootp items_find_all_is_bonus_letter (tree_rootp root)
{
    return (items_find_all_is_x(root, thing_template_is_bonus_letter));
}

tree_rootp items_find_all_is_thing (tree_rootp root)
{
    return (items_find_all_is_x(root, thing_template_is_thing));
}

tree_rootp items_find_all_is_plant (tree_rootp root)
{
    return (items_find_all_is_x(root, thing_template_is_plant));
}

tree_rootp items_find_all_is_food (tree_rootp root)
{
    return (items_find_all_is_x(root, thing_template_is_food));
}

tree_rootp items_find_all_is_floor (tree_rootp root)
{
    return (items_find_all_is_x(root, thing_template_is_floor));
}

tree_rootp items_find_all_is_exit (tree_rootp root)
{
    return (items_find_all_is_x(root, thing_template_is_exit));
}

tree_rootp items_find_all_is_letter (tree_rootp root)
{
    return (items_find_all_is_x(root, thing_template_is_letter));
}
