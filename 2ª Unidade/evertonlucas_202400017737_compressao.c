#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>
#include <stdint.h>

/*
Input:
4
5 AA AA AA AA AA
7 10 20 30 40 50 60 70
9 FF FF FF FF FF FF FF FF FF
4 FA FA C1 C1

Output:
0->HUF(20.00%)=00
1->HUF(42.86%)=9C6B50
2->HUF(22.22%)=0000
2->RLE(22.22%)=09FF
3->HUF(25.00%)=C0

 inputComp.txt outputComp.txt
*/

