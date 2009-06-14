
#define DEBUG 1

#define LENGHT 1000
#define LINE_LENGHT 80
#define EXCODE_LENGHT 4
#define MNM_LENGHT 4
#define NAME_LENGHT 12

char titu[LINE_LENGHT];

double tepa;
double tepr;
int base;
int acit;

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

dbar_t dbar[LENGHT];

struct dlin_t {
  int from;
  int to;
  double r;
  double x;
  double b;
  double tap;
  double tmn;
  double tmx;
} dlin[LENGHT];

int nbar;
int nlin;
int npv;
int npq;
double yg[LENGHT][LENGHT];
double yb[LENGHT][LENGHT];
int b1idx[LENGHT];
double b1[LENGHT][LENGHT];
int b2idx[LENGHT];
double b2[LENGHT][LENGHT];
double voltage[LENGHT];
double phase[LENGHT];
double dp[LENGHT];
double dq[LENGHT];
double dv[LENGHT];
double dth[LENGHT];

#define pow2(x) ((x) * (x))

int input_data (FILE * fd);
