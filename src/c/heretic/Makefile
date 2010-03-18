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

ALCPATH='$(ALCHEMY_HOME)/achacks:$(PATH)'

CSRC=.

O=../../../bin/heretic

# not too sophisticated dependency
OBJS=				\
		$(O)/i_sound.o		\
		$(O)/in_lude.o			\
		$(O)/tables.o			\
		$(O)/f_finale.o		\
		$(O)/d_main.o			\
		$(O)/d_net.o			\
		$(O)/g_game.o			\
		$(O)/m_misc.o			\
		$(O)/mn_menu.o		\
		$(O)/am_map.o			\
		$(O)/ct_chat.o			\
		$(O)/p_ceilng.o		\
		$(O)/p_doors.o		\
		$(O)/p_enemy.o		\
		$(O)/p_floor.o		\
		$(O)/p_inter.o		\
		$(O)/p_lights.o		\
		$(O)/p_map.o			\
		$(O)/p_maputl.o		\
		$(O)/p_mobj.o			\
		$(O)/p_plats.o		\
		$(O)/p_pspr.o			\
		$(O)/p_setup.o		\
		$(O)/p_sight.o		\
		$(O)/p_spec.o			\
		$(O)/p_switch.o		\
		$(O)/p_telept.o		\
		$(O)/p_tick.o			\
		$(O)/p_user.o			\
		$(O)/r_bsp.o			\
		$(O)/r_data.o			\
		$(O)/r_draw.o			\
		$(O)/r_main.o			\
		$(O)/r_plane.o		\
		$(O)/r_segs.o			\
		$(O)/r_things.o		\
		$(O)/w_wad.o			\
		$(O)/v_video.o		\
		$(O)/sb_bar.o			\
		$(O)/z_zone.o			\
		$(O)/info.o				\
		$(O)/sounds.o

.PHONY:	all
all:	swc
		

.PHONY: clean
clean:
	rm -f $(O)/*.o
	rm -f $(O)/heretic.swc

.PHONY: swc
swc:
	@make PATH=$(ALCPATH) $(O)/heretic.swc
	
# alchemy gcc/llvm seems to be sensitive to output path.  so do doom.swc in root project folder
# and move it to where we want
$(O)/heretic.swc:	$(OBJS) $(O)/i_flash.o
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) $(O)/i_flash.o \
	-o heretic.swc $(LIBS)
	mv heretic.swc $@ 

$(O)/%.o:	$(CSRC)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

#############################################################
#
#############################################################
