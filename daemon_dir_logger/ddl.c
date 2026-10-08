#include <sys/inotify.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

int main() 
{
    int fd = inotify_init();
    if (fd < 0) 
    {
        perror("inotify_init");
        return 1;
    }

    int wd = inotify_add_watch(fd, ".", 
                               IN_CREATE | IN_DELETE | IN_MODIFY | IN_OPEN);
    if (wd < 0) 
    {
        perror("inotify_add_watch");
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
        int len = read(fd, buf, sizeof(buf));

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

                //if (event->mask & IN_OPEN)
                //    printf("[%.24s] File has beem opened:%s\n", ctime(&now), event->name);

                if (event->mask & IN_CREATE)
                    offset = sprintf(output_buf, 
                            "[%.24s] File was maked: %s\n", ctime(&now), event->name);
                    
                if (event->mask & IN_DELETE)
                    offset = sprintf(output_buf, 
                            "[%.24s] File has been deleted: %s\n", ctime(&now), event->name);

                if (event->mask & IN_MODIFY)
                {
                    int file_hash = event->wd % 256;
                    if (now > last_mod_time[file_hash])
                    {
                        offset = sprintf(output_buf, 
                            "[%.24s] File has been modified: %s\n", ctime(&now), event->name);
                        last_mod_time[file_hash] = now;
                    }
                }

                // output to file
                if (offset > 0)
                {
                    fprintf(file, "%s\n", output_buf);
                    fflush(file);
                }
            }
            
            event = (struct inotify_event *)((char *)event + 
            sizeof(struct inotify_event) + event->len);
        }
    }
    
    fclose(file);
    inotify_rm_watch(fd, wd);
    close(fd);
    return 0;
}
