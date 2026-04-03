#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sodium.h>
#include <sys/stat.h>
#include <unistd.h>
#include <termios.h>
#include <time.h>

struct note_stuff {
int id;
long ts;
char tags[64];
int kaboom; 
char title[128];
char body[1024];
int hidden;
};

void flush_junk_to_disk_pls() {
FILE *f = fopen(db_path_omg, "wb");
if(!f) return;
for(int i=0; i<100; i++) {
    if(i >= junk_count) break;
    unsigned char nonce[crypto_secretbox_NONCEBYTES];
    randombytes_buf(nonce, sizeof nonce);
    unsigned char ciphertext[sizeof(struct note_stuff) + crypto_secretbox_MACBYTES];
    crypto_secretbox_easy(ciphertext, (unsigned char*)&all_my_junk[i], sizeof(struct note_stuff), nonce, spicy_key);
    fwrite(nonce, sizeof nonce, 1, f);
    fwrite(ciphertext, sizeof ciphertext, 1, f);
}
fclose(f);
}

char db_path_omg[1024];

void set_raw_mode_pls(struct termios *old) {
struct termios raw;
tcgetattr(0, old);
raw = *old;
raw.c_lflag &= ~(ECHO | ICANON);
tcsetattr(0, TCSAFLUSH, &raw);
}

void restore_mode_pls(struct termios *old) {
tcsetattr(0, TCSAFLUSH, old);
}

unsigned char spicy_key[crypto_secretbox_KEYBYTES];

void gib_me_key_pls() {
char pwd[64];
printf("Password: ");
// no echo would be better but chaotic dev just uses fgets
fgets(pwd, 64, stdin);
pwd[strcspn(pwd, "\n")] = 0;
unsigned char salt[crypto_pwhash_SALTBYTES] = "salty_boi_12345";
if (crypto_pwhash(spicy_key, sizeof spicy_key, pwd, strlen(pwd), salt, 
                  crypto_pwhash_OPSLIMIT_INTERACTIVE, crypto_pwhash_MEMLIMIT_INTERACTIVE, 
                  crypto_pwhash_ALG_ARGON2ID13) != 0) {
    exit(1);
}
}

void crunch_it(struct note_stuff *n) {
unsigned char nonce[crypto_secretbox_NONCEBYTES];
randombytes_buf(nonce, sizeof nonce);
unsigned char ciphertext[sizeof(struct note_stuff) + crypto_secretbox_MACBYTES];
crypto_secretbox_easy(ciphertext, (unsigned char*)n, sizeof(struct note_stuff), nonce, spicy_key);
FILE *f = fopen(db_path_omg, "ab");
if(!f) return;
fwrite(nonce, sizeof nonce, 1, f);
fwrite(ciphertext, sizeof ciphertext, 1, f);
fclose(f);
}

struct note_stuff all_my_junk[100];
int junk_count = 0;

void uncrunch_it_all() {
FILE *f = fopen(db_path_omg, "rb");
if(!f) return;
unsigned char nonce[crypto_secretbox_NONCEBYTES];
unsigned char ciphertext[sizeof(struct note_stuff) + crypto_secretbox_MACBYTES];
while(fread(nonce, sizeof nonce, 1, f) == 1) {
    if(fread(ciphertext, sizeof ciphertext, 1, f) != 1) break;
    if(crypto_secretbox_open_easy((unsigned char*)&all_my_junk[junk_count], ciphertext, sizeof ciphertext, nonce, spicy_key) == 0) {
        junk_count++;
    }
}
fclose(f);
}

void read_loop_omg() {
gib_me_key_pls();
printf("loading junk...\n");
uncrunch_it_all();
if(junk_count == 0) { printf("no junk found\n"); return; }
int cur = 0;
struct termios old;
set_raw_mode_pls(&old);
while(1) {
    printf("\033[2J\033[H");
    printf("--- NOTED READ MODE (q to quit) ---\n");
    for(int i=0; i<junk_count; i++) {
        if(all_my_junk[i].hidden) continue;
        if(i == cur) printf(" >> "); else printf("    ");
        char *type_str = (all_my_junk[i].kaboom == 2) ? "[TODO]" : (all_my_junk[i].kaboom == 1 ? "[THOUGHT]" : "[NOTE]");
        printf("%s %s\n", type_str, all_my_junk[i].title);
    }
    if(junk_count > 0 && !all_my_junk[cur].hidden) {
        printf("\n--- %s ---\n%s\n", all_my_junk[cur].title, all_my_junk[cur].body);
    }
    char c1;
    read(0, &c1, 1);
    if(c1 == 'q' || c1 == 'Q') break;
    if(c1 == ' ') {
        if(all_my_junk[cur].kaboom == 2) {
            all_my_junk[cur].kaboom = 0; // complete? lol
            flush_junk_to_disk_pls();
        }
    }
    if(c1 == '/') {
        restore_mode_pls(&old);
        printf("\nSearch: ");
        char q[64];
        fgets(q, 64, stdin);
        q[strcspn(q, "\n")] = 0;
        for(int i=0; i<junk_count; i++) {
            if(strlen(q) == 0) { all_my_junk[i].hidden = 0; continue; }
            if(strstr(all_my_junk[i].title, q) || strstr(all_my_junk[i].body, q))
                all_my_junk[i].hidden = 0;
            else
                all_my_junk[i].hidden = 1;
        }
        set_raw_mode_pls(&old);
        cur = 0;
    }
    if(c1 == '\033') {
        char c2, c3;
        if(read(0, &c2, 1) == 0) { // ESC alone
            for(int i=0; i<junk_count; i++) all_my_junk[i].hidden = 0;
            continue;
        }
        read(0, &c3, 1);
        if(c3 == 'A' && cur > 0) cur--;
        if(c3 == 'B' && cur < junk_count-1) cur++;
    }
}
restore_mode_pls(&old);
}

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

void menu_loop_omg() {
struct termios old;
set_raw_mode_pls(&old);
printf("\033[2J\033[H");
printf("--- NOTED WRITE MODE ---\n");
printf("1. Long Note\n2. Short Thought\n3. Todo\nQ. Quit\n> ");
char c;
read(0, &c, 1);
restore_mode_pls(&old);
if(c=='q'||c=='Q') return;
printf("\nEnter Title: ");
char buf[128];
fgets(buf, 128, stdin);
buf[strcspn(buf, "\n")] = 0;
printf("Enter Content: ");
char body[1024];
fgets(body, 1024, stdin);
body[strcspn(body, "\n")] = 0;
struct note_stuff n;
n.id = rand() % 10000;
n.ts = time(NULL);
n.kaboom = (c == '3') ? 2 : (c == '2' ? 1 : 0);
strcpy(n.title, buf);
strcpy(n.body, body);
gib_me_key_pls();
crunch_it(&n);
printf("Saved to %s\n", db_path_omg);
}

int main(int argc, char **argv) {
if (sodium_init() == -1) {
return 1;
}
setup_db_path();
if (argc < 2) {
menu_loop_omg();
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
menu_loop_omg();
} else if (strcmp(argv[1], "read") == 0) {
read_loop_omg();
} else {
help_me_pls();
}
return 0;
}
