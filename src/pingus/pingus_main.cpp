// src/pingus/pingus_main.cpp
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Pingus - A free Lemmings clone
// Copyright (C) 1998-2008 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#include "pingus/pingus_main.hpp"

#include <iostream>
#include <memory>
#include <signal.h>

#ifndef DISABLE_EDITOR
#include "editor/editor_level.hpp"
#include "editor/editor_screen.hpp"
#endif

#include "engine/input/manager.hpp"
#include "engine/system/sdl_system.hpp"
#include "pingus/config_manager.hpp"
#include "pingus/screens/demo_session.hpp"
#include "pingus/screens/level_menu.hpp"
#include "pingus/screens/pingus_menu.hpp"
#include "pingus/worldmap/worldmap_screen.hpp"
#include "util/log.hpp"
#include "util/string_util.hpp"
#include "util/system.hpp"
#include "util/i18n.hpp"

#ifdef __WII__
#  include "util/wii.hpp"
#endif

#ifdef __VITA__
#  include "util/vita.hpp"
#endif

#include "util/command_line.hpp"

#include "engine/screen/screen_manager.hpp"
#include "pingus/globals.hpp"
#include "pingus/options.hpp"
#include "pingus/path_manager.hpp"
#include "pingus/plf_res_mgr.hpp"

#include "engine/sound/sound.hpp"
#include <fstream>
#include "pingus/resource.hpp"
#include "pingus/savegame_manager.hpp"
#include "pingus/screens/credits.hpp"
#include "pingus/screens/font_test_screen.hpp"
#include "pingus/screens/start_screen.hpp"
#include "pingus/screens/story_screen.hpp"
#include "pingus/stat_manager.hpp"
#include "pingus/worldobj_factory.hpp"

namespace pingus {

PingusMain::PingusMain() :
  cmd_options()
{
}

PingusMain::~PingusMain()
{
}

void
PingusMain::read_rc_file (void)
{
  if (!cmd_options.no_config_file.is_set() ||
      !cmd_options.no_config_file.get())
  {
    std::string filename = System::get_userdir() + "config";

    if (!System::exist(filename))
    {
      log_info("{}: config file not found", filename);
    }
    else
    {
      try
      {
        CommandLineOptions options;
        options.merge(Options::from_file(Pathname(filename, Pathname::SYSTEM_PATH)));
        options.merge(cmd_options);
        cmd_options = options;
      }
      catch(const std::exception& err)
      {
        log_error("{}", err.what());
      }
    }
  }
}

void
PingusMain::apply_args()
{
  // FIXME: merge cmd_options with stuff read from config file here
  auto& options = cmd_options;

  if (options.software_cursor.is_set())
    globals::software_cursor = options.software_cursor.get();

  // Sound
  if (options.disable_music.is_set())
    globals::music_enabled = !options.disable_music.get();

  if (options.disable_sound.is_set())
    globals::sound_enabled = !options.disable_sound.get();

  // Misc
  if (options.auto_scrolling.is_set())
    globals::auto_scrolling = options.auto_scrolling.get();

  if (options.drag_drop_scrolling.is_set())
    globals::drag_drop_scrolling = options.drag_drop_scrolling.get();

  if (options.developer_mode.is_set())
    globals::developer_mode = options.developer_mode.get();

  if (options.speed.is_set())
    globals::game_speed = options.speed.get();

  if (options.desiredfps.is_set())
    globals::desired_fps = options.desiredfps.get();

  if (options.tile_size.is_set())
    globals::tile_size = options.tile_size.get();
}

void
PingusMain::parse_args(int argc, char** argv)
{
  CommandLine argp;
  argp.add_usage("[OPTIONS]... [FILE]");
  argp.add_doc("Pingus is a puzzle game where you need to guide a bunch of "
               "little penguins around the world.");

  argp.add_group("General Options:");
  argp.add_option('h', "help", "", "Displays this help");
  argp.add_option('V', "version", "", "Print version number and exit");
  argp.add_option('v', "verbose", "", "Enable info level log output");
  argp.add_option('D', "debug", "", "Enable debug level log output");
  argp.add_option('Q', "quiet", "", "Disable all log output");

  argp.add_group("Display Options:");
  argp.add_option('w', "window", "", "Start in Window Mode");
  argp.add_option('f', "fullscreen", "", "Start in Fullscreen");
  argp.add_option('r', "renderer", "RENDERER",
                  "Use the given renderer (default: sdl)");
  argp.add_option('g', "geometry", "{width}x{height}",
                  "Set the window resolution for pingus (default: 800x600)");
  argp.add_option('R', "fullscreen-resolution", "{width}x{height}",
                  "Set the resolution used in fullscreen mode (default: 800x600)");
  argp.add_option(346, "software-cursor", "", "Enable software cursor");

  argp.add_group("Game Options:");
  argp.add_option(337, "no-auto-scrolling", "", "Disable automatic scrolling");
  argp.add_option(338, "drag-drop-scrolling", "",
                  "Enable drag'n drop scrolling");

  argp.add_group("Sound Options:");
  argp.add_option('s', "disable-sound", "", "Disable sound");
  argp.add_option('m', "disable-music", "", "Disable music");

#ifndef DISABLE_EDITOR
  argp.add_group("Editor Options:");
  argp.add_option('e', "editor", "", "Loads the level editor");
#endif

  argp.add_group("Directory Options:");
  argp.add_option('d', "datadir", "DIR", "Load game datafiles from DIR");
  argp.add_option('u', "userdir", "DIR",
                  "Load config files and store savegames in DIR");
  argp.add_option('a', "addon", "DIR", "Load game modifications from DIR");
  argp.add_option(342, "no-cfg-file", "", "Don't read ~/.pingus/config");
  argp.add_option('c', "config", "FILE", "Read config options from FILE");
  argp.add_option(360, "controller", "FILE",
                  "Uses the controller given in FILE");

  argp.add_group("Debug Options:");
  argp.add_option(334, "developer-mode", "",
                  "Enables some special features for developers");
  argp.add_option('t', "speed", "SPEED",
                  "Set the game speed (0=fastest, >0=slower)");
  argp.add_option('k', "fps", "FPS",
                  "Set the desired game framerate (frames per second)");
  argp.add_option(344, "tile-size", "INT",
                  "Set the size of the map tiles (default: 32)");

  argp.parse_args(argc, argv);
  argp.set_help_indent(20);

  while (argp.next())
  {
    switch (argp.get_key())
    {
      case 'r': // --renderer
        if (argp.get_argument() == "help")
        {
          std::cout << "Available renderers: " << std::endl;
          std::cout << "     sdl: Software rendering" << std::endl;
          std::cout << "  opengl: Hardware accelerated graphics" << std::endl;
          std::cout << "    null: No rendering at all, for debugging" << std::endl;
          exit(EXIT_SUCCESS);
        }
        else
        {
          cmd_options.framebuffer_type.set(
              framebuffer_type_from_string(argp.get_argument()));
        }
        break;

#ifndef DISABLE_EDITOR
      case 'e': // -e, --editor
        cmd_options.editor.set(true);
        break;
#endif

      case 't': // -t, --set-speed
        cmd_options.speed.set(StringUtil::to<int>(argp.get_argument()));
        break;

      case 'k': // -k, --set-fps
        cmd_options.desiredfps.set(StringUtil::to<float>(argp.get_argument()));
        break;

      case 's': // -s, --disable-sound
        cmd_options.disable_sound.set(true);
        break;

      case 'm': // -m, --disable-music
        cmd_options.disable_music.set(true);
        break;

      case 'g':
      {
        Size size;
        if (sscanf(argp.get_argument().c_str(), "%dx%d", &size.width, &size.height) != 2)
        {
          std::cout << "Resolution std::string is wrong, it should be like: \n"
                    << "\"640x480\" or \"800x600\"" << std::endl;
          exit(EXIT_FAILURE);
        }
        cmd_options.geometry.set(size);
      }
      break;

      case 'R':
      {
        Size size;
        if (sscanf(argp.get_argument().c_str(), "%dx%d", &size.width, &size.height) != 2)
        {
          std::cout << "Resolution std::string is wrong, it should be like: \n"
                    << "\"640x480\" or \"800x600\"" << std::endl;
          exit(EXIT_FAILURE);
        }
        cmd_options.fullscreen_resolution.set(size);
      }
      break;

      case 'd': // -d, --datadir
        cmd_options.datadir.set(argp.get_argument());
        break;

      case 'a': // -a, --addon
        g_path_manager.add_overlay_path(argp.get_argument());
        break;

      case 'u': // -u, --userdir
        cmd_options.userdir.set(argp.get_argument());
        break;

      case 'V':
        std::cout << "Pingus " VERSION "\n"
                     "Copyright (C) 1998-2011 Ingo Ruhnke <grumbel@gmail.com>\n"
                     "See the file AUTHORS for a complete list of contributors.\n"
                     "Pingus comes with ABSOLUTELY NO WARRANTY. This is free "
                     "software, and you are\n"
                     "welcome to redistribute it under certain conditions; see "
                     "the file LICENSE for details."
                  << std::endl;
        exit(EXIT_SUCCESS);
        break;

      case 'f': // --fullscreen
        cmd_options.fullscreen.set(true);
        break;

      case 'w': // --window
        cmd_options.fullscreen.set(false);
        break;

      case 334: // --developer-mode
        cmd_options.developer_mode.set(true);
        globals::developer_mode = true;
        break;

      case 337:
        cmd_options.auto_scrolling.set(false);
        break;

      case 338:
        cmd_options.drag_drop_scrolling.set(true);
        break;

      case 342: // --no-cfg-file
        cmd_options.no_config_file.set(true);
        break;

      case 344:
        cmd_options.tile_size.set(StringUtil::to<int>(argp.get_argument()));
        break;

      case 346:
        cmd_options.software_cursor.set(true);
        break;

      case 'c':
        cmd_options.merge(Options::from_file(Pathname(argp.get_argument(), Pathname::SYSTEM_PATH)));
        break;

      case 'D':
        logmich::set_log_level(logmich::kDebug);
        break;

      case 'v':
        logmich::set_log_level(logmich::kInfo);
        break;

      case 'Q':
        logmich::set_log_level(logmich::kNone);
        break;

      case 360:
        cmd_options.controller.set(argp.get_argument());
        break;

      case 'h':
        argp.print_help();
        exit(EXIT_SUCCESS);
        break;

      case CommandLine::REST_ARG:
        if (!cmd_options.rest.is_set())
        {
          cmd_options.rest.set(argp.get_argument());
        }
        else
        {
          std::cout << "Wrong argument: '" << argp.get_argument() << "'" << std::endl;
          std::cout << "You can only give one file argument," << std::endl;
          exit(EXIT_FAILURE);
        }
        break;

      default:
        std::cout << "Error: Got " << argp.get_key() << " " << argp.get_argument() << std::endl;
        break;
    }
  }
}
// Get all filenames and directories
void
PingusMain::init_path_finder()
{
  if (cmd_options.userdir.is_set())
    System::set_userdir(cmd_options.userdir.get());

  System::init_directories();

  if (cmd_options.datadir.is_set())
  {
    g_path_manager.set_path(cmd_options.datadir.get());
  }
  else
  {
#ifdef __WII__
    // On Wii, get data directory from SD/USB storage
    g_path_manager.set_path(Wii::get_data_dir());
#elif defined(__VITA__)
    // On Vita, game data is in the app bundle (app0:)
    g_path_manager.set_path(Vita::get_data_dir());
#else
    // assume game is run from source dir
    g_path_manager.set_path("data");
#endif
  }
}

void
PingusMain::print_greeting_message()
{
  std::string greeting = "Welcome to Pingus " VERSION;
  greeting += "!";
  std::cout <<  greeting << std::endl;
  for (unsigned int i = 0; i < greeting.length(); ++i)
    std::cout.put('=');
  std::cout << std::endl;

  std::cout << "userdir:                 " << System::get_userdir() << std::endl;
  std::cout << "datadir:                 " << g_path_manager.get_path() << std::endl;

  if (globals::sound_enabled)
    std::cout << "sound support:           enabled" << std::endl;
  else
    std::cout << "sound support:           disabled" << std::endl;

  if (globals::music_enabled)
    std::cout << "music support:           enabled" << std::endl;
  else
    std::cout << "music support:           disabled" << std::endl;

  std::cout << "fullscreen:              ";
  if (cmd_options.fullscreen.is_set() && cmd_options.fullscreen.get())
  {
    std::cout << cmd_options.fullscreen_resolution.get().width << "x"
              << cmd_options.fullscreen_resolution.get().height << std::endl;
  }
  else
  {
    std::cout << "disabled" << std::endl;
  }

  std::cout << std::endl;
}

void
PingusMain::start_game ()
{
  input::Manager input_manager;
  input::ControllerPtr input_controller;

  std::string controller_path;

  if (!cmd_options.controller.is_set())
  {
#ifdef __WII__
    // On Wii, default to the Wii controller config if nothing else is specified
    // Note: We use DATA_PATH here because the file is in the game data, not user config
    controller_path = "controller/wii.scm";
    input_controller = input_manager.create_controller(Pathname(controller_path, Pathname::DATA_PATH));
#elif defined(__VITA__)
    // On Vita, use the Vita controller config
    controller_path = "controller/vita.scm";
    input_controller = input_manager.create_controller(Pathname(controller_path, Pathname::DATA_PATH));
#else
    controller_path = "controller/default.scm";
    input_controller = input_manager.create_controller(Pathname(controller_path, Pathname::DATA_PATH));
#endif
  }
  else
  {
    controller_path = cmd_options.controller.get();
    input_controller = input_manager.create_controller(Pathname(controller_path, Pathname::SYSTEM_PATH));
  }

  log_info("Vita startup: controller config {} loaded", controller_path);
  ScreenManager  screen_manager(input_manager, input_controller);

#ifndef DISABLE_EDITOR
  if (cmd_options.editor.is_set() && cmd_options.editor.get())
  { // Editor
    std::shared_ptr<editor::EditorScreen> editor =
        std::make_shared<editor::EditorScreen>();
    // optionally load a map in the editor if it was given
    if (cmd_options.rest.is_set())
      editor->load(Pathname(cmd_options.rest.get(), Pathname::SYSTEM_PATH));

    screen_manager.push_screen(editor);
  }
  else
#endif
  if (cmd_options.rest.is_set())
  { // just start the map that was passed on the command line
    if (cmd_options.rest.get().ends_with(".pingus-demo"))
    { // Demo file
      screen_manager.push_screen(std::make_shared<DemoSession>(
          Pathname(cmd_options.rest.get(), Pathname::SYSTEM_PATH)));
    }
    else if (cmd_options.rest.get().ends_with(".font"))
    {
      Pathname filename(cmd_options.rest.get(), Pathname::SYSTEM_PATH);
      screen_manager.push_screen(std::make_shared<FontTestScreen>(filename));
    }
    else if (cmd_options.rest.get().ends_with(".credits"))
    {
      Pathname filename(cmd_options.rest.get(), Pathname::SYSTEM_PATH);
      screen_manager.push_screen(std::make_shared<Credits>(filename));
    }
    else if (cmd_options.rest.get().ends_with(".worldmap"))
    {
      Pathname filename(cmd_options.rest.get(), Pathname::SYSTEM_PATH);

      std::shared_ptr<::pingus::worldmap::WorldmapScreen> worldmap_screen =
          std::make_shared<::pingus::worldmap::WorldmapScreen>();
      worldmap_screen->load(filename);
      ScreenManager::instance()->push_screen(worldmap_screen);
    }
    else if (cmd_options.rest.get().ends_with(".story"))
    {
      screen_manager.push_screen(
          std::make_shared<StoryScreen>(FileReader::parse(
              Pathname(cmd_options.rest.get(), Pathname::SYSTEM_PATH))));
    }
    else if (cmd_options.rest.get().ends_with(".levelset"))
    {
      std::shared_ptr<LevelMenu> lvlm = std::make_shared<LevelMenu>();
      std::unique_ptr<Levelset> levelset = Levelset::from_file(
          Pathname(cmd_options.rest.get(), Pathname::SYSTEM_PATH));
      lvlm->set_levelset(levelset.release());
      screen_manager.push_screen(lvlm);
    }
    else
    { // Level file
      screen_manager.push_screen(
          std::make_shared<StartScreen>(PLFResMgr::load_plf_from_filename(
              Pathname(cmd_options.rest.get(), Pathname::SYSTEM_PATH))));
    }
  }
  else // start a normal game
  {
    log_info("starting normal game");
    screen_manager.push_screen(std::make_shared<PingusMenu>());
    log_info("done: starting normal game");
  }

  screen_manager.display();
}

int
PingusMain::run(int argc, char** argv)
{
#ifdef __VITA__
  logmich::set_log_level(logmich::kInfo);
#else
  logmich::set_log_level(logmich::kWarning);
#endif

  try
  {
    parse_args(argc, argv);

#ifdef __VITA__
    // Ensure user-writable Vita base dir exists and redirect logs early.
    try {
      System::create_dir(Vita::get_base_dir());
    } catch (...) {
      // ignore failures here; later code will handle fatal errors.
    }

    static std::shared_ptr<std::ofstream> vita_log_file;
    if (!vita_log_file) {
      vita_log_file = std::make_shared<std::ofstream>(Vita::get_base_dir() + "pingus.log", std::ios::app);
      if (vita_log_file->is_open()) {
        std::cout.rdbuf(vita_log_file->rdbuf());
        std::cerr.rdbuf(vita_log_file->rdbuf());
        log_info("Vita logging redirected to {}", Vita::get_base_dir() + "pingus.log");
      } else {
        log_warn("Failed to open Vita log file: {}", Vita::get_base_dir() + "pingus.log");
      }
    }
#endif

    init_path_finder();
    read_rc_file();
    apply_args();

#ifdef __WII__
    // Force Wii-appropriate defaults AFTER parsing arguments and config files
    // to prevent accidental overrides.
    cmd_options.software_cursor.set(true);
    cmd_options.fullscreen.set(true);
    cmd_options.resizable.set(false);
    cmd_options.fullscreen_resolution.set(Size(640, 480));
    cmd_options.geometry.set(Size(640, 480));
    cmd_options.framebuffer_type.set(OPENGL_FRAMEBUFFER);
    // Ensure globals are updated with these forced values
    globals::software_cursor = true;
#endif

#ifdef __VITA__
    // Force Vita-appropriate defaults AFTER parsing arguments and config files
    // to prevent accidental overrides.
    cmd_options.software_cursor.set(true);
    cmd_options.fullscreen.set(true);
    cmd_options.resizable.set(false);
    cmd_options.fullscreen_resolution.set(Size(960, 544));
    cmd_options.geometry.set(Size(960, 544));
    cmd_options.framebuffer_type.set(OPENGL_FRAMEBUFFER);
    // Ensure globals are updated with these forced values
    globals::software_cursor = true;
#endif

    print_greeting_message();

    // init the display
#ifdef __VITA__
    FramebufferType fbtype = OPENGL_FRAMEBUFFER;
#else
    FramebufferType fbtype = SDL_FRAMEBUFFER;
#endif
    if (cmd_options.framebuffer_type.is_set())
    {
      fbtype = cmd_options.framebuffer_type.get();
    }

    bool fullscreen = cmd_options.fullscreen.is_set() ? cmd_options.fullscreen.get() : false;
    bool resizable  = cmd_options.resizable.is_set()  ? cmd_options.resizable.get()  : true;

    Size screen_size(800, 600);
    if (fullscreen)
    {
      if (cmd_options.fullscreen_resolution.is_set())
      {
        screen_size = cmd_options.fullscreen_resolution.get();
      }
    }
    else
    {
      if (cmd_options.geometry.is_set())
      {
        screen_size = cmd_options.geometry.get();
      }
    }

    SDLSystem system;
    try
    {
      system.create_window(fbtype, screen_size, fullscreen, resizable);
      log_info("Vita startup: window created with renderer {} {}x{}", framebuffer_type_to_string(fbtype), screen_size.width, screen_size.height);
    }
    catch(const std::exception& err)
    {
      if (fbtype == SDL_FRAMEBUFFER)
      {
        throw;
      }
      else
      {
        log_error("couldn't create window, falling back to SDL: {}", err.what());
        system.create_window(SDL_FRAMEBUFFER, screen_size, fullscreen, resizable);

        // Update both command line options and config manager to reflect reality
        cmd_options.framebuffer_type.set(SDL_FRAMEBUFFER);
        config_manager.set_renderer(SDL_FRAMEBUFFER);
      }
    }

    log_info("Vita startup: beginning game initialization");

    // init other components
    SavegameManager savegame_manager("savegames/savegames.scm");
    StatManager stat_manager("savegames/variables.scm");

    // FIXME: turn these into RAII
    Resource::init();
    fonts::init();
    sound::PingusSound::init();

    config_manager.apply(cmd_options);

    // Initialize translation system (i18n) after config is applied
    // so the configured language from the config file is used
    {
      std::string lang = config_manager.get_language();
      i18n::init(lang);
      log_info("Initialized i18n with language: {}", lang);
    }

    // start and run the actual game
    start_game();
  }
  catch (const std::bad_alloc&)
  {
    std::cout << "Pingus: Out of memory!" << std::endl;
  }
  catch (const std::exception& a)
  {
    std::cout << "Pingus: Standard exception caught!:\n" << a.what() << std::endl;
  }
  catch (...)
  {
    std::cout << "Pingus: Unknown throw caught!" << std::endl;
  }

  sound::PingusSound::deinit();
  fonts::deinit();
  WorldObjFactory::deinit();
  Resource::deinit();

  return 0;
}


} // namespace pingus

// EOF
