
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
	int newSym;
	char * name;
	utype * reflist;
	enum type type;
} symbol;

//typedef struct levelplacements {
//	int size;
//	symbol * levelplacementslist[1024];
//} levelplacements;

symbol * lookup(char *, enum type);
void addlevel(int, int, char *);
void addlabel(int, char *);
void adduser(int, char *);
void addfile(int, char *);
char * leveltojson(symbol * sym);
char * labeltojson(symbol * sym);
char * leveldataformat(symbol * sym);
