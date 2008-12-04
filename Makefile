################################################################
#
# $Id:$
#
# $Log:$
#
CC=gcc  # gcc or g++

CFLAGS=-swc -w -O3 -DNORMALUNIX -I$(ALCHEMY_HOME) #-Wall -DUSEASM -DLINUX 
#LDFLAGS=-L/usr/X11R6/lib
LIBS=-lnsl -lm

MXMLC=mxmlc
MXMLCFLAGS=--target-player=10.0.0 -default-frame-rate=35 -default-size=960,600 -default-background-color=0x000000 \
	-optimize=true -frames.frame=2,DoomGame $(AS3SRC)/DoomContainer.as \
	-title="Doom" -creator="id Software" -date="12/02/2008"
MXMLCLIBS=-library-path+=$(O)/doom.swc,$(O)/DoomPreloader.swc

ALCPATH='$(ALCHEMY_HOME)/achacks:$(PATH)'

AS3SRC=src/as3
CSRC=src/c

O=bin

# not too sophisticated dependency
OBJS=				\
		$(O)/doomdef.o		\
		$(O)/doomstat.o		\
		$(O)/dstrings.o		\
		$(O)/i_system.o		\
		$(O)/i_sound.o		\
		$(O)/i_video.o		\
		$(O)/i_net.o			\
		$(O)/tables.o			\
		$(O)/f_finale.o		\
		$(O)/f_wipe.o 		\
		$(O)/d_main.o			\
		$(O)/d_net.o			\
		$(O)/d_items.o		\
		$(O)/g_game.o			\
		$(O)/m_menu.o			\
		$(O)/m_misc.o			\
		$(O)/m_argv.o  		\
		$(O)/m_bbox.o			\
		$(O)/m_fixed.o		\
		$(O)/m_swap.o			\
		$(O)/m_cheat.o		\
		$(O)/m_random.o		\
		$(O)/am_map.o			\
		$(O)/p_ceilng.o		\
		$(O)/p_doors.o		\
		$(O)/p_enemy.o		\
		$(O)/p_floor.o		\
		$(O)/p_inter.o		\
		$(O)/p_lights.o		\
		$(O)/p_map.o			\
		$(O)/p_maputl.o		\
		$(O)/p_plats.o		\
		$(O)/p_pspr.o			\
		$(O)/p_setup.o		\
		$(O)/p_sight.o		\
		$(O)/p_spec.o			\
		$(O)/p_switch.o		\
		$(O)/p_mobj.o			\
		$(O)/p_telept.o		\
		$(O)/p_tick.o			\
		$(O)/p_saveg.o		\
		$(O)/p_user.o			\
		$(O)/r_bsp.o			\
		$(O)/r_data.o			\
		$(O)/r_draw.o			\
		$(O)/r_main.o			\
		$(O)/r_plane.o		\
		$(O)/r_segs.o			\
		$(O)/r_sky.o			\
		$(O)/r_things.o		\
		$(O)/w_wad.o			\
		$(O)/wi_stuff.o		\
		$(O)/v_video.o		\
		$(O)/st_lib.o			\
		$(O)/st_stuff.o		\
		$(O)/hu_stuff.o		\
		$(O)/hu_lib.o			\
		$(O)/s_sound.o		\
		$(O)/z_zone.o			\
		$(O)/info.o				\
		$(O)/sounds.o

AS3OBJS=							\
		$(AS3SRC)/DoomContainer.as	\
		$(AS3SRC)/DoomGame.as		\
		$(AS3SRC)/Preloader.as		\
		$(O)/DoomPreloader.swc

.PHONY:	all
all:	$(O)/DoomGame.swf
		

.PHONY: clean
clean:
	rm -f $(O)/*.o
	rm -f $(O)/doom.swc
	rm -f $(O)/DoomGame.swf

.PHONY: swc
swc:
	@make PATH=$(ALCPATH) $(O)/doom.swc

$(O)/DoomGame.swf: $(AS3OBJS) swc
	$(MXMLC) $(MXMLCFLAGS) $(MXMLCLIBS) -o $@
	
# alchemy gcc/llvm seems to be sensitive to output path.  so do doom.swc in root project folder
# and move it to where we want
$(O)/doom.swc:	$(OBJS) $(O)/i_main.o
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) $(O)/i_main.o \
	-o doom.swc $(LIBS)
	mv doom.swc $@ 

$(O)/%.o:	$(CSRC)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

#############################################################
#
#############################################################
