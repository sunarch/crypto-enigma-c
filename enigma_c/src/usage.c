
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "usage.h"
#include "wheels.h"
#include "util_debug.h"
#include "util_validate.h"

unsigned short calculate_index_after_wiring_rule(unsigned short index_before, signed short wiring_rule) {

    // default index_after value, to be overwritten in this function
    // any number >= 26 should produce segmentation fault
    unsigned short index_after = 31;

    signed short index_plus_rule = ((signed short) index_before) + wiring_rule;

    // modulo 26
    if (index_plus_rule >= 0) {
        index_plus_rule = index_plus_rule % 26;
        index_after = (unsigned short) index_plus_rule;
    }
    else {
        index_plus_rule = (signed short) (26 - (abs(index_plus_rule) % 26));
        index_after = (unsigned short) index_plus_rule;
    }

    return index_after;
}

char get_wheel_output(unsigned short wheel_number, unsigned short mode, char input_char) {
    #ifdef DEBUG
        inc_debug_indent();  // to function call level
        print_debug_indent();
        printf("FUNC: char get_wheel_output(...) // unsigned short wheel_number = %u // unsigned short mode = %u // char input_char = %c\n", wheel_number, mode, input_char);
        inc_debug_indent();  // to function result level
    #endif

    // validate wheel_number
    validate_wheel_number_any(wheel_number, get_used_wheel_count());

    char output_char = '?';
    char *p_occurence;
    unsigned short index = 0;
    unsigned short wheel_offset = 0;
    unsigned short offset_index = 0;
    signed short wiring_rule = 0;

    // index from input (input char location in wiring alphabet)
    p_occurence = strchr(ABC_LOW, input_char);
    if(p_occurence == NULL) {
        printf("Failed check with (p_occurence == NULL): (%p == NULL)\n", p_occurence);
        printf("Exiting...\n");
        exit(1);
    }
    index = p_occurence - ABC_LOW;
    if (index >= 26) {
        printf("Failed check with (index >= 26): (%u >= 26)\n", index);
        printf("Exiting...\n");
        exit(1);
    }
    #ifdef DEBUG
        print_debug_indent();
        printf("index of input char in alphabet // %u // <= input_char\n", index);
    #endif

    // wiring rule from offset index
    wheel_offset = get_wheel_offset(wheel_number);
    offset_index = (index + wheel_offset) % 26;
    switch(mode) {
        case WHEEL_MODE_UKW:
            // use the WHEEL_MODE_FRONT method for WHEEL_MODE_UKW
            // front and reverse should be same for UKW
            // no break.
        case WHEEL_MODE_FRONT:
            wiring_rule = get_wheel_wiring_rules_front(wheel_number)[offset_index];
            break;
        case WHEEL_MODE_REVERSE:
            wiring_rule = get_wheel_wiring_rules_reverse(wheel_number)[offset_index];
            break;
        default:
            exit(1);
    }
    #ifdef DEBUG
        print_debug_indent();
        printf("wheel offset for wheel %u // %u\n", wheel_number, wheel_offset);
        print_debug_indent();
        printf("wiring rule from offset index // %d\n", wiring_rule);
    #endif

    // index after wiring rule applied
    //index = (unsigned short) (((signed short) index + wiring_rule) % 26);
    index = calculate_index_after_wiring_rule(index, wiring_rule);
    #ifdef DEBUG
        print_debug_indent();
        printf("index after wiring rule applied // %u // => output_char\n", index);
    #endif

    // output char
    output_char = ABC_LOW[index];

    #ifdef DEBUG
        print_debug_indent();
        printf("RETURN // output_char = %c\n", output_char);
        dec_debug_indent();  // to function call level
        dec_debug_indent();  // to caller level
    #endif

    return output_char;
}

void turn_wheel(unsigned short wheel_number) {
    #ifdef DEBUG
        inc_debug_indent();  // to function call level
        print_debug_indent();
        printf("FUNC: void turn_wheel(...) // unsigned short wheel_number = %u\n", wheel_number);
    #endif

    // validate wheel_number
    validate_wheel_number_any(wheel_number, get_used_wheel_count());

    // move wheel offset by 1
    set_wheel_offset(wheel_number, ((get_wheel_offset(wheel_number) + 1) % 26));

    #ifdef DEBUG
        // CHECK included in setter function
        dec_debug_indent();  // to caller level
    #endif
}


void advance_wheels(void) {
    #ifdef DEBUG
        inc_debug_indent();  // to function call level
        print_debug_indent();
        printf("FUNC: void advance_wheels(void)\n");
    #endif

    unsigned short wheel_count = get_used_wheel_count();
    unsigned short current_wheel_offset = 0;

    for (unsigned short n = 1; n <= wheel_count; ++n) {
        current_wheel_offset = get_wheel_offset(n);

        if (current_wheel_offset < 25) {
            turn_wheel(n);
            break;
        }
        else if (current_wheel_offset == 25) {
            turn_wheel(n);
            continue;
        }
        else {
            exit(1);
        }
    }

    #ifdef DEBUG
        inc_debug_indent();  // to function result level
        current_wheel_offset = get_wheel_offset(0); // use var current_wheel_offset from this function
        print_debug_indent();
        printf("CHECK: get_wheel_offset(...) // wheel_number = 0 // %u // should always be 0 (UKW)\n", current_wheel_offset);
        for (unsigned short n = 1; n <= wheel_count; ++n) {
            current_wheel_offset =  get_wheel_offset(n); // use var current_wheel_offset from this function
            print_debug_indent();
            printf("CHECK: get_wheel_offset(...) // wheel_number = %u // %u\n", n, current_wheel_offset);
        }
        dec_debug_indent();  // to function call level
        dec_debug_indent();  // to caller level
    #endif
}

void print_config_section(signed short *wheel_wiring_rules) {
    // this function prints info, no need for debug messages
    // print the config for one side of one wheel

    signed short current_wheel_wiring_rule = 0;

    for (unsigned short n_wiring_rules_i = 0; n_wiring_rules_i < 26; ++n_wiring_rules_i) {
        printf("             // position: ");
        if (n_wiring_rules_i < 10) {
            // offset one digit numbers
            printf(" "); // print an extra space
        }
        printf("%u (%c)", n_wiring_rules_i, ABC_LOW[n_wiring_rules_i]);

        printf(" // rule: ");
        current_wheel_wiring_rule = wheel_wiring_rules[n_wiring_rules_i];
        if (current_wheel_wiring_rule >= -9) {
            // offset numbers -X , XX and first space of X
            printf(" "); // print an extra space
        }
        if (current_wheel_wiring_rule >= 0 && current_wheel_wiring_rule <= 9) {
            // offset numbers X (second space)
            printf(" "); // print an extra space
        }
        printf("%d ", current_wheel_wiring_rule);
        printf("(%c)\n", ABC_LOW[calculate_index_after_wiring_rule(n_wiring_rules_i, current_wheel_wiring_rule)]);
    }
}

void print_config(void) {
    #ifdef DEBUG
        inc_debug_indent(); // to function outer level
        print_debug_indent();
        printf("START of configuration\n");
        inc_debug_indent(); // to function inner level
    #endif

    unsigned short wheel_count = get_used_wheel_count();
    signed short *wheel_wiring_rules;

    printf("used wheel count: %u\n", wheel_count);

    // wiring rules front
    printf("wiring rules front:\n");
    for (unsigned short n_wheels_i = 1; n_wheels_i <= wheel_count; ++n_wheels_i) {
        printf("    wheel %u : - wiring rules front\n", n_wheels_i);
        wheel_wiring_rules = get_wheel_wiring_rules_front(n_wheels_i);
        print_config_section(wheel_wiring_rules);
    }

    // UKW
    printf("wiring rules UKW:\n");
        printf("    wheel %u (UKW) :\n", UKW_INDEX);
        // front and reverse should be same for UKW, use front
        wheel_wiring_rules = get_wheel_wiring_rules_front(UKW_INDEX);
        print_config_section(wheel_wiring_rules);

    // wiring rules reverse
    printf("wiring rules reverse:\n");
    for (unsigned short n_wheels_i = wheel_count; n_wheels_i >= 1; --n_wheels_i) {
        printf("    wheel %u : - wiring rules reverse\n", n_wheels_i);
        wheel_wiring_rules = get_wheel_wiring_rules_reverse(n_wheels_i);
        print_config_section(wheel_wiring_rules);
    }

    #ifdef DEBUG
        dec_debug_indent(); // to function outer level
        print_debug_indent();
        printf("END of configuration\n");
        dec_debug_indent(); // to caller level
    #endif
}

char *process_message(char *p_input_string) {
    #ifdef DEBUG
        inc_debug_indent(); // to function outer level
        print_debug_indent();
        printf("FUNC: char *process_message(...) // char *p_input_string = %s\n", p_input_string);
        inc_debug_indent(); // to function inner level
    #endif

    unsigned short wheel_count = get_used_wheel_count();

    unsigned long msg_len = strlen(p_input_string);

    char output_string[msg_len];
    char *p_output_string = output_string;

    char current_char = '?';
    bool letter_is_alphabetic = false;

    for (unsigned long n = 0; n < msg_len; ++n) {

        current_char = p_input_string[n];

        // start of character processing
        #ifdef DEBUG
            print_debug_indent();
            printf("%lu./%lu  CHAR to be processed: %c\n", (n + 1), (msg_len), current_char);
            inc_debug_indent(); // to character processing inner level
        #endif

        // preprocess character: uppercase to lowercase, skip special
        unsigned short index;
        char *p_occurence;

        p_occurence = strchr(ABC_LOW, current_char);
        if(p_occurence == NULL) { // character not in uppercase alphabet

            p_occurence = strchr(ABC_UPP, current_char);
            if(p_occurence == NULL) { // character not in uppercase alphabet
                // put character through unchanged
                p_output_string[n] = current_char;
                letter_is_alphabetic = false;
            }
            else { // uppercase character: convert to lowercase
                index = p_occurence - ABC_UPP;
                current_char = ABC_LOW[index];
                letter_is_alphabetic = true;
            }
        }
        else { // lowercase character: proceed normally
            letter_is_alphabetic = true;
        }

        if(letter_is_alphabetic) {

            // first-to-last pass / front pass
            #ifdef DEBUG
                print_debug_indent();
                printf("first-to-last pass / front pass\n");
            #endif
            for (unsigned short n = 1; n <= wheel_count; ++n) {
                current_char = get_wheel_output(n, WHEEL_MODE_FRONT, current_char);
            }

            // UKW pass (same front and reverse)
            #ifdef DEBUG
                print_debug_indent();
                printf("UKW pass (same front and reverse)\n");
            #endif
            current_char = get_wheel_output(UKW_INDEX, WHEEL_MODE_UKW, current_char);

            // last-to-first pass / reverse pass
            #ifdef DEBUG
                print_debug_indent();
                printf("last-to-first pass / reverse pass\n");
            #endif
            for (unsigned short n = wheel_count; n >= 1; --n) {
                current_char = get_wheel_output(n, WHEEL_MODE_REVERSE, current_char);
            }
        }

        // end of character processing
        #ifdef DEBUG
            dec_debug_indent(); // to function inner level
            print_debug_indent();
            printf("%lu./%lu  CHAR after processing: %c\n", (n + 1), (msg_len), current_char);
        #endif

        p_output_string[n] = current_char;

        advance_wheels();

    }

    #ifdef DEBUG
        print_debug_indent();
        printf("RETURN // p_output_string = %s\n", p_output_string);
        dec_debug_indent(); // to function outer level
        dec_debug_indent(); // to caller level
    #endif

    return p_output_string;
}
