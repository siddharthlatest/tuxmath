/*
  setup.c

  For TuxMath
  Contains some globals (screen surface, images, some option flags, etc.)
  as well as the function to load data files (images, sounds, music)
  and display a "Loading..." screen.

  by Bill Kendrick
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/

  Modified by David Bruce
  dbruce@tampabay.rr.com

  Part of "Tux4Kids" Project
  http://www.tux4kids.net/
  Subversion repository:
  https://svn.tux4kids.net/tuxmath

 
  August 26, 2001 - August 8, 2006.
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* FIXME maybe unistd.h not needed, even less sure about portability */
//#include <unistd.h>

#include <SDL.h>

#ifndef NOSOUND
#include <SDL_mixer.h>
#endif

#include <SDL_image.h>


#include "tuxmath.h"
#include "mathcards.h"
#include "setup.h"
#include "fileops.h"
#include "game.h"



/* Global data used in setup.c:              */
/* (These are now 'extern'd in "tuxmath.h") */
SDL_Surface* screen;
SDL_Surface* images[NUM_IMAGES];

#ifndef NOSOUND
Mix_Chunk* sounds[NUM_SOUNDS];
Mix_Music* musics[NUM_MUSICS];
#endif


/* Local function prototypes: */
void initialize_options(void);
void handle_command_args(int argc, char* argv[]);
void initialize_SDL(void);
void load_data_files(void);

int initialize_game_options(void);
void seticon(void);
void usage(int err, char * cmd);

void cleanup_memory(void);



/* --- Set-up function - now in four easier-to-digest courses! --- */

void setup(int argc, char * argv[])
{
  /* initialize settings and read in config files: */
  initialize_options();
  /* Command-line code now in own function: */
  handle_command_args(argc, argv);
  /* SDL setup in own function:*/
  initialize_SDL();
  /* Read image and sound files: */
  load_data_files();
}




/* Set up mathcards with default values for math question options, */
/* set up game_options with defaults for general game options,     */
/* then read in global config file, followed if desired by user's  */
/* own config file:                                                */
void initialize_options(void)
{
  /* Initialize MathCards backend for math questions: */
  if (!MC_Initialize())
  {
    printf("\nUnable to initialize MathCards\n");
    fprintf(stderr, "\nUnable to initialize MathCards\n");
    exit(1);
  }

  /* initialize game_options struct with defaults DSB */
  game_options = malloc(sizeof(game_option_type));
  if (!initialize_game_options())
  {
    printf("\nUnable to initialize game_options\n");
    fprintf(stderr, "\nUnable to initialize game_options\n");
    cleanup_on_error();
    exit(1);
  }

  /* Now that MathCards and game_options initialized using  */
  /* hard-coded defaults, read options from disk and mofify */
  /* as needed. First read in installation-wide settings:   */
  if (!read_global_config_file())
  {
    fprintf(stderr, "\nCould not find global config file.\n");
    /* can still proceed using hard-coded defaults.         */
  }

  /* Now read in user-specific settings, if desired.  By    */
  /* default, this restores settings from the player's last */
  /* game:                                                  */
  if (game_options->per_user_config)
  {
    if (!read_user_config_file())
    {
      fprintf(stderr, "\nCould not find user's config file.\n");
      /* can still proceed using hard-coded defaults.         */
    }
  }

}




/* Handle any arguments passed from command line */
void handle_command_args(int argc, char* argv[])
{
  int i, j, found;

  for (i = 1; i < argc; i++)
  {
    if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0)
    {
      /* Display help message: */

      printf("\nTux, of Math Command\n\n"
        "Use the number keys on the keyboard to answer math equations.\n"
        "If you don't answer a comet's math equation before it hits\n"
        "one of your cities, the city's shields will be destroyed.\n"
        "If that city is hit by another comet, it is destroyed completely.\n"
	"When you lose all of your cities, the game ends.\n\n");

      printf("Note: all settings are now stored in a config file named 'options' in\n"
             "a hidden directory named './tuxmath' within the user's home directory.\n"
             "The file consists of simple name/value pairs. It is much easier\n"
             "to edit this file to set game parameters than to use the command-line\n"
             "arguments listed below. Also, many options are not selectable from the\n"
             "command line. The config file contains extensive comments detailing how\n"
             "to configure the behavior of Tuxmath.\n\n");

      printf("Run the game with:\n"
        "--optionfile filename  - read config settings from named file. The locations\n"
        "                         searched for a file with a matching name are the\n"
        "                         current working directory, the absolute path of the\n"
        "                         filename, tuxmath's missions directory, the user's\n"
        "                         tuxmath directory, and the user's home.\n"
        "--playthroughlist      - to ask each question only once, allowing player to\n"
        "                         win game if all questions successfully answered\n"

        "--answersfirst   - to ask questions in format: ? + num2 = num3\n"
        "                   instead of default format: num1 + num2 = ?\n"
        "--answersmiddle  - to ask questions in format: num1 + ? = num3\n"
        "                   instead of default format: num1 + num2 = ?\n"
        "--nosound        - to disable sound/music\n"
	"--nobackground   - to disable background photos (for slower systems)\n"
	"--fullscreen     - to run in fullscreen, if possible (vs. windowed)\n"
        "--keypad         - to enable the on-sceen numeric keypad\n"
	"--demo           - to run the program as a cycling demonstration\n"
	"--speed S        - set initial speed of the game\n"
	"                   (S may be fractional, default is 1.0)\n"
        "--allownegatives - to allow answers to be less than zero\n"
	"--operator OP    - to automatically play with particular operators\n"
	"                   OP may be one of:\n");

      for (j = 0; j < NUM_OPERS; j++)
        printf("                   \"%s\"\n", oper_opts[j]);

      printf("            or:\n");
      
      for (j = 0; j < NUM_OPERS; j++)
        printf("                   \"%s\"\n", oper_alt_opts[j]);

      printf("\n");
    
      cleanup_on_error();
      exit(0);
    }
    else if (strcmp(argv[i], "--copyright") == 0 ||
	     strcmp(argv[i], "-c") == 0)
    {
      printf(
	"\n\"Tux, of Math Command\" version " VERSION ", Copyright (C) 2001 Bill Kendrick\n"
        "This program is free software; you can redistribute it and/or\n"
        "modify it under the terms of the GNU General Public License\n"
        "as published by the Free Software Foundation.  See COPYING.txt\n"
	"\n"
	"This program is distributed in the hope that it will be useful,\n"
	"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
	"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"
	"\n");

      cleanup_on_error();
      exit(0);
    }
    else if (strcmp(argv[i], "--usage") == 0 ||
	     strcmp(argv[i], "-u") == 0)
    {
      /* Display (happy) usage: */
	    
      usage(0, argv[0]);
    }
    /* TODO implement --optionfile filename */
    else if (0 == strcmp(argv[i], "--optionfile"))
    {
      if (i >= argc - 1)
      {
	fprintf(stderr, "%s option requires an argument (filename)\n", argv[i]);
	usage(1, argv[0]);
      }
      else /* try to read file named in following arg: */
      {
        if (!read_named_config_file(argv[i + 1])) 
        {
          fprintf(stderr, "Could not read config file: %s\n", argv[i + 1]);
        }
      }
      i++; /* so program doesn't barf on next arg (the filename) */
    }
    else if (strcmp(argv[i], "--fullscreen") == 0 ||
	     strcmp(argv[i], "-f") == 0)
    {
      game_options->fullscreen = 1;
    }
    else if (strcmp(argv[i], "--nosound") == 0 ||
	     strcmp(argv[i], "-s") == 0 ||
	     strcmp(argv[i], "--quiet") == 0 ||
	     strcmp(argv[i], "-q") == 0)
    {
      game_options->use_sound = 0;
    }
    else if (strcmp(argv[i], "--version") == 0 ||
	     strcmp(argv[i], "-v") == 0)
    {
      printf("Tux, of Math Command (\"tuxmath\")\n"
	     "Version " VERSION "\n");
      cleanup_on_error();
      exit(0);
    }
    else if (strcmp(argv[i], "--nobackground") == 0 ||
             strcmp(argv[i], "-b") == 0)
    {
      game_options->use_bkgd = 0;
    }
    else if (strcmp(argv[i], "--demo") == 0 ||
	     strcmp(argv[i], "-d") == 0)
    {
      game_options->demo_mode = 1;
    }
    else if (strcmp(argv[i], "--keypad") == 0 ||
             strcmp(argv[i], "-k") == 0)
    {
      game_options->use_keypad = 1;
    }
    else if (strcmp(argv[i], "--allownegatives") == 0 ||
             strcmp(argv[i], "-n") == 0)
    {
      MC_SetAllowNegatives(1);
    }
    else if (strcmp(argv[i], "--playthroughlist") == 0 ||
             strcmp(argv[i], "-l") == 0)
    {
      MC_SetPlayThroughList(1);
    }
    else if (strcmp(argv[i], "--answersfirst") == 0)
    {
      MC_SetFormatAnswerLast(0);
      MC_SetFormatAnswerFirst(1);
      MC_SetFormatAnswerMiddle(0);
    }
    else if (strcmp(argv[i], "--answersmiddle") == 0)
    {
      MC_SetFormatAnswerLast(0);
      MC_SetFormatAnswerFirst(0);
      MC_SetFormatAnswerMiddle(1);
    }
    else if (strcmp(argv[i], "--speed") == 0 ||
	     strcmp(argv[i], "-s") == 0)
    {
      if (i >= argc - 1)
      {
	fprintf(stderr, "%s option requires an argument\n", argv[i]);
	usage(1, argv[0]);
      }

      game_options->speed = strtod(argv[i + 1], (char **) NULL);

      if (game_options->speed <= 0)
      {
	fprintf(stderr, "Invalided argument to %s: %s\n",
		argv[i], argv[i + 1]);
	usage(1, argv[0]);
      }

      i++;
    }

    /* FIXME this does not currently work */
    else if (strcmp(argv[i], "--operator") == 0 ||
	     strcmp(argv[i], "-o") == 0)
    {
      if (i >= argc - 1)
      {
	fprintf(stderr, "%s option requires an argument\n", argv[i]);
	usage(1, argv[0]);
      }
     
      found = 0; 
      for (j = 0; j < NUM_OPERS; j++)
      {
	if (strcmp(argv[i + 1], oper_opts[j]) == 0 ||
	    strcmp(argv[i + 1], oper_alt_opts[j]) == 0)
	{
          found = 1;
          opers[j] = 1;
	}
      }

      if (found == 0)
      {
	fprintf(stderr, "Unrecognized operator %s\n", argv[i + 1]);
	usage(1, argv[0]);
      }

      i++;
    }
    else
    /* TODO try to match unrecognized strings to config file names */
    {
      /* Display 'made' usage: */

      fprintf(stderr, "Unknown option: %s\n", argv[i]);
      usage(1, argv[0]);
    }
  }/* end of command-line args */


  if (game_options->demo_mode && game_options->use_keypad)
  {
    fprintf(stderr, "No use for keypad in demo mode!\n");
    game_options->use_keypad = 0;
  }
}




void initialize_SDL(void)
{
  /* Init SDL Video: */
  screen = NULL;

  if (SDL_Init(SDL_INIT_VIDEO) < 0)
  {
    fprintf(stderr,
           "\nError: I could not initialize video!\n"
	   "The Simple DirectMedia error that occured was:\n"
	   "%s\n\n", SDL_GetError());
    cleanup_on_error();
    exit(1);
  }


  #ifndef NOSOUND
  /* Init SDL Audio: */
  if (game_options->use_sound)
  { 
    if (SDL_Init(SDL_INIT_AUDIO) < 0)
    {
      fprintf(stderr,
            "\nWarning: I could not initialize audio!\n"
            "The Simple DirectMedia error that occured was:\n"
            "%s\n\n", SDL_GetError());
      game_options->sound_available = 0;
    }
  }

 
  if (game_options->use_sound)
  {
    if (Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 2048) < 0)
    {
      fprintf(stderr,
	      "\nWarning: I could not set up audio for 44100 Hz "
	      "16-bit stereo.\n"
	      "The Simple DirectMedia error that occured was:\n"
	      "%s\n\n", SDL_GetError());
      game_options->sound_available = 0;
    }
  }
  #endif


  if (game_options->fullscreen)
  {
    screen = SDL_SetVideoMode(640, 480, 16, SDL_FULLSCREEN | SDL_HWSURFACE);

    if (screen == NULL)
    {
      fprintf(stderr,
              "\nWarning: I could not open the display in fullscreen mode.\n"
	      "The Simple DirectMedia error that occured was:\n"
	      "%s\n\n", SDL_GetError());
      game_options->fullscreen = 0;
    }
  }

  if (!game_options->fullscreen)
  {
    screen = SDL_SetVideoMode(640, 480, 16, SDL_HWSURFACE);
  }

  if (screen == NULL)
  {
    fprintf(stderr,
            "\nError: I could not open the display.\n"
	    "The Simple DirectMedia error that occured was:\n"
	    "%s\n\n", SDL_GetError());
    cleanup_on_error();
    exit(1);
  }

  seticon();

  SDL_WM_SetCaption("Tux, of Math Command", "TuxMath");
}



void load_data_files(void)
{
  if (!load_image_data())
  {
    fprintf(stderr, "\nCould not load image file - exiting!\n");
    cleanup_on_error();
    exit(1);
  }
  
  if (!load_sound_data())
  {
    fprintf(stderr, "\nCould not load sound file - attempting to proceed without sound.\n");
    game_options->sound_available = 0;
  }

  
  /* FIXME what does this do? */
//   for (i = images[IMG_LOADING]->h; i >= 0; i = i - 10)
//     {
//       SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
// 
//       dest.x = (screen->w - images[IMG_TITLE]->w) / 2;
//       dest.y = i;
//       dest.w = images[IMG_TITLE]->w;
//       dest.h = images[IMG_TITLE]->h;
//       
//       SDL_BlitSurface(images[IMG_TITLE], NULL, screen, &dest);
//       SDL_Flip(screen);
//       SDL_Delay(10);
//     }
}

/* save options and free heap */
/* use for successful exit */
void cleanup(void)
{
  write_user_config_file();
  cleanup_memory();
}



/* save options and free heap */
/* use for fail exit */
void cleanup_on_error(void)
{
  cleanup_memory();
}



/* free any heap memory used during game DSB */
void cleanup_memory(void)
{
  SDL_Quit();
  if (game_options)
  {
    free(game_options);
    game_options = 0;
  }
  /* frees any heap used by MathCards: */
  MC_EndGame();
}



int initialize_game_options(void)
{
  /* bail out if no struct */
  if (!game_options)
    return 0;

  /* set general game options */
  game_options->per_user_config = DEFAULT_PER_USER_CONFIG;
  game_options->use_sound = DEFAULT_USE_SOUND;
  game_options->fullscreen = DEFAULT_FULLSCREEN;
  game_options->use_bkgd = DEFAULT_USE_BKGD;
  game_options->demo_mode = DEFAULT_DEMO_MODE;
  game_options->oper_override = DEFAULT_OPER_OVERRIDE;
  game_options->use_keypad = DEFAULT_USE_KEYPAD;
  game_options->speed = DEFAULT_SPEED;
  game_options->allow_speedup = DEFAULT_ALLOW_SPEEDUP;
  game_options->speedup_factor = DEFAULT_SPEEDUP_FACTOR;
  game_options->max_speed = DEFAULT_MAX_SPEED;
  game_options->slow_after_wrong = DEFAULT_SLOW_AFTER_WRONG;
  game_options->starting_comets = DEFAULT_STARTING_COMETS;
  game_options->extra_comets_per_wave = DEFAULT_EXTRA_COMETS_PER_WAVE;
  game_options->max_comets = DEFAULT_MAX_COMETS;
  game_options->save_summary = DEFAULT_SAVE_SUMMARY;
  game_options->sound_available = DEFAULT_SOUND_AVAILABLE;
  game_options->use_feedback = DEFAULT_USE_FEEDBACK;
  game_options->danger_level = DEFAULT_DANGER_LEVEL;
  game_options->danger_level_speedup = DEFAULT_DANGER_LEVEL_SPEEDUP;
  game_options->danger_level_max = DEFAULT_DANGER_LEVEL_MAX;
  game_options->city_expl_handicap = DEFAULT_CITY_EXPL_HANDICAP;

  game_options->num_cities = DEFAULT_NUM_CITIES;   /* MUST BE AN EVEN NUMBER! */
  game_options->num_bkgds = DEFAULT_NUM_BKGDS;
  game_options->max_city_colors = DEFAULT_MAX_CITY_COLORS;

  #ifdef TUXMATH_DEBUG
  print_game_options(stdout, 0);
  #endif

  return 1;
}

/* Returns true if only if the player wants to use sound */
/* and the sound system is actually available:           */
int opts_using_sound(void)
{
  return (game_options->use_sound && game_options->sound_available);
}



/* Set the application's icon: */

void seticon(void)
{
  int masklen;
  Uint8 * mask;
  SDL_Surface * icon;
  
  
  /* Load icon into a surface: */
  
  icon = IMG_Load(DATA_PREFIX "/images/icon.png");
  if (icon == NULL)
    {
      fprintf(stderr,
              "\nWarning: I could not load the icon image: %s\n"
              "The Simple DirectMedia error that occured was:\n"
              "%s\n\n", DATA_PREFIX "images/icon.png", SDL_GetError());
      return;
    }
  
  
  /* Create mask: */
  
  masklen = (((icon -> w) + 7) / 8) * (icon -> h);
  mask = malloc(masklen * sizeof(Uint8));
  memset(mask, 0xFF, masklen);
  
  
  /* Set icon: */
  
  SDL_WM_SetIcon(icon, mask);
  
  
  /* Free icon surface & mask: */
  
  free(mask);
  SDL_FreeSurface(icon);
  
  
  /* Seed random-number generator: */
  
  srand(SDL_GetTicks());
}

void usage(int err, char * cmd)
{
  FILE * f;

  if (err == 0)
    f = stdout;
  else
    f = stderr;

  fprintf(f,
   "\nUsage: %s {--help | --usage | --copyright}\n"
   "          [--optionfile <filename>]\n"
   "          [--playthroughlist] [--answersfirst] [--answersmiddle]\n"
   "       %s [--fullscreen] [--nosound] [--nobackground]\n"
   "          [--demo] [--keypad] [--allownegatives]\n"
   "          [--operator {add | subtract | multiply | divide} ...]\n"
   "          [--speed <val>]\n"
    "\n", cmd, cmd);

  exit (err);
}
