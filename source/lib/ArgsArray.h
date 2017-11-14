#ifndef _MERGETIFF_ARGS_ARRAY
#define _MERGETIFF_ARGS_ARRAY

#include <memory>
#include <string>
#include <vector>
using std::string;
using std::vector;

namespace mergetiff {

class ArgsArray
{
	public:
		
		//Creates an empty list
		inline ArgsArray() {}
		
		//Initialises the list with values
		inline ArgsArray(const vector<string>& args)
		{
			for (auto arg : args) {
				this->add(arg);
			}
		}
		
		//Adds an argument to the list
		inline void add(const string& arg)
		{
			this->args.push_back( vector<char>(arg.size()+1) );
			memcpy(this->args.back().data(), arg.c_str(), arg.size()+1);
		}
		
		//Returns an argv-style structure containing the arguments
		inline char** get()
		{
			//Clear the structure and add the pointers to each of the args
			this->structure.clear();
			for (auto& arg : this->args) {
				this->structure.push_back(arg.data());
			}
			
			//Add a nullptr at the end of the array and return the result
			this->structure.push_back(nullptr);
			return this->structure.data();
		}
		
		//Determines if the list is empty
		inline bool empty() const {
			return this->args.empty();
		}
		
	private:
		vector< vector<char> > args;
		vector<char*> structure;
};

} //End namespace mergetiff

#endif
