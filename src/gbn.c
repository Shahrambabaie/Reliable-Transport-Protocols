#include "../include/simulator.h"
#include<stdlib.h>
#include<stdio.h>
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

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/
int window_size = 0;
struct pkt *packets_in_window = NULL;
struct message_node {
    char data[20];
    struct message_node *next;
};


struct message_node *head = NULL;

int num_packets_in_window = 0;
int window_head_ptr = 0;
int window_tail_ptr = 0;
int is_timer_active = 0;
int window_full = 0;
int a_seq_num = 0;
int b_seq_num = 0;

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


int calc_chksum(char data[], int seq_num, int ack_num){
  int checksum = 0;
  for(int i=0;i<20;i++){
    checksum += (unsigned char)data[i];
  }
  checksum += seq_num + ack_num;
  return checksum;
}

void send_next_packet(){
  if (head == NULL || head->data == NULL){
    printf("no messages to send\n");
    return;
  }
  if(num_packets_in_window >= window_size){
    printf("Window full\n");
    return;
  }
  memcpy(packets_in_window[window_tail_ptr].payload, head->data, 20);
  packets_in_window[window_tail_ptr].seqnum = a_seq_num;
  packets_in_window[window_tail_ptr].acknum = 123;
  packets_in_window[window_tail_ptr].checksum = calc_chksum(packets_in_window[window_tail_ptr].payload, a_seq_num, 123);
  pop_buffer();
  tolayer3(0, packets_in_window[window_tail_ptr]);
  a_seq_num++;
  window_tail_ptr = (window_tail_ptr+1)%window_size;
  num_packets_in_window++;
  if(is_timer_active == 0){
    starttimer(0, 50.0);
    is_timer_active = 1;
  }
}


/* called from layer 5, passed the data to be sent to other side */
void A_output(message)
  struct msg message;
{
  add_to_buffer(&message);
  if(num_packets_in_window >= window_size){
    return;
  }
  send_next_packet();
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(packet)
  struct pkt packet;
{
  if (packet.checksum != calc_chksum(packet.payload, packet.seqnum, packet.acknum)){
    return;
  }
  if (packet.acknum != packets_in_window[window_head_ptr].seqnum) {
    return;
  }
  num_packets_in_window--;
  stoptimer(0);
  window_head_ptr = (window_head_ptr+1)%window_size;
  is_timer_active = 0;
  if (num_packets_in_window > 0){
    starttimer(0, 50.0);
    is_timer_active = 1;
  }
  send_next_packet();
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
  for (int i=0;i<num_packets_in_window;i++){
    tolayer3(0, packets_in_window[(window_head_ptr + i)%window_size]);
  }
  starttimer(0, 50.0);
}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
  window_size = getwinsize();
  packets_in_window = malloc(sizeof(struct pkt)*window_size);
  if (packets_in_window == NULL){
    printf("gone\n");
    return;
  }
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
    b_seq_num++;
  }
  tolayer3(1, packet);
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{

}
