#include <stdio.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <fstream>

#if 0
/*                01234567890123456789012345 */

int bsearch(const unsigned char *chars, int len, char find, int *pc)
{
    int start=0;
    int end = len;
    *pc = 0;

    while(start <= end)
    {
        int probe = start + ((end-start)/2);
        (*pc)++;
        int v = find - chars[probe];
        if(v == 0)
            return probe;
        if(v < 0)
        {
            end = probe - 1;
        }
/*        if(find > chars[probe])*/
        else
        {
            start = probe + 1;
        }
    }
    return -1;
}


int main(int argc, char *argv[])
{
#define MAX 256
    unsigned char tst[MAX];
    int x;
    int n_probes;
    for (x = 0; x<MAX; x++)
        tst[x] = (char)x;
    for (x=101; x<128; x++)
    {
        int ix = bsearch(tst, MAX, tst[x], &n_probes);
        printf("%d %d %d %c \n", x, ix, n_probes, tst[ix]);
    }
}
#endif

int debug = 0;
#if 0
int Bsearch(const char *strings[], int len, const char *target, int *ixtop, int *pc, int *ps)
{
    int start=0;
    int end = len;
    int ix_match = len -1;
    int ix_fcmatch;
    *pc = 0;
    *ps = 0;

    unsigned char find = (unsigned char)*target;
    int cmp;
    int probe;
    const char *sprobe;
    while(start <= end && start < len) 
    {
        probe = start + ((end-start)/2);
        sprobe = strings[probe];
        (*pc)++;
        if((cmp = find - (unsigned char)*sprobe))
        {
            if(cmp < 0)
                end = probe - 1;
            else
                start = probe + 1;
        }
        else
            break;
    }

    ix_fcmatch = ix_match = probe;
    while(start <= end && start < len)
    {
        (*ps)++;

        ix_match = probe;
        cmp = strcmp(target, sprobe);
        if (debug)
            printf("\t\t\t %d - %d %d) cmp=%d  %s == %s\n", start, end, probe, cmp, target, sprobe);
        if (cmp == 0)
            break;

        if(cmp < 0)
        {
            end = probe - 1;
        }
        else
        {
            start = probe + 1;
        }
        probe = start + ((end-start)/2);
        sprobe = strings[probe];
    }

    *ixtop = ix_match;
    if (cmp && (*target != strings[ix_match][0]))
        *ixtop = ix_fcmatch;
    if (debug)
        printf("\t\t\t %d - %d %d\n", start, end, probe);
    return cmp==0;
}


int main(int argc, char *argv[])
{
#define XMAX 100
#define YMAX 5
    int ixa;
    char stst[XMAX][YMAX];
    const char *tst[XMAX];
    int x,y;
    int cmp_chars, cmp_strs;
    for (x = 0; x<XMAX; x++)
    {
        tst[x] = stst[x];
    }
    for (x = 0; x<XMAX; x++)
    {
        for(y=0; y<YMAX; y++)
        {
            stst[x][y] = (char)x;
        }
        stst[x][YMAX-1] = 0;
    }

    stst[98][0] = 97;
    for (ixa = 1; ixa < argc; ixa++)
    {
        const char *srch = argv[ixa];
        int ix;
        if (debug)
            printf("%s:\n", srch);
        int found = Bsearch(tst, XMAX, srch, &ix, &cmp_chars, &cmp_strs);
        printf("%s:  found=%d nc=%d ns=%d ix=%d \"%s\" \n", srch, found, cmp_chars, cmp_strs, ix, tst[ix]);
    }
}
#endif

int Bsearch(std::vector<std::string> strings, int len, const char *target, int *ixtop, int *pc, int *ps)
{
    int start=0;
    int end = len;
    *pc = 0;
    *ps = 0;

    char uctarget = (unsigned char)*target;
    unsigned char ucprobe;
    int ixprobe, ix_bm;
    const char *sprobe;
    unsigned offset = 0;
    unsigned tlen = strlen(target);
    while((start <= end) && (start < len) && (offset <= tlen))
    {
        ixprobe = start + ((end-start)/2);
        sprobe = strings[ixprobe].c_str();

        if (debug)
            printf("\t\t\t %d - %d ix=%d o=%d %s \n", start, end, ixprobe, offset, sprobe);
        ucprobe = 0;
        if (offset < strlen(sprobe))
            ucprobe = (unsigned char)sprobe[offset];

        (*pc)++;
        if(uctarget != ucprobe)
        {
            if(uctarget < ucprobe)
                end = ixprobe - 1;
            else
                start = ixprobe + 1;
        }
        else
        {
            offset++;
            uctarget = (unsigned char)target[offset];
            ix_bm = ixprobe;
        }
    }

    if (debug)
        printf("\t\t\t %d - %d %d %d\n", start, end, ixprobe, ix_bm);
    *ixtop = ixprobe;
    if (uctarget == ucprobe)
        return 1;
    *ixtop = ix_bm;
    return 0;
}

int main(int argc, char *argv[])
{
    int ixa=1;
    std::vector<std::string> strings;
    if (argc > ixa)
    {
        if ((0 ==strcmp(argv[ixa], "debug"))
                || (0 ==strcmp(argv[ixa], "-d"))
                || (0 ==strcmp(argv[ixa], "--debug"))
           )
        {
            debug = 1;
            ixa++;
        }
    }

    if (argc > ixa)
    {
        char line[500];
        std::ifstream ifs(argv[ixa]);
        if (ifs.is_open())
        {
            for (; ifs.getline(line, 100-1); )
                strings.push_back(std::string(line));
        }

        ixa++;
    }

    for (;ixa < argc; ixa++)
    {
        int cmp_chars, cmp_strs;
        const char *srch = argv[ixa];
        int ix;
        if (debug)
            printf("%s:\n", srch);
        int found = Bsearch(strings, strings.size(), srch, &ix, &cmp_chars, &cmp_strs);
        std::cout << srch << "   found=" << found << " nc=" << cmp_chars << " ns=" << cmp_strs << " " << ix << " " << strings[ix] << std::endl;
    }
}
