COMPILER_FLAGS=-g -O2 -I/usr/local/include/SDL -D_GNU_SOURCE=1 -D_THREAD_SAFE # AUTOGEN
    
CLANG_COMPILER_WARNINGS=-Wall -fmessage-length=0 -fdiagnostics-print-source-range-info -fdiagnostics-show-category=id -fdiagnostics-parseable-fixits -Wno-trigraphs -Wmissing-prototypes -Wreturn-type -Wparentheses -Wswitch -Wno-unused-parameter -Wunused-variable -Wunused-value -Wno-sign-conversion # AUTOGEN
GCC_COMPILER_WARNINGS=-Wall -fmessage-length=0 -Wno-trigraphs -Wmissing-prototypes -Wreturn-type -Wparentheses -Wswitch -Wno-unused-parameter -Wunused-variable -Wunused-value # AUTOGEN
GXX_COMPILER_WARNINGS=-Wall -fmessage-length=0 -Wno-trigraphs -Wreturn-type -Wparentheses -Wswitch -Wno-unused-parameter -Wunused-variable -Wunused-value # AUTOGEN
COMPILER_WARNINGS=$(GCC_COMPILER_WARNINGS) # AUTOGEN
COMPILER_WARNINGS=$(GXX_COMPILER_WARNINGS) # AUTOGEN
COMPILER_WARNINGS=$(CLANG_COMPILER_WARNINGS) # AUTOGEN
CC=clang # AUTOGEN
# CC=gcc # AUTOGEN
# CC=cc # AUTOGEN
# CC=g++ # AUTOGEN
    
EXE= # AUTOGEN
LDLIBS=-L/usr/local/lib /usr/local/lib/libSDLmain.a /usr/local/lib/libSDL.a -Wl,-framework,OpenGL -Wl,-framework,Cocoa -Wl,-framework,ApplicationServices -Wl,-framework,Carbon -Wl,-framework,AudioToolbox -Wl,-framework,AudioUnit -Wl,-framework,IOKit -lSDL_ttf -lSDL_mixer # AUTOGEN
CFLAGS=$(COMPILER_FLAGS) $(COMPILER_WARNINGS) # AUTOGEN


NAME=gorynlich
OBJDIR=.o

GAME_OBJECTS=				\
    $(OBJDIR)/backtrace.o 		\
    $(OBJDIR)/color.o			\
    $(OBJDIR)/command.o 		\
    $(OBJDIR)/config.o			\
    $(OBJDIR)/demarshal.o 		\
    $(OBJDIR)/dir.o 			\
    $(OBJDIR)/dir_test.o 		\
    $(OBJDIR)/enum_test.o 		\
    $(OBJDIR)/file.o 			\
    $(OBJDIR)/font.o 			\
    $(OBJDIR)/gl.o 			\
    $(OBJDIR)/glshapes.o 		\
    $(OBJDIR)/item.o 			\
    $(OBJDIR)/level.o 			\
    $(OBJDIR)/log.o 			\
    $(OBJDIR)/main.o 			\
    $(OBJDIR)/map.o 			\
    $(OBJDIR)/marshal.o 		\
    $(OBJDIR)/marshal_test.o 		\
    $(OBJDIR)/math.o 			\
    $(OBJDIR)/music.o 			\
    $(OBJDIR)/sound.o 			\
    $(OBJDIR)/miniz.o 			\
    $(OBJDIR)/miniz_test.o 		\
    $(OBJDIR)/mzip_file.o 		\
    $(OBJDIR)/mzip_file_test.o 		\
    $(OBJDIR)/mzip_lib.o 		\
    $(OBJDIR)/pixel.o 			\
    $(OBJDIR)/player.o 			\
    $(OBJDIR)/point.o 			\
    $(OBJDIR)/ptrcheck.o 		\
    $(OBJDIR)/ptrcheck_test.o 		\
    $(OBJDIR)/ramdisk_read.o 		\
    $(OBJDIR)/resource.o 		\
    $(OBJDIR)/sdl.o 			\
    $(OBJDIR)/slre.o 			\
    $(OBJDIR)/stb_image.o 		\
    $(OBJDIR)/string.o 			\
    $(OBJDIR)/string_ext.o 		\
    $(OBJDIR)/string_test.o 		\
    $(OBJDIR)/tex.o 			\
    $(OBJDIR)/thing.o 			\
    $(OBJDIR)/thing_ai.o 		\
    $(OBJDIR)/thing_item.o 		\
    $(OBJDIR)/thing_tick.o 		\
    $(OBJDIR)/thing_timer.o 		\
    $(OBJDIR)/thing_anim.o 		\
    $(OBJDIR)/thing_collision.o 	\
    $(OBJDIR)/thing_template.o 		\
    $(OBJDIR)/thing_tile.o 		\
    $(OBJDIR)/tile.o 			\
    $(OBJDIR)/timer.o 			\
    $(OBJDIR)/init_fn.o 		\
    $(OBJDIR)/time.o 			\
    $(OBJDIR)/token.o 			\
    $(OBJDIR)/tree.o 			\
    $(OBJDIR)/tree_test.o 		\
    $(OBJDIR)/ttf.o 			\
    $(OBJDIR)/util.o 			\
    $(OBJDIR)/wid.o 			\
    $(OBJDIR)/wid_console.o 		\
    $(OBJDIR)/wid_dirlist.o 		\
    $(OBJDIR)/wid_text_input.o 		\
    $(OBJDIR)/wid_editor.o 		\
    $(OBJDIR)/wid_editor_buttons.o 	\
    $(OBJDIR)/wid_editor_map.o 		\
    $(OBJDIR)/wid_game_map.o 		\
    $(OBJDIR)/wid_splash.o 		\
    $(OBJDIR)/wid_intro.o 		\
    $(OBJDIR)/wid_intro_about.o 	\
    $(OBJDIR)/wid_intro_settings.o 	\
    $(OBJDIR)/wid_hiscore.o 		\
    $(OBJDIR)/wid_intro_help.o 		\
    $(OBJDIR)/wid_intro_guide.o 	\
    $(OBJDIR)/wid_game_over.o 		\
    $(OBJDIR)/wid_popup.o 		\
    $(OBJDIR)/wid_test.o 		\
    $(OBJDIR)/wid_textbox.o 		\
    $(OBJDIR)/wid_tooltip.o 		\
    $(OBJDIR)/wid_button.o 		\

RAMDISK_WRITE_OBJECTS=			\
    $(OBJDIR)/ramdisk_write.o 		\

RAMDISK_DATA_OBJECTS=			\
    $(OBJDIR)/ramdisk_data_fragment_0.o \
    $(OBJDIR)/ramdisk_data_fragment_1.o \
    $(OBJDIR)/ramdisk_data_fragment_2.o \
    $(OBJDIR)/ramdisk_data_fragment_3.o \
    $(OBJDIR)/ramdisk_data_fragment_final.o \

MZIP_OBJECTS=				\
    $(OBJDIR)/util_basic.o 		\
    $(OBJDIR)/string.o 			\
    $(OBJDIR)/file.o 			\
    $(OBJDIR)/miniz.o 			\
    $(OBJDIR)/mzip_lib.o 		\
    $(OBJDIR)/mzip_file.o 		\
    $(OBJDIR)/mzip.o 			\

MUNZIP_OBJECTS=				\
    $(OBJDIR)/util_basic.o 		\
    $(OBJDIR)/file.o 			\
    $(OBJDIR)/string.o 			\
    $(OBJDIR)/miniz.o 			\
    $(OBJDIR)/mzip_lib.o 		\
    $(OBJDIR)/mzip_file.o 		\
    $(OBJDIR)/munzip.o 			\

#
# compile
#
$(OBJDIR)/%.o: %.c
	@echo $(CC) $(CFLAGS) -c -o $@ $<
	@$(CC) $(CFLAGS) -c -o $@ $<

#
# link
#
ALL_GAME_OBJECTS=$(GAME_OBJECTS) $(RAMDISK_DATA_OBJECTS)

TARGET_GAME=$(NAME)$(EXE)

$(TARGET_GAME): $(ALL_GAME_OBJECTS)
	$(CC) $(ALL_GAME_OBJECTS) $(LDLIBS) -o ../$(TARGET_GAME)

#
# make_ramdisk
#
TARGET_RAMDISK=make_ramdisk$(EXE)

$(TARGET_RAMDISK): $(RAMDISK_WRITE_OBJECTS)
	@echo $(CC) $(CFLAGS) $(RAMDISK_WRITE_OBJECTS) -lz -o $(TARGET_RAMDISK)
	@$(CC) $(CFLAGS) $(RAMDISK_WRITE_OBJECTS) -lz -o $(TARGET_RAMDISK)

#
# mzip
#
TARGET_MZIP=mzip$(EXE)

$(TARGET_MZIP): $(MZIP_OBJECTS)
	@echo $(CC) $(CFLAGS) $(MZIP_OBJECTS) -lz -o $(TARGET_MZIP)
	@$(CC) $(CFLAGS) $(MZIP_OBJECTS) -lz -o $(TARGET_MZIP)

#
# munzip
#
TARGET_MUNZIP=munzip$(EXE)

$(TARGET_MUNZIP): $(MUNZIP_OBJECTS)
	@echo $(CC) $(CFLAGS) $(MUNZIP_OBJECTS) -lz -o $(TARGET_MUNZIP)
	@$(CC) $(CFLAGS) $(MUNZIP_OBJECTS) -lz -o $(TARGET_MUNZIP)

#
# To force clean and avoid "up to date" warning.
#
.PHONY: clean
.PHONY: clobber

clean:
	rm -rf ../$(TARGET_GAME) ../stdout.txt ../stderr.txt
	rm -rf $(GAME_OBJECTS) $(TARGET_GAME) stdout.txt stderr.txt
	rm -rf $(TARGET_MZIP)
	rm -rf $(TARGET_MUNZIP)
	rm -rf $(TARGET_RAMDISK)

clobber: clean
	rm -rf $(OBJDIR)
	mkdir -p $(OBJDIR)

other: $(TARGET_RAMDISK) $(TARGET_MZIP) $(TARGET_MUNZIP)

valgrind:
	valgrind --gen-suppressions=all --leak-check=full --suppressions=valgrind.suppress --error-limit=no ../$(NAME)

all: $(TARGET_GAME)
# DO NOT DELETE

.o/backtrace.o: /usr/include/execinfo.h /usr/include/sys/cdefs.h
.o/backtrace.o: /usr/include/stdio.h /usr/include/_types.h
.o/backtrace.o: /usr/include/sys/_types.h /usr/include/machine/_types.h
.o/backtrace.o: /usr/include/i386/_types.h /usr/include/secure/_stdio.h
.o/backtrace.o: /usr/include/secure/_common.h /usr/include/stdlib.h
.o/backtrace.o: /usr/include/Availability.h
.o/backtrace.o: /usr/include/AvailabilityInternal.h /usr/include/sys/wait.h
.o/backtrace.o: /usr/include/sys/signal.h /usr/include/sys/appleapiopts.h
.o/backtrace.o: /usr/include/machine/signal.h /usr/include/i386/signal.h
.o/backtrace.o: /usr/include/i386/_structs.h /usr/include/sys/_structs.h
.o/backtrace.o: /usr/include/machine/_structs.h /usr/include/sys/resource.h
.o/backtrace.o: /usr/include/machine/endian.h /usr/include/i386/endian.h
.o/backtrace.o: /usr/include/sys/_endian.h
.o/backtrace.o: /usr/include/libkern/_OSByteOrder.h
.o/backtrace.o: /usr/include/libkern/i386/_OSByteOrder.h
.o/backtrace.o: /usr/include/alloca.h /usr/include/machine/types.h
.o/backtrace.o: /usr/include/i386/types.h /usr/include/string.h
.o/backtrace.o: /usr/include/secure/_string.h main.h /usr/include/stdint.h
.o/backtrace.o: ptrcheck.h backtrace.h
.o/color.o: /usr/include/stdlib.h /usr/include/Availability.h
.o/color.o: /usr/include/AvailabilityInternal.h /usr/include/_types.h
.o/color.o: /usr/include/sys/_types.h /usr/include/sys/cdefs.h
.o/color.o: /usr/include/machine/_types.h /usr/include/i386/_types.h
.o/color.o: /usr/include/sys/wait.h /usr/include/sys/signal.h
.o/color.o: /usr/include/sys/appleapiopts.h /usr/include/machine/signal.h
.o/color.o: /usr/include/i386/signal.h /usr/include/i386/_structs.h
.o/color.o: /usr/include/sys/_structs.h /usr/include/machine/_structs.h
.o/color.o: /usr/include/sys/resource.h /usr/include/machine/endian.h
.o/color.o: /usr/include/i386/endian.h /usr/include/sys/_endian.h
.o/color.o: /usr/include/libkern/_OSByteOrder.h
.o/color.o: /usr/include/libkern/i386/_OSByteOrder.h /usr/include/alloca.h
.o/color.o: /usr/include/machine/types.h /usr/include/i386/types.h
.o/color.o: /usr/include/string.h /usr/include/secure/_string.h
.o/color.o: /usr/include/secure/_common.h glapi.h main.h
.o/color.o: /usr/include/stdint.h ptrcheck.h color.h tree.h
.o/command.o: /usr/include/string.h /usr/include/_types.h
.o/command.o: /usr/include/sys/_types.h /usr/include/sys/cdefs.h
.o/command.o: /usr/include/machine/_types.h /usr/include/i386/_types.h
.o/command.o: /usr/include/secure/_string.h /usr/include/secure/_common.h
.o/command.o: /usr/include/stdlib.h /usr/include/Availability.h
.o/command.o: /usr/include/AvailabilityInternal.h /usr/include/sys/wait.h
.o/command.o: /usr/include/sys/signal.h /usr/include/sys/appleapiopts.h
.o/command.o: /usr/include/machine/signal.h /usr/include/i386/signal.h
.o/command.o: /usr/include/i386/_structs.h /usr/include/sys/_structs.h
.o/command.o: /usr/include/machine/_structs.h /usr/include/sys/resource.h
.o/command.o: /usr/include/machine/endian.h /usr/include/i386/endian.h
.o/command.o: /usr/include/sys/_endian.h /usr/include/libkern/_OSByteOrder.h
.o/command.o: /usr/include/libkern/i386/_OSByteOrder.h /usr/include/alloca.h
.o/command.o: /usr/include/machine/types.h /usr/include/i386/types.h slre.h
.o/command.o: main.h /usr/include/stdint.h ptrcheck.h command.h token.h
.o/command.o: tree.h
.o/config.o: main.h /usr/include/stdint.h ptrcheck.h marshal.h tree.h
.o/config.o: config.h /usr/include/string.h /usr/include/_types.h
.o/config.o: /usr/include/sys/_types.h /usr/include/sys/cdefs.h
.o/config.o: /usr/include/machine/_types.h /usr/include/i386/_types.h
.o/config.o: /usr/include/secure/_string.h /usr/include/secure/_common.h
.o/demarshal.o: /usr/include/stdlib.h /usr/include/Availability.h
.o/demarshal.o: /usr/include/AvailabilityInternal.h /usr/include/_types.h
.o/demarshal.o: /usr/include/sys/_types.h /usr/include/sys/cdefs.h
.o/demarshal.o: /usr/include/machine/_types.h /usr/include/i386/_types.h
.o/demarshal.o: /usr/include/sys/wait.h /usr/include/sys/signal.h
.o/demarshal.o: /usr/include/sys/appleapiopts.h /usr/include/machine/signal.h
.o/demarshal.o: /usr/include/i386/signal.h /usr/include/i386/_structs.h
.o/demarshal.o: /usr/include/sys/_structs.h /usr/include/machine/_structs.h
.o/demarshal.o: /usr/include/sys/resource.h /usr/include/machine/endian.h
.o/demarshal.o: /usr/include/i386/endian.h /usr/include/sys/_endian.h
.o/demarshal.o: /usr/include/libkern/_OSByteOrder.h
.o/demarshal.o: /usr/include/libkern/i386/_OSByteOrder.h
.o/demarshal.o: /usr/include/alloca.h /usr/include/machine/types.h
.o/demarshal.o: /usr/include/i386/types.h /usr/include/ctype.h
.o/demarshal.o: /usr/include/runetype.h /usr/include/stdio.h
.o/demarshal.o: /usr/include/secure/_stdio.h /usr/include/secure/_common.h
.o/demarshal.o: /usr/include/string.h /usr/include/secure/_string.h main.h
.o/demarshal.o: /usr/include/stdint.h ptrcheck.h marshal.h tree.h ramdisk.h
.o/dir.o: /usr/include/dirent.h /usr/include/_types.h
.o/dir.o: /usr/include/sys/_types.h /usr/include/sys/cdefs.h
.o/dir.o: /usr/include/machine/_types.h /usr/include/i386/_types.h
.o/dir.o: /usr/include/sys/dirent.h /usr/include/Availability.h
.o/dir.o: /usr/include/AvailabilityInternal.h /usr/include/libgen.h
.o/dir.o: /usr/include/stdlib.h /usr/include/sys/wait.h
.o/dir.o: /usr/include/sys/signal.h /usr/include/sys/appleapiopts.h
.o/dir.o: /usr/include/machine/signal.h /usr/include/i386/signal.h
.o/dir.o: /usr/include/i386/_structs.h /usr/include/sys/_structs.h
.o/dir.o: /usr/include/machine/_structs.h /usr/include/sys/resource.h
.o/dir.o: /usr/include/machine/endian.h /usr/include/i386/endian.h
.o/dir.o: /usr/include/sys/_endian.h /usr/include/libkern/_OSByteOrder.h
.o/dir.o: /usr/include/libkern/i386/_OSByteOrder.h /usr/include/alloca.h
.o/dir.o: /usr/include/machine/types.h /usr/include/i386/types.h
.o/dir.o: /usr/include/string.h /usr/include/secure/_string.h
.o/dir.o: /usr/include/secure/_common.h /usr/include/sys/param.h
.o/dir.o: /usr/include/sys/types.h /usr/include/sys/syslimits.h
.o/dir.o: /usr/include/machine/param.h /usr/include/i386/param.h
.o/dir.o: /usr/include/i386/_param.h /usr/include/limits.h
.o/dir.o: /usr/include/machine/limits.h /usr/include/i386/limits.h
.o/dir.o: /usr/include/i386/_limits.h /usr/include/sys/stat.h
.o/dir.o: /usr/include/unistd.h /usr/include/sys/unistd.h
.o/dir.o: /usr/include/sys/select.h /usr/include/sys/_select.h main.h
.o/dir.o: /usr/include/stdint.h ptrcheck.h dir.h tree.h ramdisk.h
.o/dir_test.o: /usr/include/stdio.h /usr/include/_types.h
.o/dir_test.o: /usr/include/sys/_types.h /usr/include/sys/cdefs.h
.o/dir_test.o: /usr/include/machine/_types.h /usr/include/i386/_types.h
.o/dir_test.o: /usr/include/secure/_stdio.h /usr/include/secure/_common.h
.o/dir_test.o: /usr/include/stdlib.h /usr/include/Availability.h
.o/dir_test.o: /usr/include/AvailabilityInternal.h /usr/include/sys/wait.h
.o/dir_test.o: /usr/include/sys/signal.h /usr/include/sys/appleapiopts.h
.o/dir_test.o: /usr/include/machine/signal.h /usr/include/i386/signal.h
.o/dir_test.o: /usr/include/i386/_structs.h /usr/include/sys/_structs.h
.o/dir_test.o: /usr/include/machine/_structs.h /usr/include/sys/resource.h
.o/dir_test.o: /usr/include/machine/endian.h /usr/include/i386/endian.h
.o/dir_test.o: /usr/include/sys/_endian.h /usr/include/libkern/_OSByteOrder.h
.o/dir_test.o: /usr/include/libkern/i386/_OSByteOrder.h /usr/include/alloca.h
.o/dir_test.o: /usr/include/machine/types.h /usr/include/i386/types.h main.h
.o/dir_test.o: /usr/include/stdint.h ptrcheck.h dir.h tree.h
.o/dir_test.o: /usr/include/string.h /usr/include/secure/_string.h
.o/enum_test.o: /usr/include/stdio.h /usr/include/_types.h
.o/enum_test.o: /usr/include/sys/_types.h /usr/include/sys/cdefs.h
.o/enum_test.o: /usr/include/machine/_types.h /usr/include/i386/_types.h
.o/enum_test.o: /usr/include/secure/_stdio.h /usr/include/secure/_common.h
.o/enum_test.o: /usr/include/stdlib.h /usr/include/Availability.h
.o/enum_test.o: /usr/include/AvailabilityInternal.h /usr/include/sys/wait.h
.o/enum_test.o: /usr/include/sys/signal.h /usr/include/sys/appleapiopts.h
.o/enum_test.o: /usr/include/machine/signal.h /usr/include/i386/signal.h
.o/enum_test.o: /usr/include/i386/_structs.h /usr/include/sys/_structs.h
.o/enum_test.o: /usr/include/machine/_structs.h /usr/include/sys/resource.h
.o/enum_test.o: /usr/include/machine/endian.h /usr/include/i386/endian.h
.o/enum_test.o: /usr/include/sys/_endian.h
.o/enum_test.o: /usr/include/libkern/_OSByteOrder.h
.o/enum_test.o: /usr/include/libkern/i386/_OSByteOrder.h
.o/enum_test.o: /usr/include/alloca.h /usr/include/machine/types.h
.o/enum_test.o: /usr/include/i386/types.h /usr/include/string.h
.o/enum_test.o: /usr/include/secure/_string.h main.h /usr/include/stdint.h
.o/enum_test.o: ptrcheck.h enum.h tree.h
.o/file.o: /usr/include/stdio.h /usr/include/_types.h
.o/file.o: /usr/include/sys/_types.h /usr/include/sys/cdefs.h
.o/file.o: /usr/include/machine/_types.h /usr/include/i386/_types.h
.o/file.o: /usr/include/secure/_stdio.h /usr/include/secure/_common.h
.o/file.o: /usr/include/string.h /usr/include/secure/_string.h
.o/file.o: /usr/include/errno.h /usr/include/sys/errno.h
.o/file.o: /usr/include/unistd.h /usr/include/sys/unistd.h
.o/file.o: /usr/include/sys/select.h /usr/include/sys/appleapiopts.h
.o/file.o: /usr/include/sys/_structs.h /usr/include/machine/_structs.h
.o/file.o: /usr/include/i386/_structs.h /usr/include/sys/_select.h
.o/file.o: /usr/include/sys/stat.h /usr/include/Availability.h
.o/file.o: /usr/include/AvailabilityInternal.h main.h /usr/include/stdint.h
.o/file.o: ptrcheck.h mzip_file.h file.h time.h /usr/include/sys/time.h
.o/font.o: main.h /usr/include/stdint.h ptrcheck.h ttf.h enum.h tree.h font.h
.o/gl.o: glapi.h main.h /usr/include/stdint.h ptrcheck.h config.h gl.h
.o/glshapes.o: /usr/include/math.h /usr/include/architecture/i386/math.h
.o/glshapes.o: /usr/include/sys/cdefs.h glapi.h main.h /usr/include/stdint.h
.o/glshapes.o: ptrcheck.h gl.h color.h
.o/init_fn.o: main.h /usr/include/stdint.h ptrcheck.h tree.h init_fn.h
.o/init_fn.o: init_fn_private.h /usr/include/string.h /usr/include/_types.h
.o/init_fn.o: /usr/include/sys/_types.h /usr/include/sys/cdefs.h
.o/init_fn.o: /usr/include/machine/_types.h /usr/include/i386/_types.h
.o/init_fn.o: /usr/include/secure/_string.h /usr/include/secure/_common.h
.o/item.o: main.h /usr/include/stdint.h ptrcheck.h tree.h thing.h
.o/item.o: item_private.h item.h thing_template.h /usr/include/string.h
.o/item.o: /usr/include/_types.h /usr/include/sys/_types.h
.o/item.o: /usr/include/sys/cdefs.h /usr/include/machine/_types.h
.o/item.o: /usr/include/i386/_types.h /usr/include/secure/_string.h
.o/item.o: /usr/include/secure/_common.h
.o/level.o: /usr/include/errno.h /usr/include/sys/errno.h
.o/level.o: /usr/include/sys/cdefs.h slre.h main.h /usr/include/stdint.h
.o/level.o: ptrcheck.h /usr/include/string.h /usr/include/_types.h
.o/level.o: /usr/include/sys/_types.h /usr/include/machine/_types.h
.o/level.o: /usr/include/i386/_types.h /usr/include/secure/_string.h
.o/level.o: /usr/include/secure/_common.h wid.h wid_popup.h level.h
.o/level.o: level_private.h thing.h thing_template.h command.h token.h
.o/level.o: wid_button.h time.h /usr/include/sys/time.h
.o/level.o: /usr/include/sys/_structs.h /usr/include/machine/_structs.h
.o/level.o: /usr/include/i386/_structs.h /usr/include/sys/appleapiopts.h
.o/level.o: /usr/include/sys/_select.h marshal.h tree.h wid_game_map.h
.o/level.o: wid_editor_map.h sdl.h map.h timer.h wid_hiscore.h music.h
.o/level.o: sound.h wid_console.h
.o/log.o: slre.h main.h /usr/include/stdint.h ptrcheck.h backtrace.h
.o/log.o: wid_console.h time.h /usr/include/sys/time.h
.o/log.o: /usr/include/sys/cdefs.h /usr/include/sys/_types.h
.o/log.o: /usr/include/machine/_types.h /usr/include/i386/_types.h
.o/log.o: /usr/include/sys/_structs.h /usr/include/machine/_structs.h
.o/log.o: /usr/include/i386/_structs.h /usr/include/sys/appleapiopts.h
.o/log.o: /usr/include/sys/_select.h command.h token.h thing.h level.h item.h
.o/log.o: tree.h timer.h init_fn.h wid.h
.o/main.o: /usr/include/libgen.h /usr/include/sys/cdefs.h
.o/main.o: /usr/include/unistd.h /usr/include/_types.h
.o/main.o: /usr/include/sys/_types.h /usr/include/machine/_types.h
.o/main.o: /usr/include/i386/_types.h /usr/include/sys/unistd.h
.o/main.o: /usr/include/sys/select.h /usr/include/sys/appleapiopts.h
.o/main.o: /usr/include/sys/_structs.h /usr/include/machine/_structs.h
.o/main.o: /usr/include/i386/_structs.h /usr/include/sys/_select.h slre.h
.o/main.o: main.h /usr/include/stdint.h ptrcheck.h gl.h config.h
.o/main.o: wid_console.h command.h token.h color.h ttf.h enum.h tree.h font.h
.o/main.o: /usr/include/math.h /usr/include/architecture/i386/math.h tex.h
.o/main.o: music.h sound.h tile.h thing.h thing_template.h wid.h sdl.h
.o/main.o: wid_editor.h wid_game_map.h wid_intro_about.h wid_intro_help.h
.o/main.o: wid_intro_guide.h wid_game_over.h wid_intro_settings.h
.o/main.o: wid_hiscore.h wid_intro.h /usr/include/string.h
.o/main.o: /usr/include/secure/_string.h /usr/include/secure/_common.h dir.h
.o/main.o: file.h player.h level.h resource.h init_fn.h ramdisk.h
.o/map.o: main.h /usr/include/stdint.h ptrcheck.h map.h tree.h wid.h thing.h
.o/map.o: thing_template.h thing_tile.h level.h level_private.h
.o/marshal.o: /usr/include/string.h /usr/include/_types.h
.o/marshal.o: /usr/include/sys/_types.h /usr/include/sys/cdefs.h
.o/marshal.o: /usr/include/machine/_types.h /usr/include/i386/_types.h
.o/marshal.o: /usr/include/secure/_string.h /usr/include/secure/_common.h
.o/marshal.o: /usr/include/math.h /usr/include/architecture/i386/math.h
.o/marshal.o: /usr/include/stdio.h /usr/include/secure/_stdio.h main.h
.o/marshal.o: /usr/include/stdint.h ptrcheck.h file.h mzip_file.h marshal.h
.o/marshal.o: tree.h string_ext.h enum.h
.o/marshal_test.o: /usr/include/stdlib.h /usr/include/Availability.h
.o/marshal_test.o: /usr/include/AvailabilityInternal.h /usr/include/_types.h
.o/marshal_test.o: /usr/include/sys/_types.h /usr/include/sys/cdefs.h
.o/marshal_test.o: /usr/include/machine/_types.h /usr/include/i386/_types.h
.o/marshal_test.o: /usr/include/sys/wait.h /usr/include/sys/signal.h
.o/marshal_test.o: /usr/include/sys/appleapiopts.h
.o/marshal_test.o: /usr/include/machine/signal.h /usr/include/i386/signal.h
.o/marshal_test.o: /usr/include/i386/_structs.h /usr/include/sys/_structs.h
.o/marshal_test.o: /usr/include/machine/_structs.h
.o/marshal_test.o: /usr/include/sys/resource.h /usr/include/machine/endian.h
.o/marshal_test.o: /usr/include/i386/endian.h /usr/include/sys/_endian.h
.o/marshal_test.o: /usr/include/libkern/_OSByteOrder.h
.o/marshal_test.o: /usr/include/libkern/i386/_OSByteOrder.h
.o/marshal_test.o: /usr/include/alloca.h /usr/include/machine/types.h
.o/marshal_test.o: /usr/include/i386/types.h /usr/include/string.h
.o/marshal_test.o: /usr/include/secure/_string.h
.o/marshal_test.o: /usr/include/secure/_common.h main.h /usr/include/stdint.h
.o/marshal_test.o: ptrcheck.h marshal.h tree.h enum.h
.o/math.o: /usr/include/math.h /usr/include/architecture/i386/math.h
.o/math.o: /usr/include/sys/cdefs.h main.h /usr/include/stdint.h ptrcheck.h
.o/miniz.o: /usr/include/stdio.h /usr/include/_types.h
.o/miniz.o: /usr/include/sys/_types.h /usr/include/sys/cdefs.h
.o/miniz.o: /usr/include/machine/_types.h /usr/include/i386/_types.h
.o/miniz.o: /usr/include/secure/_stdio.h /usr/include/secure/_common.h
.o/miniz.o: /usr/include/assert.h /usr/include/stdlib.h
.o/miniz.o: /usr/include/Availability.h /usr/include/AvailabilityInternal.h
.o/miniz.o: /usr/include/sys/wait.h /usr/include/sys/signal.h
.o/miniz.o: /usr/include/sys/appleapiopts.h /usr/include/machine/signal.h
.o/miniz.o: /usr/include/i386/signal.h /usr/include/i386/_structs.h
.o/miniz.o: /usr/include/sys/_structs.h /usr/include/machine/_structs.h
.o/miniz.o: /usr/include/sys/resource.h /usr/include/machine/endian.h
.o/miniz.o: /usr/include/i386/endian.h /usr/include/sys/_endian.h
.o/miniz.o: /usr/include/libkern/_OSByteOrder.h
.o/miniz.o: /usr/include/libkern/i386/_OSByteOrder.h /usr/include/alloca.h
.o/miniz.o: /usr/include/machine/types.h /usr/include/i386/types.h time.h
.o/miniz.o: /usr/include/sys/time.h /usr/include/sys/_select.h
.o/miniz.o: /usr/include/string.h /usr/include/secure/_string.h
.o/miniz.o: /usr/include/sys/stat.h /usr/include/utime.h
.o/miniz_test.o: /usr/include/string.h /usr/include/_types.h
.o/miniz_test.o: /usr/include/sys/_types.h /usr/include/sys/cdefs.h
.o/miniz_test.o: /usr/include/machine/_types.h /usr/include/i386/_types.h
.o/miniz_test.o: /usr/include/secure/_string.h /usr/include/secure/_common.h
.o/miniz_test.o: miniz.h /usr/include/stdio.h /usr/include/secure/_stdio.h
.o/miniz_test.o: /usr/include/stdlib.h /usr/include/Availability.h
.o/miniz_test.o: /usr/include/AvailabilityInternal.h /usr/include/sys/wait.h
.o/miniz_test.o: /usr/include/sys/signal.h /usr/include/sys/appleapiopts.h
.o/miniz_test.o: /usr/include/machine/signal.h /usr/include/i386/signal.h
.o/miniz_test.o: /usr/include/i386/_structs.h /usr/include/sys/_structs.h
.o/miniz_test.o: /usr/include/machine/_structs.h /usr/include/sys/resource.h
.o/miniz_test.o: /usr/include/machine/endian.h /usr/include/i386/endian.h
.o/miniz_test.o: /usr/include/sys/_endian.h
.o/miniz_test.o: /usr/include/libkern/_OSByteOrder.h
.o/miniz_test.o: /usr/include/libkern/i386/_OSByteOrder.h
.o/miniz_test.o: /usr/include/alloca.h /usr/include/machine/types.h
.o/miniz_test.o: /usr/include/i386/types.h time.h /usr/include/sys/time.h
.o/miniz_test.o: /usr/include/sys/_select.h main.h /usr/include/stdint.h
.o/miniz_test.o: ptrcheck.h
.o/munzip.o: /usr/include/stdio.h /usr/include/_types.h
.o/munzip.o: /usr/include/sys/_types.h /usr/include/sys/cdefs.h
.o/munzip.o: /usr/include/machine/_types.h /usr/include/i386/_types.h
.o/munzip.o: /usr/include/secure/_stdio.h /usr/include/secure/_common.h
.o/munzip.o: /usr/include/stdlib.h /usr/include/Availability.h
.o/munzip.o: /usr/include/AvailabilityInternal.h /usr/include/sys/wait.h
.o/munzip.o: /usr/include/sys/signal.h /usr/include/sys/appleapiopts.h
.o/munzip.o: /usr/include/machine/signal.h /usr/include/i386/signal.h
.o/munzip.o: /usr/include/i386/_structs.h /usr/include/sys/_structs.h
.o/munzip.o: /usr/include/machine/_structs.h /usr/include/sys/resource.h
.o/munzip.o: /usr/include/machine/endian.h /usr/include/i386/endian.h
.o/munzip.o: /usr/include/sys/_endian.h /usr/include/libkern/_OSByteOrder.h
.o/munzip.o: /usr/include/libkern/i386/_OSByteOrder.h /usr/include/alloca.h
.o/munzip.o: /usr/include/machine/types.h /usr/include/i386/types.h
.o/munzip.o: /usr/include/string.h /usr/include/secure/_string.h
.o/munzip.o: /usr/include/errno.h /usr/include/sys/errno.h main.h
.o/munzip.o: /usr/include/stdint.h ptrcheck.h file.h mzip_lib.h mzip_file.h
.o/music.o: main.h /usr/include/stdint.h ptrcheck.h music.h ramdisk.h tree.h
.o/music.o: config.h
.o/mzip.o: /usr/include/stdio.h /usr/include/_types.h
.o/mzip.o: /usr/include/sys/_types.h /usr/include/sys/cdefs.h
.o/mzip.o: /usr/include/machine/_types.h /usr/include/i386/_types.h
.o/mzip.o: /usr/include/secure/_stdio.h /usr/include/secure/_common.h
.o/mzip.o: /usr/include/stdlib.h /usr/include/Availability.h
.o/mzip.o: /usr/include/AvailabilityInternal.h /usr/include/sys/wait.h
.o/mzip.o: /usr/include/sys/signal.h /usr/include/sys/appleapiopts.h
.o/mzip.o: /usr/include/machine/signal.h /usr/include/i386/signal.h
.o/mzip.o: /usr/include/i386/_structs.h /usr/include/sys/_structs.h
.o/mzip.o: /usr/include/machine/_structs.h /usr/include/sys/resource.h
.o/mzip.o: /usr/include/machine/endian.h /usr/include/i386/endian.h
.o/mzip.o: /usr/include/sys/_endian.h /usr/include/libkern/_OSByteOrder.h
.o/mzip.o: /usr/include/libkern/i386/_OSByteOrder.h /usr/include/alloca.h
.o/mzip.o: /usr/include/machine/types.h /usr/include/i386/types.h
.o/mzip.o: /usr/include/string.h /usr/include/secure/_string.h main.h
.o/mzip.o: /usr/include/stdint.h ptrcheck.h file.h mzip_file.h
.o/mzip_file.o: /usr/include/stdio.h /usr/include/_types.h
.o/mzip_file.o: /usr/include/sys/_types.h /usr/include/sys/cdefs.h
.o/mzip_file.o: /usr/include/machine/_types.h /usr/include/i386/_types.h
.o/mzip_file.o: /usr/include/secure/_stdio.h /usr/include/secure/_common.h
.o/mzip_file.o: /usr/include/string.h /usr/include/secure/_string.h
.o/mzip_file.o: /usr/include/errno.h /usr/include/sys/errno.h main.h
.o/mzip_file.o: /usr/include/stdint.h ptrcheck.h file.h mzip_file.h
.o/mzip_file.o: mzip_lib.h
.o/mzip_file_test.o: /usr/include/stdio.h /usr/include/_types.h
.o/mzip_file_test.o: /usr/include/sys/_types.h /usr/include/sys/cdefs.h
.o/mzip_file_test.o: /usr/include/machine/_types.h /usr/include/i386/_types.h
.o/mzip_file_test.o: /usr/include/secure/_stdio.h
.o/mzip_file_test.o: /usr/include/secure/_common.h /usr/include/string.h
.o/mzip_file_test.o: /usr/include/secure/_string.h main.h
.o/mzip_file_test.o: /usr/include/stdint.h ptrcheck.h mzip_file.h
.o/mzip_lib.o: /usr/include/string.h /usr/include/_types.h
.o/mzip_lib.o: /usr/include/sys/_types.h /usr/include/sys/cdefs.h
.o/mzip_lib.o: /usr/include/machine/_types.h /usr/include/i386/_types.h
.o/mzip_lib.o: /usr/include/secure/_string.h /usr/include/secure/_common.h
.o/mzip_lib.o: miniz.h /usr/include/stdio.h /usr/include/secure/_stdio.h
.o/mzip_lib.o: /usr/include/stdlib.h /usr/include/Availability.h
.o/mzip_lib.o: /usr/include/AvailabilityInternal.h /usr/include/sys/wait.h
.o/mzip_lib.o: /usr/include/sys/signal.h /usr/include/sys/appleapiopts.h
.o/mzip_lib.o: /usr/include/machine/signal.h /usr/include/i386/signal.h
.o/mzip_lib.o: /usr/include/i386/_structs.h /usr/include/sys/_structs.h
.o/mzip_lib.o: /usr/include/machine/_structs.h /usr/include/sys/resource.h
.o/mzip_lib.o: /usr/include/machine/endian.h /usr/include/i386/endian.h
.o/mzip_lib.o: /usr/include/sys/_endian.h /usr/include/libkern/_OSByteOrder.h
.o/mzip_lib.o: /usr/include/libkern/i386/_OSByteOrder.h /usr/include/alloca.h
.o/mzip_lib.o: /usr/include/machine/types.h /usr/include/i386/types.h time.h
.o/mzip_lib.o: /usr/include/sys/time.h /usr/include/sys/_select.h main.h
.o/mzip_lib.o: /usr/include/stdint.h ptrcheck.h mzip_lib.h
.o/pixel.o: main.h /usr/include/stdint.h ptrcheck.h pixel.h
.o/player.o: main.h /usr/include/stdint.h ptrcheck.h thing.h player.h level.h
.o/point.o: /usr/include/math.h /usr/include/architecture/i386/math.h
.o/point.o: /usr/include/sys/cdefs.h main.h /usr/include/stdint.h ptrcheck.h
.o/ptrcheck.o: /usr/include/stdio.h /usr/include/_types.h
.o/ptrcheck.o: /usr/include/sys/_types.h /usr/include/sys/cdefs.h
.o/ptrcheck.o: /usr/include/machine/_types.h /usr/include/i386/_types.h
.o/ptrcheck.o: /usr/include/secure/_stdio.h /usr/include/secure/_common.h
.o/ptrcheck.o: /usr/include/stdlib.h /usr/include/Availability.h
.o/ptrcheck.o: /usr/include/AvailabilityInternal.h /usr/include/sys/wait.h
.o/ptrcheck.o: /usr/include/sys/signal.h /usr/include/sys/appleapiopts.h
.o/ptrcheck.o: /usr/include/machine/signal.h /usr/include/i386/signal.h
.o/ptrcheck.o: /usr/include/i386/_structs.h /usr/include/sys/_structs.h
.o/ptrcheck.o: /usr/include/machine/_structs.h /usr/include/sys/resource.h
.o/ptrcheck.o: /usr/include/machine/endian.h /usr/include/i386/endian.h
.o/ptrcheck.o: /usr/include/sys/_endian.h /usr/include/libkern/_OSByteOrder.h
.o/ptrcheck.o: /usr/include/libkern/i386/_OSByteOrder.h /usr/include/alloca.h
.o/ptrcheck.o: /usr/include/machine/types.h /usr/include/i386/types.h
.o/ptrcheck.o: /usr/include/string.h /usr/include/secure/_string.h slre.h
.o/ptrcheck.o: main.h /usr/include/stdint.h ptrcheck.h backtrace.h time.h
.o/ptrcheck.o: /usr/include/sys/time.h /usr/include/sys/_select.h tree.h
.o/ptrcheck.o: command.h token.h
.o/ptrcheck_test.o: main.h /usr/include/stdint.h ptrcheck.h
.o/ramdisk_data_fragment_0.o: /usr/include/stdint.h ramdisk.h
.o/ramdisk_data_fragment_final.o: /usr/include/stdint.h ramdisk.h
.o/ramdisk_read.o: /usr/include/libgen.h /usr/include/sys/cdefs.h
.o/ramdisk_read.o: /usr/include/string.h /usr/include/_types.h
.o/ramdisk_read.o: /usr/include/sys/_types.h /usr/include/machine/_types.h
.o/ramdisk_read.o: /usr/include/i386/_types.h /usr/include/secure/_string.h
.o/ramdisk_read.o: /usr/include/secure/_common.h miniz.h /usr/include/stdio.h
.o/ramdisk_read.o: /usr/include/secure/_stdio.h /usr/include/stdlib.h
.o/ramdisk_read.o: /usr/include/Availability.h
.o/ramdisk_read.o: /usr/include/AvailabilityInternal.h
.o/ramdisk_read.o: /usr/include/sys/wait.h /usr/include/sys/signal.h
.o/ramdisk_read.o: /usr/include/sys/appleapiopts.h
.o/ramdisk_read.o: /usr/include/machine/signal.h /usr/include/i386/signal.h
.o/ramdisk_read.o: /usr/include/i386/_structs.h /usr/include/sys/_structs.h
.o/ramdisk_read.o: /usr/include/machine/_structs.h
.o/ramdisk_read.o: /usr/include/sys/resource.h /usr/include/machine/endian.h
.o/ramdisk_read.o: /usr/include/i386/endian.h /usr/include/sys/_endian.h
.o/ramdisk_read.o: /usr/include/libkern/_OSByteOrder.h
.o/ramdisk_read.o: /usr/include/libkern/i386/_OSByteOrder.h
.o/ramdisk_read.o: /usr/include/alloca.h /usr/include/machine/types.h
.o/ramdisk_read.o: /usr/include/i386/types.h time.h /usr/include/sys/time.h
.o/ramdisk_read.o: /usr/include/sys/_select.h main.h /usr/include/stdint.h
.o/ramdisk_read.o: ptrcheck.h file.h ramdisk.h ramdisk_files.c
.o/ramdisk_write.o: /usr/include/sys/types.h /usr/include/sys/appleapiopts.h
.o/ramdisk_write.o: /usr/include/sys/cdefs.h /usr/include/machine/types.h
.o/ramdisk_write.o: /usr/include/i386/types.h /usr/include/i386/_types.h
.o/ramdisk_write.o: /usr/include/sys/_types.h /usr/include/machine/_types.h
.o/ramdisk_write.o: /usr/include/machine/endian.h /usr/include/i386/endian.h
.o/ramdisk_write.o: /usr/include/sys/_endian.h
.o/ramdisk_write.o: /usr/include/libkern/_OSByteOrder.h
.o/ramdisk_write.o: /usr/include/libkern/i386/_OSByteOrder.h
.o/ramdisk_write.o: /usr/include/sys/_structs.h
.o/ramdisk_write.o: /usr/include/machine/_structs.h
.o/ramdisk_write.o: /usr/include/i386/_structs.h /usr/include/stdint.h
.o/ramdisk_write.o: /usr/include/stdio.h /usr/include/_types.h
.o/ramdisk_write.o: /usr/include/secure/_stdio.h
.o/ramdisk_write.o: /usr/include/secure/_common.h /usr/include/string.h
.o/ramdisk_write.o: /usr/include/secure/_string.h /usr/include/stdlib.h
.o/ramdisk_write.o: /usr/include/Availability.h
.o/ramdisk_write.o: /usr/include/AvailabilityInternal.h
.o/ramdisk_write.o: /usr/include/sys/wait.h /usr/include/sys/signal.h
.o/ramdisk_write.o: /usr/include/machine/signal.h /usr/include/i386/signal.h
.o/ramdisk_write.o: /usr/include/sys/resource.h /usr/include/alloca.h
.o/ramdisk_write.o: /usr/include/assert.h /usr/include/zlib.h
.o/ramdisk_write.o: /usr/include/zconf.h ramdisk.h ramdisk_files.c
.o/resource.o: main.h /usr/include/stdint.h ptrcheck.h resource.h tex.h
.o/resource.o: tile.h thing_template.h init_fn.h
.o/sdl.o: glapi.h main.h /usr/include/stdint.h ptrcheck.h gl.h wid.h
.o/sdl.o: wid_console.h color.h time.h /usr/include/sys/time.h
.o/sdl.o: /usr/include/sys/cdefs.h /usr/include/sys/_types.h
.o/sdl.o: /usr/include/machine/_types.h /usr/include/i386/_types.h
.o/sdl.o: /usr/include/sys/_structs.h /usr/include/machine/_structs.h
.o/sdl.o: /usr/include/i386/_structs.h /usr/include/sys/appleapiopts.h
.o/sdl.o: /usr/include/sys/_select.h thing.h sdl.h init_fn.h wid_splash.h
.o/sdl.o: config.h
.o/slre.o: /usr/include/stdio.h /usr/include/_types.h
.o/slre.o: /usr/include/sys/_types.h /usr/include/sys/cdefs.h
.o/slre.o: /usr/include/machine/_types.h /usr/include/i386/_types.h
.o/slre.o: /usr/include/secure/_stdio.h /usr/include/secure/_common.h
.o/slre.o: /usr/include/assert.h /usr/include/ctype.h /usr/include/runetype.h
.o/slre.o: /usr/include/string.h /usr/include/secure/_string.h slre.h
.o/sound.o: main.h /usr/include/stdint.h ptrcheck.h sound.h ramdisk.h tree.h
.o/sound.o: config.h music.h
.o/stb_image.o: stb_image.h /usr/include/stdio.h /usr/include/_types.h
.o/stb_image.o: /usr/include/sys/_types.h /usr/include/sys/cdefs.h
.o/stb_image.o: /usr/include/machine/_types.h /usr/include/i386/_types.h
.o/stb_image.o: /usr/include/secure/_stdio.h /usr/include/secure/_common.h
.o/stb_image.o: /usr/include/math.h /usr/include/architecture/i386/math.h
.o/stb_image.o: /usr/include/stdlib.h /usr/include/Availability.h
.o/stb_image.o: /usr/include/AvailabilityInternal.h /usr/include/sys/wait.h
.o/stb_image.o: /usr/include/sys/signal.h /usr/include/sys/appleapiopts.h
.o/stb_image.o: /usr/include/machine/signal.h /usr/include/i386/signal.h
.o/stb_image.o: /usr/include/i386/_structs.h /usr/include/sys/_structs.h
.o/stb_image.o: /usr/include/machine/_structs.h /usr/include/sys/resource.h
.o/stb_image.o: /usr/include/machine/endian.h /usr/include/i386/endian.h
.o/stb_image.o: /usr/include/sys/_endian.h
.o/stb_image.o: /usr/include/libkern/_OSByteOrder.h
.o/stb_image.o: /usr/include/libkern/i386/_OSByteOrder.h
.o/stb_image.o: /usr/include/alloca.h /usr/include/machine/types.h
.o/stb_image.o: /usr/include/i386/types.h /usr/include/memory.h
.o/stb_image.o: /usr/include/string.h /usr/include/secure/_string.h
.o/stb_image.o: /usr/include/stdarg.h main.h /usr/include/stdint.h ptrcheck.h
.o/string.o: /usr/include/string.h /usr/include/_types.h
.o/string.o: /usr/include/sys/_types.h /usr/include/sys/cdefs.h
.o/string.o: /usr/include/machine/_types.h /usr/include/i386/_types.h
.o/string.o: /usr/include/secure/_string.h /usr/include/secure/_common.h
.o/string.o: /usr/include/stdlib.h /usr/include/Availability.h
.o/string.o: /usr/include/AvailabilityInternal.h /usr/include/sys/wait.h
.o/string.o: /usr/include/sys/signal.h /usr/include/sys/appleapiopts.h
.o/string.o: /usr/include/machine/signal.h /usr/include/i386/signal.h
.o/string.o: /usr/include/i386/_structs.h /usr/include/sys/_structs.h
.o/string.o: /usr/include/machine/_structs.h /usr/include/sys/resource.h
.o/string.o: /usr/include/machine/endian.h /usr/include/i386/endian.h
.o/string.o: /usr/include/sys/_endian.h /usr/include/libkern/_OSByteOrder.h
.o/string.o: /usr/include/libkern/i386/_OSByteOrder.h /usr/include/alloca.h
.o/string.o: /usr/include/machine/types.h /usr/include/i386/types.h
.o/string.o: /usr/include/stdio.h /usr/include/secure/_stdio.h
.o/string.o: /usr/include/ctype.h /usr/include/runetype.h main.h
.o/string.o: /usr/include/stdint.h ptrcheck.h
.o/string_ext.o: main.h /usr/include/stdint.h ptrcheck.h string_ext.h enum.h
.o/string_ext.o: tree.h color.h tex.h tile.h
.o/string_test.o: main.h /usr/include/stdint.h ptrcheck.h
.o/string_test.o: /usr/include/string.h /usr/include/_types.h
.o/string_test.o: /usr/include/sys/_types.h /usr/include/sys/cdefs.h
.o/string_test.o: /usr/include/machine/_types.h /usr/include/i386/_types.h
.o/string_test.o: /usr/include/secure/_string.h /usr/include/secure/_common.h
.o/string_test.o: string_ext.h enum.h tree.h
.o/tex.o: glapi.h stb_image.h /usr/include/stdio.h /usr/include/_types.h
.o/tex.o: /usr/include/sys/_types.h /usr/include/sys/cdefs.h
.o/tex.o: /usr/include/machine/_types.h /usr/include/i386/_types.h
.o/tex.o: /usr/include/secure/_stdio.h /usr/include/secure/_common.h main.h
.o/tex.o: /usr/include/stdint.h ptrcheck.h gl.h tex.h ramdisk.h tree.h
.o/thing.o: main.h /usr/include/stdint.h ptrcheck.h tree.h thing_private.h
.o/thing.o: thing.h thing_template.h thing_timer.h wid.h marshal.h level.h
.o/thing.o: time.h /usr/include/sys/time.h /usr/include/sys/cdefs.h
.o/thing.o: /usr/include/sys/_types.h /usr/include/machine/_types.h
.o/thing.o: /usr/include/i386/_types.h /usr/include/sys/_structs.h
.o/thing.o: /usr/include/machine/_structs.h /usr/include/i386/_structs.h
.o/thing.o: /usr/include/sys/appleapiopts.h /usr/include/sys/_select.h
.o/thing.o: /usr/include/string.h /usr/include/_types.h
.o/thing.o: /usr/include/secure/_string.h /usr/include/secure/_common.h
.o/thing.o: wid_textbox.h wid_game_map.h color.h config.h sound.h
.o/thing_ai.o: main.h /usr/include/stdint.h ptrcheck.h tree.h thing.h
.o/thing_ai.o: thing_private.h level_private.h map.h level.h item.h
.o/thing_anim.o: slre.h main.h /usr/include/stdint.h ptrcheck.h tree.h
.o/thing_anim.o: thing.h thing_private.h thing_tile.h wid.h time.h
.o/thing_anim.o: /usr/include/sys/time.h /usr/include/sys/cdefs.h
.o/thing_anim.o: /usr/include/sys/_types.h /usr/include/machine/_types.h
.o/thing_anim.o: /usr/include/i386/_types.h /usr/include/sys/_structs.h
.o/thing_anim.o: /usr/include/machine/_structs.h /usr/include/i386/_structs.h
.o/thing_anim.o: /usr/include/sys/appleapiopts.h /usr/include/sys/_select.h
.o/thing_anim.o: command.h token.h
.o/thing_collision.o: main.h /usr/include/stdint.h ptrcheck.h thing.h
.o/thing_collision.o: thing_template.h wid.h level.h wid_tooltip.h
.o/thing_collision.o: wid_game_map.h sound.h timer.h
.o/thing_item.o: main.h /usr/include/stdint.h ptrcheck.h tree.h thing.h
.o/thing_item.o: thing_template.h thing_private.h item.h wid_game_map.h map.h
.o/thing_item.o: wid.h level.h sound.h
.o/thing_template.o: main.h /usr/include/stdint.h ptrcheck.h tree.h
.o/thing_template.o: thing_template_private.h thing_template.h thing_tile.h
.o/thing_template.o: marshal.h
.o/thing_tick.o: main.h /usr/include/stdint.h ptrcheck.h tree.h
.o/thing_tick.o: thing_private.h thing.h thing_template.h thing_timer.h
.o/thing_tick.o: timer.h wid.h wid_game_map.h level.h wid_console.h time.h
.o/thing_tick.o: /usr/include/sys/time.h /usr/include/sys/cdefs.h
.o/thing_tick.o: /usr/include/sys/_types.h /usr/include/machine/_types.h
.o/thing_tick.o: /usr/include/i386/_types.h /usr/include/sys/_structs.h
.o/thing_tick.o: /usr/include/machine/_structs.h /usr/include/i386/_structs.h
.o/thing_tick.o: /usr/include/sys/appleapiopts.h /usr/include/sys/_select.h
.o/thing_tick.o: color.h
.o/thing_tile.o: main.h /usr/include/stdint.h ptrcheck.h tree.h
.o/thing_tile.o: thing_tile_private.h thing_template_private.h thing_tile.h
.o/thing_tile.o: marshal.h
.o/thing_timer.o: main.h /usr/include/stdint.h ptrcheck.h tree.h
.o/thing_timer.o: thing_private.h thing.h thing_timer.h timer.h wid.h color.h
.o/tile.o: glapi.h main.h /usr/include/stdint.h ptrcheck.h gl.h tile.h tree.h
.o/tile.o: tex.h tile_private.h
.o/time.o: main.h /usr/include/stdint.h ptrcheck.h time.h
.o/time.o: /usr/include/sys/time.h /usr/include/sys/cdefs.h
.o/time.o: /usr/include/sys/_types.h /usr/include/machine/_types.h
.o/time.o: /usr/include/i386/_types.h /usr/include/sys/_structs.h
.o/time.o: /usr/include/machine/_structs.h /usr/include/i386/_structs.h
.o/time.o: /usr/include/sys/appleapiopts.h /usr/include/sys/_select.h sdl.h
.o/timer.o: main.h /usr/include/stdint.h ptrcheck.h tree.h timer.h
.o/timer.o: timer_private.h /usr/include/string.h /usr/include/_types.h
.o/timer.o: /usr/include/sys/_types.h /usr/include/sys/cdefs.h
.o/timer.o: /usr/include/machine/_types.h /usr/include/i386/_types.h
.o/timer.o: /usr/include/secure/_string.h /usr/include/secure/_common.h
.o/timer.o: time.h /usr/include/sys/time.h /usr/include/sys/_structs.h
.o/timer.o: /usr/include/machine/_structs.h /usr/include/i386/_structs.h
.o/timer.o: /usr/include/sys/appleapiopts.h /usr/include/sys/_select.h
.o/token.o: /usr/include/string.h /usr/include/_types.h
.o/token.o: /usr/include/sys/_types.h /usr/include/sys/cdefs.h
.o/token.o: /usr/include/machine/_types.h /usr/include/i386/_types.h
.o/token.o: /usr/include/secure/_string.h /usr/include/secure/_common.h
.o/token.o: /usr/include/stdio.h /usr/include/secure/_stdio.h
.o/token.o: /usr/include/stdlib.h /usr/include/Availability.h
.o/token.o: /usr/include/AvailabilityInternal.h /usr/include/sys/wait.h
.o/token.o: /usr/include/sys/signal.h /usr/include/sys/appleapiopts.h
.o/token.o: /usr/include/machine/signal.h /usr/include/i386/signal.h
.o/token.o: /usr/include/i386/_structs.h /usr/include/sys/_structs.h
.o/token.o: /usr/include/machine/_structs.h /usr/include/sys/resource.h
.o/token.o: /usr/include/machine/endian.h /usr/include/i386/endian.h
.o/token.o: /usr/include/sys/_endian.h /usr/include/libkern/_OSByteOrder.h
.o/token.o: /usr/include/libkern/i386/_OSByteOrder.h /usr/include/alloca.h
.o/token.o: /usr/include/machine/types.h /usr/include/i386/types.h slre.h
.o/token.o: main.h /usr/include/stdint.h ptrcheck.h token.h
.o/tree.o: /usr/include/stdlib.h /usr/include/Availability.h
.o/tree.o: /usr/include/AvailabilityInternal.h /usr/include/_types.h
.o/tree.o: /usr/include/sys/_types.h /usr/include/sys/cdefs.h
.o/tree.o: /usr/include/machine/_types.h /usr/include/i386/_types.h
.o/tree.o: /usr/include/sys/wait.h /usr/include/sys/signal.h
.o/tree.o: /usr/include/sys/appleapiopts.h /usr/include/machine/signal.h
.o/tree.o: /usr/include/i386/signal.h /usr/include/i386/_structs.h
.o/tree.o: /usr/include/sys/_structs.h /usr/include/machine/_structs.h
.o/tree.o: /usr/include/sys/resource.h /usr/include/machine/endian.h
.o/tree.o: /usr/include/i386/endian.h /usr/include/sys/_endian.h
.o/tree.o: /usr/include/libkern/_OSByteOrder.h
.o/tree.o: /usr/include/libkern/i386/_OSByteOrder.h /usr/include/alloca.h
.o/tree.o: /usr/include/machine/types.h /usr/include/i386/types.h
.o/tree.o: /usr/include/string.h /usr/include/secure/_string.h
.o/tree.o: /usr/include/secure/_common.h main.h /usr/include/stdint.h
.o/tree.o: ptrcheck.h tree.h
.o/tree_test.o: /usr/include/stdio.h /usr/include/_types.h
.o/tree_test.o: /usr/include/sys/_types.h /usr/include/sys/cdefs.h
.o/tree_test.o: /usr/include/machine/_types.h /usr/include/i386/_types.h
.o/tree_test.o: /usr/include/secure/_stdio.h /usr/include/secure/_common.h
.o/tree_test.o: /usr/include/stdlib.h /usr/include/Availability.h
.o/tree_test.o: /usr/include/AvailabilityInternal.h /usr/include/sys/wait.h
.o/tree_test.o: /usr/include/sys/signal.h /usr/include/sys/appleapiopts.h
.o/tree_test.o: /usr/include/machine/signal.h /usr/include/i386/signal.h
.o/tree_test.o: /usr/include/i386/_structs.h /usr/include/sys/_structs.h
.o/tree_test.o: /usr/include/machine/_structs.h /usr/include/sys/resource.h
.o/tree_test.o: /usr/include/machine/endian.h /usr/include/i386/endian.h
.o/tree_test.o: /usr/include/sys/_endian.h
.o/tree_test.o: /usr/include/libkern/_OSByteOrder.h
.o/tree_test.o: /usr/include/libkern/i386/_OSByteOrder.h
.o/tree_test.o: /usr/include/alloca.h /usr/include/machine/types.h
.o/tree_test.o: /usr/include/i386/types.h /usr/include/string.h
.o/tree_test.o: /usr/include/secure/_string.h main.h /usr/include/stdint.h
.o/tree_test.o: ptrcheck.h tree.h time.h /usr/include/sys/time.h
.o/tree_test.o: /usr/include/sys/_select.h
.o/ttf.o: glapi.h stb_image.h /usr/include/stdio.h /usr/include/_types.h
.o/ttf.o: /usr/include/sys/_types.h /usr/include/sys/cdefs.h
.o/ttf.o: /usr/include/machine/_types.h /usr/include/i386/_types.h
.o/ttf.o: /usr/include/secure/_stdio.h /usr/include/secure/_common.h main.h
.o/ttf.o: /usr/include/stdint.h ptrcheck.h /usr/include/math.h
.o/ttf.o: /usr/include/architecture/i386/math.h gl.h pixel.h color.h bits.h
.o/ttf.o: tex.h tile.h ttf.h enum.h tree.h ramdisk.h time.h
.o/ttf.o: /usr/include/sys/time.h /usr/include/sys/_structs.h
.o/ttf.o: /usr/include/machine/_structs.h /usr/include/i386/_structs.h
.o/ttf.o: /usr/include/sys/appleapiopts.h /usr/include/sys/_select.h
.o/ttf.o: string_ext.h /usr/include/string.h /usr/include/secure/_string.h
.o/util.o: /usr/include/stdlib.h /usr/include/Availability.h
.o/util.o: /usr/include/AvailabilityInternal.h /usr/include/_types.h
.o/util.o: /usr/include/sys/_types.h /usr/include/sys/cdefs.h
.o/util.o: /usr/include/machine/_types.h /usr/include/i386/_types.h
.o/util.o: /usr/include/sys/wait.h /usr/include/sys/signal.h
.o/util.o: /usr/include/sys/appleapiopts.h /usr/include/machine/signal.h
.o/util.o: /usr/include/i386/signal.h /usr/include/i386/_structs.h
.o/util.o: /usr/include/sys/_structs.h /usr/include/machine/_structs.h
.o/util.o: /usr/include/sys/resource.h /usr/include/machine/endian.h
.o/util.o: /usr/include/i386/endian.h /usr/include/sys/_endian.h
.o/util.o: /usr/include/libkern/_OSByteOrder.h
.o/util.o: /usr/include/libkern/i386/_OSByteOrder.h /usr/include/alloca.h
.o/util.o: /usr/include/machine/types.h /usr/include/i386/types.h
.o/util.o: /usr/include/string.h /usr/include/secure/_string.h
.o/util.o: /usr/include/secure/_common.h main.h /usr/include/stdint.h
.o/util.o: ptrcheck.h
.o/util_basic.o: /usr/include/stdio.h /usr/include/_types.h
.o/util_basic.o: /usr/include/sys/_types.h /usr/include/sys/cdefs.h
.o/util_basic.o: /usr/include/machine/_types.h /usr/include/i386/_types.h
.o/util_basic.o: /usr/include/secure/_stdio.h /usr/include/secure/_common.h
.o/util_basic.o: /usr/include/stdlib.h /usr/include/Availability.h
.o/util_basic.o: /usr/include/AvailabilityInternal.h /usr/include/sys/wait.h
.o/util_basic.o: /usr/include/sys/signal.h /usr/include/sys/appleapiopts.h
.o/util_basic.o: /usr/include/machine/signal.h /usr/include/i386/signal.h
.o/util_basic.o: /usr/include/i386/_structs.h /usr/include/sys/_structs.h
.o/util_basic.o: /usr/include/machine/_structs.h /usr/include/sys/resource.h
.o/util_basic.o: /usr/include/machine/endian.h /usr/include/i386/endian.h
.o/util_basic.o: /usr/include/sys/_endian.h
.o/util_basic.o: /usr/include/libkern/_OSByteOrder.h
.o/util_basic.o: /usr/include/libkern/i386/_OSByteOrder.h
.o/util_basic.o: /usr/include/alloca.h /usr/include/machine/types.h
.o/util_basic.o: /usr/include/i386/types.h /usr/include/strings.h
.o/util_basic.o: /usr/include/string.h /usr/include/secure/_string.h
.o/util_basic.o: /usr/include/stdarg.h main.h /usr/include/stdint.h
.o/util_basic.o: ptrcheck.h
.o/wid.o: glapi.h slre.h main.h /usr/include/stdint.h ptrcheck.h gl.h wid.h
.o/wid.o: color.h ttf.h enum.h tree.h tex.h tile.h thing.h thing_template.h
.o/wid.o: thing_tile.h command.h token.h time.h /usr/include/sys/time.h
.o/wid.o: /usr/include/sys/cdefs.h /usr/include/sys/_types.h
.o/wid.o: /usr/include/machine/_types.h /usr/include/i386/_types.h
.o/wid.o: /usr/include/sys/_structs.h /usr/include/machine/_structs.h
.o/wid.o: /usr/include/i386/_structs.h /usr/include/sys/appleapiopts.h
.o/wid.o: /usr/include/sys/_select.h wid_console.h wid_tooltip.h marshal.h
.o/wid.o: /usr/include/string.h /usr/include/_types.h
.o/wid.o: /usr/include/secure/_string.h /usr/include/secure/_common.h
.o/wid.o: config.h sound.h
.o/wid_button.o: main.h /usr/include/stdint.h ptrcheck.h wid.h color.h ttf.h
.o/wid_button.o: enum.h tree.h wid_button.h config.h
.o/wid_console.o: slre.h main.h /usr/include/stdint.h ptrcheck.h
.o/wid_console.o: wid_console.h command.h token.h wid.h color.h ttf.h enum.h
.o/wid_console.o: tree.h
.o/wid_dirlist.o: /usr/include/errno.h /usr/include/sys/errno.h
.o/wid_dirlist.o: /usr/include/sys/cdefs.h slre.h main.h
.o/wid_dirlist.o: /usr/include/stdint.h ptrcheck.h wid_dirlist.h wid.h
.o/wid_dirlist.o: color.h ttf.h enum.h tree.h dir.h /usr/include/string.h
.o/wid_dirlist.o: /usr/include/_types.h /usr/include/sys/_types.h
.o/wid_dirlist.o: /usr/include/machine/_types.h /usr/include/i386/_types.h
.o/wid_dirlist.o: /usr/include/secure/_string.h /usr/include/secure/_common.h
.o/wid_dirlist.o: wid_popup.h config.h
.o/wid_editor.o: /usr/include/libgen.h /usr/include/sys/cdefs.h
.o/wid_editor.o: /usr/include/errno.h /usr/include/sys/errno.h main.h
.o/wid_editor.o: /usr/include/stdint.h ptrcheck.h wid.h color.h
.o/wid_editor.o: wid_editor_buttons.h wid_editor_map.h wid_editor.h
.o/wid_editor.o: wid_popup.h wid_tooltip.h wid_dirlist.h wid_intro.h file.h
.o/wid_editor.o: /usr/include/string.h /usr/include/_types.h
.o/wid_editor.o: /usr/include/sys/_types.h /usr/include/machine/_types.h
.o/wid_editor.o: /usr/include/i386/_types.h /usr/include/secure/_string.h
.o/wid_editor.o: /usr/include/secure/_common.h marshal.h tree.h
.o/wid_editor.o: wid_text_input.h wid_textbox.h level.h
.o/wid_editor_buttons.o: main.h /usr/include/stdint.h ptrcheck.h wid.h
.o/wid_editor_buttons.o: color.h wid_editor.h wid_editor_buttons.h tree.h
.o/wid_editor_buttons.o: thing_template.h thing_template_private.h
.o/wid_editor_map.o: main.h /usr/include/stdint.h ptrcheck.h wid.h color.h
.o/wid_editor_map.o: tile.h thing_tile.h wid_editor.h wid_editor_map.h
.o/wid_editor_map.o: wid_editor_buttons.h thing_template.h map.h tree.h
.o/wid_editor_map.o: level.h config.h
.o/wid_game_map.o: main.h /usr/include/stdint.h ptrcheck.h wid.h color.h
.o/wid_game_map.o: tile.h thing_tile.h thing_template.h /usr/include/string.h
.o/wid_game_map.o: /usr/include/_types.h /usr/include/sys/_types.h
.o/wid_game_map.o: /usr/include/sys/cdefs.h /usr/include/machine/_types.h
.o/wid_game_map.o: /usr/include/i386/_types.h /usr/include/secure/_string.h
.o/wid_game_map.o: /usr/include/secure/_common.h wid_popup.h wid_textbox.h
.o/wid_game_map.o: wid_game_map.h wid_tooltip.h wid_intro.h player.h thing.h
.o/wid_game_map.o: item.h tree.h map.h wid_button.h sdl.h config.h level.h
.o/wid_game_map.o: level_private.h
.o/wid_game_over.o: main.h /usr/include/stdint.h ptrcheck.h wid.h color.h
.o/wid_game_over.o: wid_intro.h sdl.h thing_template.h wid_popup.h
.o/wid_game_over.o: wid_game_over.h
.o/wid_hiscore.o: main.h /usr/include/stdint.h ptrcheck.h wid.h color.h
.o/wid_hiscore.o: wid_hiscore.h /usr/include/string.h /usr/include/_types.h
.o/wid_hiscore.o: /usr/include/sys/_types.h /usr/include/sys/cdefs.h
.o/wid_hiscore.o: /usr/include/machine/_types.h /usr/include/i386/_types.h
.o/wid_hiscore.o: /usr/include/secure/_string.h /usr/include/secure/_common.h
.o/wid_hiscore.o: wid_text_input.h marshal.h tree.h
.o/wid_intro.o: main.h /usr/include/stdint.h ptrcheck.h wid.h tex.h color.h
.o/wid_intro.o: wid_intro.h wid_popup.h wid_editor.h wid_game_map.h
.o/wid_intro.o: wid_intro_about.h wid_intro_settings.h wid_intro_help.h
.o/wid_intro.o: wid_intro_guide.h wid_game_over.h wid_hiscore.h music.h sdl.h
.o/wid_intro.o: level.h
.o/wid_intro_about.o: main.h /usr/include/stdint.h ptrcheck.h wid.h
.o/wid_intro_about.o: wid_popup.h wid_intro_about.h
.o/wid_intro_guide.o: main.h /usr/include/stdint.h ptrcheck.h wid.h tex.h
.o/wid_intro_guide.o: color.h wid_intro.h wid_intro_guide.h sdl.h
.o/wid_intro_help.o: main.h /usr/include/stdint.h ptrcheck.h wid.h
.o/wid_intro_help.o: wid_popup.h wid_intro_help.h
.o/wid_intro_settings.o: main.h /usr/include/stdint.h ptrcheck.h wid.h
.o/wid_intro_settings.o: color.h wid_popup.h wid_intro_settings.h config.h
.o/wid_intro_settings.o: /usr/include/string.h /usr/include/_types.h
.o/wid_intro_settings.o: /usr/include/sys/_types.h /usr/include/sys/cdefs.h
.o/wid_intro_settings.o: /usr/include/machine/_types.h
.o/wid_intro_settings.o: /usr/include/i386/_types.h
.o/wid_intro_settings.o: /usr/include/secure/_string.h
.o/wid_intro_settings.o: /usr/include/secure/_common.h music.h
.o/wid_popup.o: slre.h main.h /usr/include/stdint.h ptrcheck.h wid.h color.h
.o/wid_popup.o: string_ext.h enum.h tree.h ttf.h wid_popup.h
.o/wid_splash.o: main.h /usr/include/stdint.h ptrcheck.h wid.h color.h tex.h
.o/wid_splash.o: init_fn.h tree.h wid_splash.h
.o/wid_test.o: slre.h main.h /usr/include/stdint.h ptrcheck.h wid.h color.h
.o/wid_text_input.o: slre.h main.h /usr/include/stdint.h ptrcheck.h
.o/wid_text_input.o: wid_text_input.h wid.h color.h ttf.h enum.h tree.h
.o/wid_text_input.o: config.h
.o/wid_textbox.o: main.h /usr/include/stdint.h ptrcheck.h wid_textbox.h wid.h
.o/wid_textbox.o: color.h string_ext.h enum.h tree.h ttf.h
.o/wid_tooltip.o: main.h /usr/include/stdint.h ptrcheck.h wid.h color.h
.o/wid_tooltip.o: string_ext.h enum.h tree.h ttf.h wid_tooltip.h config.h
