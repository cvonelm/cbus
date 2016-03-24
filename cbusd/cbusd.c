/*
 *  
 *  Copyright (c) 2016 Christian von Elm <c.vonelm@gmail.com>
 *
 *  This file is open source, see the LICENSE file in the top directory of this repository
 *  for more information
 */

#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h> 
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/signal.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>

#include <ftw.h>

#include <ctype.h>

#include "cbusd.h"
/*globals*/
char *socket_path;
char *rules_path;
char *auth_path;
char *base_dir;
char *VERSION = "0.1.1 Daemon Days";
 CBUS_conn *start, *it;
fd_set master;
int verbose = 0;
int drop = 0;
int cbusd_clear_dir(char *path);
int cbusd_clear_dir(char *path)
{
    printf("%s\n", path);
    DIR *dir;
    struct dirent *entry;

    if (!(dir = opendir(path)))
    {
        return CBUS_ERR_OTHER;
    }
    if (!(entry = readdir(dir)))
    {
        return CBUS_ERR_OTHER;
    }

    do 
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }
        
        if(entry->d_type == DT_DIR || entry->d_type == DT_REG)
        {
            char *new_path = malloc(strlen(path) 
                    + strlen(entry->d_name) + 2);
            if(new_path == NULL)
            {
                return CBUS_ERR_NOMEM;
            }
            memcpy(new_path, path, strlen(path));
            new_path[strlen(path)] = '/';
            memcpy(new_path + strlen(path) + 1,
                    entry->d_name, strlen(entry->d_name) + 1);
            printf("%s\n", path); 
            if (entry->d_type == DT_DIR) 
            {
                cbusd_clear_dir(new_path);
                if(remove(new_path) == -1)
                {
                    return CBUS_ERR_OTHER;
                }
                
            }
            else if(entry->d_type == DT_REG)
            {
                if(remove(new_path) == -1)
                {
                    return CBUS_ERR_OTHER;
                }

            }
        }
    } while (entry = readdir(dir));
    closedir(dir);
}
int main(int argc, char **argv)
{
    srand(time(NULL));
    /* We handle SIGPIPE in-code */
    signal(SIGPIPE, SIG_IGN);

    parse_args(argc, argv);
    cbusd_clear_dir(auth_path);
    mkdir(auth_path, 0777);
    cbusd_gen_auth(rules_path, auth_path);
    
    int own_fd;
    struct sockaddr_un local, remote;
    if((own_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
    {
        fprintf(stderr, "Couldn't create socket: %s\n", strerror(errno));
        return 1;
    }
    /* As we are using select(), use NONBLOCKing mode */
    if(fcntl(own_fd, F_SETFL, O_NONBLOCK) == -1)
    {
        fprintf(stderr, "Couldn't put the socket into nonblocking mode: %s\n",
                strerror(errno));
    }
    local.sun_family = AF_UNIX;

    memcpy(local.sun_path, socket_path, strlen(socket_path) + 1);

    /*There may be some dead socket laying around from a previous execution */
    unlink(local.sun_path);

    size_t len = strlen(local.sun_path) + sizeof(local.sun_family);
    if(bind(own_fd, (struct sockaddr *)&local, len) == -1)
    {
        fprintf(stderr, "Couldn't bind to %s: %s\n", socket_path, strerror(errno));
        return 1;
    }
    if(listen(own_fd, 5) == -1)
    {
        fprintf(stderr, "Couldn't listen: %s\n", strerror(errno));
        return 1;
    }
    if(chmod(socket_path, 0666) == -1)
    {
        fprintf(stderr, "Couldn't change socket permissions:%s\n",
                strerror(errno));
    }

    fd_set  read_fds;
    FD_SET(own_fd, &master);
    int fd_max = own_fd;
    /* Have an immutable start */
    start = calloc(1, sizeof( CBUS_conn));
    if(start == NULL)
    {
        fprintf(stderr, "Out of memory!\n");
        exit(1);
    }
    start->address = "/_error";
    start->fd = -1;
    it = start;
    while(1)
    {
        /*copy list as select modifies the list */
        read_fds = master;
        if(select(fd_max+1, &read_fds, NULL, NULL, NULL) == -1)
        {
            fprintf(stderr, "Couldn't select(): %s\n", strerror(errno));
            exit(1);
        }
        /* got a new connection */
        if(FD_ISSET(own_fd, &read_fds))
        {
            struct sockaddr_un remote;
            size_t remote_len = sizeof(remote);
            int newfd = accept(own_fd,
                    (struct sockaddr *)&remote, (socklen_t *)&remote_len);
            if(newfd == -1)
            {
                fprintf(stderr, "Couldn't accept: %s\n", strerror(errno));
            }
            else
            {
                it->next = calloc(1, sizeof( CBUS_conn));
                if(it->next == NULL)
                {
                    fprintf(stderr, "Out of Memory!\n");
                    exit(1);
                }
                it->next->before = it;
                it = it->next;

                it->fd = newfd;
                it->path = base_dir;
                FD_SET(it->fd, &master);
                fd_max = newfd+1;
                char name[32];
                memcpy(name, "/_unnamed/", strlen("/_unnamed/"));
                cbusd_rand_str(name + 10, 21);
                name[31] = 0;

                it->address = str_copy(name);
                if(it->address == NULL)
                {
                    fprintf(stderr, "Out of Memory!\n");
                    exit(1);
                }
                if(verbose == 1)
                {
                    fprintf(stderr, "New connection with name: %s\n", name);
                }
                char *msg = cbus_construct_msg(
                        CBUS_TYPE_FN_RETURN,
                        0,
                        "",
                        "/_daemon",
                        "",
                        "/name",
                        "s",
                        name);
                if(msg == NULL)
                {
                    fprintf(stderr, "Out of Memory!\n");
                    exit(1);
                }
                if(cbus_send_msg(it, msg) < 0)
                {
                    cbusd_disconnect(it);
                }
            }
        }

        /*Check every socket for data */
         CBUS_conn *data_it = start->next;
        while(data_it != NULL)
        {
            if(FD_ISSET(data_it->fd, &read_fds))
            {
                int err = 0;
                 CBUS_msg *cur_msg = cbus_get_msg(data_it, &err);
                if(err < 0)
                {
                    cbusd_disconnect(data_it);
                }
                if(cur_msg != NULL)
                {
                    if(cbusd_process(data_it, cur_msg) < 0)
                    {
                        cbusd_disconnect(data_it);
                        continue;
                    }
                    cbus_reset_conn(data_it);
                    cbus_free_msg(cur_msg);
                }
            }
            data_it = data_it->next;
        }
    }

    return 0;

}
void cbusd_rand_str(char *str, int len)
{
    int c = 0;
    for(;c < len;c++)
    {
        for(;;)
        {
            char x = rand() % 255;
            if(isalnum(x))
            {
                str[c] = x;
                break;
            }
        }
    }

}

int cbusd_gen_auth(char *source, char *dest)
{
    DIR *dir;
    struct dirent *entry;

    if (!(dir = opendir(source)))
    {
        return CBUS_ERR_OTHER;
    }
    if (!(entry = readdir(dir)))
    {
        return CBUS_ERR_OTHER;
    }

    do 
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }
        
        if(entry->d_type == DT_DIR || entry->d_type == DT_REG)
        {
            char *new_source = malloc(strlen(source) 
                    + strlen(entry->d_name) + 2);
            if(new_source == NULL)
            {
                return CBUS_ERR_NOMEM;
            }
            memcpy(new_source, source, strlen(source));
            new_source[strlen(source)] = '/';
            memcpy(new_source + strlen(source) + 1,
                    entry->d_name, strlen(entry->d_name) + 1);
            
            char *new_dest = malloc(strlen(dest) 
                    + strlen(entry->d_name) + 2);
            if(new_dest == NULL)
            {
                return CBUS_ERR_NOMEM;
            }
            memcpy(new_dest, dest, strlen(source));
            new_dest[strlen(dest)] = '/';
            memcpy(new_dest + strlen(dest) + 1,
                    entry->d_name, strlen(entry->d_name) + 1);

            if (entry->d_type == DT_DIR) {
                struct stat st;

                stat(new_source, &st);
                mkdir(new_dest, st.st_mode);
                chown(new_dest, st.st_uid, st.st_gid);

                cbusd_gen_auth(new_source, new_dest);
            }
            else if(entry->d_type == DT_REG)
            {
                FILE *f = fopen(new_dest, "w");
                if(f == NULL)
                {
                    perror("Couldn't create file: ");
                    exit(1);
                }
                perror("");
                char token[32];
                cbusd_rand_str(token, 32);
                printf("%p\n", f);
                fwrite(token, 1, 32, f);
                struct stat st;
                stat(new_source, &st);
                chown(new_dest, st.st_uid, st.st_gid);
                chmod(new_dest, st.st_mode);
                fclose(f);
            }
        }
    } while (entry = readdir(dir));
    closedir(dir);
}
void cbusd_disconnect( CBUS_conn *conn)
{
    if(verbose == 1)
    {
        fprintf(stderr, "conn disconnected: %s\n", conn->address);
    }
    FD_CLR(conn->fd, &master);
    conn->before->next = conn->next;
    if(conn == it)
    {
        it = conn->before;
    }
    if(conn->next != NULL)
    {
        conn->next->before = conn->before;
    }
    if(conn->address != NULL)
    {
        free(conn->address);
    }
}
int cbusd_check_name(char *name)
{
    if(!cbus_check_name(name))
    {
        if(verbose == 1)
        {
            fprintf(stderr, "The name %s does not conform to the rules\n", name);
        }
        return 0;
    }
     CBUS_conn *search_it = start->next;
    for(;search_it != NULL;search_it = search_it->next)
    {
        if(strstr(search_it->address, name) == search_it->address)
        {
            if(*(search_it->address + strlen(name)) == '/')
            {
                return 0;
            }
            else if(strcmp(search_it->address, name) == 0)
            {
                return 0;
            }
        }
    }
    return 1;
}
 CBUS_conn *cbusd_get_conn_by_address(char *address)
{
     CBUS_conn *search_it = start->next;
    for(;search_it != NULL;search_it = search_it->next)
    {
        if(strcmp(search_it->address, address) == 0)
        {
            break;
        }
    }
    return search_it;
}
int cbusd_process( CBUS_conn *sender,  CBUS_msg *msg)
{
    if(drop == 1)
    {
        cbus_print_msg(msg);
    }
    if(strcmp(msg->from, sender->address) != 0)
    {
        fprintf(stderr, "%s==%s\n", msg->from, sender->address);
        return cbus_send_err(sender, msg, CBUS_ERR_CONNECTION,
                "Wrong sender field");


    }

    if(cbus_check_auth(sender, msg) == 0)
    {
        return cbus_send_err(sender, msg, CBUS_ERR_NO_AUTH, 
                "Permission denied");
    }

    if(*msg->token != 0)
    {
        *msg->token = 0;
        memmove(msg->token + 1, msg->from,
                msg->length -  (strlen(msg->token) + (3 * sizeof(uint32_t))) - 1);
        msg = cbus_parse_msg(msg->msg);
    }
    if(msg->type == CBUS_TYPE_FN_CALL || msg->type == CBUS_TYPE_FN_RETURN
            || msg->type == CBUS_TYPE_FN_ERR)
    {
        if(fn_call_matches(msg, "/_daemon", "", ""))
        {
            if(verbose == 1)
            {
                fprintf(stderr, "The connection named %s requested a daemon method\n",
                        sender->address);
            }
            return cbusd_handle_request(sender, msg);
        }

         CBUS_conn *recipient = cbusd_get_conn_by_address(msg->to);
        if(recipient == NULL)
        {
             CBUS_conn *conn = start;
            if(verbose == 1)
            {
                fprintf(stderr, "client %s requested %s which wasn't found\n",
                        sender->address, msg->to);
            }
            return cbus_send_err(sender, msg, CBUS_ERR_NOT_FOUND, "The requested name was not found");
        }
        return cbus_send_msg(recipient, msg->msg);
    }
    if(msg->type == CBUS_TYPE_SIGNAL)
    {
         CBUS_conn *client_it = start;
        for(;client_it != NULL;client_it = client_it->next)
        {
             CBUS_sub *sub_it = client_it->subs;
            for(;sub_it != NULL;sub_it = sub_it->next)
            {
                if(signal_matches(msg, sub_it->sender_name, sub_it->signal_name, ""))
                {
                    cbus_send_msg(client_it, msg->msg);
                    break;
                }
            }
        }
    }
    return 0;
}
char *str_copy(char *str)
{
    char *result = malloc(strlen(str) + 1);
    memcpy(result, str, strlen(str) + 1);
    return result;
}
int cbusd_handle_request( CBUS_conn *sender,  CBUS_msg *msg)
{
    if(fn_call_matches(msg, "/_daemon", "/request/name", "s"))
    {
        if(verbose == 1)
        {
            fprintf(stderr, "Connection requested name %s\n", msg->args->str_value);
        }
        if(!cbusd_check_name(msg->args->str_value))
        {
            return cbus_send_err(sender, msg, CBUS_ERR_CONFLICT,
                    "The requested address already exists");
        }
        sender->address = str_copy(msg->args->str_value);
        return cbus_send_return(sender, msg, "");
    }
    else if(fn_call_matches(msg, "/_daemon","/subscribe", "ss"))
    {
        if(verbose == 1)
        {
            fprintf(stderr, "Subscribe request from %s for signal %s\n", msg->from,
                    msg->args->next->str_value);
        }
         CBUS_sub *sub_it;
        if(sender->subs == NULL)
        {
            sender->subs = calloc(1, sizeof( CBUS_sub));
            sub_it = sender->subs;
        }
        else
        {
            sub_it = sender->subs;
            for(;sub_it->next != NULL;sub_it = sub_it->next);
            sub_it->next = calloc(1, sizeof( CBUS_sub));
        }
        sub_it->sender_name = str_copy(msg->args->str_value);
        sub_it->signal_name = str_copy(msg->args->next->str_value);
        sub_it->serial = msg->serial;
        return cbus_send_return(sender, msg, "");
    }
    else
    {
        return cbus_send_err(sender, msg, CBUS_ERR_UNKNOWN_FN, "This function is not defined");
    }
    return 0;
}
void parse_args(int argc,char ** argv)
{
    int i = 1;
    if(argc == 1)
    {
        print_usage();
        exit(0);
    }
    for(;i < argc;i++)
    {
        if(strcmp(argv[i], "-b") == 0)
        {
            if(i + 1 < argc)
            {
                base_dir = argv[++i];
                socket_path = malloc(strlen(argv[i]) + strlen("/sock") + 1);
                memcpy(socket_path, argv[i], strlen(argv[i]));
                memcpy(socket_path + strlen(argv[i]), "/sock", strlen("/sock") + 1);
                rules_path = malloc(strlen(base_dir) + strlen("/rules"));
                memcpy(rules_path, base_dir, strlen(base_dir));
                memcpy(rules_path + strlen(base_dir), "/rules",
                        strlen("/rules"));
                auth_path = malloc(strlen(base_dir) + strlen("/auth"));
                memcpy(auth_path, base_dir, strlen(base_dir));
                memcpy(auth_path + strlen(base_dir), "/auth",
                        strlen("/auth"));
            }
            else
            {
                fprintf(stderr, "-b needs one positional argument\n");
                fprintf(stderr, "see -h for more information\n");
                exit(1);
            }
        }
        else if(strcmp(argv[i], "-h") == 0)
        {
            print_usage();
            exit(0);
        }
        else if(strcmp(argv[i] ,"-v") == 0)
        {
            verbose=1;
        }
        else if(strcmp(argv[i], "-d") == 0)
        {
            drop=1;
        }
        else
        {
            fprintf(stderr, "Option not recognized: %s\n", argv[i]);
            exit(1);
        }

    }
}

void print_usage()
{
    fprintf(stderr, "The CBUS-daemon %s\n",  VERSION);
    fprintf(stderr, "Usage: cbusd -b [CBUS base dir]\n"
            "Optional Arguments:\n"
            "\t-h show this help and exit\n"
            "\t-v show debugging information\n"
            "\t-d drop all packages to stderr\n"
            );
}
