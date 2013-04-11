#include <fstream>
class FIFONode;
class DTPPacket;
#define MAX 99999;
typedef struct 
{
    Address a;
    char name[1000];
}sendpair;

typedef map< Time,sendpair*, ltTime> SendMap;
typedef map< Time,sendpair*, ltTime>::iterator SendMap_iter;
typedef pair<Time, sendpair*> SendMapPair;

typedef map< Time,DTPPacket*, ltTime> Window;
typedef map< Time,DTPPacket*, ltTime>::iterator Window_iter;
typedef pair<Time,DTPPacket*> WindowPair;
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
	void insert_p(Time s,Address d,char* f);
	void terminate(Address d);
	int sync_bit;
	int term_bit;
	int     retrans_bit;
	int packets_in_window;
	SendMap dest_map;
	ifstream in_file;
	ofstream out_file;
	//int dest[MAX];
	//int dest_count,dest_pointer;
 private:
	Time retrans_timer, last_transmit;
    Address	destination;		// Target address
    Time	start;			// Start sending at
    Time	retrans;	// Inter-packet time
    int		packets_to_send;	// number of packets
    int		sent_so_far;
    Window sent_window;	
	
    
};
