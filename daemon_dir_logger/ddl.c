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

    while (1) 
    {
        int len = read(fd, buf, sizeof(buf));
        struct inotify_event *event = (struct inotify_event *)buf;
        time_t now = time(NULL);

        while ((char *)event < buf + len) 
        {
            if (event->len > 0)
            {
                if (event->mask & IN_OPEN)
                    printf("File has beem opened: %s at %s\n", event->name, ctime(&now));
                if (event->mask & IN_CREATE)
                    printf("File was maked: %s at %s\n", event->name, ctime(&now));
                if (event->mask & IN_DELETE)
                    printf("File has been deleted: %s\n at %s\n", event->name, ctime(&now));
                if (event->mask & IN_MODIFY)
                {
                    int file_hash = event->wd % 256;
                    if (now > last_mod_time[file_hash])
                    {
                        printf("File has been modified: %s at \n", event->name, ctime(&now));
                        last_mod_time[file_hash] = now;
                    }
                }
            }
            
            event = (struct inotify_event *)((char *)event + 
            sizeof(struct inotify_event) + event->len);
        }
    }
    
    inotify_rm_watch(fd, wd);
    close(fd);
    return 0;
}
