// Test thrift de-serialization/serialization without search algo


//  C
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// THRIFT
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TTransportUtils.h>
#include <thrift/transport/TFDTransport.h>
	namespace atp = apache::thrift::protocol;
	namespace att = apache::thrift::transport;

// THRIFT -- STREAMCORUPS
#include "streamcorpus_types.h"
#include "streamcorpus_constants.h"
	namespace sc = streamcorpus;

// STD
#include <list>
	using std::list;
#include <iostream>
	using std::cerr;
	using std::cout;
	using std::endl;

#include <chrono>
	namespace chrono = std::chrono;


int main(int argc, char **argv) {
	
	if (argc!=3)  { cerr << "usage: test_thoughput_speed stream-corpus-file\n";  exit(1); }

	//////////////////////////////////////////////////////////////////////////////////  CONFIG

	constexpr long max_items = 1000; 		cerr << "max-items: " << max_items << endl;
	cerr << std::fixed;
	
	////////////////////////////////////////////////////////////////////////////////// FILES

        cerr << "input  file: " << argv[1] << endl;
        cerr << "output file: " << argv[2] << endl;
	int input_fh  = *argv[1]=='-'  ?  0 : open (argv[1], O_RDONLY); 	if (input_fh  == -1) { cerr << "error: can't open " << argv[1] << endl; exit(2); } ;
	int output_fh = *argv[2]=='-'  ?  1 : creat(argv[2], 664);     	if (output_fh == -1) { cerr << "error: can't open " << argv[2] << endl; exit(3); } ;

	/////////////////////////////////////////////////////////////////////////////////// TRANSPORT / PROTOCOL
	
	// input
	boost::shared_ptr<att::TFDTransport>	        innerTransportInput(new att::TFDTransport(input_fh));
	boost::shared_ptr<att::TBufferedTransport>	transportInput(new att::TBufferedTransport(innerTransportInput));
	boost::shared_ptr<atp::TBinaryProtocol>		protocolInput(new atp::TBinaryProtocol(transportInput));
	transportInput->open();

	// output 
	boost::shared_ptr<att::TFDTransport>		transportOutput(new att::TFDTransport(output_fh));
	boost::shared_ptr<atp::TBinaryProtocol>		protocolOutput(new atp::TBinaryProtocol(transportOutput));
	transportOutput->open();

	//////////////////////////////////////////////////////////////////////////////////  DE-SERIALIZATION 
	auto                 start         = chrono::high_resolution_clock ::now();
	auto 		     start100      = chrono::high_resolution_clock ::now();
	sc::StreamItem       stream_item;
	list<sc::StreamItem> test_objects;
	
	

	for (long item_count=0;   item_count < max_items;   ++item_count) {
		try { stream_item.read(protocolInput.get()); }
		catch (...) { break; }

			if (item_count % 100 == 0  &&  item_count) {
				auto diff  = chrono::high_resolution_clock ::now() - start100;
				double sec = chrono::duration_cast<chrono::nanoseconds>(diff).count()/1e9;

				cerr	<< "-- item read: " << item_count 
					<< "   \tavg time per item: "  << sec/100 << " sec"
					<< "   \titems/sec: "  << 100 / sec << endl;

				start100 = chrono::high_resolution_clock ::now();
			}

		test_objects.push_back(stream_item);
	}

			{
			auto diff  = chrono::high_resolution_clock ::now() - start;
			double sec   = chrono::duration_cast<chrono::nanoseconds>(diff).count()/1e9;

			cerr << "\nDE-SERIALIZATION\n";
			cerr << "test_objects: " << test_objects.size() << endl;
			cerr << "time: "         << sec << " sec\n";
			cerr << "objects/sec: "  << test_objects.size() / sec << endl;
			}

	///////////////////////////////////////////////////////////////////////////////  SERIALIZATION 
		       
	start     = chrono::high_resolution_clock ::now();
	start100  = chrono::high_resolution_clock ::now();
	size_t write_count = 0;
	    		
	for (auto& object : test_objects) {
		try { object.write(protocolOutput.get()); }
		catch (...) { break; }
		if (write_count % 100 == 0  &&  write_count) {
			auto diff  = chrono::high_resolution_clock ::now() - start100;
			double sec = chrono::duration_cast<chrono::nanoseconds>(diff).count()/1e9;

			cerr	<< "-- items written: " << write_count 
				<< "   \tavg time per item: "  << sec/100 << " sec"
				<< "   \titems/sec: "  << 100 / sec << endl;

			start100 = chrono::high_resolution_clock ::now();
		}
		++write_count;
	}
	transportOutput->flush();

			{
			auto diff  = chrono::high_resolution_clock ::now() - start;
			double sec   = chrono::duration_cast<chrono::nanoseconds>(diff).count()/1e9;

			cerr << "\nSERIALIZATION\n";
			cerr << "test_objects written: " << write_count << endl;
			cerr << "test_objects: " << test_objects.size() << endl;
			cerr << "time: "      << sec << " sec" << endl;
			cerr << "objects/sec: " << test_objects.size() / sec << endl;
			}
}
