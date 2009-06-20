#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fpot.h"

int input_data (FILE * fd)
{
  char line[LINE_LENGHT];
  int ch;
  char excode[EXCODE_LENGHT];
  char mnm [MNM_LENGHT];
  double cte;
  int i;
  char *sptr;
  char data[13];
  
  base = 100.;
  tepa = 1. / base;
  tepr = 1. / base;
  acit = 30;

  while (!feof(fd)) {

    if (fgets(line, LINE_LENGHT, fd) == NULL) {
      printf("Unable to read file.\n");
      return -1;
    }
    
    if (line[0] == '(') continue;


    sscanf(line, "%s", excode);

    if (strncmp("TITU", excode, EXCODE_LENGHT) == 0) {
      if (fgets(titu, LINE_LENGHT, fd) == NULL) {
        printf("Unable to read file.\n");
        return -1;
      }
    }

    else if (strncmp("DCTE", excode, EXCODE_LENGHT) == 0) {

      for (;;) {
        
        do {
          if (fgets(line, LINE_LENGHT, fd) == NULL) {
            printf("Unable to read file.\n");
            return -1;
          }
        } while (!feof(fd) && (line[0] == '('));
      
        if (strncmp(line, "9999", 4) == 0) break;
        
        i = 0;
        while (sscanf(&line[i], "%s %lf", mnm, &cte) == 2) {
          i +=12;

          if (strncmp("TEPA", mnm, MNM_LENGHT) == 0) {
            tepa = cte;
          }
          else if (strncmp("TEPR", mnm, MNM_LENGHT) == 0) {
            tepr = cte;
          }
          else if (strncmp("BASE", mnm, MNM_LENGHT) == 0) {
            base = cte;
          }
          else if (strncmp("ACIT", mnm, MNM_LENGHT) == 0) {
            acit = cte;
          }
        }    
      }
    }

    else if (strncmp("DBAR", excode, EXCODE_LENGHT) == 0) {

      nbar = 0;
      npv = 0;
      npq = 0;
      
      for (;;) {
        
        do {
          memset(line, ' ', LINE_LENGHT);
          if (fgets(line, LINE_LENGHT, fd) == NULL) {
            printf("Unable to read file.\n");
            return -1;
          }
        } while (!feof(fd) && (line[0] == '('));
      
        if (strncmp(line, "9999", 4) == 0) {
          dbar[nbar].id = 9999;
          break;
        }

        line[strlen(line)-1] = ' ';

        sptr = line;
        strncpy(data, sptr, 4); data[4] = 0;
        if (strncmp(data, "    ", 4) == 0) {
          printf("Bar number field is mandatory.\n");
          return -1;
        }
        else {
          dbar[nbar].id = atoi(data);
        }

        sptr = line + 5;
        switch (*sptr) {
          case ' ':
          case 'A':
          case '0':
            break;
          case 'E':
          case '1':
          case 'M':
          case '2':
            printf("Operation not implemeted %c.\n", *sptr);
            return -1;
          default:
            printf("Unknown operation %c.\n", *sptr);
            return -1;
        }

        sptr = line + 7;
        switch (*sptr) {
          case ' ':
          case '0':
            dbar[nbar].type = 0;
            npq++;
            break;
          case '1':
            dbar[nbar].type = 1;
            npv++;
            break;
          case '2':
            dbar[nbar].type = 2;
            break;
          case '3':
            npq++;
            dbar[nbar].type = 3;
            break;

          default:
            printf("Unknown bar type %c\n", *sptr);
            return -1;
        }

        sptr = line + 8;
        dbar[nbar].base = *sptr == ' ' ? '0' : *sptr ;

        sptr = line + 9;
        strncpy((char *)dbar[nbar].name, sptr, NAME_LENGHT);

        sptr = line + 21;
        dbar[nbar].group = *sptr == ' ' ? '0' : *sptr ;

        sptr = line + 22;
        strncpy(data, sptr, 4); data[4] = 0;
        dbar[nbar].voltage = 0.001 * atoi(data);

        sptr = line + 26;
        strncpy(data, sptr, 4); data[4] = 0;
        sscanf(data, "%lf", &dbar[nbar].phase);

        dbar[nbar].phase = M_PI * dbar[nbar].phase / 180.;

        sptr = line + 30;
        strncpy(data, sptr, 5); data[5] = 0;
        sscanf(data, "%lf", &dbar[nbar].pg);

        sptr = line + 35;
        strncpy(data, sptr, 5); data[5] = 0;
        sscanf(data, "%lf", &dbar[nbar].qg);

        sptr = line + 40;
        strncpy(data, sptr, 5); data[5] = 0;
        if (strncmp(data, "     ", 5) == 0) {
          if (dbar[nbar].type == 2) {
            dbar[nbar].qn = -9999.;
          }
          else {
            dbar[nbar].qn = 0.;
          }
        }
        else {
          sscanf(data, "%lf", &dbar[nbar].qn);
        }

        sptr = line + 45;
        strncpy(data, sptr, 5); data[5] = 0;
        if (strncmp(data, "     ", 5) == 0) {
          if (dbar[nbar].type == 2) {
            dbar[nbar].qm = 9999.;
          }
          else {
            dbar[nbar].qm = 0.;
          }
        }
        else {
          sscanf(data, "%lf", &dbar[nbar].qm);
        }

        sptr = line + 50;
        strncpy(data, sptr, 5); data[5] = 0;
        if (strncmp(data, "     ", 5) == 0) {
          dbar[nbar].bc = dbar[nbar].id;
        }
        else {
          dbar[nbar].bc = atoi(data);
        }

        sptr = line + 55;
        strncpy(data, sptr, 5); data[5] = 0;
        if (strncmp(data, "     ", 5) == 0) {
          dbar[nbar].pl = 0.;
        }
        else {
          sscanf(data, "%lf", &dbar[nbar].pl);
        }

        sptr = line + 60;
        strncpy(data, sptr, 5); data[5] = 0;
        if (strncmp(data, "     ", 5) == 0) {
          dbar[nbar].ql = 0.;
        }
        else {
          sscanf(data, "%lf", &dbar[nbar].ql);
        }

        sptr = line + 65;
        strncpy(data, sptr, 5); data[5] = 0;
        if (strncmp(data, "     ", 5) == 0) {
          dbar[nbar].sh = 0.;
        }
        else {
          sscanf(data, "%lf", &dbar[nbar].sh);
        }

        dbar[nbar].sh /= base;
        
        sptr = line + 70;
        strncpy(data, sptr, 2); data[2] = 0;
        if (strncmp(data, "  ", 2) == 0) {
          dbar[nbar].area = 0;
        }
        else {
          dbar[nbar].area = atoi(data);
        }

        sptr = line + 72;
        strncpy(data, sptr, 4); data[4] = 0;
        dbar[nbar].cf = 0.001 * atoi(data);
        
        nbar++;
      }
    }

    else if (strncmp("DLIN", excode, EXCODE_LENGHT) == 0) {

      nlin = 0;
      
      for (;;) {
        
        do {
          memset(line, ' ', LINE_LENGHT);
          if (fgets(line, LINE_LENGHT, fd) == NULL) {
            printf("Unable to read file.\n");
            return -1;
          }
        } while (!feof(fd) && (line[0] == '('));
      
        if (strncmp(line, "9999", 4) == 0) {
          break;
        }

        line[strlen(line)-1] = ' ';
        
        sptr = line;
        strncpy(data, sptr, 4); data[4] = 0;
        if (strncmp(data, "    ", 4) == 0) {
          printf("Bar number field is mandatory.\n");
          return -1;
        }
        else {
          dlin[nlin].from = atoi(data);
        }

        sptr = line + 5;
        switch (*sptr) {
          case ' ':
          case 'A':
          case '0':
            break;
          case 'E':
          case '1':
          case 'M':
          case '2':
            printf("Operation not implemeted %c.\n", *sptr);
            return -1;
          default:
            printf("Unknown operation %c.\n", *sptr);
            return -1;
        }

        sptr = line + 8;
        strncpy(data, sptr, 4); data[4] = 0;
        if (strncmp(data, "    ", 4) == 0) {
          printf("Bar number field is mandatory.\n");
          return -1;
        }
        else {
          dlin[nlin].to = atoi(data);
        }

        sptr = line + 17;
        strncpy(data, sptr, 6); data[6] = 0;
        if (strncmp(data, "      ", 6) == 0) {
          dlin[nlin].r = 0.;
        }
        else {
          dlin[nlin].r = 0.01 * atof(data);
        }

        sptr = line + 23;
        strncpy(data, sptr, 6); data[6] = 0;
        if (strncmp(data, "      ", 6) == 0) {
          dlin[nlin].x = 0.;
        }
        else {
          dlin[nlin].x = 0.01 * atof(data);
        }

        sptr = line + 29;
        strncpy(data, sptr, 6); data[6] = 0;
        if (strncmp(data, "      ", 6) == 0) {
          dlin[nlin].b = 0.;
        }
        else {
          dlin[nlin].b = atof(data) / base;
        }

        sptr = line + 35;
        strncpy(data, sptr, 5); data[5] = 0;
        if (strncmp(data, "     ", 5) == 0) {
          dlin[nlin].tap = 0.;
        }
        else {
          dlin[nlin].tap = atof(data);
        }

        sptr = line + 40;
        
        strncpy(data, sptr, 5); data[5] = 0;
        if (strncmp(data, "     ", 5) == 0) {
          dlin[nlin].tmn = 0.;
        }
        else {
          dlin[nlin].tmn = atof(data);
        }

        sptr = line + 40;
        strncpy(data, sptr, 5); data[5] = 0;
        if (strncmp(data, "     ", 5) == 0) {
          dlin[nlin].tmn = 0.;
        }
        else {
          dlin[nlin].tmn = atof(data);
        }

        sptr = line + 45;
        
        strncpy(data, sptr, 5); data[5] = 0;
        if (strncmp(data, "     ", 5) == 0) {
          dlin[nlin].tmx = 0.;
        }
        else {
          dlin[nlin].tmx = atof(data);
        }
          
        nlin++;
      }
    }

    else if (strncmp("DGER", excode, EXCODE_LENGHT) == 0) {

      nger = 0;

      for (;;) {
        
        do {
          if (fgets(line, LINE_LENGHT, fd) == NULL) {
            printf("Unable to read file.\n");
            return -1;
          }
        } while (!feof(fd) && (line[0] == '('));
      
        if (strncmp(line, "9999", 4) == 0) {
          break;
        }

        line[strlen(line)-1] = ' ';

        sptr = line;
        strncpy(data, sptr, 4); data[4] = 0;
        if (strncmp(data, "    ", 4) == 0) {
          printf("Bar number field is mandatory.\n");
          return -1;
        }
        else {
          dger[nger].id = atoi(data);
        }

        sptr = line + 7;
        strncpy(data, sptr, 5); data[5] = 0;
        if (strncmp(data, "     ", 5) == 0) {
          dger[nger].pmn = 0.;
        }
        else {
          dger[nger].pmn = atof(data) / base;
        }

        sptr = line + 13;
        strncpy(data, sptr, 5); data[5] = 0;
        if (strncmp(data, "     ", 5) == 0) {
          dger[nger].pmx = 0.;
        }
        else {
          dger[nger].pmx = atof(data) / base;
        }

        nger++;
      }
    }

    else if (strncmp("DGLT", excode, EXCODE_LENGHT) == 0) {

      for (;;) {
        
        do {
          if (fgets(line, LINE_LENGHT, fd) == NULL) {
            printf("Unable to read file.\n");
            return -1;
          }
        } while (!feof(fd) && (line[0] == '('));
      
        if (strncmp(line, "99", 2) == 0) {
          break;
        }
      }
    }

    else if (strncmp("DGBT", excode, EXCODE_LENGHT) == 0) {

      for (;;) {
        
        do {
          if (fgets(line, LINE_LENGHT, fd) == NULL) {
            printf("Unable to read file.\n");
            return -1;
          }
        } while (!feof(fd) && (line[0] == '('));
      
        if (strncmp(line, "99", 2) == 0) {
          break;
        }
      }
    }

    else if (strncmp("FIM", excode, EXCODE_LENGHT) == 0) {
      return 0;
    }

    else {
      printf("Execution code %s not implemented\n", excode);
      return -1;
    }
  }
  
  return -1;
}
