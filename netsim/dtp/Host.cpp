#include "../netsim/common.h"
#include "../netsim/Node.h"
#include "../netsim/FIFONode.h"
#include "../netsim/Packet.h"
#include "DTPPacket.h"
#include "../netsim/PacketScheduler.h"
#include "../netsim/Timer.h"
#include "../netsim/Scheduler.h"
#include "Host.h"
#include "Router.h"
#include <iostream>
#include <fstream>
#include <string.h>
#include <string>
#include <math.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>





#define MAX_QUEUE 99999
#define timeout 4000
#define K 4
#define G 0.5
#define alpha 0.125
#define beta 0.25
#define Initial_Window 1
#define Initial_Window_Threshold 1000





Host::Host(Address a) : FIFONode(a,MAX_QUEUE)	// Null queue size
{
        sync_bit=1;
        term_bit=0;
        retrans=0;
        retrans_bit=0;
        packets_in_window=0;
        window_size=Initial_Window;
        Window_threshold=Initial_Window_Threshold;
        ter_seq=MAX_QUEUE;
        sent_so_far=0;
        recv_so_far=0;
        RTO=timeout;
        ECN=0;
        flag=0;
        last_ack=0;
        sent_so_far=0;
        recv_so_far=0;
        RTT=0;
        sent_window.clear();
        recv_window.clear();
        sender=0;
        ACKcount_flag=0;
        alpha1=0;
        File_size=0;
        TRACE(TRL3,"Initialized host with address %d\n",a);// Empty
}

Host::~Host()
{
    // Empty
}
void
Host::reset()
{
	sync_bit=1;
        term_bit=0;
        retrans=0;
        retrans_bit=0;
        packets_in_window=0;
        window_size=Initial_Window;
        Window_threshold=Initial_Window_Threshold;
        ter_seq=MAX_QUEUE;
        RTO=timeout;
        ECN=0;
        flag=0;
        last_ack=0;
        sent_so_far=0;
        recv_so_far=0;
        RTT=0;
        sent_window.clear();
        recv_window.clear();
        sender=0;
        ACKcount_flag=0;
        alpha1=0;
        //File_size=0;
        
}
void
Host::get_dest(Packet* pkt)
{
	destination=((DTPPacket*)pkt)->source;
}
void
Host::receive(Packet* pkt)
{
   	TRACE(TRL3,"Received packet at DTP-Host: %d\n",address());
	((DTPPacket*)pkt)->print();
	//((DTPPacket*)pkt)->print_header();
	//cout<<term_bit<<endl;
	write=0;
	flag=0;
	ECN=0;
	ECN=((DTPPacket*)pkt)->ECN;
	
	if(!sender)
	{
	if(((DTPPacket*)pkt)->id==recv_so_far+1)
	{
	        recv_so_far+=1;
	        write=1;
	}
	}
	else
        	recv_so_far=((DTPPacket*)pkt)->id;
	
	if(term_bit==0)
	{
		if (((DTPPacket*)pkt)->sync_bit==1&&sync_bit==1)
		{
			get_dest(pkt);
			sync_bit=2;
			TRACE(TRL4,"(Time:%d) node %d Got SYN from node %d\n",scheduler->time(),address(),destination);
			handle_timer((void*)1);
			retrans=scheduler->time() +RTO;
			set_timer(retrans, NULL);
			TRACE(TRL4,"Next Retransmit at DTP-Host %d is set at Time: %d\n",address(),retrans);
			//cout<<"Next Retransmit:"<<retrans<<endl;
			retrans_bit=1;
			TRACE(TRL4,"(Time:%d) node %d Sent SYN-ACK to node %d\n",scheduler->time(),address(),destination);
			
		}
		else if (((DTPPacket*)pkt)->sync_bit==2&&sync_bit==3)
		{
			handle_timer((void*)1);
		
		}
		else if (((DTPPacket*)pkt)->sync_bit==2&&sync_bit==1)
		{
			if(retrans>=scheduler->time())
			{
				cancel_timer(retrans, NULL);
				retrans_bit=0;
			}
			get_dest(pkt);	
			sync_bit=3;
			RTT=(scheduler->time()-last_transmit);
			RTTVAR=RTT/2;
			RTO = (RTT + K*RTTVAR);
			last_ack=2;
			TRACE(TRL4,"(Time:%d) node %d Got SYN-ACK from node %d\n",scheduler->time(),address(),destination);
			flag=1;
			//fprintf(stderr,"Got SYN, node %d, %d %d\n",address(),RTO,last_transmit);
			//fprintf(stderr,"Got SYN-ACK, node %d\n",address());	
			handle_timer((void*)1);
			TRACE(TRL4,"(Time:%d) node %d Sent ACK to node %d\n",scheduler->time(),address(),destination);
			send_file();
			
			//terminate(destination);
			
			
		
		}
		else if (((DTPPacket*)pkt)->sync_bit==3&&sync_bit==2)
		{
			//fprintf(stderr,"Got ACK, node %d\n",address());
			if(retrans>=scheduler->time())
			{
				cancel_timer(retrans, NULL);
				retrans_bit=0;
				retrans=0;
			}
			sync_bit=3;
			get_dest(pkt);
			Time tm=scheduler->time();
			TRACE(TRL4,"(Time:%d) node %d Got ACK from node %d\n",scheduler->time(),address(),destination);
			    
			TRACE(TRL3, "Established FDTP flow from %d to %d (%d)\n",  destination,address(),tm);
			flag=1;
			if(((DTPPacket*)pkt)->id>recv_so_far)
	                {
	                        flag=0;
	                        recv_so_far+=1;
	                        if(((DTPPacket*)pkt)->id==recv_so_far+1)
                                {
                                        recv_so_far+=1;
	                                write=1;
	                        }
	                        //handle_timer((void*)1);
	                }
			//out_file.open("example.txt", ios::out);
			//term_bit=1;
				
			/* 	Node* nd = (scheduler->get_node)(destination);
			((Host*)nd)->terminate(address()); */
			
		}

	
	}
	
	if(sync_bit==3&&!flag)
	{			
	        if(((DTPPacket*)pkt)->FIN==0)
		{
		
		
		if(!sender)
		{
		       // cout<<"khg"<<endl;
		        if(sync_bit==2)
		        {
		                sync_bit=3;
		                if(retrans>=scheduler->time())
			        {
			        	cancel_timer(retrans, NULL);
				        retrans_bit=0;
			        	retrans=0;
			        }
			 }
			 recv_window_sync((DTPPacket*)pkt);
		}
		else
		{
		             
	                //cout<<"jddjdkdkd"<<endl;
	                sent_window_sync((DTPPacket*)pkt);
		        send_file();
		}
		/* 		if(retrans>=scheduler->time())
		{
				cancel_timer(retrans, NULL);
				retrans_bit=0;
		} */
		//fprintf(stderr,"Dropped, node %d\n",address());
		}
		else if(((DTPPacket*)pkt)->FIN==1&&((DTPPacket*)pkt)->ACK==0)
		{
		     
		     if(!sender)
		     {
		        if(term_bit!=2)
		        {
		        if(((DTPPacket*)pkt)->id==recv_so_far)
		        {
		                DTPPacket*	pkt1 = new DTPPacket;
                                pkt1->source = address();
                                pkt1->destination = destination;
                                pkt1->length = HEADER_SIZE;
                                if(write)
                                        sent_so_far+=1;
                                pkt1->id = sent_so_far;
                              	pkt1->ack_id =recv_so_far;
                        	pkt1->sync_bit=sync_bit;
                        	pkt1->FIN=1;
                                pkt1->ACK=1;
                                pkt1->ECN=0;
                                pkt1->ECN1=ECN;
                                pkt1->file_size=File_size;
        	                if (send(pkt1)) 
                                {
                                   TRACE(TRL3,"Sent packet from DTP-Host: %d\n",address());
	                           pkt1->print();//_header();
	                         }
	                        TRACE(TRL4,"(Time:%d) node %d Sent FIN-ACK to node %d\n",scheduler->time(),address(),destination);
	                        //term_bit=2;
	                        //terminate(destination);
		        }
		        else
		        {
		                term_bit=1;
		                ter_seq=((DTPPacket*)pkt)->id;
		        }
		        }
		      }
		      else
		      {
		        sent_window_sync((DTPPacket*)pkt);
		      }
		     
                }
                else if(((DTPPacket*)pkt)->FIN==1&&((DTPPacket*)pkt)->ACK==1)
	        {
	            // cout<<"aaa"<<endl;
	            
	             if(sender)
		     {
		         if(term_bit==1)
		         {
		                term_bit=2;
		                sent_window_sync((DTPPacket*)pkt);
		                TRACE(TRL3, "Tore down FDTP flow from %d to %d (%d)\n", address(),destination,scheduler->time());
		                Node* nd = (scheduler->get_node)(destination);
		                ((Host*)nd)->term_bit=2;
		                 //cout<<"ccc"<<term_bit<<endl;
		               // sent_window_sync((DTPPacket*)pkt);
		                
		          }
		       }
		 }
        }     
	
	
   // if(pkt)
    delete pkt;
} 


void
Host::handle_timer(void* cookie)
{
    
	DTPPacket*	pkt = new DTPPacket;
	//if(*cookie==1)
	//cout<<"Prad:"<<(int*)cookie<<endl;
	//New Connection Setup//
	if((sync_bit==1&&term_bit==0)||(sync_bit==3&&term_bit==2))
	{
	        SendMap_iter head = dest_map.find(scheduler->time());
		if(head!=dest_map.end())
		{
		if(retrans==0)
		{	sync();
			last_transmit=scheduler->time();
			retrans=scheduler->time() +RTO;
			set_timer(retrans, NULL);
			TRACE(TRL4,"Next Retransmit at DTP-Host %d is set at Time: %d\n",address(),retrans);
			retrans_bit=1;
			TRACE(TRL4,"(Time:%d) node %d Sent SYN to node %d\n",scheduler->time(),address(),destination);
		}
		}
	}
        
        //Retransmit of Data Packet//
        if(sync_bit==3)
	{
		if(retrans_bit==2)
		{	
			//cout<<"kk";
			//last_transmit=scheduler->time();
			if(retrans==scheduler->time())
			{
			retrans=scheduler->time() +RTO; ////Implement Exponential Backoff////
			set_timer(retrans, NULL);
			TRACE(TRL4,"Next Retransmit at DTP-Host %d is set at Time: %d\n",address(),retrans);
			retrans_bit=2;
			//cout<<"kk";
			copy_pkt(pkt,retransmit_pkt);
			pkt->ack_id =recv_so_far;
			pkt->ECN=0;
			pkt->ECN1=ECN;
			pkt->file_size=File_size;
			if (send(pkt)) 
			{
                                        TRACE(TRL3,"Retransmit packet from DTP-Host: %d\n",address());
	                                pkt->print();//_header();
	                }
	                
	                Window_threshold=window_size/2;
	                window_size=1;
	                TRACE(TRL3,"Current Window Size at DTP-Host: %d is %f\n",address(),window_size);
	                }
	                //cout<<"kk";
	                goto timer_end;
		}
	}
        
        
        //char* d = &(pkt->data[0]);
        // pkt->FIN=term_bit;
        pkt->FIN=0;
        pkt->ACK=0;
        pkt->ECN=0;
        pkt->ECN1=ECN;
        if(term_bit==2&&!sender)
        {
                pkt->FIN=1;
                pkt->ACK=1;
        }
        pkt->source = address();
        pkt->destination = destination;
        pkt->length = HEADER_SIZE;
        pkt->file_size = File_size;
        sent_so_far+=1;
        
        //Retransmit of Connection setup Packet
	if(retrans==scheduler->time()&&retrans_bit==1)
	{
                sent_so_far-=1;
		retrans=scheduler->time() +RTO;
		set_timer(retrans, NULL);
		TRACE(TRL4,"Next Retransmit at DTP-Host %d is set at Time: %d\n",address(),retrans);
		TRACE(TRL3,"RETRANSMIT");
		pkt->id = sent_so_far;
	        pkt->ack_id =recv_so_far;
         	pkt->sync_bit=sync_bit;
                if (send(pkt)) 
                {
                       TRACE(TRL3,"Retransmit packet from DTP-Host: %d\n",address());
                       pkt->print();//_header();
                }
                goto timer_end;
	} 
	
	
	//For Connection Setup and Acks
	
	//fprintf(stderr,"\n %d",sent_so_far)
	pkt->id = sent_so_far;
	pkt->ack_id =recv_so_far;
 	pkt->sync_bit=sync_bit;
        if (send(pkt)) 
        {
                TRACE(TRL3,"Sent packet from DTP-Host: %d\n",address());
	        pkt->print();//_header();
        }
	
	timer_end: return;
}



void
Host::sync()
{
	SendMap_iter head = dest_map.find(scheduler->time());
	sendpair* newpair=(*head).second;
	destination = newpair->a;
        dest_map.erase(head);
	
	//cout<<newpair->name<<endl;
	in_file.open (newpair->name, ios::in); 
	struct stat filestatus;
        stat(newpair->name, &filestatus );
        TRACE(TRL4,"The new file to be transfered from DTP-Host %d is: %s (Size: %ld bytes)\n",address(),newpair->name,filestatus.st_size);
        //cout << "File Size: "<<filestatus.st_size << " bytes\n";

        File_size=filestatus.st_size;
        cout << "File Size: "<<File_size << " bytes\n"<<endl;
        Node* nd = (scheduler->get_node)(destination);
	char name[99999];
	strcpy(name,newpair->name);
	strcat(name,"_dest");
        strcpy((((Host*)nd)->out_file),name);
	ofstream file1(name, ios::out);
	file1.close();
	reset();
	((Host*)nd)->reset();
	sender=1;
	
	/*sync_bit=1;
	term_bit=0;
	retrans=0;
	retrans_bit=0;
	((Host*)nd)->sync_bit=1;
	((Host*)nd)->term_bit=0;
	((Host*)nd)->retrans=0;
	((Host*)nd)->retrans_bit=0;
	flag=0;
	((Host*)nd)->flag=0;
    //sent_so_far = 0;*/
}





void
Host::send_file()
{
        char line[PAYLOAD_SIZE];
        char c;
        int i=0;
	if (in_file.is_open())
        {       
        while ( in_file.good()&&!in_file.eof())
        {       
                if(packets_in_window>=window_size)
                {    
                        goto ss;
                }
                else
                {
                        packets_in_window++;
                }
                i=0;
	        while (!in_file.eof())
                {
	                line[i]=in_file.get();
	                i++;
	                if(i==PAYLOAD_SIZE-1)
	                        break;
	        }
	        if(in_file.eof())
	                i-=1;
	        line[i]='\0';     
                DTPPacket* pkt= new DTPPacket;
                DTPPacket* pkt11= new DTPPacket;
                char* d;

                sent_so_far+=1;
                d = &(pkt->data[0]);
                strcpy(d,line);
	        //cout<<d<<endl;
	        //pkt->FIN=term_bit;
                pkt->FIN=0;
                pkt->ACK=0;
                pkt->ECN=0;
		pkt->ECN1=ECN;
		pkt->file_size=File_size;
		//cout << "File Size: "<<File_size << " bytes\n"<<endl;
                pkt->source = address();
                pkt->destination = destination;
                pkt->length = HEADER_SIZE+i;
	        //fprintf(stderr,"\n %d",sent_so_far)
	        pkt->id = sent_so_far;
	        pkt->ack_id =recv_so_far;
 	        pkt->sync_bit=sync_bit;
 	        copy_pkt(pkt11,pkt);
 	        if (send(pkt)) 
                {
                        TRACE(TRL3,"Sent packet from DTP-Host: %d\n",address());
	                pkt->print();//_header();
	        }
	        {
	                /*int sss=(pkt11->id);
	                cout<<sss<<endl;
	                Window1_iter head2;
	                DTPPacket* pkt1;
	                if(!sent_window.empty())
	                {
	                head2 =sent_window.begin();
	                pkt1 = (*head2).second;
	                 cout<<"aaaa"<<endl<<endl;
	                pkt1->print_header();
	                }*/
	                Window1Pair entry(pkt11->id,pkt11);
	                sent_window.insert(entry); 
	                id_timePair entry_time(pkt11->id,scheduler->time());
	                send_time.insert(entry_time) ;
	                /*head2 =sent_window.begin();
	                pkt1 = (*head2).second;
	                 cout<<"aaaa"<<endl<<endl;
	                pkt1->print_header();*/
	        } 
        }
        in_file.close();
        //terminate(destination);
        
        
        }
        if(packets_in_window<window_size&&term_bit==0)
        {    
                terminate(destination);
        }
        ss:     Window1_iter head1 =sent_window.begin();
	        if(!sent_window.empty())
	        {
	                retransmit_pkt=(*head1).second;
	                //cout<<"NEW RETRANS PACKET:"<<endl<<endl;
	                //retransmit_pkt->print_header();
	        }
	        else
                        retransmit_pkt=NULL;
        if(retrans_bit==0&&retransmit_pkt!=NULL)
	{
	        last_transmit=scheduler->time();
	        retrans_bit=2;
	       	retrans=scheduler->time() +RTO;
	       	set_timer(retrans, NULL);
	       	TRACE(TRL4,"Next Retransmit at DTP-Host %d is set at Time: %d\n",address(),retrans);
	}
	//display();		        
        return;
}

void Host:: recv_window_sync(DTPPacket* pkt)
{
      	DTPPacket* pkt11=new DTPPacket;
        copy_pkt(pkt11,pkt);//cout<<"aaaaaa"<<endl;
      	if((pkt11->id)<recv_so_far)
	{
	        //handle_timer((void*)1);
	}
	else
	{
	Window1_iter head = recv_window.find(pkt11->id);
      	ofstream file1(out_file, ios::out | ios::app);
	if(head!=recv_window.end())
	{
	        //handle_timer((void*)1);
	}
	else if((pkt11->id)==recv_so_far&&write)
	{
	       
	        //cout<<"aa:"<<pkt11->data<<endl;
	        if (file1.is_open()&&file1.good())
	        {
	               // cout<<"hh"<<endl;
	                file1<<pkt11->data;
	        }
                
	        if(!recv_window.empty())
	        {
	        Window1_iter head1 =recv_window.begin();
	        //display(recv_window);
	        while(head1!=recv_window.end())
	        {
	                DTPPacket *pkt1;
	                pkt1=(*head1).second;
	                if(pkt1->id!=recv_so_far+1)
	                {
	                        break;
	                }
	                recv_so_far+=1;
	                
	                file1<<pkt1->data;
	               // cout<<"bb"<<pkt1->data<<endl;
	                recv_window.erase(head1);
	                head1 =recv_window.begin();
	        }
	        }
	                
         }
         else if((pkt11->id)>recv_so_far)
         {      Window1Pair entry(pkt11->id,pkt11);
	        recv_window.insert(entry);
	        //set_timer(retrans, NULL);
	 }   
	   
	// display(recv_window);
	 file1.close();
	 }
	 if(ter_seq==recv_so_far+1&&write)
	 {
                DTPPacket*	pkt2 = new DTPPacket;
                pkt2->source = address();
                pkt2->destination = destination;
                pkt2->length = HEADER_SIZE;
                sent_so_far+=1;
                pkt2->id = sent_so_far;
                recv_so_far+=1;
                pkt2->ack_id =recv_so_far;
                pkt2->sync_bit=sync_bit;
                pkt2->FIN=1;
                pkt2->ACK=1;
                pkt2->ECN=0;
                pkt2->ECN1=ECN;
                pkt2->file_size=File_size;
                if (send(pkt2)) 
                {
                        TRACE(TRL3,"Sent packet from DTP-Host: %d\n",address());
                        pkt2->print();//_header();
                }
                TRACE(TRL4,"(Time:%d) node %d Sent FIN-ACK to node %d\n",scheduler->time(),address(),destination);
                term_bit=2;
	 }
	 else
	 {
	        handle_timer((void*)1);
	 }
}

void Host:: sent_window_sync(DTPPacket* pkt)
{
        //cout<<address()<<endl;
        //display();
        //cout<<"jddjdkdkd"<<endl;
        //cout<<"last_ack="<<last_ack<<endl;
        if(pkt->ack_id<last_ack)
        {
                //retransmit();
        }
        else if(pkt->ack_id==last_ack)
        {
                dup_ack_count+=1;
                //cout<<"DUP:"<<dup_ack_count<<endl;
                if((dup_ack_count%2)==0)
                {
                        //cout<<"DUP:"<<dup_ack_count<<endl;
                        window_size=window_size/2;
                        Window_threshold=window_size;
                        if(retrans_bit==2)
		        {	
			        if(retrans>scheduler->time())
			        {
			                cancel_timer(retrans, NULL);
			                retrans=scheduler->time() +RTO; 
			                set_timer(retrans, NULL);
			                TRACE(TRL4,"Next Retransmit at DTP-Host %d is set at Time: %d\n",address(),retrans);
			                DTPPacket *pkt_r= new DTPPacket;
			                copy_pkt(pkt_r,retransmit_pkt);
			                pkt_r->ack_id =recv_so_far;
			                pkt_r->ECN=0;
			                pkt_r->ECN1=ECN;
			                pkt_r->file_size=File_size;
			                if (send(pkt_r)) 
			                {
                                                TRACE(TRL3,"Retransmit packet (Due to Three Dup-Acks) from DTP-Host: %d\n",address());
        	                                pkt_r->print();//_header();
        	                        }
        	               }
	                }
                }
        }
        else
        {//cout<<"jddjdkdkd"<<endl;
                //DTPPacket*	pkt10 = new DTPPacket;
	       // copy_pkt(pkt10,pkt);
	        dup_ack_count=0;
	        congestion_control(pkt);
	        
                if(retrans>=scheduler->time())
	        {
	                cancel_timer(retrans, NULL);
                        retrans=0;
                        retrans_bit=0;
                 }
	
	//cout<<"cccc"<<endl<<endl;
	//pkt->print_header();
        last_ack=pkt->ack_id;
        DTPPacket *pkt1;
        Window1_iter head = sent_window.find(pkt->ack_id);
	//if(!sent_window.empty()&&head==sent_window.end())
	//{
	  //      Window1_iter head22 =sent_window.end();
	    //    pkt1=(*head22).second;
	       // cout<<last_ack<<"END:"<<endl;
	               //cout<<"bb"<<pkt1->data<<endl;
	                //recv_window.erase(head1);
	                //head1 =recv_window.begin();
	//}
	if(head!=sent_window.end()&&!sent_window.empty())
	{
	        //cout<<"jddjdkdkd"<<endl;
	        Window1_iter head1 =sent_window.begin();
	        if(head1==head)
	        {
	                pkt1=(*head1).second;
	                //cout<<"bbbb"<<endl<<endl;
	                //pkt1->print_header();
	               // last_ack++;
	                sent_window.erase(head1);
	                packets_in_window--;
	        }
	        else
	        {
	        while(head1!=head)
	        {
	                head1 =sent_window.begin();
	                pkt1=(*head1).second;
	                //cout<<"bbbb"<<endl<<endl;
	                //pkt1->print_header();
	                // last_ack++;;
	                sent_window.erase(head1);
	                packets_in_window--;
	        }
	        }
	        //update window
	        //retransmit
	}
	
	//display();
	
	RTTVAR=(1 - beta)*RTTVAR +beta*abs(RTT - (scheduler->time()-last_transmit));
	
	RTT=(1 - alpha) * RTT + alpha *(scheduler->time()-last_transmit);
	
	RTO = (RTT + K*RTTVAR);
	TRACE(TRL4,"Current RTT: %d\n Current RTO: %d",scheduler->time()-last_transmit,RTO);
        id_time_iter id_timer = send_time.find(pkt->ack_id+1);

	                //cout<<"kk";	
	if(id_timer!=send_time.end()&&!send_time.empty())
	{
	        last_transmit=(*id_timer).second;
	}
	else
	{
	        last_transmit=scheduler->time();	
	}
	//cout<<"last_transmit:"<<last_transmit<<endl;           
	        
	Window1_iter head1 =sent_window.begin();
	if(head1!=sent_window.end())
	        retransmit_pkt=(*head1).second;
	else
                retransmit_pkt=NULL;
                
        if(retrans_bit==0&&retransmit_pkt!=NULL)
        {
              	if(retransmit_pkt->FIN!=1)
              	{
              	DTPPacket*	pkt9 = new DTPPacket;
	        copy_pkt(pkt9,retransmit_pkt);
	        pkt9->ack_id =recv_so_far;
	        pkt9->file_size=File_size;
	        if (send(pkt9)) 
	        {
                        TRACE(TRL3,"Retransmit packet from DTP-Host: %d\n",address());
	                pkt9->print();//_header();
	        }
	        }
	        retrans_bit=2;
                retrans=scheduler->time() +RTO;
                set_timer(retrans, NULL);
                TRACE(TRL4,"Next Retransmit at DTP-Host %d is set at Time: %d\n",address(),retrans);
        }       
        }
        //cout<<"last_ack="<<last_ack<<endl;
        return;
}
void 
Host::congestion_control(DTPPacket* pkt)
{
        //int i=window_size;
        TRACE(TRL3,"Previous Window Size at DTP-Host: %d is %f\n",address(),window_size);
        if(ACKcount_flag)
        {
                if(ACKcounter>0)
                {
                        ACKcounter--;
                        if(pkt->ECN1==1)
                        {
                               ECNcount++;
                        }
               }
                else
                        ACKcount_flag=0;
                        
        }
        else
        {
                if(ACKcount)
                alpha1=alpha1*(1-G)+G*((float)ECNcount)/((float)ACKcount);
                //cout<<"alpha1"<<alpha1<<endl;
                ACKcount=floor(window_size);
                ACKcounter=ACKcount;
                ECNcount=0;
                ACKcount_flag=1;
                window_size=window_size*(1-alpha1/2);
        }
        
        
        int j=(pkt->ack_id)-last_ack;
        if(File_size)
                File_size=File_size-j*PAYLOAD_SIZE;
        if(File_size<=0)
                File_size=0;        
                
        if(window_size+j<Window_threshold)
        {
                window_size=window_size+j;
        }
        else if(window_size<Window_threshold&&Window_threshold<=window_size+j)
        {
                window_size=Window_threshold;
        }
        else if(Window_threshold<=window_size)
        {
                window_size=window_size+((float)j)/window_size;
                //cout<<"ff"<<Window_threshold<<endl;
        }
        else;
        /*if(pkt->ECN1==1)
        {
                if(ACKcount_flag)
                        ECNcount++;
                window_size=window_size/2;               
        }
        else
        {
                int j=(pkt->ack_id)-last_ack;
               // last_ack;
                //cout<<"aa"<<Window_threshold<<"dd"<<j<<endl;
                if(window_size+j<Window_threshold)
                {
                        window_size=window_size+j;
                }
                else if(window_size<Window_threshold&&Window_threshold<=window_size+j)
                {
                        //cout<<"gg"<<endl;
                        window_size=Window_threshold;
                }
                else if(Window_threshold<=window_size)
                {
                        window_size=window_size+((float)j)/window_size;
                        //cout<<"ff"<<Window_threshold<<endl;
                }
                else;
                
                
        }*/
        TRACE(TRL3,"Current Window Size at DTP-Host: %d is %f\n",address(),window_size);
        //delete pkt;
        return;
}

void 
Host::display(Window1 w1)
{
        
        
        if(!w1.empty())
	{
	        //cout<<"jddjdkdkd"<<endl;
	        Window1_iter head1 =w1.begin();
	        DTPPacket* pkt1;
	        while(head1!=w1.end())
	        {
	                pkt1=(*head1).second;
	                //cout<<"dddd"<<endl<<endl;
	                pkt1->print();//_header();
	                head1=w1.upper_bound(pkt1->id);
	        }
	}
}
void Host::copy_pkt(DTPPacket* pkt_to,DTPPacket* pkt_from)
{
        pkt_to->FIN=pkt_from->FIN;
        pkt_to->ACK=pkt_from->ACK;
        pkt_to->source = pkt_from->source;
        pkt_to->destination = pkt_from->destination;
        pkt_to->length = pkt_from->length;
        pkt_to->id = pkt_from->id;
        pkt_to->ack_id =pkt_to->ack_id;
        pkt_to->sync_bit=pkt_from->sync_bit; 
        strcpy(pkt_to->data,pkt_from->data);
        pkt_to->ECN=pkt_from->ECN;
        pkt_to->ECN1=pkt_from->ECN1;
        pkt_to->file_size=pkt_from->file_size;
}

void Host::insert_p(Time s,Address d,char* f)
{
	sendpair* newpair=new sendpair;
	newpair->a=d;
	strcpy(newpair->name,f);
	SendMapPair entry(s,newpair);
	dest_map.insert(entry);
}
void
Host::terminate(Address d)
{
	DTPPacket*	pkt11 = new DTPPacket;
        destination = d;
	term_bit=1;
	 //handle_timer((void*)1);
	DTPPacket*	pkt12 = new DTPPacket;
        pkt12->source = address();
        pkt12->destination = destination;
        pkt12->length = HEADER_SIZE;
        sent_so_far+=1;
        pkt12->id = sent_so_far;
	pkt12->ack_id =recv_so_far;
 	pkt12->sync_bit=sync_bit;
 	pkt12->FIN=1;
        pkt12->ACK=0;
        pkt12->ECN=0;
        pkt12->ECN1=ECN;
        pkt12->file_size=File_size;
        copy_pkt(pkt11,pkt12);
        Window1Pair entry(pkt11->id,pkt11);
        sent_window.insert(entry);
        if (send(pkt12)) 
        {
                TRACE(TRL3,"Sent packet from DTP-Host: %d\n",address());
	        pkt12->print();//_header();
	}

	TRACE(TRL4,"(Time:%d) node %d Sent FIN to node %d\n",scheduler->time(),address(),destination);
			
}
