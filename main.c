
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <linux/limits.h>

#include "global.h"
#include "md5.h"

#define PATH_DELIMITER '/'

void calc_md5(const char * filename, char * digest_str)
{
    FILE * file = fopen(filename, "r");
    if (file != NULL)
    {
        const int buff_size = 4 * 1024;
        unsigned char buff[buff_size], digest[16];
        MD5_CTX ctx;
        int num;

        MD5Init(&ctx);
        while ( (num = fread(buff, 1, buff_size, file)) > 0 )
        {
            MD5Update(&ctx, buff, num);
        }
        MD5Final(digest, &ctx);

        fclose(file);

        for (num = 0; num < 16; num++)
        {
            sprintf(&digest_str[num * 2], "%.2x", digest[num]);
        }
    }
}

int match(const char * filename, const char * filemask)
{
    while (*filemask)
    {
        if (*filemask == '?')
        {
            if (!*filename)
                return 0;

            filename++;
            filemask++;
        }
        else if (*filemask == '*')
        {
            if (match(filename, filemask + 1))
                return 1;

            if (*filename && match(filename + 1, filemask))
                return 1;

            return 0;
        }
        else
        {
            if (*filename++ != *filemask++)
                return 0;
        }
    } // while

    return !*filename && !*filemask;
}

int main(int argc, char * argv[])
{
    if (argc < 2)
    {
        printf("usage:\rchksum <path/filemask>\r");
        return 0;
    }

    char * fileparam = argv[1];
    char * delim, * wildcard;
    char filename[PATH_MAX];
    int pathlen;

    delim = strrchr(fileparam, PATH_DELIMITER);
    pathlen = (delim != NULL) ? delim - fileparam + 1 : 0;
    strncpy(filename, fileparam, pathlen);
    wildcard = &fileparam[pathlen];

    DIR * dir = opendir(filename);
    struct dirent * de;
    while ( (de = readdir(dir)) != NULL )
    {
        if ( (de->d_type != DT_DIR) && match(de->d_name, wildcard) )
        {
            char digest_str[16 * 2 + 1];

            strcpy(&filename[pathlen], de->d_name);
            calc_md5(filename, digest_str);

            printf("%s %s\r\n", digest_str, de->d_name);
        }
    }
    closedir(dir);

    return 0;
}
