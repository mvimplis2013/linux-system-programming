#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>

#define SERVER_PORT 12345

#define TRUE    1
#define FALSE   0

void 
main(int argc, char *argv[]) 
{
    int i, len, rc, on = 1;
    int listen_sd, max_sd, new_SD;
    int desc_ready, end_server = FALSE;
    int close_conn;
    char buffer[80];
    struct sockaddr_in addr;
    struct timeval timeout;
    struct fd_set master_set, working_set;

    /*******************************************************
     * Create an AF_INET stream socket to receive incoming *
     * connections on.                                     * 
     * *****************************************************/
    listen_sd = socket(AF_INET, SOCKET_STREAM, 0);
    if (listen_sd < 0) {
        perror("socket() failed");
        exit(-1);
    }

    /*******************************************************
     * Allow socket descriptor to be reusable              *
     * *****************************************************/
    rc = setsockopt(listen_sd, SOL_SOCKET, SO_REUSABLE, 
            (char *) &on, sizeof(on));
    if (rc < 0) {
        perror("setsockopt() failed");
        close(listen_sd);
        exit(EXIT_FAILURE);
    }

    /**********************************************************
     * Set socket to be nonblocking. All of the sockets for   * 
     * incomming connections will also ne nonblocking since   *
     * they will inherit that state from the listening socket *
     * ********************************************************/
    rc = ioctl(listen_sd, FIONBIO, (char *) &on);
    if (rc < 0) {
        perror("ioctl failed");
        close(listen_sd);
        exit(EXIT_FAILURE);
    }

    /**********************************************************
     * Bind the socket                                        *
     * ********************************************************/
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(SERVER_PORT);
    rc = bind(listen_sd, (struct sockaddr *) &addr, sizeof(addr));
    if (rc < 0) {
        perror("bind failed");
        close(listen_sd);
        exit(EXIT_FAILURE);
    }

    /*********************************************************
     * Set the listen back log                               *
     * *******************************************************/
    rc = listen(listen_sd, 32);
    if (rc < 0) {
        perror("listen failed");
        close(listen_sd);
        exit(EXIT_FAILURE);
    }

    /*********************************************************
     * Initialize the master fd_set                          *
     * *******************************************************/
    FD_ZERO(&master_set);
    max_sd = listen_sd;
    FD_SET(listen_sd, &master_set);

    /*********************************************************
     * Initialize the timval struct to 3 minutes. If no      *
     * activity after 3 minutes this program will end.       *
     * *******************************************************/
    timeout.tv_sec = 2 * 60;
    timeout.tv_usec = 0;

    /**********************************************************
     * Loop waiting for incomming connects of for incomming   *
     * data on any of the connected sockets.
     * ********************************************************/
    do {
        /******************************************************
         * Copy the master fd_set over to the working fd_set. *
         * ****************************************************/
        memcpy(&workin_set, &master_set, sizeof(master_set));

        /******************************************************
         * Call select() and wait for 5 minutes for it to     *
         * complete.                                          *
         * ****************************************************/
        printf("Waiting on select() ...\n");
        rc = select(max_sd+1, &working_set, NULL, NULL, &timeout);

        /******************************************************
         * Check to see if the select call failed.            *
         * ****************************************************/
        if (rc < 0) {
            perror("    select failed");
            break;
        }

        /******************************************************
         * One or more descriptors are readable. Need to      *
         * determine which ones they are.                     *
         * ****************************************************/
        desc_ready = rc;
        for (i=0; i<=max_sd && desc_ready >0; i++) {
            if (FD_ISSET(i, &working_set)) {
                /**********************************************
                 * A descriptor was found that was readable - *
                 * one less has to be looked for. This is     *
                 * being done so that we can stop looking at  *
                 * the working set once we have found all of  *
                 * the descriptors that were ready            *
                 * ********************************************/
                desc_ready -= 1;

                if (i==listen_sd) {
                    printf("    Listening socket is readable\n");

                    /******************************************
                     * Accept all incoming connections that are
                     * queued up on the listening socket before
                     * we loop back and call select again.
                     * ****************************************/

                    do {
                        /**************************************
                         * Accept each incoming connection. If
                         * accept fails with EWOULDBLOCK, then 
                         * we have accepted all of them. Any 
                         * other failure on accept will cause us
                         * to end the server.
                         * *************************************/
                        new_sd = accept(listen_sd, NULL, NULL);
                        if (new_sd < 0) {
                            if (errno != EWOULDBLOCK) {
                                perror("    accept failure");
                                end_server = TRUE;
                            }
                            break;
                        }

                        /******************************************
                         * Add the new incoming connection to the *
                         * master read set.                       *
                         * ****************************************/
                        printf("    New incoming connection - %d\n", 
                            new_sd);
                        FD_SET(new_sd, &master_set);
                        if (new_sd > max_sd) {
                            max_sd = new_sd;
                        } 

                        /******************************************
                         * Loop back up and accept another incoming
                         * connection.
                         * ****************************************/
                    } while (new_sd != -1);
                } else {
                    /* *******************************************
                     * This is not the lsitening socket, therefore
                     * an existing connection must be readable.
                     * *******************************************/
                    printf("    Descriptor %d is readable\n", i);
                    close_conn = FALSE;

                    /********************************************
                     * Receive all incoming data on this socket
                     * before we loop back and call select again
                     * ******************************************/
                    do {
                        /****************************************
                         * Receive data on this connection until 
                         * the recv fails with EWOULDBLOCK. If any
                         * other failure occurs, the connection will
                         * be closed.
                         * ***************************************/
                        rc = recv(i, buffer, sizeof(buffer), 0);
                        if (rc < 0) {
                            if (errno != EWOULDBLOCK) {
                                perror("    recv() failed");
                                close_conn = TRUE;
                            }
                            break;
                        }

                        /*******************************************
                         * Check to see if the connection has been 
                         * closed by the client.
                         * *****************************************/
                        if (rc == 0) {
                            printf("    Connection closed\n");
                            close_conn = TRUE;
                            break;
                        }

                        /******************************************
                         * Data was received
                         * ****************************************/
                        len = rc;
                        printf("    %d bytes received\n", len);

                        /******************************************
                         * Echo the data back to the client
                         * ****************************************/
                        rc = send(i, buffer, len, 0);
                        if (rc < 0) {
                            perror("    send() failed");
                            close_conn = TRUE;
                            break;
                        }
                    } while(TRUE);

                    /*******************************************
                     * If the close_conn flag was turned on, we 
                     * need to clean up this active connection.
                     * This clean up process includes removing 
                     * the descriptor from the master set and 
                     * determining the new maximum descriptor 
                     * value based on the bits turned on in 
                     * master set.
                     * *****************************************/
                    if (close_conn) {
                        close(i);
                        FD_CLR(i, &master_set);
                        if (i == max_sd) {
                            while (FD_ISSET(max_sed, &master_set) == FALSE) {
                                max_sd -= 1;
                            }
                        }
                    }
                    
                } /* End of existing connection is readabble */
            } /* End of if (FD_ISSET()) */
        } /* End of loop through selectable descriptors */

    } while (ebd_server == FALSE);

    /********************************************************
     * Clean up all of the sockets that are open
     * ******************************************************/
    for (i=0; i<=max_sd; i++) {
        if (FD_ISSET(i, &master_set)) {
            close(i);
        }
    }     
}
