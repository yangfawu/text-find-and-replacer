#include "hw6.h"

typedef unsigned int uint;

int main(int argc, char *argv[]) {
    if (argc < 7)
        return MISSING_ARGUMENT;
    
    // disable debug from getopt
    // opterr = 0;

    uint seen_s = 0, seen_r = 0, seen_w = 0, seen_l = 0, input_file_err = 0, output_file_err = 0, l_arg_err = 0, wild_err = 0;
    char *search_text, *replace_text;
    long start_line, end_line;

    for (uint i=0; i<argc; i++)
        printf("%s ", argv[i]);
    printf("\n");

    // flag can be a char or -1
    int flag;
    extern char *optarg;
    while ((flag = getopt(argc, argv, "s:r:wl:")) != -1) {
        // printf("flag=%s\n", flag);
        switch (flag) {
        case 's':
            if (seen_s)
                return DUPLICATE_ARGUMENT;
            seen_s = 1;
            
            search_text = optarg;
            break;
        case 'r':
            if (seen_r)
                return DUPLICATE_ARGUMENT;
            seen_r = 1;
            
            replace_text = optarg;
            break;
        case 'w':
            if (seen_w)
                return DUPLICATE_ARGUMENT;
            seen_w = 1;
            /* code */
            break;
        case 'l':
            if (seen_l)
                return DUPLICATE_ARGUMENT;
            seen_l = 1;
            
            if (optarg == NULL ||
                *optarg == '-1' || 
                sscanf(optarg, "%ld,%ld", &start_line, &end_line) != 2 ||
                start_line < 0 || 
                end_line < 0 || 
                start_line > end_line
            ) {
                l_arg_err = 1;
                break;   
            }
            break;
        default:
            // unreachable thanks to getopt
            printf("unknown flag = %d", flag);
            break;
        }
    }

    // make sure we have enough arguments for the files
    extern int optind;
    if ((optind + 1) >= argc)
        return INPUT_FILE_MISSING;

    FILE *in_fp, *out_fp;
    if ((in_fp = fopen(argv[argc - 2], "r")) == NULL)
        return INPUT_FILE_MISSING;
    if ((out_fp = fopen(argv[argc - 1], "w")) == NULL)
        return OUTPUT_FILE_UNWRITABLE;
    
    if (!seen_s || search_text == NULL || *search_text == '-')
        return S_ARGUMENT_MISSING;
    
    if (!seen_r || replace_text == NULL || *replace_text == '-')
        return R_ARGUMENT_MISSING;

    if (seen_l && l_arg_err)
        return L_ARGUMENT_INVALID;
    
    // TODO: handle WILDCARD_INVALID

    fclose(in_fp);
    fclose(out_fp);
    return EXIT_SUCCESS;
}
