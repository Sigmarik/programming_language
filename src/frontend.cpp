/**
 * @file main.c
 * @author Ilya Kudryashov (kudriashov.it@phystech.edu)
 * @brief Equation differentiator.
 * @version 0.1
 * @date 2022-08-26
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <ctype.h>

#include "lib/util/dbg/debug.h"
#include "lib/util/argparser.h"
#include "lib/alloc_tracker/alloc_tracker.h"
#include "lib/file_helper.h"

#include "utils/config.h"

#include "lib/grammar/bin_tree.h"
#include "lib/grammar/grammar.h"

#include "utils/frontend_utils.h"

#define MAIN

#define MAKE_WRAPPER(name) void* __wrapper_##name[] = {&name}

int main(const int argc, const char** argv) {
    atexit(log_end_program);

    start_local_tracking();

    unsigned int log_threshold = STATUS_REPORTS;
    MAKE_WRAPPER(log_threshold);

    ActionTag line_tags[] = {
        #include "cmd_flags/main_flags.h"
    };
    const int number_of_tags = ARR_SIZE(line_tags);

    parse_args(argc, argv, number_of_tags, line_tags);
    log_init("program_log.html", log_threshold, &errno);
    print_label();

    const char* input_file_name = get_input_file_name(argc, argv, DEFAULT_IN_FILE_NAME);
    const char* output_file_name = get_output_file_name(argc, argv, DEFAULT_OUT_FILE_NAME);

    const char* text = read_whole(input_file_name);
    track_allocation(text, free_variable);

    LexStack lexemes = lexify(text);
    track_allocation(lexemes, LexStack_dtor);

    for (size_t id = 0; id < lexemes.size; ++id) {
        CharAddress address = lexemes.buffer[id].address;
        _log_printf(STATUS_REPORTS, "status", "Lexeme %s at char %d of line %d.\n", 
                    LEXEME_NAMES[lexemes.buffer[id].type], (int)address.index, (int)address.line);
    }

    int caret = 0;
    TreeNode* tree = parse_program(lexemes, &caret);
    track_allocation(tree, TreeNode_dtor);

    TreeNode_dump(tree, ABSOLUTE_IMPORTANCE);

    FILE* output_file = fopen(output_file_name, "w");
    _LOG_FAIL_CHECK_(output_file, "error", ERROR_REPORTS, return_clean(EXIT_FAILURE), &errno, ENOENT);
    track_allocation(output_file, fclose_void);

    TreeNode_export(tree, output_file);

    return_clean(errno == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
}
