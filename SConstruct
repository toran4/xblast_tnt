list=Split("""src/xblast.c src/util.c src/str_util.c src/intro.c src/introdat.c src/map.c src/sprite.c 
	src/color.c src/status.c src/player.c src/bomb.c src/action.c src/event.c src/image.c src/shrink.c 
	src/func.c src/info.c src/ini_file.c src/atom.c src/scramble.c src/demo.c src/debug.c src/level.c 
	src/random.c src/game_local.c src/game_demo.c src/game_server.c src/game_client.c src/game.c 
	src/menu.c src/menu_player.c src/menu_level.c src/menu_control.c src/menu_game.c 
	src/menu_network.c src/menu_extras.c src/mi_tool.c src/mi_base.c src/mi_button.c 
	src/mi_color.c src/mi_combo.c src/mi_cyclic.c src/mi_host.c src/mi_int.c src/mi_keysym.c 
	src/mi_label.c src/mi_player.c src/mi_string.c src/mi_tag.c src/mi_toggle.c src/mi_map.c 
	src/mi_stat.c src/client.c src/server.c src/central.c src/network.c src/com.c src/browse.c src/com_to_server.c 
	src/com_listen.c src/com_to_client.c src/com_newgame.c src/com_stream.c src/com_dg_client.c  
	src/com_to_central.c src/com_from_central.c src/com_central.c src/user.c src/dat_rating.c 
	src/com_dg_server.c src/com_dgram.c src/com_query.c src/com_browse.c src/com_reply.c 
	src/com_base.c src/net_socket.c src/net_tele.c src/net_dgram.c src/cfg_main.c src/cfg_level.c 
	src/cfg_player.c src/cfg_game.c src/cfg_control.c src/cfg_stat.c src/cfg_demo.c 
	src/cfg_xblast.c src/chat.c  src/x11_common.c src/x11_event.c src/x11_atom.c src/x11_config.c
	src/x11_msgbox.c	src/x11c_init.c src/x11c_image.c src/x11c_text.c
	src/x11c_tile.c src/x11c_sprite.c 	src/x11c_pixmap.c src/x11_sound.c
	src/x11_socket.c src/x11_joystick.c src/bot.c  src/menu_edit.c src/version.c""")
opts = Options()
opts.Add(PathOption('Datadir',
                               'Path to configuration file',
                               '.'))

env = Environment(options = opts,
                             CPPDEFINES={'GAME_DATADIR' : '"$CONFIG"'},
	CPPPATH = ['src', '/usr/include'],
        LIBPATH = ['/usr/lib','/usr/X11R6/lib'] ) 

if 'bar' in COMMAND_LINE_TARGETS:
          print "Don't forget to copy `bar' to the archive!"
          print "Don't forget to copy `bar' to the archive!"

conf = Configure(env)
if not conf.CheckLib('m'):
        print 'Did not find libm.a or m.lib, exiting!'
        Exit(1)
if not conf.CheckLib('X11'):
        print 'Did not find lib X11 , exiting!'
        Exit(1)
env = conf.Finish()
env.Program('xblast',list)