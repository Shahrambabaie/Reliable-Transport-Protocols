#include "../include/simulator.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
/* ******************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose

   This code should be used for PA2, unidirectional data transfer 
   protocols (from A to B). Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets will be delivered in the order in which they were sent
     (although some can be lost).
**********************************************************************/

/********* STUDENTS WRITE THE NEXT SIX ROUTINES *********/

/* called from layer 5, passed the data to be sent to other side */

struct message_node {
    char data[20];
    struct message_node *next;
};


struct message_node *head = NULL;
struct pkt curr_packet;
int packet_in_transit = 0;
int a_seq_num = 0;
int b_seq_num = 0;

int calc_chksum(char data[], int seq_num, int ack_num){
  int checksum = 0;
  for(int i=0;i<20;i++){
    checksum += (unsigned char)data[i];
  }
  checksum += seq_num + ack_num;
  return checksum;
}



int add_to_buffer(struct msg *message){
  struct message_node *new = malloc(sizeof(struct message_node));
  if(new == NULL){
    printf("out of mem\n");
    return 0;
  }
  memcpy(new->data, message->data, 20);
  new->next = NULL;
  if (head == NULL){
      head = new;
      return 0;
  }
  struct message_node *temp = head;
  while(temp->next != NULL)
      temp = temp->next;
  temp->next = new;
  return 0;
}

void pop_buffer(){
    if (head==NULL){
      printf("head null\n");
      return;
    }
    struct message_node *temp = head;
    head = head->next;
    free(temp);
    return;
}

void send_next_packet(){
  struct message_node *msg_to_send = head;
  if (head == NULL || head->data == NULL){
    printf("no messages to send\n");
    return;
  }
  struct pkt packet;
  memcpy(packet.payload, msg_to_send->data, 20);
  packet.seqnum = a_seq_num;
  packet.acknum = 123;
  packet.checksum = calc_chksum(packet.payload, packet.seqnum, packet.acknum);
  tolayer3(0, packet);
  starttimer(0, 20.0);
}

void A_output(message)
  struct msg message;
{ 
  add_to_buffer(&message);
  if (packet_in_transit == 1){
    return;
  }
  packet_in_transit = 1;
  send_next_packet();
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(packet)
  struct pkt packet;
{
  if (packet.checksum != calc_chksum(packet.payload, packet.seqnum, packet.acknum)){
    return;
  }
  if (packet.acknum != a_seq_num) {
    return;
  }
  a_seq_num = (a_seq_num+1)%2;
  packet_in_transit=0;
  stoptimer(0);
  pop_buffer();
  send_next_packet();
}



/* called when A's timer goes off */
void A_timerinterrupt()
{
  if(packet_in_transit == 1){
    send_next_packet();
    starttimer(0, 20.0);
  }
}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{

}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(packet)
  struct pkt packet;
{ 
  if (packet.checksum != calc_chksum(packet.payload, packet.seqnum, packet.acknum))
    return; 
  packet.acknum = packet.seqnum;
  packet.checksum = calc_chksum(packet.payload, packet.seqnum, packet.acknum);
  if (packet.seqnum == b_seq_num){
    tolayer5(1, packet.payload);
    b_seq_num = (b_seq_num +1)%2;
  }
  tolayer3(1, packet);
}

/* the following routine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{

}


