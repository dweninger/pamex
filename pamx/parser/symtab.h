
#define NHASH 9997

enum type{LEVEL, LABEL, USER_NAME, FILE_NAME};

// Classification marking
typedef struct levelRef {
	int placement; // Placement num in hierarchy
	union utype * next;
	int lineno;
} levelRef;

// Control marking
typedef struct labelRef {
	union utype * next;
	int lineno;	
} labelRef;

typedef struct userRef {
	union utype * next;
	int lineno;
} userRef;

typedef struct fileRef {
	union utype * next;
	int lineno;
} fileRef;

typedef union utype {
	levelRef * level;
	labelRef * label;
	userRef * user;
	fileRef * file;
} utype;

typedef struct symbol {
	int new_sym;
	char * name;
	utype * ref_list;
	enum type type;
} symbol;

symbol * lookup(char *, enum type);
void add_level(int, int, char *);
void add_label(int, char *);
void add_user(int, char *);
void add_file(int, char *);
char * format_level_data(symbol * sym);
