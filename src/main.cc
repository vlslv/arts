#include "arts.h"
#include "parameters.h"
#include "messages.h"
#include "exceptions.h"
#include "file.h"
#include "wsv.h"		// Automatically generated!
#include "workspace.h"
#include "methods.h"
#include "parser.h"
#include "md.h"


/** Print the error message and exit. */
void give_up(const string& message)
{
  out0 << message << '\n';
  exit(1);
}


/** The arts executor. This executes the methods specified in tasklist
    on the given workspace. It also checks for errors during the
    method execution and stops the program if an error has
    occured. FIXME: Eventually, it should do a good housekeeping of
    which variables are occupied and which are not.

    @param workspace Output. The workspace to act on.
    @param tasklist The list of methods to execute (including keyword data).

    @authore Stefan Buehler  */
void executor(WorkSpace& workspace, const ARRAY<MRecord>& tasklist)
{
  // The method description lookup table:
  extern const ARRAY<MdRecord> md_data;

  // The workspace variable lookup table:
  extern const ARRAY<WsvRecord> wsv_data;
  
  // The array holding the pointers to the getaway functions:
  extern const void (*getaways[])(WorkSpace&, const MRecord&);

  // We need a place to remember which workspace variables are
  // occupied and which aren't.
  // 
  // For some weird reason, ARRAYs of bool do not work, although all
  // other types seem to work fine. So in this single case, I'll use
  // the stl vector directly.
  std::vector<bool> occupied(wsv_data.size(),false);

  out3 << "\nExecuting methods:\n";

  for (size_t i=0; i<tasklist.size(); ++i)
    {
      // Runtime method data for this method:
      const MRecord&  mrr = tasklist[i];
      // Method data for this method:
      const MdRecord& mdd = md_data[mrr.Id()];
      // Needed to store variable lists:
      ARRAY<size_t> v;
      
      try
	{
	  out1 << "- " << mdd.Name() << '\n';

	
	  { // Check if all specific input variables are occupied:
	    v = mdd.Input();
	    for (size_t s=0; s<v.size(); ++s)
	      if (!occupied[v[s]])
		give_up("Method "+mdd.Name()+" needs input variable: "+
			wsv_data[v[s]].Name());
	  }

	  { // Check if all generic input variables are occupied:
	    v = mrr.Input();
	    for (size_t s=0; s<v.size(); ++s)
	      if (!occupied[v[s]])
		give_up("Generic Method "+mdd.Name()+" needs input variable: "+
			wsv_data[v[s]].Name());
	  }

	  // Call the getaway function:
	  getaways[mrr.Id()]
	    ( workspace, mrr );

	  { // Flag the specific output variables as occupied:
	    v = mdd.Output();
	    for (size_t s=0; s<v.size(); ++s) occupied[v[s]] = true;
	  }

	  { // Flag the generic output variables as occupied:
	    v = mrr.Output();
	    for (size_t s=0; s<v.size(); ++s) occupied[v[s]] = true;
	  }

	}
      catch (runtime_error x)
	{
	  out0 << "Error in method: " << mdd.Name() << '\n'
	       << x.what() << '\n';
	  exit(1);
	}
    }
}


/** Remind the user of --help and exit return value 1. */
void polite_goodby()
{
  cerr << "Try `arts --help' for help.\n";
  exit(1);
}


/** This is the main function of ARTS. (You never guessed that, did you?)
    The getopt_long function is used to parse the command line parameters.
 
    \begin{verbatim}
    Overview:
    1. Get command line parameters.
    2. Evaluate the command line parameters. (This also checks if the
       parameters make sense, where necessary.) 
    \end{verbatim}

    @return    0=ok, 1=error
    @param     argc Number of command line parameters 
    @param     argv Values of command line parameters
    @author    Stefan Buehler */
int main (int argc, char **argv)
{
  extern const Parameters parameters; // Global variable that holds
                                      // all command line parameters. 

  //---------------< 1. Get command line parameters >---------------
  if ( get_parameters(argc, argv) )
    {
      // Print an error message and exit:
      polite_goodby();
    }

  //----------< 2. Evaluate the command line parameters >----------
  if (parameters.help)
    {
      // Just print a help message and then exit.
      cerr << '\n' << parameters.usage << "\n\n";
      cerr << parameters.helptext << "\n\n";
      return(0);
    }

  if (parameters.version)
    {
      extern const string full_name;
      // Just print version information and then exit.
      cerr << "This is " << full_name << '\n';
      return(0);
    }


  // Set the reporting level, either the default or based on
  // reporting. If reporting was specified, check if the values make
  // sense. The value -1 for reporting means that it was (probably)
  // not given on the command line, since this is the initialization
  // value.
  {
    // The global variable that specifies the output level for screen
    // and file:
    extern Messages messages;
    int r = parameters.reporting;

    if ( -1 == r )
      {
	// Reporting was not specified, set default. (Only the
	// important stuff to the screen, everything to the file.)
	messages.screen = 1;
	messages.file   = 3;
      }
    else
      {
	// Reporting was specified. Check consistency and set report
	// level accordingly. 
	
	// Separate the two digits by taking modulo 10:
	int s = r / 10;
	int f = r % 10;
	//	cout << "s=" << s << " f=" << f << '\n';

	if ( s<0 || s>3 || f<0 || f>3 )
	  {
	    cerr << "Illegal value specified for --reporting (-r).\n"
		 << "The specified value is " << r << ", which would be\n"
		 << "interpreted as screen=" << s << ", file=" << f << ".\n"
		 << "Only values of 0-3 are allowed for screen and file.\n";
	    return(1);
	  }
	messages.screen = s;
	messages.file   = f;
      }
  }


  // For the next couple of options we need to have the workspce and
  // method lookup data.


  // Initialize the md data:
  define_md_data();

  // Initialize the wsv data:
  define_wsv_data();

  // Initialize MdMap
  define_md_map();

  // Initialize WsvMap
  define_wsv_map();

  // Make all these data visible:
  extern const ARRAY<MdRecord>  md_data;
  extern const ARRAY<WsvRecord> wsv_data;
  extern const std::map<string, size_t> MdMap;
  extern const std::map<string, size_t> WsvMap;


  // Now we are set to deal with the more interesting command line
  // switches. 


  // React to option `methods'. If given the argument `all', it
  // should simply prints a list of all methods. If given the name of
  // a variable, it should print all methods that produce this
  // variable as output.
  if ( "" != parameters.methods )
    {
      cerr << "Option `methods' is not yet implemented, sorry.\n";
      return(1);
    }
  
  // React to option `workspacevariables'. If given the argument `all',
  // it should simply prints a list of all variables. If given the
  // name of a method, it should print all variables that are needed
  // by that method.
  if ( "" != parameters.workspacevariables )
    {
      cerr << "Option `workspacevariables' is not yet implemented, sorry.\n";
      return(1);
    }

  // React to option `describe'. This should print the description
  // string of the given workspace variable or method.
  if ( "" != parameters.describe )
    {
      // Let's first assume it is a method that the user wants to have
      // described.

      // Find method id:
      map<string, size_t>::const_iterator i =
	MdMap.find(parameters.describe);
      if ( i != MdMap.end() )
	{
	  // If we are here, then the given name matches a method.
	  cout << md_data[i->second] << '\n';
	  return(0);
	}

      // Ok, let's now assume it is a variable that the user wants to have
      // described.

      // Find wsv id:
      i = WsvMap.find(parameters.describe);
      if ( i != WsvMap.end() )
	{
	  // If we are here, then the given name matches a workspace
	  // variable. 
	  cout << wsv_data[i->second] << '\n';
	  return(0);
	}

      // If we are here, then the given name does not match anything.
      cerr << "The name " << parameters.describe
	   << " matches neither method nor variable.\n";

      return(1);      
    }


  // Ok, we are past all the special options. This means the user
  // wants to get serious and really do a calculation. Check if we
  // have at least one control file:
  if ( 0 == parameters.controlfiles.size() )
    {
      cerr << "You must specify at least one control file name.\n";
      polite_goodby();
    }

  // Set the basename according to the first control file, if not
  // explicitly specified.
  if ( "" == parameters.basename )
    {
      extern string basename;
      basename = parameters.controlfiles[0];
      // Find the last . in the name
      string::size_type p = basename.rfind('.');
      // Kill everything starting from the `.'
      basename.erase(p);
    }



  //--------------------< Open report file >--------------------
  // This one needs its own little try block, because we have to
  // write error messages to cerr directly since the report file
  // will not exist.
  try
    {
      extern const string basename;     // Basis for file name
      extern ofstream report_file;	// Report file pointer

      //      cout << "rep = " << basename+".rep" << '\n';
      open_output_file(report_file, basename+".rep");
    }
  catch (runtime_error x)
    {
      cerr << x.what() << '\n'
	   << "I have to be able to write to my report file.";
      exit(1);
    }

  // Now comes the global try block. Exceptions caught after this
  // one are general stuff like file opening errors.
  // FIXME: Maybe this is not really necessary, since methods
  // using files could always check for these errors? Think about
  // which way is easier.
  try
    {
      // Some global variables that we need:
      extern WorkSpace workspace;
      //      extern ARRAY<WsvRecord> wsv_data;
      //      extern ARRAY<MdRecord> md_data;

      {
	// Output program name and version number: 
	// The name (PACKAGE) and the major and minor version number
	// (VERSION) are set in configure.in. The configuration tools
	// place them in the file config.h, which is included in arts.h.
	// The subminor number is set in version.cc, which is linked with
	// arts.

	extern const string subversion;
  
	out1 << PACKAGE << " " << VERSION << "." << subversion << '\n';
      }


      // The list of methods to execute and their keyword data from
      // the control file. 
      ARRAY<MRecord> tasklist;

      // The text of the controlfile.
      SourceText text;
	
      // Read the control text from the control files:
      out3 << "\nReading control files:\n";
      for ( size_t i=0; i<parameters.controlfiles.size(); ++i )
	{
	  out3 << "- " << parameters.controlfiles[i] << '\n';
	  text.AppendFile(parameters.controlfiles[i]);
	}

      // Call the parser to parse the control text:
      parse_main(tasklist, text);

      // Call the executor:
      executor(workspace, tasklist);

    }
  catch (runtime_error x)
    {
      out0 << x.what() << '\n';
      exit(1);
    }

  out1 << "Goodbye.\n";
  return(0);
}
