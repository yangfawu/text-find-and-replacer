#include "hw6.h"

#define LINE_BUFFER_SIZE 10000

typedef unsigned int uint;

int count_lines(FILE *fp);

void handle_simple(FILE *in_fp, FILE *out_fp, uint start_line, uint end_line, char *search_text, char *replace_text);

void handle_prefix(FILE *in_fp, FILE *out_fp, uint start_line, uint end_line, char *search_text, char *replace_text);

void handle_suffix(FILE *in_fp, FILE *out_fp, uint start_line, uint end_line, char *search_text, char *replace_text);

int main(int argc, char *argv[]) {
    if (argc < 7)
        return MISSING_ARGUMENT;
    
    // disable debug from getopt
    // opterr = 0;

    uint seen_s = 0, seen_r = 0, seen_w = 0, seen_l = 0, l_arg_err = 0;
    char *search_text, *replace_text;
    long start_line, end_line;

    // print command onto log
    // for (int i=0; i<argc; i++)
    //     printf("%s ", argv[i]);
    // printf("\n");

    // flag can be a char or -1
    int flag;
    extern char *optarg;
    while ((flag = getopt(argc, argv, "s:r:wl:")) != -1) {
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

            if (optarg == NULL || *optarg == '-') {
                l_arg_err = 1;
                break;
            }

            char *token = strtok(optarg, ",");
            if (token == NULL || sscanf(token, "%ld", &start_line) != 1) {
                l_arg_err = 1;
                break;
            }
            
            token = strtok(NULL, ",");
            if (token == NULL || !isdigit(*token) || sscanf(token, "%ld", &end_line) != 1) {
                l_arg_err = 1;
                break;
            }
            
            if (
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
            // printf("unknown flag = %d", flag);
            break;
        }
    }

    // make sure we have enough arguments for the files
    // extern int optind;
    // if ((optind) > argc - 2) {
    //     for (int i=0; i<argc; i++)
    //         printf("%s ", argv[i]);
    //     printf("\n");
    //     printf("failed input 1\n");
    //     return INPUT_FILE_MISSING;
    // }
    
    // for (int i=0; i<argc; i++)
    //     printf("%s ", argv[i]);
    // printf("\n");

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
    
    uint wild_prefix = 0;
    if (seen_w) {
        uint search_text_len = strlen(search_text);

        uint suffix = *search_text == '*';
        uint prefix = *(search_text + search_text_len - 1) == '*';
        if (!(prefix ^ suffix))
            return WILDCARD_INVALID;
        
        if (prefix) {
            wild_prefix = 1;

            // we truncate search text to remove asterisk
            *(search_text + search_text_len - 1) = '\0';
        } else {
            // we move pointer to exclude asterisk
            search_text++;
        }
    }

    uint total_lines = count_lines(in_fp);
    if (!seen_l) {
        start_line = 1;
        end_line = total_lines;
    }
    if (end_line > total_lines)
        end_line = total_lines;
    if (start_line > end_line)
        start_line = end_line;
    
    if (seen_w) {
        if (wild_prefix)
            handle_prefix(in_fp, out_fp, start_line, end_line, search_text, replace_text);
        else
            handle_suffix(in_fp, out_fp, start_line, end_line, search_text, replace_text);
    } else handle_simple(in_fp, out_fp, start_line, end_line, search_text, replace_text);

    // close pointers
    fclose(in_fp);
    fclose(out_fp);
    return EXIT_SUCCESS;
}

int count_lines(FILE *fp) {
    int count = 1;
    for (char c=fgetc(fp); c != EOF; c=fgetc(fp)) {
        if (c == '\n')
            count++;
    }
    rewind(fp);
    return count;
}

void handle_simple(FILE *in_fp, FILE *out_fp, uint start_line, uint end_line, char *search_text, char *replace_text) {
    uint search_match_replace = !strcmp(search_text, replace_text);
    uint search_text_len = strlen(search_text);
    uint replace_text_len = strlen(replace_text);

    char a[LINE_BUFFER_SIZE], b[LINE_BUFFER_SIZE];
    char *line_buffer = a;
    char *temp_buffer = b;
    char *line_buffer_p_copy;

    uint curr_line = 1;
    while (fgets(line_buffer, LINE_BUFFER_SIZE, in_fp) != NULL) {
        // copy the pointer
        line_buffer_p_copy = line_buffer;

        // only modify line if curr_line is in range and replace actually replaces
        if (!search_match_replace && curr_line >= start_line && curr_line <= end_line) {
            // latest will store our progress in parsing the line
            char *latest;

            uint dist_from_start = 0;
            char *text_after_occurence;

            // this while loop will replace all search_text in line
            while ((latest = strstr(line_buffer, search_text)) != NULL) {
                // we make a copy of line buffer because we will cut off its end later
                strcpy(temp_buffer, line_buffer);

                // we do pointer subtraction to find index of search_text occurence
                dist_from_start = latest - line_buffer;

                // we cut off the line at the occurence index
                line_buffer[dist_from_start] = '\0';

                // now the line is just all the text before the occurence
                // so we add the replace_text
                strcat(line_buffer, replace_text);

                // then we add the stuff in the old line after the occurence
                text_after_occurence = temp_buffer + dist_from_start + search_text_len;
                strcat(line_buffer, text_after_occurence);

                line_buffer+= dist_from_start + replace_text_len;
            }
        }

        fputs(line_buffer_p_copy, out_fp);
        curr_line++;
    }
}

void handle_prefix(FILE *in_fp, FILE *out_fp, uint start_line, uint end_line, char *search_text, char *replace_text) {
    uint search_text_len = strlen(search_text);
    uint replace_text_len = strlen(replace_text);

    char a[LINE_BUFFER_SIZE], b[LINE_BUFFER_SIZE];
    char *line_buffer = a;
    char *temp_buffer = b;
    char *line_buffer_p_copy;

    uint curr_line = 1;
    while (fgets(line_buffer, LINE_BUFFER_SIZE, in_fp) != NULL) {
        // copy the pointer
        line_buffer_p_copy = line_buffer;

        // only modify line if curr_line is in range and replace actually replaces
        if (curr_line >= start_line && curr_line <= end_line) {
            // latest will store our progress in parsing the line
            char *latest;

            uint dist_from_start = 0;
            char *text_after_occurence;

            // this while loop will replace all search_text in line
            while ((latest = strstr(line_buffer, search_text)) != NULL) {
                // we make a copy of line buffer because we will cut off its end later
                strcpy(temp_buffer, line_buffer);

                // we do pointer subtraction to find index of search_text occurence
                dist_from_start = latest - line_buffer;

                // verify that this is indeed a prefix
                if (dist_from_start == 0 || ispunct(*(latest - 1)) || isspace(*(latest - 1))) {
                    // find out how many more characters we need to remove after occurrence to get whole word
                    uint offset = 0;
                    latest+= search_text_len;
                    while (*latest != '\0' && *latest != '\n' && !ispunct(*latest) && !isspace(*latest)) {
                        offset++;
                        latest++;
                    }
                    
                    // we cut off the line at the occurence index
                    line_buffer[dist_from_start] = '\0';

                    // now the line is just all the text before the occurence
                    // so we add the replace_text
                    strcat(line_buffer, replace_text);

                    // then we add the stuff in the old line after the occurence
                    text_after_occurence = temp_buffer + dist_from_start + search_text_len + offset;
                    strcat(line_buffer, text_after_occurence);

                    line_buffer+= dist_from_start + replace_text_len;
                } else {
                    // if not, then the occurence we found needs to be skipped
                    // we simply move past the occurence
                    line_buffer+= dist_from_start + search_text_len;
                }
            }
        }

        fputs(line_buffer_p_copy, out_fp);
        curr_line++;
    }
}

void handle_suffix(FILE *in_fp, FILE *out_fp, uint start_line, uint end_line, char *search_text, char *replace_text) {
    uint search_text_len = strlen(search_text);
    uint replace_text_len = strlen(replace_text);

    char a[LINE_BUFFER_SIZE], b[LINE_BUFFER_SIZE];
    char *line_buffer = a;
    char *temp_buffer = b;
    char *line_buffer_p_copy;

    uint curr_line = 1;
    while (fgets(line_buffer, LINE_BUFFER_SIZE, in_fp) != NULL) {
        // copy the pointer
        line_buffer_p_copy = line_buffer;

        // only modify line if curr_line is in range and replace actually replaces
        if (curr_line >= start_line && curr_line <= end_line) {
            // latest will store our progress in parsing the line
            char *latest;

            uint occurrence_dist_from_start = 0;
            uint word_dist_from_start = 0;
            char *text_after_occurence;

            // this while loop will replace all search_text in line
            while ((latest = strstr(line_buffer, search_text)) != NULL) {
                // we make a copy of line buffer because we will cut off its end later
                strcpy(temp_buffer, line_buffer);

                // we do pointer subtraction to find index of search_text occurence
                occurrence_dist_from_start = latest - line_buffer;

                // verify that this is indeed a suffix
                if (*(latest + search_text_len) == '\n' || 
                    *(latest + search_text_len) == '\0' || 
                    ispunct(*(latest + search_text_len)) || 
                    isspace(*(latest + search_text_len))
                ) {

                    // search for termination before occurrence
                    word_dist_from_start = occurrence_dist_from_start;
                    while (word_dist_from_start > 0) {
                        if (ispunct(*(latest - 1)) || isspace(*(latest - 1)))
                            break;
                        latest--;
                        word_dist_from_start--;
                    }
                    
                    // we cut off the line at the occurence index
                    line_buffer[word_dist_from_start] = '\0';

                    // now the line is just all the text before the occurence
                    // so we add the replace_text
                    strcat(line_buffer, replace_text);

                    // then we add the stuff in the old line after the occurence
                    text_after_occurence = temp_buffer + occurrence_dist_from_start + search_text_len;
                    strcat(line_buffer, text_after_occurence);

                    line_buffer+= word_dist_from_start + replace_text_len;
                } else {
                    // if not, then the occurence we found needs to be skipped
                    // we simply move past the occurence
                    line_buffer+= occurrence_dist_from_start + search_text_len;
                }
            }
        }

        fputs(line_buffer_p_copy, out_fp);
        curr_line++;
    }
}
