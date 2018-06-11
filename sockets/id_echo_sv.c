/*
 * id_echo_sv.c
 * 
 * This program implements a daemon that provides the UDP "echo"
 * service. It reads datagrams and then sends copies back to
 * to the originating address.