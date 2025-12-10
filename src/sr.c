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
struct packet_window{
  struct pkt packet;
  float timer_start;
};

struct packet_window *packets_in_window = NULL;
struct packet_window *b_packets_in_window = NULL;
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
  if (head == NULL || head->data == NULL){
    printf("no messages to send\n");
    return;
  }
  if(num_packets_in_window >= window_size){
    printf("Window full\n");
    return;
  }
  memcpy(packets_in_window[window_tail_ptr].packet.payload, head->data, 20);
  packets_in_window[window_tail_ptr].packet.seqnum = a_seq_num;
  packets_in_window[window_tail_ptr].packet.acknum = 123;
  packets_in_window[window_tail_ptr].packet.checksum = calc_chksum(packets_in_window[window_tail_ptr].packet.payload, a_seq_num, 123);
  pop_buffer();
  tolayer3(0, packets_in_window[window_tail_ptr].packet);
  a_seq_num++;
  packets_in_window[window_tail_ptr].timer_start = get_sim_time();
  window_tail_ptr = (window_tail_ptr+1)%window_size;
  num_packets_in_window++;
  starttimer(0, 100.0);
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
  if (packet.acknum != packets_in_window[window_head_ptr].packet.seqnum) {
    return;
  }
  num_packets_in_window--;
  window_head_ptr = (window_head_ptr+1)%window_size;
  send_next_packet();
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
  for(int i=0;i<num_packets_in_window;i++){
    if (get_sim_time() - packets_in_window[(window_head_ptr+i)%window_size].timer_start >= 50.0){
      tolayer3(0, packets_in_window[(window_head_ptr+i)%window_size].packet);
      starttimer(0, 100.0);
      packets_in_window[(window_head_ptr+i)%window_size].timer_start = get_sim_time();
    }
  }
}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
   window_size = getwinsize();
  packets_in_window = malloc(sizeof(struct packet_window)*window_size);
  if (packets_in_window == NULL){
    printf("gone\n");
    return;
  }  
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */
int b_window_head = 0;
int b_window_tail = 0;
int last_acked_packet=-1;
int b_packets_in_wind = 0;
/* called from layer 3, when a packet arrives for layer 4 at B*/

void B_input(packet)
  struct pkt packet;
{
  if (packet.checksum != calc_chksum(packet.payload, packet.seqnum, packet.acknum))
    return; 
  //printf("%d, %d\n", packet.seqnum, );
  if (packet.seqnum == last_acked_packet+1){

    packet.acknum = packet.seqnum;
    packet.checksum = calc_chksum(packet.payload, packet.seqnum, packet.acknum);
    tolayer3(1, packet);
    tolayer5(1, packet.payload);
    last_acked_packet++;
    while(b_packets_in_window[b_window_head].packet.seqnum == last_acked_packet+1 && b_packets_in_wind>0){
      tolayer3(1, b_packets_in_window[b_window_head].packet);
      tolayer5(1, b_packets_in_window[b_window_head].packet.payload);
      last_acked_packet++;
      b_window_head = (b_window_head+1)%window_size;
      b_packets_in_wind--;
      if (b_packets_in_wind<=0)
        break;
    }
  }
  else{
    b_packets_in_window[b_window_tail].packet.seqnum = packet.seqnum;  
    b_packets_in_window[b_window_tail].packet.acknum = packet.seqnum;
    b_packets_in_window[b_window_tail].packet.checksum = calc_chksum(packet.payload, packet.seqnum, packet.seqnum);
    memcpy(b_packets_in_window[b_window_tail].packet.payload, packet.payload, 20);
    tolayer3(1, b_packets_in_window[b_window_tail].packet);
    b_window_tail = (b_window_tail+1)%window_size;
    b_packets_in_wind++;
  }
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
  window_size = getwinsize();
  b_packets_in_window = malloc(sizeof(struct packet_window)*window_size);
  if (b_packets_in_window == NULL){
    printf("gone\n");
    return;
  }
}
