AUTOMAKE_OPTIONS = foreign subdir-objects

if SDL

sdl_files=src/sdl_common.h src/sdl_event.h  src/sdl_config.h  src/sdl_image.h src/sdl_keysym.h\
	src/sdl_text.h src/sdl_tile.h src/sdl_sprite.h 	src/sdl_pixmap.h\
	src/sdl_common.c src/sdl_event.c src/sdl_atom.c src/sdl_config.c\
	src/sdl_msgbox.c	src/sdl_init.c src/sdl_image.c src/sdl_text.c\
	src/sdl_tile.c src/sdl_sprite.c src/sdl_pixmap.c src/sdl_keysym.c\
	src/sdl_socket.c src/sdl_joystick.c src/sdl_socket.h src/sdl_sprite.h src/sdl_joystick.h\
	src/sdl_sound.c
x11_files= src/x11_sound.h 

else

if XBSNDSRV
soundbinaries =  xbsndsrv
else
soundbinaries =
endif

sdl_files=
x11_files=	src/x11_common.c src/x11_event.c src/x11_atom.c src/x11_config.c\
	src/x11_msgbox.c	src/x11c_init.c src/x11c_image.c src/x11c_text.c\
	src/x11c_tile.c src/x11c_sprite.c 	src/x11c_pixmap.c src/x11_sound.c\
	src/x11_socket.c src/x11_joystick.c\
	src/x11_common.h src/x11_event.h  src/x11_config.h  src/x11c_image.h\
	src/x11c_text.h src/x11c_tile.h src/x11c_sprite.h 	src/x11c_pixmap.h\
	src/x11_sound.h src/x11_socket.h src/x11_joystick.h  

endif

bin_PROGRAMS =  ${soundbinaries} xblast

xblast_SOURCES=src/xblast.c src/util.c src/str_util.c src/intro.c src/introdat.c src/map.c src/sprite.c \
	src/color.c src/status.c src/player.c src/action.c src/bomb.c src/event.c src/image.c\
	src/shrink.c src/func.c src/info.c src/ini_file.c src/atom.c src/scramble.c \
	src/demo.c src/debug.c src/level.c src/random.c \
	src/game_local.c src/game_demo.c src/game_server.c src/game_client.c src/game.c\
	src/menu.c src/menu_player.c src/menu_level.c src/menu_control.c src/menu_game.c\
	src/menu_network.c src/menu_extras.c	src/mi_tool.c src/mi_base.c\
	src/mi_button.c src/mi_color.c src/mi_combo.c src/mi_cyclic.c\
	src/mi_host.c src/mi_int.c src/mi_keysym.c src/mi_label.c src/mi_player.c\
	src/mi_string.c 	src/mi_tag.c src/mi_toggle.c src/mi_map.c src/mi_stat.c	src/client.c\
	src/server.c src/network.c src/com.c src/browse.c	src/com_to_server.c\
	src/com_listen.c src/com_to_client.c src/com_stream.c 	src/com_dg_client.c\
	src/com_dg_server.c src/com_dgram.c 	src/com_query.c src/com_browse.c\
	src/com_reply.c src/com_base.c 	src/net_socket.c src/net_tele.c\
	src/net_dgram.c 	src/central.c src/com_newgame.c src/com_to_central.c\
	src/com_from_central.c src/com_central.c src/user.c src/dat_rating.c \
	src/cfg_main.c src/cfg_level.c src/cfg_player.c src/cfg_game.c\
	src/cfg_control.c 	src/cfg_stat.c src/cfg_demo.c src/cfg_xblast.c\
	src/bot.c src/xblast.h src/util.h \
	src/str_util.h src/intro.h src/introdat.h src/map.h src/sprite.h 	src/color.h \
	src/status.h src/player.h src/action.h src/bomb.h src/event.h src/image.h \
	src/shrink.h src/func.h src/info.h src/ini_file.h src/atom.h src/scramble.h \
	src/demo.h src/debug.h src/level.h src/random.h \
	src/game_local.h src/game_demo.h src/game_server.h src/game_client.h src/game.h\
	src/menu.h src/menu_player.h src/menu_level.h src/menu_control.h src/menu_game.h\
	src/menu_network.h src/menu_extras.h	src/mi_tool.h src/mi_base.h\
	src/mi_button.h src/mi_color.h src/mi_combo.h src/mi_cyclic.h\
	src/mi_host.h src/mi_int.h src/mi_keysym.h src/mi_label.h src/mi_player.h\
	src/mi_string.h 	src/mi_tag.h src/mi_toggle.h src/mi_map.h src/mi_stat.h	src/client.h\
	src/server.h src/network.h src/com.h src/browse.h	src/com_to_server.h\
	src/com_listen.h src/com_to_client.h src/com_stream.h \
	src/com_dg_client.h src/com_dg_server.h src/com_dgram.h \
	src/com_query.h src/com_browse.h src/com_reply.h src/com_base.h \
	src/net_socket.h src/net_tele.h src/net_dgram.h 	src/central.h\
	src/com_newgame.h src/com_to_central.h src/com_from_central.h src/com_central.h\
	src/user.h src/dat_rating.h 	src/cfg_main.h src/cfg_level.h src/cfg_player.h\
	src/cfg_game.h src/cfg_control.h 	src/cfg_stat.h src/cfg_demo.h src/cfg_xblast.h\
	src/bot.h src/shrinkdat.h\
	src/version.h  src/x11c_sprite.h src/socket.h  src/gui.h src/geom.h config.h src/snd.h\
	src/menu_layout.h src/chat.h src/chat.c src/menu_edit.c src/menu_edit.h src/version.c\
    ${sdl_files} ${x11_files}

EXTRA_DIST = xblast.man autogen.sh \
	packaging/windows/Makefile.w32 packaging/windows/Makefile.windows \
	packaging/windows/*.ico packaging/windows/*.coff packaging/windows/*.rc \
	packaging/windows/*.wxs packaging/windows/*.dsp packaging/windows/*.dsw \
	packaging/windows/*.rtf packaging/windows/*.sh packaging/windows/*.bat \
	src/w32*.h src/w32*.c src/time.c src/timeval.h src/SDL_vkeys.h src/config-mingw.h Imakefile\
	image/misc/*.ppm image/misc/*.pbm  image/explosion/*.pbm \
	image/explosion/*.ppm image/score/*.epm image/score/*.ppm \
	image/sprite/*.pbm image/sprite/*.ppm image/block/*.ppm \
	image/sprite/*.epm level/*.xal sounds/*.raw

xbsndsrv_SOURCES = src/xbsndsrv.c src/snd.h 

game_datadir=@game_datadir@
localedir = $(game_datadir)/locale
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@

install-data-local:
	@$(NORMAL_INSTALL)
	if test -d $(srcdir)/image; then \
	  $(mkinstalldirs) $(DESTDIR)$(game_datadir); \
	  $(mkinstalldirs) $(DESTDIR)$(game_datadir)/image; \
	  cp -r $(srcdir)/image/* $(DESTDIR)$(game_datadir)/image;\
	fi; \
	if test -d $(srcdir)/level; then \
	  $(mkinstalldirs) $(DESTDIR)$(game_datadir)/ ; \
	  $(mkinstalldirs) $(DESTDIR)$(game_datadir)/level; \
	  cp -r $(srcdir)/level/* $(DESTDIR)$(game_datadir)/level;\
	fi; \
	if test -d $(srcdir)/sounds; then \
	  $(mkinstalldirs) $(DESTDIR)$(game_datadir)/ ; \
	  $(mkinstalldirs) $(DESTDIR)$(game_datadir)/sounds; \
	  cp -r $(srcdir)/sounds/* $(DESTDIR)$(game_datadir)/sounds;\
	fi; \
	if test -f $(srcdir)/Vera.ttf; then \
	  $(mkinstalldirs) $(DESTDIR)$(game_datadir); \
	  $(INSTALL_DATA) $(srcdir)/Vera.ttf $(DESTDIR)$(game_datadir)/Vera.ttf; \
	fi;

indent:
	 indent -npro -gnu -ts4 -i4 -br -lp -sob -l100 -ss -ncs src/*.[ch]

SUBDIRS = po

ACLOCAL_AMFLAGS = -I m4
