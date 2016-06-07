
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <linux/limits.h>

#include "global.h"
#include "md5.h"

#define PATH_DELIMITER '/'
#define DIGEST_SIZE (16)
#define BUFFER_SIZE (4 * 1024)

void calc_md5(const char * filename, char * digest_str)
{
    FILE * file;
    char buff[BUFFER_SIZE];
    unsigned char digest[DIGEST_SIZE];
    MD5_CTX ctx;
    int num;

    if ( (file = fopen(filename, "r")) != NULL )
    {
        MD5Init(&ctx);
        while ( (num = fread(buff, 1, BUFFER_SIZE, file)) > 0 )
        {
            MD5Update(&ctx, buff, num);
        }
        MD5Final(digest, &ctx);

        fclose(file);

        for (num = 0; num < DIGEST_SIZE; num++)
        {
            sprintf(&digest_str[num * 2], "%.2x", digest[num]);
        }
    }
}

int match(const char * filename, const char * filemask)
{
    while (*filemask)
    {
        if (*filemask == '*')
        {
            if (match(filename, filemask + 1))
                return 1;

            if (*filename && match(filename + 1, filemask))
                return 1;

            return 0;
        }
        else if (*filemask == '?')
        {
            if (!*filename)
                return 0;

            filename++;
            filemask++;
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
        return 1;
    }

    char * fileparam = argv[1];
    char * delim, * wildcard;
    char filename[PATH_MAX];
    int pathlen = 0;

    if ( (delim = strrchr(fileparam, PATH_DELIMITER)) != NULL )
        pathlen = delim - fileparam + 1;
    strncpy(filename, fileparam, pathlen);
    wildcard = &fileparam[pathlen];

    DIR * dir = opendir(filename);
    struct dirent * de;
    while ( (de = readdir(dir)) != NULL )
    {
        if ( (de->d_type != DT_DIR) && match(de->d_name, wildcard) )
        {
            char digest_str[DIGEST_SIZE * 2 + 1];

            strcpy(&filename[pathlen], de->d_name);
            calc_md5(filename, digest_str);

            printf("%s %s\r\n", digest_str, de->d_name);
        }
    }
    closedir(dir);

    return 0;
}
