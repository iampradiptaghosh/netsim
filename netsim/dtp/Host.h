#include <fstream>
class FIFONode;
class DTPPacket;
#define MAX 99999;
typedef struct 
{
    Address a;
    char name[1000];
}sendpair;
struct ltaddr {
    bool operator() (int t1, int t2) const {
        return (t1 < t2);
    }
};
typedef map< Time,sendpair*, ltTime> SendMap;
typedef map< Time,sendpair*, ltTime>::iterator SendMap_iter;
typedef pair<Time, sendpair*> SendMapPair;



typedef map< int,DTPPacket*,ltaddr> Window1;
typedef map< int,DTPPacket*,ltaddr>::iterator Window1_iter;
typedef pair<int,DTPPacket*> Window1Pair;



class Host : public FIFONode {
 public:
    Host(Address a);
    ~Host();
        void get_dest(Packet* pkt);
	void handle_timer(void* cookie);
    // Receive packet and prints its length
    //void sync(Packet* pkt);
	void receive(Packet* pkt);
	void send_file();			// Incoming packet
	void sync();
	void sent_window_sync(DTPPacket* pkt);
	void recv_window_sync(DTPPacket* pkt);
	void insert_p(Time s,Address d,char* f);
	void terminate(Address d);
	void display(Window1 w1);
	void copy_pkt(DTPPacket* pkt_to,DTPPacket* pkt_from);
	int sync_bit;
	int term_bit;
	int     retrans_bit;
	int packets_in_window;
	int window_size;
	DTPPacket* retransmit_pkt;
	//DTPPacket* pkt11;
	SendMap dest_map;
	ifstream in_file;
	fstream out_file;
	bool sender;
	Window1 sent_window;	
        Window1 recv_window;
	//int dest[MAX];
	//int dest_count,dest_pointer;
 private:
	Time retrans_timer, last_transmit;
    Address	destination;		// Target address
    Time	start;			// Start sending at
    Time	retrans;	// Inter-packet time
    int		packets_to_send;	// number of packets
    int		sent_so_far;
    int		recv_so_far;
    int         wrote_so_far;
    int         last_ack;
    	
	
    
};
