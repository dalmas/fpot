
#define DEBUG 1

#define LENGHT 1000
#define LINE_LENGHT 80
#define EXCODE_LENGHT 4
#define MNM_LENGHT 4
#define NAME_LENGHT 12

typedef struct {
  int id;
  int type;
  char base;
  char *name[NAME_LENGHT];
  char group;
  double voltage;
  double phase;
  double pg;
  double qg;
  double qn;
  double qm;
  int bc;
  double pl;
  double ql;
  double sh;
  int area;
  double cf;
} dbar_t;

typedef struct {
  int from;
  int to;
  double r;
  double x;
  double b;
  double tap;
  double tmn;
  double tmx;
} dlin_t;

typedef struct 
{
  int id;
  double pmn;
  double pmx;
} dger_t;
  
#define pow2(x) ((x) * (x))

extern char titu[LINE_LENGHT];

extern double tepa;
extern double tepr;
extern double base;
extern int acit;

extern dbar_t dbar[LENGHT];
extern dlin_t dlin[LENGHT];
extern dger_t dger[LENGHT];

extern int nbar;
extern int nlin;
extern int nger;
extern int npv;
extern int npq;

int input_data (FILE * fd);
