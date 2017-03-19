/*
 * Description: udp event
 *     History: damonyang@tencent.com, 2013/11/25, create
 */

# pragma once

# include <stddef.h>
# include <stdint.h>
# include <sys/time.h>
# include <netinet/in.h>
# include <stdint.h>

/*
* Udp_ev based on libevent to achieve some simple and useful UDP programming interface, including:
 * 1, the server interface
 * 2, the client interface
 * 3, timing tasks
 * 4, Timer (Session with timeout mechanism)
 */

/* Packet size */
# define UE_BUFFER_SIZE UINT16_MAX

/* Socket context */
struct ue_context
{
    int                 name;
    int                 sockfd;
    time_t              create_time;
    struct sockaddr_in  client_addr;
    void                *pkg;
    size_t              pkg_len;
};

/************************Server interface*****************************/

/* UDP packets handle callback functions */
typedef int ue_handle_udp_cb(struct ue_context *uc);

/*
* Create a socket and bind to the top IP and port
 * Parameters:
 * Name: socket identifier, any int type number, can not be repeated, used to uniquely identify a port
 * Ip: IP address, if NULL, bound to INADDR_ANY
 * Port: bound port, can not be 0
 * Handler: UDP packet handling callback function, after the success of the call after the call
 *
 * Negative value indicates a failure
 */
int ue_create(int name, char const *ip, int port, ue_handle_udp_cb *handler);

/* Print out all open socket, can be used for debugging */
int ue_trace(void);

enum
{
    UE_LOG_INFO,
    UE_LOG_WARN,
    UE_LOG_ERROR,
};

/* 
* Log callback function
 * Severity that log severity level, divided into three levels: UE_LOG_INFO / UE_LOG_WARN / UE_LOG_ERROR
 * Msg for log content
 */
typedef void ue_handle_log_cb(int severity, char const *msg);

/* Set the log callback function, if not set, udp_ev does not print any error log */
int ue_set_log_callback(ue_handle_log_cb *cb);

/* Receipt callback function */
typedef void ue_handle_loop_cb(void);

/*
* Run the service to enter the main loop until ue_exit is called
 * Loop If not NULL, each time a successful UDP packet is received, call UDP packets before processing the callback function
 * Failed to return negative
 */
int ue_run(ue_handle_loop_cb *loop);

/*
* Send a UDP packet
 * Parameters:
 * Name: socket identifier
 * Addr: the address sent
 * Pkg: the starting address of the UDP packet to be sent
 * Pkg_len: UDP packet length to be sent
 *
 * Failed to return negative
 */
int ue_send(int name, struct sockaddr_in *addr, void *pkg, size_t pkg_len);

/* Exit the service and can be called in the signal processing function */
int ue_exit(void);

/* The function is returned immediately after the specified time after the first call */
int ue_exit_later(struct timeval *tv);

/******************************************************************/


/****************************Client interface****************************/

/* 
* Assign a struct sockaddr_in type structure
 * If ip is NULL, the ip value is INADDR_ANY
 * Failed to return negative
 */
int ue_addr_assign(struct sockaddr_in *addr, char const *ip, int port);

/*
* Create a socket and bind it to the top ip and port
 * If ip is NULL, bind to INADDR_ANY
 * If port is 0, a port is specified by the operating system
 * Failed to return NULL
 */
struct ue_context *ue_create_context(char const *ip, int port);
/*
* Send a UDP packet through the specified socket
 * Failed to return negative
 */
int ue_send_by_context(struct ue_context *uc, struct sockaddr_in *addr, void *pkg, size_t pkg_len);

/*
* Receive a UDP packet from the specified socket with a timeout function
 * Parameters:
 * Uc: socket
 * Buf: receive buffer start address
 * Buf_size: packet buffer length
 * Timeout: timeout time, if it is NULL does not timeout
 * return value:
 * <0: failed
 * == 0: timeout
 *> 0: success, received package and package saved in uc
 */
int ue_recv_by_context(struct ue_context *uc, void *buf, size_t buf_size, struct timeval *timeout);

/*Close a socket */
int ue_close_context(struct ue_context *uc);

/******************************************************************/


/**************************Timing task********************************/

/* Timing task callback function */
typedef void ue_handle_cron_cb(void);

/*
 * Set a timed task, interval for the interval
 * Failed to return negative
 */
int ue_cron(struct timeval *interval, ue_handle_cron_cb *cron);

/******************************************************************/


/************************ Timer ***********************************/

/*Timer context */
struct ue_timer
{
    struct timeval  timeout;
    size_t          session_size;
};

/* Timer timeout callback function */
typedef void ue_timeout_cb(void *session);

/*
* Create a timer, each timer has the same timeout, session size and timeout callback function
 * Users in the timer on the basis of add, get, del operation
 * Parameters:
 * Timeout: timeout time
 * Session_size: session size
 * Timeout_callback: timeout callback function
 *
 * Returns NULL to indicate that the creation failed
 */
struct ue_timer *ue_timer_create(struct timeval *timeout, \
        size_t session_size, ue_timeout_cb *timeout_callback);

/*
* Add a node on the specified timer to return the address of the session.
 * If the parameter session is not NULL, the session will be copied to the actual session, otherwise
 * Initialize the actual session to 0.
 * * Sequence is the serial number of the node, the serial number is a non-zero integer.
 */
void *ue_timer_add(struct ue_timer *ut, void *session, uint32_t *sequence);

/* Get the session address of a node based on the serial number*/
void *ue_timer_get(uint32_t sequence);

/* Delete a node by serial number */
void ue_timer_del(uint32_t sequence);

/* Returns the number of nodes in the specified timer */
size_t ue_timer_num(struct ue_timer *ut);

/* Returns the serial number of the specified session */
uint32_t ue_timer_sequence(void *session);

/* Returns the timer at which the session is specified */
struct ue_timer *ue_timer_which(void *session);

/******************************************************************/

