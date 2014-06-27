#include <stdio.h>
#include <ctype.h>

#define mprintf printf

void sscanf_addhoc_replacement (char *str, unsigned long long *var1, unsigned long long *var2){ 
	char *ptr, tok[]=":";
	char *rest, *prt_uax, *c;
	c=str;
	
	//ptr = strpbrk (c, tok);
	//printf("ptr %s and *prt %s \n", ptr, *ptr);
	
	if ((ptr = strpbrk (c, ":\n")) > 0){
			//printf("offset %d\n", offset);
		ptr[0] = 0;
			printf ("%s\n", c);
		*var1=strtol(c, &rest, 0);
			printf ("%s (%lu)\n", c, *var1);
		c = ptr+1;
		if (rest && *rest)
			printf ("-1 -> Primero\n");
		else{
			while (ptr++){
				if (isalpha(ptr[0])){
					printf("%c\n", ptr[0]);
					ptr[0]=0;
					break;
				}
			}
			*var2=strtol(c, &rest, 0);
				printf ("%s (%lu)\n", c, *var2);
			if (rest && *rest)
				printf ("-1 -> Segundo\n");
		} //if ((offset = strpbrk (c, tok) - c) > 0)
	}
}

int sscanf_addhoc_replacement1 (char *str, unsigned long long *var1, unsigned long long *var2){ 
	char *c, *ptr, *rest, tok[]= ":";
	int processed = 0;
	c=str;
mprintf("---1 %s\n",__func__);
mprintf("%s\n", c);


	if ((ptr = strpbrk (c, tok)) > 0){
		mprintf("c %s\n", c);
		mprintf("ptr %s\n", ptr);
		ptr[0] = 0;
		mprintf("c %s\n", c);
		mprintf("ptr %s\n", ptr);
		*var1=strtol(c, &rest, 0);
		c = ptr+1;
		mprintf("1st ptr %s var1 %llu c %s\n", ptr, *var1, c);
		if (!(rest && *rest))
		{
			processed++;
			while (ptr++){
				if (isalpha(ptr[0])){
					ptr[0]=0;
					break;
				}
			}
			*var2=strtol(c, &rest, 0);
			mprintf("2nd ptr %s var2 %llu c %s\n", ptr, *var2, c);
			if (! (rest && *rest))
				processed ++;
		} //if ((offset = strpbrk (c, tok) - c) > 0)	
	}
	return processed;
}

int main (void){
	
	unsigned long long utc, vec[2];
	unsigned long long nanos;
	unsigned int offset=1;
	char c1[128];// = "12:34s";
	char *ptr, tok[]=":";
	char *rest, *prt_uax, *c;
	
	while(1) {
		printf("Enter sting: ");
		scanf("%s", &c1);
		c=c1;
		/*ptr = strpbrk (c, tok);
		*ptr = 0;
		ptr++;
		nanos=strtol(c, &rest, 0);
		printf ("%s (%lu) ", c, nanos);
		if (rest && *rest)
			printf ("is not a number\n");
		else
			printf ("is a number\n");
		printf ("%s\n", ptr);
		prt_uax = ptr;
		
		while (prt_uax++){
			if (isalpha(prt_uax[0])){
				*prt_uax = 0;
				break;
			}
		}
		nanos=strtol(ptr, &rest, 0);
		printf ("%s (%lu) ", ptr, nanos);
		if (rest && *rest)
			printf ("is not a number\n");
		else
			printf ("is a number\n");*/
		printf("nanos %lu, utc %lu\n", nanos, utc);
		offset=sscanf_addhoc_replacement1 (c, &nanos, &utc);
		printf("nanos %lu, utc %lu, offset %i\n", nanos, utc, offset);

		//printf ("%s\n", ptr);
	}
		
	return 0;
}
