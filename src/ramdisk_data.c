#include <stdint.h>
#include "ramdisk.h"

extern unsigned char *data_gfx_explosion_png_start_ asm("data_gfx_explosion_png_start_");
extern unsigned char *data_gfx_explosion_png_end_ asm("data_gfx_explosion_png_end_");
static const unsigned char *const data_gfx_explosion_png_start =
    (const unsigned char *const) (char*)&data_gfx_explosion_png_start_;
static const unsigned char *const data_gfx_explosion_png_end   =
    (const unsigned char *const) (char*)&data_gfx_explosion_png_end_;

extern unsigned char *data_gfx_gorynlich_png_start_ asm("data_gfx_gorynlich_png_start_");
extern unsigned char *data_gfx_gorynlich_png_end_ asm("data_gfx_gorynlich_png_end_");
static const unsigned char *const data_gfx_gorynlich_png_start =
    (const unsigned char *const) (char*)&data_gfx_gorynlich_png_start_;
static const unsigned char *const data_gfx_gorynlich_png_end   =
    (const unsigned char *const) (char*)&data_gfx_gorynlich_png_end_;

extern unsigned char *data_gfx_help1_png_start_ asm("data_gfx_help1_png_start_");
extern unsigned char *data_gfx_help1_png_end_ asm("data_gfx_help1_png_end_");
static const unsigned char *const data_gfx_help1_png_start =
    (const unsigned char *const) (char*)&data_gfx_help1_png_start_;
static const unsigned char *const data_gfx_help1_png_end   =
    (const unsigned char *const) (char*)&data_gfx_help1_png_end_;

extern unsigned char *data_gfx_help2_png_start_ asm("data_gfx_help2_png_start_");
extern unsigned char *data_gfx_help2_png_end_ asm("data_gfx_help2_png_end_");
static const unsigned char *const data_gfx_help2_png_start =
    (const unsigned char *const) (char*)&data_gfx_help2_png_start_;
static const unsigned char *const data_gfx_help2_png_end   =
    (const unsigned char *const) (char*)&data_gfx_help2_png_end_;

extern unsigned char *data_gfx_help3_png_start_ asm("data_gfx_help3_png_start_");
extern unsigned char *data_gfx_help3_png_end_ asm("data_gfx_help3_png_end_");
static const unsigned char *const data_gfx_help3_png_start =
    (const unsigned char *const) (char*)&data_gfx_help3_png_start_;
static const unsigned char *const data_gfx_help3_png_end   =
    (const unsigned char *const) (char*)&data_gfx_help3_png_end_;

extern unsigned char *data_gfx_pipes_png_start_ asm("data_gfx_pipes_png_start_");
extern unsigned char *data_gfx_pipes_png_end_ asm("data_gfx_pipes_png_end_");
static const unsigned char *const data_gfx_pipes_png_start =
    (const unsigned char *const) (char*)&data_gfx_pipes_png_start_;
static const unsigned char *const data_gfx_pipes_png_end   =
    (const unsigned char *const) (char*)&data_gfx_pipes_png_end_;

extern unsigned char *data_gfx_plant_png_start_ asm("data_gfx_plant_png_start_");
extern unsigned char *data_gfx_plant_png_end_ asm("data_gfx_plant_png_end_");
static const unsigned char *const data_gfx_plant_png_start =
    (const unsigned char *const) (char*)&data_gfx_plant_png_start_;
static const unsigned char *const data_gfx_plant_png_end   =
    (const unsigned char *const) (char*)&data_gfx_plant_png_end_;

extern unsigned char *data_gfx_road_png_start_ asm("data_gfx_road_png_start_");
extern unsigned char *data_gfx_road_png_end_ asm("data_gfx_road_png_end_");
static const unsigned char *const data_gfx_road_png_start =
    (const unsigned char *const) (char*)&data_gfx_road_png_start_;
static const unsigned char *const data_gfx_road_png_end   =
    (const unsigned char *const) (char*)&data_gfx_road_png_end_;

extern unsigned char *data_gfx_rocket_png_start_ asm("data_gfx_rocket_png_start_");
extern unsigned char *data_gfx_rocket_png_end_ asm("data_gfx_rocket_png_end_");
static const unsigned char *const data_gfx_rocket_png_start =
    (const unsigned char *const) (char*)&data_gfx_rocket_png_start_;
static const unsigned char *const data_gfx_rocket_png_end   =
    (const unsigned char *const) (char*)&data_gfx_rocket_png_end_;

extern unsigned char *data_gfx_shell_png_start_ asm("data_gfx_shell_png_start_");
extern unsigned char *data_gfx_shell_png_end_ asm("data_gfx_shell_png_end_");
static const unsigned char *const data_gfx_shell_png_start =
    (const unsigned char *const) (char*)&data_gfx_shell_png_start_;
static const unsigned char *const data_gfx_shell_png_end   =
    (const unsigned char *const) (char*)&data_gfx_shell_png_end_;

extern unsigned char *data_gfx_sprites_large_png_start_ asm("data_gfx_sprites_large_png_start_");
extern unsigned char *data_gfx_sprites_large_png_end_ asm("data_gfx_sprites_large_png_end_");
static const unsigned char *const data_gfx_sprites_large_png_start =
    (const unsigned char *const) (char*)&data_gfx_sprites_large_png_start_;
static const unsigned char *const data_gfx_sprites_large_png_end   =
    (const unsigned char *const) (char*)&data_gfx_sprites_large_png_end_;

extern unsigned char *data_gfx_sprites_med_png_start_ asm("data_gfx_sprites_med_png_start_");
extern unsigned char *data_gfx_sprites_med_png_end_ asm("data_gfx_sprites_med_png_end_");
static const unsigned char *const data_gfx_sprites_med_png_start =
    (const unsigned char *const) (char*)&data_gfx_sprites_med_png_start_;
static const unsigned char *const data_gfx_sprites_med_png_end   =
    (const unsigned char *const) (char*)&data_gfx_sprites_med_png_end_;

extern unsigned char *data_gfx_sprites_med2_png_start_ asm("data_gfx_sprites_med2_png_start_");
extern unsigned char *data_gfx_sprites_med2_png_end_ asm("data_gfx_sprites_med2_png_end_");
static const unsigned char *const data_gfx_sprites_med2_png_start =
    (const unsigned char *const) (char*)&data_gfx_sprites_med2_png_start_;
static const unsigned char *const data_gfx_sprites_med2_png_end   =
    (const unsigned char *const) (char*)&data_gfx_sprites_med2_png_end_;

extern unsigned char *data_gfx_sprites_small_png_start_ asm("data_gfx_sprites_small_png_start_");
extern unsigned char *data_gfx_sprites_small_png_end_ asm("data_gfx_sprites_small_png_end_");
static const unsigned char *const data_gfx_sprites_small_png_start =
    (const unsigned char *const) (char*)&data_gfx_sprites_small_png_start_;
static const unsigned char *const data_gfx_sprites_small_png_end   =
    (const unsigned char *const) (char*)&data_gfx_sprites_small_png_end_;

extern unsigned char *data_gfx_stars_png_start_ asm("data_gfx_stars_png_start_");
extern unsigned char *data_gfx_stars_png_end_ asm("data_gfx_stars_png_end_");
static const unsigned char *const data_gfx_stars_png_start =
    (const unsigned char *const) (char*)&data_gfx_stars_png_start_;
static const unsigned char *const data_gfx_stars_png_end   =
    (const unsigned char *const) (char*)&data_gfx_stars_png_end_;

extern unsigned char *data_levels_1_map_start_ asm("data_levels_1_map_start_");
extern unsigned char *data_levels_1_map_end_ asm("data_levels_1_map_end_");
static const unsigned char *const data_levels_1_map_start =
    (const unsigned char *const) (char*)&data_levels_1_map_start_;
static const unsigned char *const data_levels_1_map_end   =
    (const unsigned char *const) (char*)&data_levels_1_map_end_;

extern unsigned char *data_levels_10_map_start_ asm("data_levels_10_map_start_");
extern unsigned char *data_levels_10_map_end_ asm("data_levels_10_map_end_");
static const unsigned char *const data_levels_10_map_start =
    (const unsigned char *const) (char*)&data_levels_10_map_start_;
static const unsigned char *const data_levels_10_map_end   =
    (const unsigned char *const) (char*)&data_levels_10_map_end_;

extern unsigned char *data_levels_2_map_start_ asm("data_levels_2_map_start_");
extern unsigned char *data_levels_2_map_end_ asm("data_levels_2_map_end_");
static const unsigned char *const data_levels_2_map_start =
    (const unsigned char *const) (char*)&data_levels_2_map_start_;
static const unsigned char *const data_levels_2_map_end   =
    (const unsigned char *const) (char*)&data_levels_2_map_end_;

extern unsigned char *data_levels_3_map_start_ asm("data_levels_3_map_start_");
extern unsigned char *data_levels_3_map_end_ asm("data_levels_3_map_end_");
static const unsigned char *const data_levels_3_map_start =
    (const unsigned char *const) (char*)&data_levels_3_map_start_;
static const unsigned char *const data_levels_3_map_end   =
    (const unsigned char *const) (char*)&data_levels_3_map_end_;

extern unsigned char *data_levels_4_map_start_ asm("data_levels_4_map_start_");
extern unsigned char *data_levels_4_map_end_ asm("data_levels_4_map_end_");
static const unsigned char *const data_levels_4_map_start =
    (const unsigned char *const) (char*)&data_levels_4_map_start_;
static const unsigned char *const data_levels_4_map_end   =
    (const unsigned char *const) (char*)&data_levels_4_map_end_;

extern unsigned char *data_levels_5_map_start_ asm("data_levels_5_map_start_");
extern unsigned char *data_levels_5_map_end_ asm("data_levels_5_map_end_");
static const unsigned char *const data_levels_5_map_start =
    (const unsigned char *const) (char*)&data_levels_5_map_start_;
static const unsigned char *const data_levels_5_map_end   =
    (const unsigned char *const) (char*)&data_levels_5_map_end_;

extern unsigned char *data_levels_6_map_start_ asm("data_levels_6_map_start_");
extern unsigned char *data_levels_6_map_end_ asm("data_levels_6_map_end_");
static const unsigned char *const data_levels_6_map_start =
    (const unsigned char *const) (char*)&data_levels_6_map_start_;
static const unsigned char *const data_levels_6_map_end   =
    (const unsigned char *const) (char*)&data_levels_6_map_end_;

extern unsigned char *data_levels_7_map_start_ asm("data_levels_7_map_start_");
extern unsigned char *data_levels_7_map_end_ asm("data_levels_7_map_end_");
static const unsigned char *const data_levels_7_map_start =
    (const unsigned char *const) (char*)&data_levels_7_map_start_;
static const unsigned char *const data_levels_7_map_end   =
    (const unsigned char *const) (char*)&data_levels_7_map_end_;

extern unsigned char *data_levels_8_map_start_ asm("data_levels_8_map_start_");
extern unsigned char *data_levels_8_map_end_ asm("data_levels_8_map_end_");
static const unsigned char *const data_levels_8_map_start =
    (const unsigned char *const) (char*)&data_levels_8_map_start_;
static const unsigned char *const data_levels_8_map_end   =
    (const unsigned char *const) (char*)&data_levels_8_map_end_;

extern unsigned char *data_levels_9_map_start_ asm("data_levels_9_map_start_");
extern unsigned char *data_levels_9_map_end_ asm("data_levels_9_map_end_");
static const unsigned char *const data_levels_9_map_start =
    (const unsigned char *const) (char*)&data_levels_9_map_start_;
static const unsigned char *const data_levels_9_map_end   =
    (const unsigned char *const) (char*)&data_levels_9_map_end_;

extern unsigned char *data_map_map_light_data_start_ asm("data_map_map_light_data_start_");
extern unsigned char *data_map_map_light_data_end_ asm("data_map_map_light_data_end_");
static const unsigned char *const data_map_map_light_data_start =
    (const unsigned char *const) (char*)&data_map_map_light_data_start_;
static const unsigned char *const data_map_map_light_data_end   =
    (const unsigned char *const) (char*)&data_map_map_light_data_end_;

extern unsigned char *data_music_hiscore_wav_start_ asm("data_music_hiscore_wav_start_");
extern unsigned char *data_music_hiscore_wav_end_ asm("data_music_hiscore_wav_end_");
static const unsigned char *const data_music_hiscore_wav_start =
    (const unsigned char *const) (char*)&data_music_hiscore_wav_start_;
static const unsigned char *const data_music_hiscore_wav_end   =
    (const unsigned char *const) (char*)&data_music_hiscore_wav_end_;

extern unsigned char *data_music_intro_wav_start_ asm("data_music_intro_wav_start_");
extern unsigned char *data_music_intro_wav_end_ asm("data_music_intro_wav_end_");
static const unsigned char *const data_music_intro_wav_start =
    (const unsigned char *const) (char*)&data_music_intro_wav_start_;
static const unsigned char *const data_music_intro_wav_end   =
    (const unsigned char *const) (char*)&data_music_intro_wav_end_;

extern unsigned char *data_music_music_wav_start_ asm("data_music_music_wav_start_");
extern unsigned char *data_music_music_wav_end_ asm("data_music_music_wav_end_");
static const unsigned char *const data_music_music_wav_start =
    (const unsigned char *const) (char*)&data_music_music_wav_start_;
static const unsigned char *const data_music_music_wav_end   =
    (const unsigned char *const) (char*)&data_music_music_wav_end_;

extern unsigned char *data_sound_chomp_wav_start_ asm("data_sound_chomp_wav_start_");
extern unsigned char *data_sound_chomp_wav_end_ asm("data_sound_chomp_wav_end_");
static const unsigned char *const data_sound_chomp_wav_start =
    (const unsigned char *const) (char*)&data_sound_chomp_wav_start_;
static const unsigned char *const data_sound_chomp_wav_end   =
    (const unsigned char *const) (char*)&data_sound_chomp_wav_end_;

extern unsigned char *data_sound_chomp2_wav_start_ asm("data_sound_chomp2_wav_start_");
extern unsigned char *data_sound_chomp2_wav_end_ asm("data_sound_chomp2_wav_end_");
static const unsigned char *const data_sound_chomp2_wav_start =
    (const unsigned char *const) (char*)&data_sound_chomp2_wav_start_;
static const unsigned char *const data_sound_chomp2_wav_end   =
    (const unsigned char *const) (char*)&data_sound_chomp2_wav_end_;

extern unsigned char *data_sound_click_wav_start_ asm("data_sound_click_wav_start_");
extern unsigned char *data_sound_click_wav_end_ asm("data_sound_click_wav_end_");
static const unsigned char *const data_sound_click_wav_start =
    (const unsigned char *const) (char*)&data_sound_click_wav_start_;
static const unsigned char *const data_sound_click_wav_end   =
    (const unsigned char *const) (char*)&data_sound_click_wav_end_;

extern unsigned char *data_sound_doorbell_wav_start_ asm("data_sound_doorbell_wav_start_");
extern unsigned char *data_sound_doorbell_wav_end_ asm("data_sound_doorbell_wav_end_");
static const unsigned char *const data_sound_doorbell_wav_start =
    (const unsigned char *const) (char*)&data_sound_doorbell_wav_start_;
static const unsigned char *const data_sound_doorbell_wav_end   =
    (const unsigned char *const) (char*)&data_sound_doorbell_wav_end_;

extern unsigned char *data_sound_explosion_wav_start_ asm("data_sound_explosion_wav_start_");
extern unsigned char *data_sound_explosion_wav_end_ asm("data_sound_explosion_wav_end_");
static const unsigned char *const data_sound_explosion_wav_start =
    (const unsigned char *const) (char*)&data_sound_explosion_wav_start_;
static const unsigned char *const data_sound_explosion_wav_end   =
    (const unsigned char *const) (char*)&data_sound_explosion_wav_end_;

extern unsigned char *data_sound_letter_wav_start_ asm("data_sound_letter_wav_start_");
extern unsigned char *data_sound_letter_wav_end_ asm("data_sound_letter_wav_end_");
static const unsigned char *const data_sound_letter_wav_start =
    (const unsigned char *const) (char*)&data_sound_letter_wav_start_;
static const unsigned char *const data_sound_letter_wav_end   =
    (const unsigned char *const) (char*)&data_sound_letter_wav_end_;

extern unsigned char *data_sound_level_end_wav_start_ asm("data_sound_level_end_wav_start_");
extern unsigned char *data_sound_level_end_wav_end_ asm("data_sound_level_end_wav_end_");
static const unsigned char *const data_sound_level_end_wav_start =
    (const unsigned char *const) (char*)&data_sound_level_end_wav_start_;
static const unsigned char *const data_sound_level_end_wav_end   =
    (const unsigned char *const) (char*)&data_sound_level_end_wav_end_;

extern unsigned char *data_sound_paper_wav_start_ asm("data_sound_paper_wav_start_");
extern unsigned char *data_sound_paper_wav_end_ asm("data_sound_paper_wav_end_");
static const unsigned char *const data_sound_paper_wav_start =
    (const unsigned char *const) (char*)&data_sound_paper_wav_start_;
static const unsigned char *const data_sound_paper_wav_end   =
    (const unsigned char *const) (char*)&data_sound_paper_wav_end_;

extern unsigned char *data_sound_powerup_wav_start_ asm("data_sound_powerup_wav_start_");
extern unsigned char *data_sound_powerup_wav_end_ asm("data_sound_powerup_wav_end_");
static const unsigned char *const data_sound_powerup_wav_start =
    (const unsigned char *const) (char*)&data_sound_powerup_wav_start_;
static const unsigned char *const data_sound_powerup_wav_end   =
    (const unsigned char *const) (char*)&data_sound_powerup_wav_end_;

extern unsigned char *data_sound_ratdeath_wav_start_ asm("data_sound_ratdeath_wav_start_");
extern unsigned char *data_sound_ratdeath_wav_end_ asm("data_sound_ratdeath_wav_end_");
static const unsigned char *const data_sound_ratdeath_wav_start =
    (const unsigned char *const) (char*)&data_sound_ratdeath_wav_start_;
static const unsigned char *const data_sound_ratdeath_wav_end   =
    (const unsigned char *const) (char*)&data_sound_ratdeath_wav_end_;

extern unsigned char *data_sound_rocket_wav_start_ asm("data_sound_rocket_wav_start_");
extern unsigned char *data_sound_rocket_wav_end_ asm("data_sound_rocket_wav_end_");
static const unsigned char *const data_sound_rocket_wav_start =
    (const unsigned char *const) (char*)&data_sound_rocket_wav_start_;
static const unsigned char *const data_sound_rocket_wav_end   =
    (const unsigned char *const) (char*)&data_sound_rocket_wav_end_;

extern unsigned char *data_sound_slime_wav_start_ asm("data_sound_slime_wav_start_");
extern unsigned char *data_sound_slime_wav_end_ asm("data_sound_slime_wav_end_");
static const unsigned char *const data_sound_slime_wav_start =
    (const unsigned char *const) (char*)&data_sound_slime_wav_start_;
static const unsigned char *const data_sound_slime_wav_end   =
    (const unsigned char *const) (char*)&data_sound_slime_wav_end_;

extern unsigned char *data_sound_spam_wav_start_ asm("data_sound_spam_wav_start_");
extern unsigned char *data_sound_spam_wav_end_ asm("data_sound_spam_wav_end_");
static const unsigned char *const data_sound_spam_wav_start =
    (const unsigned char *const) (char*)&data_sound_spam_wav_start_;
static const unsigned char *const data_sound_spam_wav_end   =
    (const unsigned char *const) (char*)&data_sound_spam_wav_end_;

extern unsigned char *data_sound_thief_wav_start_ asm("data_sound_thief_wav_start_");
extern unsigned char *data_sound_thief_wav_end_ asm("data_sound_thief_wav_end_");
static const unsigned char *const data_sound_thief_wav_start =
    (const unsigned char *const) (char*)&data_sound_thief_wav_start_;
static const unsigned char *const data_sound_thief_wav_end   =
    (const unsigned char *const) (char*)&data_sound_thief_wav_end_;

extern unsigned char *data_things_brick_0_start_ asm("data_things_brick_0_start_");
extern unsigned char *data_things_brick_0_end_ asm("data_things_brick_0_end_");
static const unsigned char *const data_things_brick_0_start =
    (const unsigned char *const) (char*)&data_things_brick_0_start_;
static const unsigned char *const data_things_brick_0_end   =
    (const unsigned char *const) (char*)&data_things_brick_0_end_;

extern unsigned char *data_things_brick_1_start_ asm("data_things_brick_1_start_");
extern unsigned char *data_things_brick_1_end_ asm("data_things_brick_1_end_");
static const unsigned char *const data_things_brick_1_start =
    (const unsigned char *const) (char*)&data_things_brick_1_start_;
static const unsigned char *const data_things_brick_1_end   =
    (const unsigned char *const) (char*)&data_things_brick_1_end_;

extern unsigned char *data_things_brick_2_start_ asm("data_things_brick_2_start_");
extern unsigned char *data_things_brick_2_end_ asm("data_things_brick_2_end_");
static const unsigned char *const data_things_brick_2_start =
    (const unsigned char *const) (char*)&data_things_brick_2_start_;
static const unsigned char *const data_things_brick_2_end   =
    (const unsigned char *const) (char*)&data_things_brick_2_end_;

extern unsigned char *data_things_brick_3_start_ asm("data_things_brick_3_start_");
extern unsigned char *data_things_brick_3_end_ asm("data_things_brick_3_end_");
static const unsigned char *const data_things_brick_3_start =
    (const unsigned char *const) (char*)&data_things_brick_3_start_;
static const unsigned char *const data_things_brick_3_end   =
    (const unsigned char *const) (char*)&data_things_brick_3_end_;

extern unsigned char *data_things_brick_4_start_ asm("data_things_brick_4_start_");
extern unsigned char *data_things_brick_4_end_ asm("data_things_brick_4_end_");
static const unsigned char *const data_things_brick_4_start =
    (const unsigned char *const) (char*)&data_things_brick_4_start_;
static const unsigned char *const data_things_brick_4_end   =
    (const unsigned char *const) (char*)&data_things_brick_4_end_;

extern unsigned char *data_things_brick_5_start_ asm("data_things_brick_5_start_");
extern unsigned char *data_things_brick_5_end_ asm("data_things_brick_5_end_");
static const unsigned char *const data_things_brick_5_start =
    (const unsigned char *const) (char*)&data_things_brick_5_start_;
static const unsigned char *const data_things_brick_5_end   =
    (const unsigned char *const) (char*)&data_things_brick_5_end_;

extern unsigned char *data_things_brick_6_start_ asm("data_things_brick_6_start_");
extern unsigned char *data_things_brick_6_end_ asm("data_things_brick_6_end_");
static const unsigned char *const data_things_brick_6_start =
    (const unsigned char *const) (char*)&data_things_brick_6_start_;
static const unsigned char *const data_things_brick_6_end   =
    (const unsigned char *const) (char*)&data_things_brick_6_end_;

extern unsigned char *data_things_brick_7_start_ asm("data_things_brick_7_start_");
extern unsigned char *data_things_brick_7_end_ asm("data_things_brick_7_end_");
static const unsigned char *const data_things_brick_7_start =
    (const unsigned char *const) (char*)&data_things_brick_7_start_;
static const unsigned char *const data_things_brick_7_end   =
    (const unsigned char *const) (char*)&data_things_brick_7_end_;

extern unsigned char *data_things_brick_8_start_ asm("data_things_brick_8_start_");
extern unsigned char *data_things_brick_8_end_ asm("data_things_brick_8_end_");
static const unsigned char *const data_things_brick_8_start =
    (const unsigned char *const) (char*)&data_things_brick_8_start_;
static const unsigned char *const data_things_brick_8_end   =
    (const unsigned char *const) (char*)&data_things_brick_8_end_;

extern unsigned char *data_things_lava_0_start_ asm("data_things_lava_0_start_");
extern unsigned char *data_things_lava_0_end_ asm("data_things_lava_0_end_");
static const unsigned char *const data_things_lava_0_start =
    (const unsigned char *const) (char*)&data_things_lava_0_start_;
static const unsigned char *const data_things_lava_0_end   =
    (const unsigned char *const) (char*)&data_things_lava_0_end_;

extern unsigned char *data_things_rock_0_start_ asm("data_things_rock_0_start_");
extern unsigned char *data_things_rock_0_end_ asm("data_things_rock_0_end_");
static const unsigned char *const data_things_rock_0_start =
    (const unsigned char *const) (char*)&data_things_rock_0_start_;
static const unsigned char *const data_things_rock_0_end   =
    (const unsigned char *const) (char*)&data_things_rock_0_end_;

extern unsigned char *data_things_water_0_start_ asm("data_things_water_0_start_");
extern unsigned char *data_things_water_0_end_ asm("data_things_water_0_end_");
static const unsigned char *const data_things_water_0_start =
    (const unsigned char *const) (char*)&data_things_water_0_start_;
static const unsigned char *const data_things_water_0_end   =
    (const unsigned char *const) (char*)&data_things_water_0_end_;

extern unsigned char *data_ttf_large_ttf_pointsize40_data_start_ asm("data_ttf_large_ttf_pointsize40_data_start_");
extern unsigned char *data_ttf_large_ttf_pointsize40_data_end_ asm("data_ttf_large_ttf_pointsize40_data_end_");
static const unsigned char *const data_ttf_large_ttf_pointsize40_data_start =
    (const unsigned char *const) (char*)&data_ttf_large_ttf_pointsize40_data_start_;
static const unsigned char *const data_ttf_large_ttf_pointsize40_data_end   =
    (const unsigned char *const) (char*)&data_ttf_large_ttf_pointsize40_data_end_;

extern unsigned char *data_ttf_large_ttf_pointsize40_tga_start_ asm("data_ttf_large_ttf_pointsize40_tga_start_");
extern unsigned char *data_ttf_large_ttf_pointsize40_tga_end_ asm("data_ttf_large_ttf_pointsize40_tga_end_");
static const unsigned char *const data_ttf_large_ttf_pointsize40_tga_start =
    (const unsigned char *const) (char*)&data_ttf_large_ttf_pointsize40_tga_start_;
static const unsigned char *const data_ttf_large_ttf_pointsize40_tga_end   =
    (const unsigned char *const) (char*)&data_ttf_large_ttf_pointsize40_tga_end_;

extern unsigned char *data_ttf_med_ttf_pointsize30_data_start_ asm("data_ttf_med_ttf_pointsize30_data_start_");
extern unsigned char *data_ttf_med_ttf_pointsize30_data_end_ asm("data_ttf_med_ttf_pointsize30_data_end_");
static const unsigned char *const data_ttf_med_ttf_pointsize30_data_start =
    (const unsigned char *const) (char*)&data_ttf_med_ttf_pointsize30_data_start_;
static const unsigned char *const data_ttf_med_ttf_pointsize30_data_end   =
    (const unsigned char *const) (char*)&data_ttf_med_ttf_pointsize30_data_end_;

extern unsigned char *data_ttf_med_ttf_pointsize30_tga_start_ asm("data_ttf_med_ttf_pointsize30_tga_start_");
extern unsigned char *data_ttf_med_ttf_pointsize30_tga_end_ asm("data_ttf_med_ttf_pointsize30_tga_end_");
static const unsigned char *const data_ttf_med_ttf_pointsize30_tga_start =
    (const unsigned char *const) (char*)&data_ttf_med_ttf_pointsize30_tga_start_;
static const unsigned char *const data_ttf_med_ttf_pointsize30_tga_end   =
    (const unsigned char *const) (char*)&data_ttf_med_ttf_pointsize30_tga_end_;

extern unsigned char *data_ttf_small_ttf_pointsize20_data_start_ asm("data_ttf_small_ttf_pointsize20_data_start_");
extern unsigned char *data_ttf_small_ttf_pointsize20_data_end_ asm("data_ttf_small_ttf_pointsize20_data_end_");
static const unsigned char *const data_ttf_small_ttf_pointsize20_data_start =
    (const unsigned char *const) (char*)&data_ttf_small_ttf_pointsize20_data_start_;
static const unsigned char *const data_ttf_small_ttf_pointsize20_data_end   =
    (const unsigned char *const) (char*)&data_ttf_small_ttf_pointsize20_data_end_;

extern unsigned char *data_ttf_small_ttf_pointsize20_tga_start_ asm("data_ttf_small_ttf_pointsize20_tga_start_");
extern unsigned char *data_ttf_small_ttf_pointsize20_tga_end_ asm("data_ttf_small_ttf_pointsize20_tga_end_");
static const unsigned char *const data_ttf_small_ttf_pointsize20_tga_start =
    (const unsigned char *const) (char*)&data_ttf_small_ttf_pointsize20_tga_start_;
static const unsigned char *const data_ttf_small_ttf_pointsize20_tga_end   =
    (const unsigned char *const) (char*)&data_ttf_small_ttf_pointsize20_tga_end_;

ramdisk_t ramdisk_data[] = {
{
    /* filename */ "data/gfx/explosion.png",
    /* data     */ 0,
    /* orig_len */ 351055,
    /* len      */ 351055,
    /* uncomprs */ 1,
},
{
    /* filename */ "data/gfx/gorynlich.png",
    /* data     */ 0,
    /* orig_len */ 397308,
    /* len      */ 397308,
    /* uncomprs */ 1,
},
{
    /* filename */ "data/gfx/help1.png",
    /* data     */ 0,
    /* orig_len */ 318932,
    /* len      */ 318932,
    /* uncomprs */ 1,
},
{
    /* filename */ "data/gfx/help2.png",
    /* data     */ 0,
    /* orig_len */ 331153,
    /* len      */ 331153,
    /* uncomprs */ 1,
},
{
    /* filename */ "data/gfx/help3.png",
    /* data     */ 0,
    /* orig_len */ 371754,
    /* len      */ 371754,
    /* uncomprs */ 1,
},
{
    /* filename */ "data/gfx/pipes.png",
    /* data     */ 0,
    /* orig_len */ 27217,
    /* len      */ 27217,
    /* uncomprs */ 1,
},
{
    /* filename */ "data/gfx/plant.png",
    /* data     */ 0,
    /* orig_len */ 14161,
    /* len      */ 14161,
    /* uncomprs */ 1,
},
{
    /* filename */ "data/gfx/road.png",
    /* data     */ 0,
    /* orig_len */ 78912,
    /* len      */ 78912,
    /* uncomprs */ 1,
},
{
    /* filename */ "data/gfx/rocket.png",
    /* data     */ 0,
    /* orig_len */ 4023,
    /* len      */ 4023,
    /* uncomprs */ 1,
},
{
    /* filename */ "data/gfx/shell.png",
    /* data     */ 0,
    /* orig_len */ 257534,
    /* len      */ 257534,
    /* uncomprs */ 1,
},
{
    /* filename */ "data/gfx/sprites_large.png",
    /* data     */ 0,
    /* orig_len */ 43272,
    /* len      */ 43272,
    /* uncomprs */ 1,
},
{
    /* filename */ "data/gfx/sprites_med.png",
    /* data     */ 0,
    /* orig_len */ 73732,
    /* len      */ 73732,
    /* uncomprs */ 1,
},
{
    /* filename */ "data/gfx/sprites_med2.png",
    /* data     */ 0,
    /* orig_len */ 45460,
    /* len      */ 45460,
    /* uncomprs */ 1,
},
{
    /* filename */ "data/gfx/sprites_small.png",
    /* data     */ 0,
    /* orig_len */ 27730,
    /* len      */ 27730,
    /* uncomprs */ 1,
},
{
    /* filename */ "data/gfx/stars.png",
    /* data     */ 0,
    /* orig_len */ 23171,
    /* len      */ 23171,
    /* uncomprs */ 1,
},
{
    /* filename */ "data/levels/1.map",
    /* data     */ 0,
    /* orig_len */ 17790,
    /* len      */ 17790,
    /* uncomprs */ 1,
},
{
    /* filename */ "data/levels/10.map",
    /* data     */ 0,
    /* orig_len */ 19065,
    /* len      */ 19065,
    /* uncomprs */ 1,
},
{
    /* filename */ "data/levels/2.map",
    /* data     */ 0,
    /* orig_len */ 19573,
    /* len      */ 19573,
    /* uncomprs */ 1,
},
{
    /* filename */ "data/levels/3.map",
    /* data     */ 0,
    /* orig_len */ 21488,
    /* len      */ 21488,
    /* uncomprs */ 1,
},
{
    /* filename */ "data/levels/4.map",
    /* data     */ 0,
    /* orig_len */ 21403,
    /* len      */ 21403,
    /* uncomprs */ 1,
},
{
    /* filename */ "data/levels/5.map",
    /* data     */ 0,
    /* orig_len */ 22067,
    /* len      */ 22067,
    /* uncomprs */ 1,
},
{
    /* filename */ "data/levels/6.map",
    /* data     */ 0,
    /* orig_len */ 24491,
    /* len      */ 24491,
    /* uncomprs */ 1,
},
{
    /* filename */ "data/levels/7.map",
    /* data     */ 0,
    /* orig_len */ 25116,
    /* len      */ 25116,
    /* uncomprs */ 1,
},
{
    /* filename */ "data/levels/8.map",
    /* data     */ 0,
    /* orig_len */ 17518,
    /* len      */ 17518,
    /* uncomprs */ 1,
},
{
    /* filename */ "data/levels/9.map",
    /* data     */ 0,
    /* orig_len */ 19277,
    /* len      */ 19277,
    /* uncomprs */ 1,
},
{
    /* filename */ "data/map/map_light.data",
    /* data     */ 0,
    /* orig_len */ 12846675,
    /* len      */ 12846675,
    /* uncomprs */ 1,
},
{
    /* filename */ "data/music/hiscore.wav",
    /* data     */ 0,
    /* orig_len */ 1,
    /* len      */ 1,
    /* uncomprs */ 1,
},
{
    /* filename */ "data/music/intro.wav",
    /* data     */ 0,
    /* orig_len */ 1,
    /* len      */ 1,
    /* uncomprs */ 1,
},
{
    /* filename */ "data/music/music.wav",
    /* data     */ 0,
    /* orig_len */ 1,
    /* len      */ 1,
    /* uncomprs */ 1,
},
{
    /* filename */ "data/sound/chomp.wav",
    /* data     */ 0,
    /* orig_len */ 5065,
    /* len      */ 5065,
    /* uncomprs */ 1,
},
{
    /* filename */ "data/sound/chomp2.wav",
    /* data     */ 0,
    /* orig_len */ 56301,
    /* len      */ 56301,
    /* uncomprs */ 1,
},
{
    /* filename */ "data/sound/click.wav",
    /* data     */ 0,
    /* orig_len */ 54329,
    /* len      */ 54329,
    /* uncomprs */ 1,
},
{
    /* filename */ "data/sound/doorbell.wav",
    /* data     */ 0,
    /* orig_len */ 57645,
    /* len      */ 57645,
    /* uncomprs */ 1,
},
{
    /* filename */ "data/sound/explosion.wav",
    /* data     */ 0,
    /* orig_len */ 169785,
    /* len      */ 169785,
    /* uncomprs */ 1,
},
{
    /* filename */ "data/sound/letter.wav",
    /* data     */ 0,
    /* orig_len */ 161433,
    /* len      */ 161433,
    /* uncomprs */ 1,
},
{
    /* filename */ "data/sound/level_end.wav",
    /* data     */ 0,
    /* orig_len */ 78729,
    /* len      */ 78729,
    /* uncomprs */ 1,
},
{
    /* filename */ "data/sound/paper.wav",
    /* data     */ 0,
    /* orig_len */ 192557,
    /* len      */ 192557,
    /* uncomprs */ 1,
},
{
    /* filename */ "data/sound/powerup.wav",
    /* data     */ 0,
    /* orig_len */ 7833,
    /* len      */ 7833,
    /* uncomprs */ 1,
},
{
    /* filename */ "data/sound/ratdeath.wav",
    /* data     */ 0,
    /* orig_len */ 52901,
    /* len      */ 52901,
    /* uncomprs */ 1,
},
{
    /* filename */ "data/sound/rocket.wav",
    /* data     */ 0,
    /* orig_len */ 521869,
    /* len      */ 521869,
    /* uncomprs */ 1,
},
{
    /* filename */ "data/sound/slime.wav",
    /* data     */ 0,
    /* orig_len */ 117149,
    /* len      */ 117149,
    /* uncomprs */ 1,
},
{
    /* filename */ "data/sound/spam.wav",
    /* data     */ 0,
    /* orig_len */ 117149,
    /* len      */ 117149,
    /* uncomprs */ 1,
},
{
    /* filename */ "data/sound/thief.wav",
    /* data     */ 0,
    /* orig_len */ 44581,
    /* len      */ 44581,
    /* uncomprs */ 1,
},
{
    /* filename */ "data/things/brick_0",
    /* data     */ 0,
    /* orig_len */ 3251,
    /* len      */ 3251,
    /* uncomprs */ 1,
},
{
    /* filename */ "data/things/brick_1",
    /* data     */ 0,
    /* orig_len */ 3251,
    /* len      */ 3251,
    /* uncomprs */ 1,
},
{
    /* filename */ "data/things/brick_2",
    /* data     */ 0,
    /* orig_len */ 3251,
    /* len      */ 3251,
    /* uncomprs */ 1,
},
{
    /* filename */ "data/things/brick_3",
    /* data     */ 0,
    /* orig_len */ 3251,
    /* len      */ 3251,
    /* uncomprs */ 1,
},
{
    /* filename */ "data/things/brick_4",
    /* data     */ 0,
    /* orig_len */ 3251,
    /* len      */ 3251,
    /* uncomprs */ 1,
},
{
    /* filename */ "data/things/brick_5",
    /* data     */ 0,
    /* orig_len */ 3251,
    /* len      */ 3251,
    /* uncomprs */ 1,
},
{
    /* filename */ "data/things/brick_6",
    /* data     */ 0,
    /* orig_len */ 3251,
    /* len      */ 3251,
    /* uncomprs */ 1,
},
{
    /* filename */ "data/things/brick_7",
    /* data     */ 0,
    /* orig_len */ 3251,
    /* len      */ 3251,
    /* uncomprs */ 1,
},
{
    /* filename */ "data/things/brick_8",
    /* data     */ 0,
    /* orig_len */ 3251,
    /* len      */ 3251,
    /* uncomprs */ 1,
},
{
    /* filename */ "data/things/lava_0",
    /* data     */ 0,
    /* orig_len */ 3195,
    /* len      */ 3195,
    /* uncomprs */ 1,
},
{
    /* filename */ "data/things/rock_0",
    /* data     */ 0,
    /* orig_len */ 3192,
    /* len      */ 3192,
    /* uncomprs */ 1,
},
{
    /* filename */ "data/things/water_0",
    /* data     */ 0,
    /* orig_len */ 3252,
    /* len      */ 3252,
    /* uncomprs */ 1,
},
{
    /* filename */ "data/ttf/large.ttf_pointsize40.data",
    /* data     */ 0,
    /* orig_len */ 5589,
    /* len      */ 5589,
    /* uncomprs */ 1,
},
{
    /* filename */ "data/ttf/large.ttf_pointsize40.tga",
    /* data     */ 0,
    /* orig_len */ 1048595,
    /* len      */ 1048595,
    /* uncomprs */ 1,
},
{
    /* filename */ "data/ttf/med.ttf_pointsize30.data",
    /* data     */ 0,
    /* orig_len */ 5589,
    /* len      */ 5589,
    /* uncomprs */ 1,
},
{
    /* filename */ "data/ttf/med.ttf_pointsize30.tga",
    /* data     */ 0,
    /* orig_len */ 524307,
    /* len      */ 524307,
    /* uncomprs */ 1,
},
{
    /* filename */ "data/ttf/small.ttf_pointsize20.data",
    /* data     */ 0,
    /* orig_len */ 5589,
    /* len      */ 5589,
    /* uncomprs */ 1,
},
{
    /* filename */ "data/ttf/small.ttf_pointsize20.tga",
    /* data     */ 0,
    /* orig_len */ 262163,
    /* len      */ 262163,
    /* uncomprs */ 1,
},
    {0},
};

void ramdisk_init (void)
{
    ramdisk_data[0].data = data_gfx_explosion_png_start;
    ramdisk_data[1].data = data_gfx_gorynlich_png_start;
    ramdisk_data[2].data = data_gfx_help1_png_start;
    ramdisk_data[3].data = data_gfx_help2_png_start;
    ramdisk_data[4].data = data_gfx_help3_png_start;
    ramdisk_data[5].data = data_gfx_pipes_png_start;
    ramdisk_data[6].data = data_gfx_plant_png_start;
    ramdisk_data[7].data = data_gfx_road_png_start;
    ramdisk_data[8].data = data_gfx_rocket_png_start;
    ramdisk_data[9].data = data_gfx_shell_png_start;
    ramdisk_data[10].data = data_gfx_sprites_large_png_start;
    ramdisk_data[11].data = data_gfx_sprites_med_png_start;
    ramdisk_data[12].data = data_gfx_sprites_med2_png_start;
    ramdisk_data[13].data = data_gfx_sprites_small_png_start;
    ramdisk_data[14].data = data_gfx_stars_png_start;
    ramdisk_data[15].data = data_levels_1_map_start;
    ramdisk_data[16].data = data_levels_10_map_start;
    ramdisk_data[17].data = data_levels_2_map_start;
    ramdisk_data[18].data = data_levels_3_map_start;
    ramdisk_data[19].data = data_levels_4_map_start;
    ramdisk_data[20].data = data_levels_5_map_start;
    ramdisk_data[21].data = data_levels_6_map_start;
    ramdisk_data[22].data = data_levels_7_map_start;
    ramdisk_data[23].data = data_levels_8_map_start;
    ramdisk_data[24].data = data_levels_9_map_start;
    ramdisk_data[25].data = data_map_map_light_data_start;
    ramdisk_data[26].data = data_music_hiscore_wav_start;
    ramdisk_data[27].data = data_music_intro_wav_start;
    ramdisk_data[28].data = data_music_music_wav_start;
    ramdisk_data[29].data = data_sound_chomp_wav_start;
    ramdisk_data[30].data = data_sound_chomp2_wav_start;
    ramdisk_data[31].data = data_sound_click_wav_start;
    ramdisk_data[32].data = data_sound_doorbell_wav_start;
    ramdisk_data[33].data = data_sound_explosion_wav_start;
    ramdisk_data[34].data = data_sound_letter_wav_start;
    ramdisk_data[35].data = data_sound_level_end_wav_start;
    ramdisk_data[36].data = data_sound_paper_wav_start;
    ramdisk_data[37].data = data_sound_powerup_wav_start;
    ramdisk_data[38].data = data_sound_ratdeath_wav_start;
    ramdisk_data[39].data = data_sound_rocket_wav_start;
    ramdisk_data[40].data = data_sound_slime_wav_start;
    ramdisk_data[41].data = data_sound_spam_wav_start;
    ramdisk_data[42].data = data_sound_thief_wav_start;
    ramdisk_data[43].data = data_things_brick_0_start;
    ramdisk_data[44].data = data_things_brick_1_start;
    ramdisk_data[45].data = data_things_brick_2_start;
    ramdisk_data[46].data = data_things_brick_3_start;
    ramdisk_data[47].data = data_things_brick_4_start;
    ramdisk_data[48].data = data_things_brick_5_start;
    ramdisk_data[49].data = data_things_brick_6_start;
    ramdisk_data[50].data = data_things_brick_7_start;
    ramdisk_data[51].data = data_things_brick_8_start;
    ramdisk_data[52].data = data_things_lava_0_start;
    ramdisk_data[53].data = data_things_rock_0_start;
    ramdisk_data[54].data = data_things_water_0_start;
    ramdisk_data[55].data = data_ttf_large_ttf_pointsize40_data_start;
    ramdisk_data[56].data = data_ttf_large_ttf_pointsize40_tga_start;
    ramdisk_data[57].data = data_ttf_med_ttf_pointsize30_data_start;
    ramdisk_data[58].data = data_ttf_med_ttf_pointsize30_tga_start;
    ramdisk_data[59].data = data_ttf_small_ttf_pointsize20_data_start;
    ramdisk_data[60].data = data_ttf_small_ttf_pointsize20_tga_start;
}
