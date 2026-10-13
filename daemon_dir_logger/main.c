#define _XOPEN_SOURCE 500
#include <sys/inotify.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ftw.h>

int inotify_fd = 0;

int add_watch_callback(const char *fpath, const struct stat *sd, int typeflag,
     struct FTW *ftwbuf)
{
    if (typeflag == FTW_D)
    {
        int wd = inotify_add_watch(inotify_fd, fpath, 
            IN_CREATE | IN_DELETE | IN_MODIFY | IN_OPEN);

        if (wd < 0)
            perror("inotify_add_watch");
        else
            printf("Watching: %s\n", fpath);
    }
    return 0;
}

int main() 
{
    inotify_fd = inotify_init();
    if (inotify_fd < 0) 
    {
        perror("inotify_init");
        return 1;
    }

    if (nftw(".", add_watch_callback, 20, 0) == -1)
    {
        perror("ntfw");
        return 1;
    }

    char buf[4096];
    time_t last_mod_time[256] = {0};

    FILE *file = fopen("log.txt", "a");
    if (file == NULL)
    {
        perror("fopen failed");
        return 1;
    }

    while (1) 
    {
        int len = read(inotify_fd, buf, sizeof(buf));

        if (len < 0)
        {
            perror("read");
            break;
        }

        struct inotify_event *event = (struct inotify_event *)buf;
        time_t now = time(NULL);

        char output_buf[1024];
        int offset = 0;

        while ((char *)event < buf + len) 
        {
            // events
            if (event->len > 0)
            {
                offset = 0;

                const char *kind = (event->mask & IN_ISDIR) ? "Directory" 
                    : "File";

                // Add new directory to watch, when app is work
                if (event->mask & IN_CREATE && event->mask & IN_ISDIR)
                {
                    char path[512];
                    snprintf(path, sizeof(path), "./%s", event->name);

                    int new_wd = inotify_add_watch(inotify_fd, path, 
                        IN_CREATE | IN_DELETE | IN_MODIFY);
                    
                    if (new_wd >- 0)
                    printf("Now watching directory: %s\n", event->name);
                }
                if (event->mask & IN_OPEN)
                    offset = sprintf(output_buf, 
                        "[%.24s] %s has been opened: %s\n", 
                        ctime(&now), kind, event->name);

                if (event->mask & IN_CREATE)
                    offset = sprintf(output_buf, 
                        "[%.24s] %s was maked: %s\n", 
                        ctime(&now), kind, event->name);
                    
                if (event->mask & IN_DELETE)
                    offset = sprintf(output_buf, 
                        "[%.24s] %s has been deleted: %s\n", 
                        ctime(&now), kind, event->name);

                if (event->mask & IN_MODIFY)
                {
                    int file_hash = event->wd % 256;
                    if (now > last_mod_time[file_hash])
                    {
                        offset = sprintf(output_buf, 
                            "[%.24s] File has been modified: %s\n", 
                            ctime(&now), event->name);
                        last_mod_time[file_hash] = now;
                    }
                }

                // output to file
                if (offset > 0)
                {
                    if (event->len > 0 && strcmp(event->name, "log.txt") != 0)
                    {
                        fprintf(file, "%s", output_buf);
                        fflush(file);
                    }
                    
                }
            }
            
            event = (struct inotify_event *)((char *)event + 
            sizeof(struct inotify_event) + event->len);
        }
    }
    
    fclose(file);
    inotify_rm_watch(inotify_fd, 0);
    close(inotify_fd);
    return 0;
}