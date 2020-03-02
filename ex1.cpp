

#include <iomanip>

#include "systemc"
using namespace sc_core;
using namespace sc_dt;
using namespace std;

#include "tlm.h"
#include "../../common/mm.h"
// Initiator module generating generic payload transactions

struct Initiator: sc_module, tlm::tlm_bw_transport_if<>
{
  // TLM-2 socket, defaults to 32-bits wide, base protocol
  tlm::tlm_initiator_socket<> socket;

  SC_CTOR(Initiator)
  : socket("socket")  // Construct and name socket
  {
    socket.bind(*this);
    SC_THREAD(thread_process);
  }

  void thread_process()
  {
    // TLM-2 generic payload transaction, reused across calls to b_transport
    tlm::tlm_generic_payload* trans;
     trans = m_mm.allocate();
     sc_time delay = SC_ZERO_TIME;
    for(int i=0;i<4;i++)
    data[i] = 0xdeadbeef-i;

    // Generate a sequence of write bursts
    for (int addr = 0xC; addr < 64; addr += 8)//*********** why addr += 8 inspite of 4? since streaming width is 4 shouldn't we ***********//
                                            //*********** increment with 4 if we want to write bursts in an incremental manner ?***********// 
    {
      tlm::tlm_command cmd = tlm::TLM_WRITE_COMMAND;

      trans->acquire();
      // ...
      trans->set_command( cmd );
      trans->set_address( addr );
      trans->set_data_ptr( reinterpret_cast<unsigned char*>(&data) );
      trans->set_data_length( 16 );
      trans->set_streaming_width( 4 ); 
      byte_enable_mask = 0x00fffffful;
      trans->set_byte_enable_ptr( reinterpret_cast<unsigned char*>(&byte_enable_mask)); 
      trans->set_byte_enable_length(4);
      trans->set_dmi_allowed( false ); // Mandatory initial value //********* why mandatory? ref: ex2.************//

      trans->set_response_status( tlm::TLM_INCOMPLETE_RESPONSE ); // Mandatory initial value


      //Injecting error
       if(addr == 0xC+8)
            trans->set_address(0xe);
       if(addr == 0xC+16)
            trans->set_streaming_width(5);
       if(addr == 0xC+24)
       {
            byte_enable_mask = 0x0;
            trans->set_byte_enable_ptr( reinterpret_cast<unsigned char*>(&byte_enable_mask)); 
       }
 

      socket->b_transport( *trans, delay );  // Blocking transport call

      // Initiator obliged to check response status and delay
      if ( trans->is_response_error() )
      {

         if( trans->get_response_status() == tlm::TLM_COMMAND_ERROR_RESPONSE)
          {
            cout<<"tlm::TLM_COMMAND_ERROR_RESPONSE"<<endl;
          }
          else if( trans->get_response_status() == tlm::TLM_ADDRESS_ERROR_RESPONSE)
          {
            cout<<"tlm::TLM_ADDRESS_ERROR_RESPONSE addr:"<< addr <<endl;
          }
          else if( trans->get_response_status() == tlm::TLM_BURST_ERROR_RESPONSE)
          {
            cout<<"tlm::TLM_BURST_ERROR_RESPONSE"<<endl;
          }
          else if( trans->get_response_status() == tlm::TLM_BYTE_ENABLE_ERROR_RESPONSE)
          {
            cout<<"tlm::TLM_BYTE_ENABLE_ERROR_RESPONSE"<<endl;
          }

        //SC_REPORT_ERROR("TLM-2", "Response error from b_transport");
      }

      cout << "Completed " << (cmd ? "write" : "read") << ", addr = " << hex << addr
           << " at " << sc_time_stamp() << endl;
     // trans->release();
    }
  }

  // TLM-2 backward non-blocking transport method
  virtual tlm::tlm_sync_enum nb_transport_bw( tlm::tlm_generic_payload& trans,
                                              tlm::tlm_phase& phase, sc_time& delay )
  {
    // Dummy method
    return tlm::TLM_ACCEPTED;
  }

  // TLM-2 backward DMI method
  virtual void invalidate_direct_mem_ptr(sc_dt::uint64 start_range,
                                         sc_dt::uint64 end_range)
  {
    // Dummy method
  }

int data[4];
int byte_enable_mask;
 mm m_mm;
};


// Target module representing a simple memory

struct Target: sc_module, tlm::tlm_fw_transport_if<>
{
  // TLM-2 socket, defaults to 32-bits wide, base protocol
  tlm::tlm_target_socket<> socket;

  SC_CTOR(Target)
  : socket("socket")
  {
    socket.bind(*this);
  }

  // TLM-2 blocking transport method
  virtual void b_transport( tlm::tlm_generic_payload& trans, sc_time& delay )
  {
    tlm::tlm_command cmd = trans.get_command();
    sc_dt::uint64    adr = trans.get_address();
    unsigned char*   ptr = trans.get_data_ptr();
    unsigned int     len = trans.get_data_length();
    unsigned char*   byt = trans.get_byte_enable_ptr();
    unsigned int     bel = trans.get_byte_enable_length();
    unsigned int     wid = trans.get_streaming_width();

    if ( cmd == tlm::TLM_WRITE_COMMAND )
    {
      for (int i = 0; i < int(len); i += 4)
      {
        cout << "addr = 0x"<< hex << adr << ", data = ";
        for (int j = 3; j >= 0; j--)
          cout << hex << setw(2) << setfill('0') << int(ptr[i+j]);
        cout << ", byte enables = ";
        if (bel)
          for (int j = 3; j >= 0; j--)
            cout << hex << setw(2) << setfill('0') << int(byt[(i+j) % bel]);
        else
          cout << "00000000";
        cout << "\n";
      }
    }

    trans.set_response_status( tlm::TLM_OK_RESPONSE );


    // Diagnostic code to help with the debugging of the exercise
    // If any of these assertions fail, your solution is wrong!
    if (trans.has_mm())
      cout << "Transaction has a memory manager, address = " << &trans
           << ", ref_count = " << trans.get_ref_count() << "\n";
    else
      cout << "Transaction has no memory manager, address = " << &trans << "\n";

    if(cmd != tlm::TLM_WRITE_COMMAND){
       trans.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
    }

    if (adr % 4 != 0){
      trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);//******************** Is it correct?***********//
    }

    if(len == 16){ //******************** ??*******************//
        //trans.set_response_status(??);
    }    
    
    if ((len %4 != 0) || (wid != 4)){
        cout<<"len:"<< len<<" wid:" <<wid <<endl;
        trans.set_response_status(tlm::TLM_BURST_ERROR_RESPONSE);//******************** Is it correct?***********//
    }

    if ((bel == 4) || (bel == 8) || (bel == 12) || (bel == 16)){    //******************** ??*******************//
         //trans.set_response_status(??);
    }


    for (unsigned int i = 0; i < bel; i++)
    {
        if ((i % 4) == 3){
            if(byt[i] != 0x00){
                trans.set_response_status(tlm::TLM_BYTE_ENABLE_ERROR_RESPONSE);
            }
        }
      else{
            if(byt[i] != 0xff){
                trans.set_response_status(tlm::TLM_BYTE_ENABLE_ERROR_RESPONSE);
            }
        }
    }
 
  }

  // TLM-2 forward non-blocking transport method
  virtual tlm::tlm_sync_enum nb_transport_fw( tlm::tlm_generic_payload& trans,
                                              tlm::tlm_phase& phase, sc_time& delay )
  {
    // Dummy method (not TLM-2.0 compliant)
    return tlm::TLM_ACCEPTED;
  }

  // TLM-2 forward DMI method
  virtual bool get_direct_mem_ptr(tlm::tlm_generic_payload& trans,
                                  tlm::tlm_dmi& dmi_data)
  {
    // Dummy method
    return false;
  }

  // TLM-2 debug transport method
  virtual unsigned int transport_dbg(tlm::tlm_generic_payload& trans)
  {
    // Dummy method
    return 0;
  }
};


SC_MODULE(Top)
{
  Initiator *initiator;
  Target    *target;

  SC_CTOR(Top)
  {
    // Instantiate components
    initiator = new Initiator("initiator");
    target    = new Target   ("target");

    // One initiator is bound directly to one target with no intervening bus

    // Bind initiator socket to target socket
    initiator->socket.bind( target->socket );
  }
};


int sc_main(int argc, char* argv[])
{
  Top top("top");
  sc_start();
  return 0;
}

