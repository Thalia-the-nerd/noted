#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sodium.h>
#include <sys/stat.h>
#include <unistd.h>

char db_path_omg[1024];

void setup_db_path() {
char *home = getenv("HOME");
sprintf(db_path_omg, "%s/.local/share/noted", home);
mkdir(db_path_omg, 0700);
strcat(db_path_omg, "/notes.db");
}

void do_the_thing_version() {
printf("noted version 1.0.0\n");
}

void help_me_pls() {
printf("Usage: noted [options] [command]\n");
printf("Options:\n");
printf("  -v    Show version\n");
printf("  -h    Show this mess\n");
printf("Commands:\n");
printf("  write  Make a new note\n");
printf("  read   Look at your notes\n");
}

int main(int argc, char **argv) {
if (sodium_init() == -1) {
return 1;
}
setup_db_path();
if (argc < 2) {
help_me_pls();
return 0;
}
if (strcmp(argv[1], "-v") == 0) {
do_the_thing_version();
return 0;
}
if (strcmp(argv[1], "-h") == 0) {
help_me_pls();
return 0;
}
if (strcmp(argv[1], "write") == 0) {
printf("writing soon...\n");
} else if (strcmp(argv[1], "read") == 0) {
printf("reading soon...\n");
} else {
help_me_pls();
}
return 0;
}
