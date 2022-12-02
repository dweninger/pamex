
#define NHASH 9997

enum type{LEVEL, LABEL, USER_NAME, FILE_NAME};

struct Symbol {
	char * name;
	void * reflist;
	int type;
} Symbol;

struct LevelRef {
	int placement; // Placement num in hierarchy
	struct LevelRef * next;
	int lineno;
} LevelRef;

struct LabelRef {
	struct LabelRef * next;
	int lineno;	
} LabelRef;

struct UserRef {
	struct UserRef * next;
	int lineno;
} UserRef;

struct FileRef {
	struct FileRef * next;
	int lineno;
} FileRef;

struct Symbol * levelplacements[1024];

struct Symbol symtab[NHASH];

struct Symbol * lookup(char *, int type);
void addlevel(int, int, char *);
void addlabel(int, char *);
void adduser(int, char *);
void addfile(int, char *);
char * leveltojson(struct Symbol * sym);
char * labeltojson(struct Symbol * sym);
