/**
 * @file common_flags.h
 * @author Kudryashov Ilya (kudriashov.it@phystech.edu)
 * @brief Flags present in all programs.
 * @version 0.1
 * @date 2022-10-23
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#define GET_WRAPPER(name) __wrapper_##name

{ {'O', "owl"}, { {}, 0, print_owl },
    "print 10 owls to the screen." },

{ {'I', ""}, { GET_WRAPPER(log_threshold), 1, edit_int },
    "set log threshold to the specified number.\n"
    "\tDoes not check if integer was specified." },
