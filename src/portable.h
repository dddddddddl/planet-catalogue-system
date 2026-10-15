#ifndef PLANET_CATALOGUE_PORTABLE_H
#define PLANET_CATALOGUE_PORTABLE_H

#include <errno.h>
#include <stdarg.h>

/* Small C17 replacements for the Microsoft-specific functions in the source. */
static int catalogueOpen(FILE **stream, const char *path, const char *mode) {
    *stream = fopen(path, mode);
    return *stream != NULL ? 0 : errno;
}

static int catalogueCopy(char *destination, size_t size, const char *source) {
    if (strlen(source) >= size) {
        if (size > 0) destination[0] = '\0';
        return ERANGE;
    }
    memcpy(destination, source, strlen(source) + 1);
    return 0;
}

static int catalogueLocalTime(struct tm *destination, const time_t *source) {
    struct tm *value = localtime(source);
    if (value == NULL) return EINVAL;
    *destination = *value;
    return 0;
}

static void discardInputLine(void) {
    int character;
    while ((character = getchar()) != '\n' && character != EOF) {}
}

/* All interactive conversions in this program read exactly one value. */
static void readInput(const char *format, ...) {
    int converted;
    do {
        va_list arguments;
        va_start(arguments, format);
        converted = vscanf(format, arguments);
        va_end(arguments);
        if (converted == EOF) {
            puts("\nInput ended.");
            exit(EXIT_SUCCESS);
        }
        if (converted != 1) {
            discardInputLine();
            printf("Invalid input. Try again: ");
        }
    } while (converted != 1);
}

static void readInputLine(char *buffer, size_t size) {
    for (;;) {
        if (fgets(buffer, (int)size, stdin) == NULL) {
            puts("\nInput ended.");
            exit(EXIT_SUCCESS);
        }
        if (strchr(buffer, '\n') == NULL && !feof(stdin)) {
            discardInputLine();
            printf("Input is too long. Try again: ");
            continue;
        }
        if (buffer[0] == '\n' || strchr(buffer, '\t') != NULL) {
            printf("Enter nonempty text without tabs: ");
            continue;
        }
        return;
    }
}

#endif
