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

#include "utils/backend_utils.h"

#define MAIN

#define MAKE_WRAPPER(name) void* __wrapper_##name[] = {&name}

int main(const int argc, const char** argv) {
    atexit(log_end_program);

    start_local_tracking();

    unsigned int log_threshold = STATUS_REPORTS;
    MAKE_WRAPPER(log_threshold);

    ActionTag line_tags[] = {
        #include "cmd_flags/backend_flags.h"
    };
    const int number_of_tags = ARR_SIZE(line_tags);

    parse_args(argc, argv, number_of_tags, line_tags);
    log_init("program_log.html", log_threshold, &errno);
    print_label();

    const char* input_file_name = get_input_file_name(argc, argv, DEFAULT_TREE_FILE_NAME);
    const char* output_file_name = get_output_file_name(argc, argv, DEFAULT_BINARY_FILE_NAME);

    const char* text = read_whole(input_file_name);
    track_allocation(text, free_variable);

    LexStack lexemes = lexify(text);
    track_allocation(lexemes, LexStack_dtor);

    for (size_t id = 0; id < lexemes.size; ++id) {
        _log_printf(STATUS_REPORTS, "status", "Lexeme %s detected at char %d of line %d.\n",
                    LEXEME_NAMES[(int)lexemes.buffer[id].type], 
                    (int)lexemes.buffer[id].address.index, (int)lexemes.buffer[id].address.line);
    }

    int caret = 0;
    TreeNode* tree = parse_program_tree(lexemes, &caret);
    track_allocation(tree, TreeNode_dtor);

    TreeNode_dump(tree, ABSOLUTE_IMPORTANCE);

    FILE* output_file = fopen(output_file_name, "w");
    _LOG_FAIL_CHECK_(output_file, "error", ERROR_REPORTS, return_clean(EXIT_FAILURE), &errno, ENOENT);
    track_allocation(output_file, fclose_void);

    fputs("PUSH 1\nMOVE RAX\n", output_file);

    char* stdlib_text = read_whole(STDLIB_NAME);
    if (stdlib_text) fwrite(stdlib_text, sizeof(*stdlib_text), strlen(stdlib_text), output_file);
    free(stdlib_text);

    NameStack name_stack = NameStack_new();
    int label_count = 0;
    TreeNode_compile(&name_stack, tree, output_file, 0, &label_count);
    NameStack_dtor(&name_stack);

    fprintf(output_file, "\nEND\n");

    return_clean(errno == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
}
