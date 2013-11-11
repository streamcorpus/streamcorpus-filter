#include "multisearch.h"

#include <inttypes.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <iostream>
#include <cstdio>
#include <time.h>
#include <regex>
#include <boost/unordered_map.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <fstream>
#include <chrono>

// THRIFT -- STREAMCORUPS
#include "streamcorpus_types.h"
#include "streamcorpus_constants.h"
using namespace streamcorpus;

// THRIFT -- FILTERNAMES
#include "filternames_types.h"
#include "filternames_constants.h"
//using namespace filternames;
namespace fn = filternames;

#include <protocol/TBinaryProtocol.h>
#include <protocol/TDenseProtocol.h>
#include <protocol/TJSONProtocol.h>
#include <transport/TTransportUtils.h>
#include <transport/TFDTransport.h>
#include <transport/TFileTransport.h>

#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/program_options.hpp>

using namespace std;
using namespace boost;
using namespace boost::filesystem;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

namespace po = boost::program_options;


int main(int argc, char **argv) {
	
	clog << "Starting program" <<endl;
	auto start = chrono::high_resolution_clock ::now();

	
	string text_source("clean_visible");
	string filtername_path;
	
	bool negate(false);
	
	// Supported options.
	po::options_description desc("Allowed options");

	desc.add_options()
		("help,h", "help message")
		("text_source,t", po::value<string>(&text_source), "text source in stream item")
		("negate,n", po::value<bool>(&negate)->implicit_value(true), "negate sense of match")
		("filtername,f", po::value<string>(&filtername_path), "filtername file")
	;
	
	// Parse command line options
	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);
	
	if (vm.count("help")) {
		cout << desc << "\n";
		return 1;
	}
	
	// Create annotator object
	Annotator annotator;
	AnnotatorID annotatorID;
	annotatorID = "example-matcher-v0.1";
	
	// Annotator identifier
	annotator.annotator_id = "example-matcher-v0.1";
	
	// Time this annotator was started
	StreamTime streamtime;
	time_t seconds;
	seconds = time(NULL);
	streamtime.epoch_ticks = seconds;
	streamtime.zulu_timestamp = ctime(&seconds);
	annotator.__set_annotation_time(streamtime);

	// read FilterNames
	int scf_fh = open(filtername_path.c_str(), O_RDONLY);

					if(scf_fh==-1)  {
						cerr << "error: cann't open scf file -- '" << filtername_path << "'\n";
						exit(1);
					}

	boost::shared_ptr<TFDTransport> innerTransportScf(new TFDTransport(scf_fh));
	boost::shared_ptr<TBufferedTransport> transportScf(new TBufferedTransport(innerTransportScf));
	boost::shared_ptr<TBinaryProtocol> protocolScf(new TBinaryProtocol(transportScf));
	transportScf->open();
	
	fn::FilterNames filter_names;
	filter_names.read(protocolScf.get());

					/*// check data
					for(const auto& pr : filter_names.target_id_to_names) {
						clog << pr.first << endl;
						for(auto& name : pr.second) {
							clog << '\t' << name << endl;
						}
					}*/
	
	// Setup thrift reading and writing from stdin and stdout
	int input_fd = 0;
	int output_fd = 1;
	
	// input
	boost::shared_ptr<TFDTransport> innerTransportInput(new TFDTransport(input_fd));
	boost::shared_ptr<TBufferedTransport> transportInput(new TBufferedTransport(innerTransportInput));
	boost::shared_ptr<TBinaryProtocol> protocolInput(new TBinaryProtocol(transportInput));
	transportInput->open();

	// output 
	boost::shared_ptr<TFDTransport> transportOutput(new TFDTransport(output_fd));
	boost::shared_ptr<TBinaryProtocol> protocolOutput(new TBinaryProtocol(transportOutput));
	transportOutput->open();
	
	// Read and process all stream items
	StreamItem stream_item;
	int cnt=0;
	int matches=0;
	int written=0;
	clog << "Reading stream item content from : " << text_source << endl;
	
	while (true) {
		try {
	    		// Read stream_item from stdin
	    		stream_item.read(protocolInput.get());
	    		
	    		string content;
	    		string actual_text_source = text_source;
	    		if (text_source == "clean_visible") {
	    			content = stream_item.body.clean_visible;
	    		} else if (text_source == "clean_html") {
	    			content = stream_item.body.clean_html;
	    		} else if (text_source == "raw") {
	    			content = stream_item.body.raw;
	    		} else {
	    			cerr << "Bad text_source :" << text_source <<endl;
	    			exit(-1);
	    		}
	    		
	    		if (content.size() <= 0) {
	    			// Fall back to raw if desired text_source has no content.
	    			content = stream_item.body.raw;
	    			actual_text_source = "raw";
	    			if (content.size() <= 0) {
	    				// If all applicable text sources are empty, we have a problem and exit with an error
	    				cerr << cnt << " Error, doc id:" << stream_item.doc_id << " was empty." << endl;
	    				exit(-1);
	    			}
	    		}
            		
	    		
	    		
            		
	    		filter_names.name_to_target_ids["John Smith"] = vector<string>();
	    		unordered_map<string, set<string> > target_text_map;

			names_t  names;
            	      
	    		for(auto& pr : filter_names.name_to_target_ids)
	    			names.push_back(&(pr.first));

	    		// SEARCH CYCLE ########################################################################

			const name_t *matched_name = nullptr;
			pos_t   pos = content.begin();

			while ((pos = multisearch(pos, content.end(), names, matched_name)) != content.end()) {
			
				// found
				clog << cnt << " \tdoc-id:" << stream_item.doc_id;
				clog << "   pos:" << pos-content.begin() << " \t" << *matched_name<< "\n";
			
			
				// mapping between canonical form of target and text actually found in document
			
				// For each of the current matches, add a label to the 
				// list of labels.  A label records the character 
				// positions of the match.
				matches++;
				
				// Add the target identified to the label.  Note this 
				// should be identical to what is in the rating 
				// data we add later.
				Target target;
				target.target_id = "1";
			
				Label label;
				label.target = target;
			
				// Add the actual offsets 
				Offset offset;
				offset.type = OffsetType::CHARS;
				
				offset.first = pos - content.begin();
				offset.length = matched_name->size();
				offset.content_form = *matched_name;
			
				label.offsets[OffsetType::CHARS] = offset;
				label.__isset.offsets = true;
			
				// Add new label to the list of labels.
				stream_item.body.labels[annotatorID].push_back(label);
			
				// Map of actual text mapped 
				target_text_map[target.target_id].insert(*matched_name);

				// advance pos to begining of unsearched content
				pos += matched_name->size();
			}
	    		
	    		// Add the rating object for each target that matched in a document
	    		for ( auto match=target_text_map.begin(); match!=target_text_map.end(); ++match) {
	    			// Construct new rating
	    			Rating rating;
	    			
	    			// Flag that it contained a mention
	    			rating.__set_contains_mention(true);
	    			
	    			// Construct a target object for each match
	    			Target target;
	    			target.target_id = match->first;
	    			rating.target = target;
	    			
	    			// Copy all the strings that matched into the mentions field
	    			copy(match->second.begin(), match->second.end(), back_inserter(rating.mentions));
	    			
	    			// Subtle but vital, we need to do the following for proper serialization.
	    			rating.__isset.mentions = true;
	    			
	    			// Add the annotator from above.
	    			rating.annotator = annotator;
	    			
	    			// Push the new rating onto the rating list for this annotator.
	    			stream_item.ratings[annotatorID].push_back(rating);
	    		}
	    		
	    		if (not negate) { 
	    		        // Write stream_item to stdout if it had any ratings
	    		        if (target_text_map.size() > 0) {
	    		            stream_item.write(protocolOutput.get());
	    		            written++;
	    		        }
	    		} else if (target_text_map.size() == 0) {
	    			// Write stream_item to stdout if user requested
	    		       	// to show ones that didn't have any matches
	    			stream_item.write(protocolOutput.get());
	    			written++;
	    		}
	    		
	    		// Increment count of stream items processed
	    		cnt++;
	    	}

		catch (TTransportException e) {
			// Vital to flush the buffered output or you will lose the last one
			transportOutput->flush();
			clog << "Total stream items processed: " << cnt << endl;
			clog << "Total matches : " << matches << endl;
			clog << "Total stream items written         : " << written << endl;
			if (negate) {
				clog << " (Note, stream items written were non-matching ones)" << endl;
			}
			break;
		}
	}

	auto diff = chrono::high_resolution_clock ::now() - start;
	clog << "run time: " << chrono::duration_cast<chrono::seconds>(diff).count() << " sec" << endl;
}

