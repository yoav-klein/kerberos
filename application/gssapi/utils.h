
#ifndef __UTILS_H__
#define __UTILS_H__

int send_token(int fd, gss_buffer_desc *token);
int recv_token(int fd, gss_buffer_desc *token);

#endif /* __UTILS_H__ */