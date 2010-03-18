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

O=../../../bin/hexen

OBJS=				\
		$(O)/i_sound.o		\
		$(O)/in_lude.o			\
		$(O)/tables.o			\
		$(O)/f_finale.o		\
		$(O)/d_net.o			\
		$(O)/g_game.o			\
		$(O)/m_misc.o			\
		$(O)/mn_menu.o		\
		$(O)/am_map.o			\
		$(O)/ct_chat.o			\
		$(O)/po_man.o		\
		$(O)/p_acs.o		\
		$(O)/p_anim.o		\
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
		$(O)/p_things.o		\
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
		$(O)/sounds.o		\
		$(O)/template.o		\
		$(O)/sv_save.o		\
		$(O)/st_start.o		\
		$(O)/sn_sonix.o		\
		$(O)/sc_man.o		\
		$(O)/a_action.o		\
		$(O)/h2_main.o		\

.PHONY:	all
all:	swc
		

.PHONY: clean
clean:
	rm -f $(O)/*.o
	rm -f $(O)/hexen.swc

.PHONY: swc
swc:
	@make PATH=$(ALCPATH) $(O)/hexen.swc
	
# alchemy gcc/llvm seems to be sensitive to output path.  so do doom.swc in root project folder
# and move it to where we want
$(O)/hexen.swc:	$(OBJS) $(O)/i_flash.o
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) $(O)/i_flash.o \
	-o hexen.swc $(LIBS)
	mv hexen.swc $@ 

$(O)/%.o:	$(CSRC)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

#############################################################
#
#############################################################
