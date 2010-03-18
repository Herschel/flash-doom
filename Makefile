################################################################
#
# $Id:$
#
# $Log:$
#

MXMLC=mxmlc
MXMLCFLAGS=--target-player=10.0.0 -default-frame-rate=35 -default-size=960,600 -default-background-color=0x000000 \
	-optimize=true -frames.frame=2,DoomShim $(AS3SRC)/DoomContainer.as \
	-title="Doom" -creator="id Software" -date="12/02/2008"
MXMLCLIBS=-library-path+=$(DATA)/DoomAssets.swc,$(O)/doom/doom.swc,$(O)/heretic/heretic.swc,$(O)/hexen/hexen.swc

ALCPATH='$(ALCHEMY_HOME)/achacks:$(PATH)'

CSRC=src/c
AS3SRC=src/as3
DATA=data
O=bin

AS3OBJS=							\
		$(AS3SRC)/DoomContainer.as	\
		$(AS3SRC)/DoomGame.as		\
		$(AS3SRC)/DoomMenu.as		\
		$(AS3SRC)/Preloader.as		\
		$(DATA)/DoomAssets.swc

.PHONY:	all
all:	$(O)/DoomGame.swf

.PHONY: clean
clean:
	rm -f $(O)/*.swf
	rm -rf $(O)/*.o
	rm -rf $(O)/*.swc


$(O)/DoomGame.swf: $(AS3OBJS) $(O)/doom/doom.swc $(O)/heretic/heretic.swc $(O)/hexen/hexen.swc
	$(MXMLC) $(MXMLCFLAGS) $(MXMLCLIBS) -o $@

.PHONY: $(O)/doom/doom.swc
$(O)/doom/doom.swc:
	cd $(CSRC)/doom && make

.PHONY: $(O)/heretic/heretic.swc
$(O)/heretic/heretic.swc:
	cd $(CSRC)/heretic && make
	
.PHONY: $(O)/hexen/hexen.swc
$(O)/hexen/hexen.swc:
	cd $(CSRC)/hexen && make

#############################################################
#
#############################################################
