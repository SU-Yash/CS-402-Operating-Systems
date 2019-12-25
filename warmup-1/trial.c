#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

int main(){
	/*
	char po[24]= "abcdefghi";
	printf("%s\n", po);
	printf("%d\n", strlen(po));
	
	char buf[24];
	strncpy(buf, po, 24);
	printf("%d\n", strlen(buf));
    
    if(strlen(po) > 24){
        buf[23] = '\0';
    }
    else{
        buf[strlen(po)-1] = ' ';

        int length = 24;
        int l = strlen(po);

    	while(l<length) {
        	buf[l] = ' '; 
        	l++;
        }
        buf[l]= '\0';
    }
	
	printf("%s\n", buf);
	printf("%d\n", strlen(buf));
	*/

	/*
	int c = 100015;
	c = c*(-1);
	int cents = c%100;
	c = c/100;
	char bufs[10];
        if(c > 1000000){
            char afterSecondCommaChar[4];
            char beforeSecondCommaChar[4];
            char centsChar[4];

            int beforeFirstComma = c/1000000;
            int afterFirstComma = c%1000000;

            int beforeSecondComma = afterFirstComma/1000;
            int afterSecondComma = afterFirstComma%1000;
            
            if(beforeSecondComma < 10){
                snprintf(beforeSecondCommaChar, 4, "00%d", beforeSecondComma);
            }
            else if (beforeSecondComma < 100){
                snprintf(beforeSecondCommaChar, 4, "0%d", beforeSecondComma);
            }
            else{
                snprintf(beforeSecondCommaChar, 4, "%d", beforeSecondComma);
            }

            if(afterSecondComma < 10){
                snprintf(afterSecondCommaChar, 4, "00%d", afterSecondComma);
            }
            else if (afterSecondComma < 100){
                snprintf(afterSecondCommaChar, 4, "0%d", afterSecondComma);
            }
            else{
                snprintf(afterSecondCommaChar, 4, "%d", afterSecondComma);
            }

            if(cents == 0){
                snprintf(centsChar, 4, "00");
            }
            else if(cents < 10){
                snprintf(centsChar, 4, "0%d", cents);
            }
            else{
                snprintf(centsChar, 4, "%d", cents);
            }

            snprintf(bufs, 12, "%d,%s,%s.%s", beforeFirstComma, beforeSecondCommaChar, afterSecondCommaChar, centsChar);

        }

        else if (c > 1000){
            char afterSecondCommaChar[4];
            char centsChar[4];
            int beforeSecondComma = c/1000;
            int afterSecondComma = c%1000;
            //printf("afterSecondComma:%d", afterSecondComma);
            if(afterSecondComma < 10){
                snprintf(afterSecondCommaChar, 4, "00%d", afterSecondComma);
            }
            else if (afterSecondComma < 100){
                snprintf(afterSecondCommaChar, 4, "0%d", afterSecondComma);
            }
            else{
                snprintf(afterSecondCommaChar, 4, "%d", afterSecondComma);
            }

            if(cents == 0){
                snprintf(centsChar, 4, "00");
            }
            else if(cents < 10){
                snprintf(centsChar, 4, "0%d", cents);
            }
            else{
                snprintf(centsChar, 4, "%d", cents);
            }
            snprintf(bufs, 12, "%d,%s.%s", beforeSecondComma, afterSecondCommaChar, centsChar);
        }
        else{
            if(cents == 0){
                snprintf(bufs, 12, "%d.00", c);
            }
            else{
                if(cents < 10){
                    snprintf(bufs, 12, "%d.0%d", c, cents);
                }
                else{
                    snprintf(bufs, 12, "%d.%d", c, cents);
                }
            }
        }

	printf("buff:%s", bufs);*/

	/*
	char type = '+';
	printf("%s\n", buf);
	char amount[15];
	char space[12];
    
    int length = 12;
    int l = strlen(buf);
    int i = 0;

    while(i<length-l) {
        space[i] = ' '; 
        i++;
    }

	if(type == '-')
		snprintf(amount, 14, "(%s%s", space, buf);
		strcat(amount, ")");
	}

	else{
		snprintf(amount, 14, "%s%s", space, buf);
	}
	printf("Amount:%s\n", amount);
	printf("Amount:%d\n", strlen(amount));

char *des = "Initial Deposit\n";
//des[strlen(des)-2] = "a";
char dess[24];
strcpy(dess, des);
dess[strlen(dess)-1] = ' ';



printf("Dess Length:%d\n", strlen(dess));
int length = 24;
int l = strlen(dess);

while(l<length) {
printf("%d\n", l);
dess[l] = ' '; 
l++;
}
dess[l]= '\0';
//dess[17] = 'f';
printf("%s\n", dess);
printf("Done\n");
printf("Length:%d\n", strlen(dess));
*/
/*
char *ptr = "1723.00";
int val = 0;
int k = 0;
char ch;
char buf[15];
buf[0] = '(';
buf[13] = ')';

while(ptr[k]!= '.'){
	ch = ptr[k];
	val = val*10 + atoi(&ch);
	k++;
}
k++;
//printf("%c\n", ptr[k]);

while(ptr[k] != '\0'){
	ch = ptr[k];
	val = val*10 + atoi(&ch);
	k++;
}
//val = atof(ptr)*100;
printf("Value:%d\n", val);
//printf("%c\n", ptr[0]);
//printf("%d\n", val/100);
//printf("%d\n", val%100);
//snprintf(buf, 15, "%d", val/100);
//printf("Buff%s\n", buf);
//printf("%d", strlen(buf));
char balance[14];

balance[0] = '('; balance[13] = ')';
for(int i = 12; i > 0; i--) {
	if(i == 10){
		balance[i]='.'; continue;
	}
	if(i == 6 || i == 2){
		balance[i]=','; continue;
	}
	balance[i] = (char)(((int)'0')+(val%10));
	val= val/10;
	if(val == 0) {i--;break;}
				}
balance[14]= '\0';
printf("%s", balance);
*/


fprintf(stdout, "%d\n", time(NULL)); 
return (0);
}