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

extern unsigned char *data_map_jigsaw_map_start_ asm("data_map_jigsaw_map_start_");
extern unsigned char *data_map_jigsaw_map_end_ asm("data_map_jigsaw_map_end_");
static const unsigned char *const data_map_jigsaw_map_start =
    (const unsigned char *const) (char*)&data_map_jigsaw_map_start_;
static const unsigned char *const data_map_jigsaw_map_end   =
    (const unsigned char *const) (char*)&data_map_jigsaw_map_end_;

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

extern unsigned char *data_things_boulder_0_start_ asm("data_things_boulder_0_start_");
extern unsigned char *data_things_boulder_0_end_ asm("data_things_boulder_0_end_");
static const unsigned char *const data_things_boulder_0_start =
    (const unsigned char *const) (char*)&data_things_boulder_0_start_;
static const unsigned char *const data_things_boulder_0_end   =
    (const unsigned char *const) (char*)&data_things_boulder_0_end_;

extern unsigned char *data_things_exit_start_ asm("data_things_exit_start_");
extern unsigned char *data_things_exit_end_ asm("data_things_exit_end_");
static const unsigned char *const data_things_exit_start =
    (const unsigned char *const) (char*)&data_things_exit_start_;
static const unsigned char *const data_things_exit_end   =
    (const unsigned char *const) (char*)&data_things_exit_end_;

extern unsigned char *data_things_gem_0_start_ asm("data_things_gem_0_start_");
extern unsigned char *data_things_gem_0_end_ asm("data_things_gem_0_end_");
static const unsigned char *const data_things_gem_0_start =
    (const unsigned char *const) (char*)&data_things_gem_0_start_;
static const unsigned char *const data_things_gem_0_end   =
    (const unsigned char *const) (char*)&data_things_gem_0_end_;

extern unsigned char *data_things_gem_1_start_ asm("data_things_gem_1_start_");
extern unsigned char *data_things_gem_1_end_ asm("data_things_gem_1_end_");
static const unsigned char *const data_things_gem_1_start =
    (const unsigned char *const) (char*)&data_things_gem_1_start_;
static const unsigned char *const data_things_gem_1_end   =
    (const unsigned char *const) (char*)&data_things_gem_1_end_;

extern unsigned char *data_things_gem_2_start_ asm("data_things_gem_2_start_");
extern unsigned char *data_things_gem_2_end_ asm("data_things_gem_2_end_");
static const unsigned char *const data_things_gem_2_start =
    (const unsigned char *const) (char*)&data_things_gem_2_start_;
static const unsigned char *const data_things_gem_2_end   =
    (const unsigned char *const) (char*)&data_things_gem_2_end_;

extern unsigned char *data_things_gem_3_start_ asm("data_things_gem_3_start_");
extern unsigned char *data_things_gem_3_end_ asm("data_things_gem_3_end_");
static const unsigned char *const data_things_gem_3_start =
    (const unsigned char *const) (char*)&data_things_gem_3_start_;
static const unsigned char *const data_things_gem_3_end   =
    (const unsigned char *const) (char*)&data_things_gem_3_end_;

extern unsigned char *data_things_gem_4_start_ asm("data_things_gem_4_start_");
extern unsigned char *data_things_gem_4_end_ asm("data_things_gem_4_end_");
static const unsigned char *const data_things_gem_4_start =
    (const unsigned char *const) (char*)&data_things_gem_4_start_;
static const unsigned char *const data_things_gem_4_end   =
    (const unsigned char *const) (char*)&data_things_gem_4_end_;

extern unsigned char *data_things_gem_5_start_ asm("data_things_gem_5_start_");
extern unsigned char *data_things_gem_5_end_ asm("data_things_gem_5_end_");
static const unsigned char *const data_things_gem_5_start =
    (const unsigned char *const) (char*)&data_things_gem_5_start_;
static const unsigned char *const data_things_gem_5_end   =
    (const unsigned char *const) (char*)&data_things_gem_5_end_;

extern unsigned char *data_things_gem_6_start_ asm("data_things_gem_6_start_");
extern unsigned char *data_things_gem_6_end_ asm("data_things_gem_6_end_");
static const unsigned char *const data_things_gem_6_start =
    (const unsigned char *const) (char*)&data_things_gem_6_start_;
static const unsigned char *const data_things_gem_6_end   =
    (const unsigned char *const) (char*)&data_things_gem_6_end_;

extern unsigned char *data_things_gem_7_start_ asm("data_things_gem_7_start_");
extern unsigned char *data_things_gem_7_end_ asm("data_things_gem_7_end_");
static const unsigned char *const data_things_gem_7_start =
    (const unsigned char *const) (char*)&data_things_gem_7_start_;
static const unsigned char *const data_things_gem_7_end   =
    (const unsigned char *const) (char*)&data_things_gem_7_end_;

extern unsigned char *data_things_ladder_start_ asm("data_things_ladder_start_");
extern unsigned char *data_things_ladder_end_ asm("data_things_ladder_end_");
static const unsigned char *const data_things_ladder_start =
    (const unsigned char *const) (char*)&data_things_ladder_start_;
static const unsigned char *const data_things_ladder_end   =
    (const unsigned char *const) (char*)&data_things_ladder_end_;

extern unsigned char *data_things_lava_0_start_ asm("data_things_lava_0_start_");
extern unsigned char *data_things_lava_0_end_ asm("data_things_lava_0_end_");
static const unsigned char *const data_things_lava_0_start =
    (const unsigned char *const) (char*)&data_things_lava_0_start_;
static const unsigned char *const data_things_lava_0_end   =
    (const unsigned char *const) (char*)&data_things_lava_0_end_;

extern unsigned char *data_things_player_0_start_ asm("data_things_player_0_start_");
extern unsigned char *data_things_player_0_end_ asm("data_things_player_0_end_");
static const unsigned char *const data_things_player_0_start =
    (const unsigned char *const) (char*)&data_things_player_0_start_;
static const unsigned char *const data_things_player_0_end   =
    (const unsigned char *const) (char*)&data_things_player_0_end_;

extern unsigned char *data_things_rock_0_start_ asm("data_things_rock_0_start_");
extern unsigned char *data_things_rock_0_end_ asm("data_things_rock_0_end_");
static const unsigned char *const data_things_rock_0_start =
    (const unsigned char *const) (char*)&data_things_rock_0_start_;
static const unsigned char *const data_things_rock_0_end   =
    (const unsigned char *const) (char*)&data_things_rock_0_end_;

extern unsigned char *data_things_rock_1_start_ asm("data_things_rock_1_start_");
extern unsigned char *data_things_rock_1_end_ asm("data_things_rock_1_end_");
static const unsigned char *const data_things_rock_1_start =
    (const unsigned char *const) (char*)&data_things_rock_1_start_;
static const unsigned char *const data_things_rock_1_end   =
    (const unsigned char *const) (char*)&data_things_rock_1_end_;

extern unsigned char *data_things_skeleton_start_ asm("data_things_skeleton_start_");
extern unsigned char *data_things_skeleton_end_ asm("data_things_skeleton_end_");
static const unsigned char *const data_things_skeleton_start =
    (const unsigned char *const) (char*)&data_things_skeleton_start_;
static const unsigned char *const data_things_skeleton_end   =
    (const unsigned char *const) (char*)&data_things_skeleton_end_;

extern unsigned char *data_things_water_0_start_ asm("data_things_water_0_start_");
extern unsigned char *data_things_water_0_end_ asm("data_things_water_0_end_");
static const unsigned char *const data_things_water_0_start =
    (const unsigned char *const) (char*)&data_things_water_0_start_;
static const unsigned char *const data_things_water_0_end   =
    (const unsigned char *const) (char*)&data_things_water_0_end_;

extern unsigned char *data_ttf_large_ttf_pointsize20_data_start_ asm("data_ttf_large_ttf_pointsize20_data_start_");
extern unsigned char *data_ttf_large_ttf_pointsize20_data_end_ asm("data_ttf_large_ttf_pointsize20_data_end_");
static const unsigned char *const data_ttf_large_ttf_pointsize20_data_start =
    (const unsigned char *const) (char*)&data_ttf_large_ttf_pointsize20_data_start_;
static const unsigned char *const data_ttf_large_ttf_pointsize20_data_end   =
    (const unsigned char *const) (char*)&data_ttf_large_ttf_pointsize20_data_end_;

extern unsigned char *data_ttf_large_ttf_pointsize20_tga_start_ asm("data_ttf_large_ttf_pointsize20_tga_start_");
extern unsigned char *data_ttf_large_ttf_pointsize20_tga_end_ asm("data_ttf_large_ttf_pointsize20_tga_end_");
static const unsigned char *const data_ttf_large_ttf_pointsize20_tga_start =
    (const unsigned char *const) (char*)&data_ttf_large_ttf_pointsize20_tga_start_;
static const unsigned char *const data_ttf_large_ttf_pointsize20_tga_end   =
    (const unsigned char *const) (char*)&data_ttf_large_ttf_pointsize20_tga_end_;

extern unsigned char *data_ttf_large_ttf_pointsize30_data_start_ asm("data_ttf_large_ttf_pointsize30_data_start_");
extern unsigned char *data_ttf_large_ttf_pointsize30_data_end_ asm("data_ttf_large_ttf_pointsize30_data_end_");
static const unsigned char *const data_ttf_large_ttf_pointsize30_data_start =
    (const unsigned char *const) (char*)&data_ttf_large_ttf_pointsize30_data_start_;
static const unsigned char *const data_ttf_large_ttf_pointsize30_data_end   =
    (const unsigned char *const) (char*)&data_ttf_large_ttf_pointsize30_data_end_;

extern unsigned char *data_ttf_large_ttf_pointsize30_tga_start_ asm("data_ttf_large_ttf_pointsize30_tga_start_");
extern unsigned char *data_ttf_large_ttf_pointsize30_tga_end_ asm("data_ttf_large_ttf_pointsize30_tga_end_");
static const unsigned char *const data_ttf_large_ttf_pointsize30_tga_start =
    (const unsigned char *const) (char*)&data_ttf_large_ttf_pointsize30_tga_start_;
static const unsigned char *const data_ttf_large_ttf_pointsize30_tga_end   =
    (const unsigned char *const) (char*)&data_ttf_large_ttf_pointsize30_tga_end_;

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

extern unsigned char *data_ttf_LICENSE_txt_start_ asm("data_ttf_LICENSE_txt_start_");
extern unsigned char *data_ttf_LICENSE_txt_end_ asm("data_ttf_LICENSE_txt_end_");
static const unsigned char *const data_ttf_LICENSE_txt_start =
    (const unsigned char *const) (char*)&data_ttf_LICENSE_txt_start_;
static const unsigned char *const data_ttf_LICENSE_txt_end   =
    (const unsigned char *const) (char*)&data_ttf_LICENSE_txt_end_;

extern unsigned char *data_ttf_med_ttf_pointsize12_data_start_ asm("data_ttf_med_ttf_pointsize12_data_start_");
extern unsigned char *data_ttf_med_ttf_pointsize12_data_end_ asm("data_ttf_med_ttf_pointsize12_data_end_");
static const unsigned char *const data_ttf_med_ttf_pointsize12_data_start =
    (const unsigned char *const) (char*)&data_ttf_med_ttf_pointsize12_data_start_;
static const unsigned char *const data_ttf_med_ttf_pointsize12_data_end   =
    (const unsigned char *const) (char*)&data_ttf_med_ttf_pointsize12_data_end_;

extern unsigned char *data_ttf_med_ttf_pointsize12_tga_start_ asm("data_ttf_med_ttf_pointsize12_tga_start_");
extern unsigned char *data_ttf_med_ttf_pointsize12_tga_end_ asm("data_ttf_med_ttf_pointsize12_tga_end_");
static const unsigned char *const data_ttf_med_ttf_pointsize12_tga_start =
    (const unsigned char *const) (char*)&data_ttf_med_ttf_pointsize12_tga_start_;
static const unsigned char *const data_ttf_med_ttf_pointsize12_tga_end   =
    (const unsigned char *const) (char*)&data_ttf_med_ttf_pointsize12_tga_end_;

extern unsigned char *data_ttf_med_ttf_pointsize20_data_start_ asm("data_ttf_med_ttf_pointsize20_data_start_");
extern unsigned char *data_ttf_med_ttf_pointsize20_data_end_ asm("data_ttf_med_ttf_pointsize20_data_end_");
static const unsigned char *const data_ttf_med_ttf_pointsize20_data_start =
    (const unsigned char *const) (char*)&data_ttf_med_ttf_pointsize20_data_start_;
static const unsigned char *const data_ttf_med_ttf_pointsize20_data_end   =
    (const unsigned char *const) (char*)&data_ttf_med_ttf_pointsize20_data_end_;

extern unsigned char *data_ttf_med_ttf_pointsize20_tga_start_ asm("data_ttf_med_ttf_pointsize20_tga_start_");
extern unsigned char *data_ttf_med_ttf_pointsize20_tga_end_ asm("data_ttf_med_ttf_pointsize20_tga_end_");
static const unsigned char *const data_ttf_med_ttf_pointsize20_tga_start =
    (const unsigned char *const) (char*)&data_ttf_med_ttf_pointsize20_tga_start_;
static const unsigned char *const data_ttf_med_ttf_pointsize20_tga_end   =
    (const unsigned char *const) (char*)&data_ttf_med_ttf_pointsize20_tga_end_;

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

extern unsigned char *data_ttf_small_ttf_pointsize14_data_start_ asm("data_ttf_small_ttf_pointsize14_data_start_");
extern unsigned char *data_ttf_small_ttf_pointsize14_data_end_ asm("data_ttf_small_ttf_pointsize14_data_end_");
static const unsigned char *const data_ttf_small_ttf_pointsize14_data_start =
    (const unsigned char *const) (char*)&data_ttf_small_ttf_pointsize14_data_start_;
static const unsigned char *const data_ttf_small_ttf_pointsize14_data_end   =
    (const unsigned char *const) (char*)&data_ttf_small_ttf_pointsize14_data_end_;

extern unsigned char *data_ttf_small_ttf_pointsize14_tga_start_ asm("data_ttf_small_ttf_pointsize14_tga_start_");
extern unsigned char *data_ttf_small_ttf_pointsize14_tga_end_ asm("data_ttf_small_ttf_pointsize14_tga_end_");
static const unsigned char *const data_ttf_small_ttf_pointsize14_tga_start =
    (const unsigned char *const) (char*)&data_ttf_small_ttf_pointsize14_tga_start_;
static const unsigned char *const data_ttf_small_ttf_pointsize14_tga_end   =
    (const unsigned char *const) (char*)&data_ttf_small_ttf_pointsize14_tga_end_;

extern unsigned char *data_ttf_small_ttf_pointsize16_data_start_ asm("data_ttf_small_ttf_pointsize16_data_start_");
extern unsigned char *data_ttf_small_ttf_pointsize16_data_end_ asm("data_ttf_small_ttf_pointsize16_data_end_");
static const unsigned char *const data_ttf_small_ttf_pointsize16_data_start =
    (const unsigned char *const) (char*)&data_ttf_small_ttf_pointsize16_data_start_;
static const unsigned char *const data_ttf_small_ttf_pointsize16_data_end   =
    (const unsigned char *const) (char*)&data_ttf_small_ttf_pointsize16_data_end_;

extern unsigned char *data_ttf_small_ttf_pointsize16_tga_start_ asm("data_ttf_small_ttf_pointsize16_tga_start_");
extern unsigned char *data_ttf_small_ttf_pointsize16_tga_end_ asm("data_ttf_small_ttf_pointsize16_tga_end_");
static const unsigned char *const data_ttf_small_ttf_pointsize16_tga_start =
    (const unsigned char *const) (char*)&data_ttf_small_ttf_pointsize16_tga_start_;
static const unsigned char *const data_ttf_small_ttf_pointsize16_tga_end   =
    (const unsigned char *const) (char*)&data_ttf_small_ttf_pointsize16_tga_end_;

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

extern unsigned char *data_ttf_small_ttf_pointsize30_data_start_ asm("data_ttf_small_ttf_pointsize30_data_start_");
extern unsigned char *data_ttf_small_ttf_pointsize30_data_end_ asm("data_ttf_small_ttf_pointsize30_data_end_");
static const unsigned char *const data_ttf_small_ttf_pointsize30_data_start =
    (const unsigned char *const) (char*)&data_ttf_small_ttf_pointsize30_data_start_;
static const unsigned char *const data_ttf_small_ttf_pointsize30_data_end   =
    (const unsigned char *const) (char*)&data_ttf_small_ttf_pointsize30_data_end_;

extern unsigned char *data_ttf_small_ttf_pointsize30_tga_start_ asm("data_ttf_small_ttf_pointsize30_tga_start_");
extern unsigned char *data_ttf_small_ttf_pointsize30_tga_end_ asm("data_ttf_small_ttf_pointsize30_tga_end_");
static const unsigned char *const data_ttf_small_ttf_pointsize30_tga_start =
    (const unsigned char *const) (char*)&data_ttf_small_ttf_pointsize30_tga_start_;
static const unsigned char *const data_ttf_small_ttf_pointsize30_tga_end   =
    (const unsigned char *const) (char*)&data_ttf_small_ttf_pointsize30_tga_end_;

extern unsigned char *data_ttf_small_ttf_pointsize8_data_start_ asm("data_ttf_small_ttf_pointsize8_data_start_");
extern unsigned char *data_ttf_small_ttf_pointsize8_data_end_ asm("data_ttf_small_ttf_pointsize8_data_end_");
static const unsigned char *const data_ttf_small_ttf_pointsize8_data_start =
    (const unsigned char *const) (char*)&data_ttf_small_ttf_pointsize8_data_start_;
static const unsigned char *const data_ttf_small_ttf_pointsize8_data_end   =
    (const unsigned char *const) (char*)&data_ttf_small_ttf_pointsize8_data_end_;

extern unsigned char *data_ttf_small_ttf_pointsize8_tga_start_ asm("data_ttf_small_ttf_pointsize8_tga_start_");
extern unsigned char *data_ttf_small_ttf_pointsize8_tga_end_ asm("data_ttf_small_ttf_pointsize8_tga_end_");
static const unsigned char *const data_ttf_small_ttf_pointsize8_tga_start =
    (const unsigned char *const) (char*)&data_ttf_small_ttf_pointsize8_tga_start_;
static const unsigned char *const data_ttf_small_ttf_pointsize8_tga_end   =
    (const unsigned char *const) (char*)&data_ttf_small_ttf_pointsize8_tga_end_;

ramdisk_t ramdisk_data[] = {
{
    /* filename */ "data/gfx/explosion.png",
},
{
    /* filename */ "data/gfx/gorynlich.png",
},
{
    /* filename */ "data/gfx/help1.png",
},
{
    /* filename */ "data/gfx/help2.png",
},
{
    /* filename */ "data/gfx/help3.png",
},
{
    /* filename */ "data/gfx/pipes.png",
},
{
    /* filename */ "data/gfx/plant.png",
},
{
    /* filename */ "data/gfx/road.png",
},
{
    /* filename */ "data/gfx/rocket.png",
},
{
    /* filename */ "data/gfx/shell.png",
},
{
    /* filename */ "data/gfx/sprites_large.png",
},
{
    /* filename */ "data/gfx/sprites_med.png",
},
{
    /* filename */ "data/gfx/sprites_med2.png",
},
{
    /* filename */ "data/gfx/sprites_small.png",
},
{
    /* filename */ "data/gfx/stars.png",
},
{
    /* filename */ "data/map/jigsaw.map",
},
{
    /* filename */ "data/map/map_light.data",
},
{
    /* filename */ "data/music/hiscore.wav",
},
{
    /* filename */ "data/music/intro.wav",
},
{
    /* filename */ "data/music/music.wav",
},
{
    /* filename */ "data/sound/chomp.wav",
},
{
    /* filename */ "data/sound/chomp2.wav",
},
{
    /* filename */ "data/sound/click.wav",
},
{
    /* filename */ "data/sound/doorbell.wav",
},
{
    /* filename */ "data/sound/explosion.wav",
},
{
    /* filename */ "data/sound/letter.wav",
},
{
    /* filename */ "data/sound/level_end.wav",
},
{
    /* filename */ "data/sound/paper.wav",
},
{
    /* filename */ "data/sound/powerup.wav",
},
{
    /* filename */ "data/sound/ratdeath.wav",
},
{
    /* filename */ "data/sound/rocket.wav",
},
{
    /* filename */ "data/sound/slime.wav",
},
{
    /* filename */ "data/sound/spam.wav",
},
{
    /* filename */ "data/sound/thief.wav",
},
{
    /* filename */ "data/things/boulder_0",
},
{
    /* filename */ "data/things/exit",
},
{
    /* filename */ "data/things/gem_0",
},
{
    /* filename */ "data/things/gem_1",
},
{
    /* filename */ "data/things/gem_2",
},
{
    /* filename */ "data/things/gem_3",
},
{
    /* filename */ "data/things/gem_4",
},
{
    /* filename */ "data/things/gem_5",
},
{
    /* filename */ "data/things/gem_6",
},
{
    /* filename */ "data/things/gem_7",
},
{
    /* filename */ "data/things/ladder",
},
{
    /* filename */ "data/things/lava_0",
},
{
    /* filename */ "data/things/player_0",
},
{
    /* filename */ "data/things/rock_0",
},
{
    /* filename */ "data/things/rock_1",
},
{
    /* filename */ "data/things/skeleton",
},
{
    /* filename */ "data/things/water_0",
},
{
    /* filename */ "data/ttf/large.ttf_pointsize20.data",
},
{
    /* filename */ "data/ttf/large.ttf_pointsize20.tga",
},
{
    /* filename */ "data/ttf/large.ttf_pointsize30.data",
},
{
    /* filename */ "data/ttf/large.ttf_pointsize30.tga",
},
{
    /* filename */ "data/ttf/large.ttf_pointsize40.data",
},
{
    /* filename */ "data/ttf/large.ttf_pointsize40.tga",
},
{
    /* filename */ "data/ttf/LICENSE.txt",
},
{
    /* filename */ "data/ttf/med.ttf_pointsize12.data",
},
{
    /* filename */ "data/ttf/med.ttf_pointsize12.tga",
},
{
    /* filename */ "data/ttf/med.ttf_pointsize20.data",
},
{
    /* filename */ "data/ttf/med.ttf_pointsize20.tga",
},
{
    /* filename */ "data/ttf/med.ttf_pointsize30.data",
},
{
    /* filename */ "data/ttf/med.ttf_pointsize30.tga",
},
{
    /* filename */ "data/ttf/small.ttf_pointsize14.data",
},
{
    /* filename */ "data/ttf/small.ttf_pointsize14.tga",
},
{
    /* filename */ "data/ttf/small.ttf_pointsize16.data",
},
{
    /* filename */ "data/ttf/small.ttf_pointsize16.tga",
},
{
    /* filename */ "data/ttf/small.ttf_pointsize20.data",
},
{
    /* filename */ "data/ttf/small.ttf_pointsize20.tga",
},
{
    /* filename */ "data/ttf/small.ttf_pointsize30.data",
},
{
    /* filename */ "data/ttf/small.ttf_pointsize30.tga",
},
{
    /* filename */ "data/ttf/small.ttf_pointsize8.data",
},
{
    /* filename */ "data/ttf/small.ttf_pointsize8.tga",
},
    {0},
};

void ramdisk_init (void)
{
    ramdisk_data[0].data = data_gfx_explosion_png_start;
    ramdisk_data[0].len = data_gfx_explosion_png_end - data_gfx_explosion_png_start;
    ramdisk_data[1].data = data_gfx_gorynlich_png_start;
    ramdisk_data[1].len = data_gfx_gorynlich_png_end - data_gfx_gorynlich_png_start;
    ramdisk_data[2].data = data_gfx_help1_png_start;
    ramdisk_data[2].len = data_gfx_help1_png_end - data_gfx_help1_png_start;
    ramdisk_data[3].data = data_gfx_help2_png_start;
    ramdisk_data[3].len = data_gfx_help2_png_end - data_gfx_help2_png_start;
    ramdisk_data[4].data = data_gfx_help3_png_start;
    ramdisk_data[4].len = data_gfx_help3_png_end - data_gfx_help3_png_start;
    ramdisk_data[5].data = data_gfx_pipes_png_start;
    ramdisk_data[5].len = data_gfx_pipes_png_end - data_gfx_pipes_png_start;
    ramdisk_data[6].data = data_gfx_plant_png_start;
    ramdisk_data[6].len = data_gfx_plant_png_end - data_gfx_plant_png_start;
    ramdisk_data[7].data = data_gfx_road_png_start;
    ramdisk_data[7].len = data_gfx_road_png_end - data_gfx_road_png_start;
    ramdisk_data[8].data = data_gfx_rocket_png_start;
    ramdisk_data[8].len = data_gfx_rocket_png_end - data_gfx_rocket_png_start;
    ramdisk_data[9].data = data_gfx_shell_png_start;
    ramdisk_data[9].len = data_gfx_shell_png_end - data_gfx_shell_png_start;
    ramdisk_data[10].data = data_gfx_sprites_large_png_start;
    ramdisk_data[10].len = data_gfx_sprites_large_png_end - data_gfx_sprites_large_png_start;
    ramdisk_data[11].data = data_gfx_sprites_med_png_start;
    ramdisk_data[11].len = data_gfx_sprites_med_png_end - data_gfx_sprites_med_png_start;
    ramdisk_data[12].data = data_gfx_sprites_med2_png_start;
    ramdisk_data[12].len = data_gfx_sprites_med2_png_end - data_gfx_sprites_med2_png_start;
    ramdisk_data[13].data = data_gfx_sprites_small_png_start;
    ramdisk_data[13].len = data_gfx_sprites_small_png_end - data_gfx_sprites_small_png_start;
    ramdisk_data[14].data = data_gfx_stars_png_start;
    ramdisk_data[14].len = data_gfx_stars_png_end - data_gfx_stars_png_start;
    ramdisk_data[15].data = data_map_jigsaw_map_start;
    ramdisk_data[15].len = data_map_jigsaw_map_end - data_map_jigsaw_map_start;
    ramdisk_data[16].data = data_map_map_light_data_start;
    ramdisk_data[16].len = data_map_map_light_data_end - data_map_map_light_data_start;
    ramdisk_data[17].data = data_music_hiscore_wav_start;
    ramdisk_data[17].len = data_music_hiscore_wav_end - data_music_hiscore_wav_start;
    ramdisk_data[18].data = data_music_intro_wav_start;
    ramdisk_data[18].len = data_music_intro_wav_end - data_music_intro_wav_start;
    ramdisk_data[19].data = data_music_music_wav_start;
    ramdisk_data[19].len = data_music_music_wav_end - data_music_music_wav_start;
    ramdisk_data[20].data = data_sound_chomp_wav_start;
    ramdisk_data[20].len = data_sound_chomp_wav_end - data_sound_chomp_wav_start;
    ramdisk_data[21].data = data_sound_chomp2_wav_start;
    ramdisk_data[21].len = data_sound_chomp2_wav_end - data_sound_chomp2_wav_start;
    ramdisk_data[22].data = data_sound_click_wav_start;
    ramdisk_data[22].len = data_sound_click_wav_end - data_sound_click_wav_start;
    ramdisk_data[23].data = data_sound_doorbell_wav_start;
    ramdisk_data[23].len = data_sound_doorbell_wav_end - data_sound_doorbell_wav_start;
    ramdisk_data[24].data = data_sound_explosion_wav_start;
    ramdisk_data[24].len = data_sound_explosion_wav_end - data_sound_explosion_wav_start;
    ramdisk_data[25].data = data_sound_letter_wav_start;
    ramdisk_data[25].len = data_sound_letter_wav_end - data_sound_letter_wav_start;
    ramdisk_data[26].data = data_sound_level_end_wav_start;
    ramdisk_data[26].len = data_sound_level_end_wav_end - data_sound_level_end_wav_start;
    ramdisk_data[27].data = data_sound_paper_wav_start;
    ramdisk_data[27].len = data_sound_paper_wav_end - data_sound_paper_wav_start;
    ramdisk_data[28].data = data_sound_powerup_wav_start;
    ramdisk_data[28].len = data_sound_powerup_wav_end - data_sound_powerup_wav_start;
    ramdisk_data[29].data = data_sound_ratdeath_wav_start;
    ramdisk_data[29].len = data_sound_ratdeath_wav_end - data_sound_ratdeath_wav_start;
    ramdisk_data[30].data = data_sound_rocket_wav_start;
    ramdisk_data[30].len = data_sound_rocket_wav_end - data_sound_rocket_wav_start;
    ramdisk_data[31].data = data_sound_slime_wav_start;
    ramdisk_data[31].len = data_sound_slime_wav_end - data_sound_slime_wav_start;
    ramdisk_data[32].data = data_sound_spam_wav_start;
    ramdisk_data[32].len = data_sound_spam_wav_end - data_sound_spam_wav_start;
    ramdisk_data[33].data = data_sound_thief_wav_start;
    ramdisk_data[33].len = data_sound_thief_wav_end - data_sound_thief_wav_start;
    ramdisk_data[34].data = data_things_boulder_0_start;
    ramdisk_data[34].len = data_things_boulder_0_end - data_things_boulder_0_start;
    ramdisk_data[35].data = data_things_exit_start;
    ramdisk_data[35].len = data_things_exit_end - data_things_exit_start;
    ramdisk_data[36].data = data_things_gem_0_start;
    ramdisk_data[36].len = data_things_gem_0_end - data_things_gem_0_start;
    ramdisk_data[37].data = data_things_gem_1_start;
    ramdisk_data[37].len = data_things_gem_1_end - data_things_gem_1_start;
    ramdisk_data[38].data = data_things_gem_2_start;
    ramdisk_data[38].len = data_things_gem_2_end - data_things_gem_2_start;
    ramdisk_data[39].data = data_things_gem_3_start;
    ramdisk_data[39].len = data_things_gem_3_end - data_things_gem_3_start;
    ramdisk_data[40].data = data_things_gem_4_start;
    ramdisk_data[40].len = data_things_gem_4_end - data_things_gem_4_start;
    ramdisk_data[41].data = data_things_gem_5_start;
    ramdisk_data[41].len = data_things_gem_5_end - data_things_gem_5_start;
    ramdisk_data[42].data = data_things_gem_6_start;
    ramdisk_data[42].len = data_things_gem_6_end - data_things_gem_6_start;
    ramdisk_data[43].data = data_things_gem_7_start;
    ramdisk_data[43].len = data_things_gem_7_end - data_things_gem_7_start;
    ramdisk_data[44].data = data_things_ladder_start;
    ramdisk_data[44].len = data_things_ladder_end - data_things_ladder_start;
    ramdisk_data[45].data = data_things_lava_0_start;
    ramdisk_data[45].len = data_things_lava_0_end - data_things_lava_0_start;
    ramdisk_data[46].data = data_things_player_0_start;
    ramdisk_data[46].len = data_things_player_0_end - data_things_player_0_start;
    ramdisk_data[47].data = data_things_rock_0_start;
    ramdisk_data[47].len = data_things_rock_0_end - data_things_rock_0_start;
    ramdisk_data[48].data = data_things_rock_1_start;
    ramdisk_data[48].len = data_things_rock_1_end - data_things_rock_1_start;
    ramdisk_data[49].data = data_things_skeleton_start;
    ramdisk_data[49].len = data_things_skeleton_end - data_things_skeleton_start;
    ramdisk_data[50].data = data_things_water_0_start;
    ramdisk_data[50].len = data_things_water_0_end - data_things_water_0_start;
    ramdisk_data[51].data = data_ttf_large_ttf_pointsize20_data_start;
    ramdisk_data[51].len = data_ttf_large_ttf_pointsize20_data_end - data_ttf_large_ttf_pointsize20_data_start;
    ramdisk_data[52].data = data_ttf_large_ttf_pointsize20_tga_start;
    ramdisk_data[52].len = data_ttf_large_ttf_pointsize20_tga_end - data_ttf_large_ttf_pointsize20_tga_start;
    ramdisk_data[53].data = data_ttf_large_ttf_pointsize30_data_start;
    ramdisk_data[53].len = data_ttf_large_ttf_pointsize30_data_end - data_ttf_large_ttf_pointsize30_data_start;
    ramdisk_data[54].data = data_ttf_large_ttf_pointsize30_tga_start;
    ramdisk_data[54].len = data_ttf_large_ttf_pointsize30_tga_end - data_ttf_large_ttf_pointsize30_tga_start;
    ramdisk_data[55].data = data_ttf_large_ttf_pointsize40_data_start;
    ramdisk_data[55].len = data_ttf_large_ttf_pointsize40_data_end - data_ttf_large_ttf_pointsize40_data_start;
    ramdisk_data[56].data = data_ttf_large_ttf_pointsize40_tga_start;
    ramdisk_data[56].len = data_ttf_large_ttf_pointsize40_tga_end - data_ttf_large_ttf_pointsize40_tga_start;
    ramdisk_data[57].data = data_ttf_LICENSE_txt_start;
    ramdisk_data[57].len = data_ttf_LICENSE_txt_end - data_ttf_LICENSE_txt_start;
    ramdisk_data[58].data = data_ttf_med_ttf_pointsize12_data_start;
    ramdisk_data[58].len = data_ttf_med_ttf_pointsize12_data_end - data_ttf_med_ttf_pointsize12_data_start;
    ramdisk_data[59].data = data_ttf_med_ttf_pointsize12_tga_start;
    ramdisk_data[59].len = data_ttf_med_ttf_pointsize12_tga_end - data_ttf_med_ttf_pointsize12_tga_start;
    ramdisk_data[60].data = data_ttf_med_ttf_pointsize20_data_start;
    ramdisk_data[60].len = data_ttf_med_ttf_pointsize20_data_end - data_ttf_med_ttf_pointsize20_data_start;
    ramdisk_data[61].data = data_ttf_med_ttf_pointsize20_tga_start;
    ramdisk_data[61].len = data_ttf_med_ttf_pointsize20_tga_end - data_ttf_med_ttf_pointsize20_tga_start;
    ramdisk_data[62].data = data_ttf_med_ttf_pointsize30_data_start;
    ramdisk_data[62].len = data_ttf_med_ttf_pointsize30_data_end - data_ttf_med_ttf_pointsize30_data_start;
    ramdisk_data[63].data = data_ttf_med_ttf_pointsize30_tga_start;
    ramdisk_data[63].len = data_ttf_med_ttf_pointsize30_tga_end - data_ttf_med_ttf_pointsize30_tga_start;
    ramdisk_data[64].data = data_ttf_small_ttf_pointsize14_data_start;
    ramdisk_data[64].len = data_ttf_small_ttf_pointsize14_data_end - data_ttf_small_ttf_pointsize14_data_start;
    ramdisk_data[65].data = data_ttf_small_ttf_pointsize14_tga_start;
    ramdisk_data[65].len = data_ttf_small_ttf_pointsize14_tga_end - data_ttf_small_ttf_pointsize14_tga_start;
    ramdisk_data[66].data = data_ttf_small_ttf_pointsize16_data_start;
    ramdisk_data[66].len = data_ttf_small_ttf_pointsize16_data_end - data_ttf_small_ttf_pointsize16_data_start;
    ramdisk_data[67].data = data_ttf_small_ttf_pointsize16_tga_start;
    ramdisk_data[67].len = data_ttf_small_ttf_pointsize16_tga_end - data_ttf_small_ttf_pointsize16_tga_start;
    ramdisk_data[68].data = data_ttf_small_ttf_pointsize20_data_start;
    ramdisk_data[68].len = data_ttf_small_ttf_pointsize20_data_end - data_ttf_small_ttf_pointsize20_data_start;
    ramdisk_data[69].data = data_ttf_small_ttf_pointsize20_tga_start;
    ramdisk_data[69].len = data_ttf_small_ttf_pointsize20_tga_end - data_ttf_small_ttf_pointsize20_tga_start;
    ramdisk_data[70].data = data_ttf_small_ttf_pointsize30_data_start;
    ramdisk_data[70].len = data_ttf_small_ttf_pointsize30_data_end - data_ttf_small_ttf_pointsize30_data_start;
    ramdisk_data[71].data = data_ttf_small_ttf_pointsize30_tga_start;
    ramdisk_data[71].len = data_ttf_small_ttf_pointsize30_tga_end - data_ttf_small_ttf_pointsize30_tga_start;
    ramdisk_data[72].data = data_ttf_small_ttf_pointsize8_data_start;
    ramdisk_data[72].len = data_ttf_small_ttf_pointsize8_data_end - data_ttf_small_ttf_pointsize8_data_start;
    ramdisk_data[73].data = data_ttf_small_ttf_pointsize8_tga_start;
    ramdisk_data[73].len = data_ttf_small_ttf_pointsize8_tga_end - data_ttf_small_ttf_pointsize8_tga_start;
}
