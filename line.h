#ifndef LINE_H
#define LINE_H

int open_line (const char *path, int mode);

int read_line (int fd, char *buf, int buflen);
int send_command (int fd, const char *command, const char *arg);
int send_status_request (int fd, const char *status);

#endif /*LINE_H*/
