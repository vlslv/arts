#include "arts.h"
#include "messages.h"
#include "exceptions.h"
#include "file.h"
#include "workspace.h"
#include "methods.h"
#include "parser.h"

void SourceText::AppendFile(const string& name) 
{
  mSfLine.push_back(mText.size());
  mSfName.push_back(name);
  read_text_from_file(mText, name);    
}

void SourceText::AdvanceChar() 
{
  if ( mColumn < mText[mLine].size()-1 )
    {
      ++mColumn;
    }
  else
    {
      mLineBreak = true;
      mColumn = 0;
      do
	{
	  if (mLine>=mText.size()-1)
	    {
	      throw Eot( "",
			 this->File(),
			 this->Line(),
			 this->Column() ); 
	    }
	  else
	    {
	      ++mLine;
	    }
	}
      while ( 1 > mText[mLine].size() ); // Skip empty lines.
    }
}


void SourceText::AdvanceLine() 
{
  mLineBreak = true;
  mColumn = 0;
  do
    {
      if (mLine>=mText.size()-1)
	{
	  throw Eot( "",
		     this->File(),
		     this->Line(),
		     this->Column() ); 
	}
      else
	{
	  ++mLine;
	}
    }
  while ( 1 > mText[mLine].size() ); // Skip empty lines.
}


const string& SourceText::File()
{
  size_t i    = 0;
  bool   stop = false;

  while ( i<mSfLine.size()-1 && !stop )
    {
      if (mLine>=mSfLine[i+1]) ++i;
      else                     stop = true;
    }

  return mSfName[i];
}


int SourceText::Line()
{
  size_t i    = 0;
  bool   stop = false;

  while ( i<mSfLine.size()-1 && !stop )
    {
      if (mLine>=mSfLine[i+1]) ++i;
      else                     stop = true;
    }

  return mLine - mSfLine[i] + 1; 
}


void SourceText::Init()
{
  mLine   = 0;
  mColumn = 0;
    
  if ( 1 > mText.size() )
    {
      throw Eot( "Empty text!",
		 this->File(),
		 this->Line(),
		 this->Column() ); 
    }
  else
    {
      // Skip empty lines:
      while ( 1 > mText[mLine].size() )
	{
	  if (mLine>=mText.size()-1)
	    {
	      throw Eot( "",
			 this->File(),
			 this->Line(),
			 this->Column() ); 
	    }
	  else
	    {
	      mLineBreak = true;
	      ++mLine;
	    }
	}
    }
}


ostream& operator << (ostream& os, const SourceText& text)
{
  for (size_t i=0; i<text.mText.size();++i)
    cout << i
	 << "(" << text.mText[i].size() << ")"
	 << ": " << text.mText[i] << '\n';
  return(os);
}


/** Returns true if this character is considered whitespace. This
    includes the comment sign `\#'. This function is used by other
    functions to test for delimiting whitespace. 

    The whitespace cases implemented here must be consistent with
    eat_whitespace! 
    @see eat_whitespace  */
bool is_whitespace(const char c)
{
  switch (c) 
    {
    case ' ':
    case '\t':
    case '#':
      return true;
      break;
    default:
      return false;
      break;
    }
}

/** Eats whitespace. Comments are a special case of
    whitespace. Everything from the `\#' to the end of the line is
    eaten. 
  
    The whitespace cases implemented here must be consistent with
    is_whitespace! 
    @see is_whitespace    
  
    @exceptions UnexpectedChar Non-whitespace character encountered. */
void eat_whitespace(SourceText& text)
{
  char dummy;

//   cout << "o" << text.Current() << "o\n";

//   int i=text.Current();
//   cout << "i=" << i << '\n';

  while (is_whitespace(dummy=text.Current())) 
    {
//      cout << "o" << dummy << "o\n";
      switch (dummy)
	{
	case ' ':
	case '\t':
	  text.AdvanceChar();
	  break;
	case '#':
	  text.AdvanceLine();
	  break;
	default:
	  {
	    std::ostrstream os;
	    os << "Expected whitespace, but got `" << dummy << "'.";
	    throw UnexpectedChar( os.str(),
				  text.File(),
				  text.Line(),
				  text.Column() ); 
	    break;
	  }
	}
    }
}


/** Reads name of method, keyword, or workspace variable.
  
    These names may consist only of letters (case matters!), numbers, and
    underscores.
    Line break or any other character ends the name.
  
    Whitespace has to have been eaten before. Scanns text for the
    name, starting at position specified by line and column. */
void read_name(string& name, SourceText& text)
{
  bool stop = false;
  name = "";

  while (!stop) 
    {
      char dummy = text.Current();

      if ( isalnum(dummy) || '_'==dummy )
	{
	  name += dummy;
	  // AdvanceChar sets LineBreak if a line break occured.
	  text.LineBreak() = false;
	  text.AdvanceChar();
	  if ( text.LineBreak() ) stop = true;
	}
      else
	{
	  stop = true;
	}
    }

  //  cout << "Name: " << name << '\n';
}

/** Make sure that the current character is equal to c and go to the
    next character.
  
    @exception UnexpectedChar The character is not right. */
void assertain_character(char c, SourceText& text)
{
  if ( c != text.Current() )
    {
      std::ostrstream os;
      os << "Expected `" << c << "', but got `" << text.Current() << "'.";
      throw UnexpectedChar( os.str(),
			    text.File(),
			    text.Line(),
			    text.Column() ); 
    }
  
  text.AdvanceChar();
}

/** Reads a string, complete with quotation marks. Whitespace has to
    have been eaten before, that is, the current character must be
    the quotation mark (").
    Quotation marks inside strings are currently not possible.
  
    Line breaks inside strings are not allowed. 

    @exception IllegalLinebreak An illegal linebreak has occured. */
void parse_string(string& res, SourceText& text)
{
  bool stop = false;
  res = "";

  text.LineBreak() = false;
  assertain_character('"',text);
  if ( text.LineBreak() )
    throw IllegalLinebreak( "Line break before end of string.",
			    text.File(),
			    text.Line(),
			    text.Column() ); 

  while (!stop) 
    {
      char dummy = text.Current();
      if ( dummy != '"' )
	{
	  res += dummy;
	  text.AdvanceChar();

	  if ( text.LineBreak() )
	    throw IllegalLinebreak( "Line break before end of string.",
				    text.File(),
				    text.Line(),
				    text.Column() ); 
	}
      else
	{
	  stop = true;
	  text.AdvanceChar();
	}
    }
}

/** Reads an integer. Whitespace has to
    have been eaten before, that is, the current character must be
    a number or `+' or `-'.
  
    Whitespace or line breaks terminate the scanning!
    There are no whitespaces allowed anywhere, consisten with ANSI C
    scanf. 

    @exception IllegalLinebreak An illegal linebreak has occured. 
    @exception UnexpectedChar Unexpected character encountered. */
void read_integer(string& res, SourceText& text)
{
  bool stop = false;
  res = "";
  char dummy;
  text.LineBreak() = false;

  dummy = text.Current();
  if ( '+' == dummy || '-' == dummy )
    {
      res += dummy;
      text.AdvanceChar();
      if ( text.LineBreak() )
	throw IllegalLinebreak( "Line break after sign.",
				text.File(),
				text.Line(),
				text.Column() ); 
    }

  if (!isdigit(text.Current()))
    {
      std::ostrstream os;
      os << "Expected digit, but got `" << text.Current() << "'.";
      throw UnexpectedChar(os.str(),
			   text.File(),
			   text.Line(),
			   text.Column());
    }

  while (!stop) 
    {
      char dummy = text.Current();
      if ( isdigit(dummy) )
	{
	  res += dummy;
	  text.AdvanceChar();
	  if ( text.LineBreak() ) stop = true;
	}
      else
	{
	  stop = true;
	}
    }
}

/** Reads a floating point number. Whitespace has to
    have been eaten before, that is, the current character must be
    a number or `+' or `-'.
  
    Example numbers: 23.,  1.0,  -.3,  -3.3e5,  +3e8,  1.0E-9
  
    Illegal numbers: ., 3e, e3, 2e-
  
    Whitespace is not allowed inside the number.
    Line breaks or whitespace terminates the scanning. 

    @exception IllegalLinebreak Illegal line break.
    @exception ParseError Cannot parse this as a number. */
void read_numeric(string& res, SourceText& text)
{
  bool stop;
  res = "";
  char dummy;
  text.LineBreak() = false;

  // To make sure that there is at least one digit:
  bool found_digit = false;

  // Check if there is a sign:
  dummy = text.Current();
  if ( '+' == dummy || '-' == dummy )
    {
      res += dummy;
      text.AdvanceChar();
      if ( text.LineBreak() )
	throw IllegalLinebreak( "Linebreak after sign.",
				text.File(),
				text.Line(),
				text.Column() ); 
    }

  // There could be some digits here:
  stop = false;
  while (!stop) 
    {
      char dummy = text.Current();
      if ( isdigit(dummy) )
	{
	  found_digit = true;
	  res += dummy;
	  text.AdvanceChar();
	  if ( text.LineBreak() ) return; // Line break ends scanning immediately.
	}
      else
	{
	  stop = true;
	}
    }

  // Next there can be a decimal point
  if ( '.' == text.Current() )
    {
      res += ".";
      text.AdvanceChar();
      if ( text.LineBreak() )
	if (found_digit)
	  {
	    // Line break ends scanning immediately, if we have
	    // already found at least one digit.
	    return;
	  }
	else
	  {
	    throw IllegalLinebreak("Expected at least one digit.",
				   text.File(),
				   text.Line(),
				   text.Column());
	  }

      // ... followed by optional more digits
      stop = false;
      while (!stop) 
	{
	  char dummy = text.Current();
	  if ( isdigit(dummy) )
	    {
	      found_digit = true;
	      res += dummy;
	      text.AdvanceChar();
	      if ( text.LineBreak() ) return; // Line break ends scanning immediately.
	    }
	  else
	    {
	      stop = true;
	    }
	}    
    }

  // At this point, we must have found at least one digit.
  if (!found_digit)
    throw ParseError("Expected at least one digit.",
		     text.File(),
		     text.Line(),
		     text.Column());

  // Now there could be a `e' or `E':
  dummy = text.Current();
  if ( 'e' == dummy || 'E' == dummy )
    {
      res += dummy;
      text.AdvanceChar();
      if ( text.LineBreak() )
	throw IllegalLinebreak( "Linebreak after e/E.",
				text.File(),
				text.Line(),
				text.Column() );
      
      // Now there must be an integer (with optional sign)
      {
	string s;
	read_integer(s,text);
	res += s;
      }
    }
}

/** Use a string stream to parse an integer number. */
void parse_integer(int& n, SourceText& text)
{
  string res;
  read_integer(res, text);
  // [**These are strange tricks to work around non-compliance to ANSI 
  // of EGCS. Change this when it's no longer necessary to:
  // std::istrstream(res) >> n  (should be sufficient)]
  char *s = &res[0];
  std::istrstream is(s,res.size());
  is >> n;
}

/** Use a string stream to parse a floating point number. */
void parse_numeric(Numeric& n, SourceText& text)
{
  string res;
  read_numeric(res, text);
  // [**These are strange tricks to work around non-compliance to ANSI 
  // of EGCS. Change this when it's no longer necessary to:
  // std::istrstream(res) >> n  (should be sufficient)]
  char *s = &res[0];
  std::istrstream is(s,res.size());
  is >> n;
}

/** Read a vector of strings. This looks as follows in the control
    file: ["string1","string2"]

    Whitespace has to have been eaten before, that is, the current
    character must be `['.
  
    The empty vector is allowed.
  
    Quotation marks inside strings are currently not possible.
  
    Line breaks are allowed before and after each string. Line breaks
    inside strings are not allowed. 
   
    @see parse_string */
void parse_stringvector(ARRAY<string>& res, SourceText& text)
{
  bool first = true;		// To skip the first comma.
  res.clear();			// Clear the result vector (just in case).

  // Make sure that the current character really is `[' and proceed.
  assertain_character('[',text);
  // There might have occured a linebreak, which is fine.
  
  eat_whitespace(text);

  // Read the elements of the vector (`]' means that we have
  // reached the end):
  while ( ']' != text.Current() )
    {
      string dummy;

      if (first)
	first = false;
      else
	{
	  assertain_character(',',text);
	  eat_whitespace(text);
	}

      parse_string(dummy, text);
      res.push_back(dummy);
      eat_whitespace(text);
    }

  text.AdvanceChar();
}

/** Read a vector of integers. This looks as follows in the control
    file: [123,5,334]
    Whitespace has to have been eaten before, that is, the current
    character must be `['.
  
    The empty vector is allowed.
    
    Line breaks are allowed before and after each number. Line breaks
    inside numbers are not allowed. 
   
    @see parse_integer */
void parse_intvector(ARRAY<int>& res, SourceText& text)
{
  bool first = true;		// To skip the first comma.
  res.clear();			// Clear the result vector (just in case).

  // Make sure that the current character really is `[' and proceed.
  assertain_character('[',text);
  // There might have occured a linebreak, which is fine.
  
  eat_whitespace(text);

  // Read the elements of the vector (`]' means that we have
  // reached the end):
  while ( ']' != text.Current() )
    {
      int dummy;

      if (first)
	first = false;
      else
	{
	  assertain_character(',',text);
	  eat_whitespace(text);
	}

      parse_integer(dummy, text);
      res.push_back(dummy);
      eat_whitespace(text);
    }

  text.AdvanceChar();
}

/** Read a vector of Numerics. This looks as follows in the control
    file: [1.3, 5, 3.4]
    Whitespace has to have been eaten before, that is, the current
    character must be `['.
  
    The empty vector is allowed.
  
    Line breaks are allowed before and after each number. Line breaks
    inside numbers are not allowed. 
   
    @see parse_numeric */
void parse_numvector(ARRAY<Numeric>& res, SourceText& text)
{
  bool first = true;		// To skip the first comma.
  res.clear();			// Clear the result vector (just in case).

  // Make sure that the current character really is `[' and proceed.
  assertain_character('[',text);
  // There might have occured a linebreak, which is fine.
  
  eat_whitespace(text);

  // Read the elements of the vector (`]' means that we have
  // reached the end):
  while ( ']' != text.Current() )
    {
      Numeric dummy;

      if (first)
	first = false;
      else
	{
	  assertain_character(',',text);
	  eat_whitespace(text);
	}

      parse_numeric(dummy, text);
      res.push_back(dummy);
      eat_whitespace(text);
    }

  text.AdvanceChar();
}

/** Parse the Contents of text as ARTS control input. 

    @return True: This was the last method.

    @param id     Output. Method id.
    @param values Output. Keyword parameter values for this method.
    @param output Output. Output workspace variables (for generic methods).
    @param input  Output. Input workspace variables (for generic methods).
  
    @param text The input to parse.
   
    @see read_name
    @see eat_whitespace
    @see assertain_character
    @see parse_string
    @see parse_integer
    @see parse_numeric
    @see parse_stringvector
    @see parse_intvector
    @see parse_numvector
   
   @exception UnknownMethod
   @exception UnknownWsv
   @exception WrongWsvGroup
   @exception UnexpectedKeyword

   @author Stefan Buehler  */
bool parse_method(size_t& id, 
		  ARRAY<TokVal>& values,
		  ARRAY<size_t>& output,
		  ARRAY<size_t>& input,
		  SourceText& text,
		  const std::map<string, size_t> MdMap,
		  const std::map<string, size_t> WsvMap)
{
  const extern ARRAY<WsvRecord> wsv_data;
  const extern ARRAY<MdRecord> md_data;
  const extern ARRAY<string> wsv_group_names;

  size_t wsvid;			// Workspace variable id, is used to
				// access data in wsv_data.

  // Clear all output variables:
  id = 0;
  values.clear();
  output.clear();
  input.clear();

  {
    string methodname;
    read_name(methodname, text);

    {
      // Find method id:
      const map<string, size_t>::const_iterator i = MdMap.find(methodname);
      if ( i == MdMap.end() )
	throw UnknownMethod(methodname,
			    text.File(),
			    text.Line(),
			    text.Column());

      id = i->second;
    }
    //    cout << "id=" << id << '\n';   
    // cout << "Method: " << md_data[id].Name() << '\n';
  }

  eat_whitespace(text);

  // For generic methods the output and input workspace variables have 
  // to be parsed (given in round brackets).
  if ( 0 < md_data[id].GOutput().size() + md_data[id].GInput().size() )
    {
      //      cout << "Generic!" << id << md_data[id].Name() << '\n';
      string wsvname;
      bool first = true;	// To skip the first comma.

      assertain_character('(',text);
      eat_whitespace(text);

      // First read all output Wsvs:
      for ( size_t j=0 ; j<md_data[id].GOutput().size() ; ++j )
	{
	  if (first)
	    first = false;
	  else
	    {
	      assertain_character(',',text);
	      eat_whitespace(text);
	    }

	  read_name(wsvname, text);

	  {
	    // Find Wsv id:
	    const map<string, size_t>::const_iterator i = WsvMap.find(wsvname);
	    if ( i == WsvMap.end() )
	      throw UnknownWsv( wsvname,
				text.File(),
				text.Line(),
				text.Column() );

	    wsvid = i->second;
	  }

	  // Check that this Wsv belongs to the correct group:
	  if ( wsv_data[wsvid].Group() != md_data[id].GOutput()[j] )
	    throw WrongWsvGroup( wsvname+"Group should be "+
				 wsv_group_names[md_data[id].Output()[j]],
				 text.File(),
				 text.Line(),
				 text.Column() );

	  // Add this one to the list of output workspace variables:
	  output.push_back(wsvid);
	  
	  eat_whitespace(text);
	}

      // Then read all input Wsvs:
      for ( size_t j=0 ; j<md_data[id].GInput().size() ; ++j )
	{
	  if (first)
	    first = false;
	  else
	    {
	      assertain_character(',',text);
	      eat_whitespace(text);
	    }

	  read_name(wsvname, text);

	  {
	    // Find Wsv id:
	    const map<string, size_t>::const_iterator i = WsvMap.find(wsvname);
 	    if ( i == WsvMap.end() )
 	      throw UnknownWsv( wsvname,
 				text.File(),
 				text.Line(),
 				text.Column() );

	    wsvid = i->second;
	  }
// 	  cout << "wsvid = " << wsvid << endl;
// 	  cout << "Variable: " << wsv_data[wsvid].Name() << '\n';
// 	  cout << "wsv_data[wsvid].Group() = " << wsv_data[wsvid].Group() << endl;
// 	  cout << "md_data[id].Input()"    << md_data[id].Input() << endl;

	  // Check that this Wsv belongs to the correct group:
	  if ( wsv_data[wsvid].Group() != md_data[id].GInput()[j] )
	    throw WrongWsvGroup( wsvname+"Group should be "+
				 wsv_group_names[md_data[id].Output()[j]],
				 text.File(),
				 text.Line(),
				 text.Column() );

	  // Add this one to the list of input workspace variables:
	  input.push_back(wsvid);
	  
	  eat_whitespace(text);
	}

      assertain_character(')',text);
      eat_whitespace(text);
    }

  // Now look for the curly braces:
  assertain_character('{',text);
  eat_whitespace(text);
  
  // Look for the keywords and read the parameters:
  for ( size_t i=0 ; i<md_data[id].Keywords().size() ; ++i )
    {
      try 
	{
	  string keyname;
	  read_name(keyname,text);

	  // Is the keyname the expected keyname?
	  if ( keyname != md_data[id].Keywords()[i] )
	    {
	      throw UnexpectedKeyword( keyname,
				       text.File(),
				       text.Line(),
				       text.Column());
	    }

	  eat_whitespace(text);

	  // Look for '='
	  assertain_character('=',text);
	  eat_whitespace(text);
	}
      catch (UnexpectedKeyword x)
	{
	  // Does this method take only a single parameter? In that that case
	  // the keyword is optional, so we should simply proceed and
	  // try to read the value.
	  if (1!=md_data[id].Keywords().size())
	    {
	      throw;		// re-throw the exception
	    }
	}

      // Now parse the key value. This can be:
      // str_,    int_,    num_,
      // strvec_, intvec_, numvec_,
      switch (md_data[id].Types()[i]) 
	{
	case str_:
	  {
	    string dummy;
 	    parse_string(dummy, text);
	    values.push_back(dummy);
	    break;
	  }
	case int_:
	  {
	    int n;
	    parse_integer(n, text);
	    values.push_back(n);
	    break;
	  }
	case num_:
	  {
	    Numeric n;
	    parse_numeric(n, text);
	    values.push_back(n);
	    break;
	  }
	case strvec_:
	  {
	    ARRAY<string> dummy;
	    parse_stringvector(dummy, text);
	    values.push_back(dummy);
	    break;
	  }
	case intvec_:
	  {
	    ARRAY<int> dummy;
	    parse_intvector(dummy, text);
	    values.push_back(dummy);
	    break;
	  }
	case numvec_:
	  {
	    ARRAY<Numeric> dummy;
	    parse_numvector(dummy, text);
	    values.push_back(dummy);
	    break;
	  }
	default:
	  throw logic_error("Impossible parameter type.");
	  break;
	}

      eat_whitespace(text);

      // Check:
      //      cout << "Value: " << md_data[id].Values()[i] << '\n';
    }

  // Now look for the closing curly braces.
  // We have to catch Eot, because after a method description is a
  // good place to end the control file. 
  try
    {
      assertain_character('}',text);
    }
  catch (const Eot x)
    {
      return true;
    }
  return false;
}

/** Parse the Contents of text as ARTS control input. 
  
    @param tasklist Output. The ids and keyword parameter values for the
                    methods to run.
  
   @param text   The input to parse.
   @param MdMap  The method map.
   @param WsvMap The workspace variable map.
   
   @see eat_whitespace
   @see parse_method
         
   @author Stefan Buehler */
void parse(ARRAY<MRecord>& tasklist,
	   SourceText& text,
	   const std::map<string, size_t> MdMap,
	   const std::map<string, size_t> WsvMap)
{
  extern const ARRAY<MdRecord> md_data;
  bool last = false;
  // For method ids:
  size_t id;		
 // For keyword parameter values:
  ARRAY<TokVal> values;
  // Output workspace variables (for generic methods):
  ARRAY<size_t> output;		
  // Input workspace variables (for generic methods):
  ARRAY<size_t> input;

  out3 << "\nParsing:\n";

  eat_whitespace(text);

  while (!last)
    {
      last = parse_method(id,values,output,input,text,MdMap,WsvMap);

      // Append taks to task list:
      tasklist.push_back(MRecord(id,values,output,input));

      {
	// Everything in this block is just to generate some
	// informative output.  
	const extern ARRAY<WsvRecord> wsv_data;

	out3 << "- " << md_data[id].Name() << "\n";

	for ( size_t j=0 ; j<values.size() ; ++j )
	  {
	    out3 << "   " 
		 << md_data[id].Keywords()[j] << ": "
		 << values[j] << '\n';
	  }
	  
	// Output workspace variables for generic methods:
	if ( 0 < md_data[id].GOutput().size() + md_data[id].GInput().size() )
	  {
	    out3 << "   Output: ";
	    for ( size_t j=0 ; j<output.size() ; ++j )
	      {
		out3 << wsv_data[output[j]].Name() << " ";
	      }
	    out3 << "\n";

	    out3 << "   Input: ";
	    for ( size_t j=0 ; j<input.size() ; ++j )
	      {
		out3 << wsv_data[input[j]].Name() << " ";
	      }
	    out3 << "\n";
	  }
      }

      // If last is set, then we have anyway reached the end of the
      // text, so we don't have to eat whitespace.
      if (!last)
	try
	  {
	    eat_whitespace(text);
	  }
	catch (const Eot x)
	  {
	    last = true;
	  }
    }
}

void parse_main(ARRAY<MRecord>& tasklist, SourceText& text)
{
  std::map<string, size_t> MdMap;
  std::map<string, size_t> WsvMap;
  extern const ARRAY<MdRecord> md_data;
  extern const ARRAY<WsvRecord> wsv_data;

  // Initialize MdMap
  for ( size_t i=0 ; i<md_data.size() ; ++i)
    {
      MdMap[md_data[i].Name()] = i;
    }

  // Initialize WsvMap
  for ( size_t i=0 ; i<wsv_data.size() ; ++i)
    {
      WsvMap[wsv_data[i].Name()] = i;
    }

  try 
    {
      text.Init();
      parse(tasklist,text,MdMap,WsvMap);
    }
  catch (const Eot x)
    {
      // Unexpected end of the source text:
      out0 << "Unexpected end of control script.\n";
      out0 << "File: " << x.file() << '\n';
      out0 << "Line: " << x.line() << '\n';
      exit(true);
    }
  catch (const UnexpectedChar x)
    {
      // Unexpected Character:
      out0 << "Unexpected character:\n";
      out0 << x.what()   << '\n';
      out0 << "File: "   << x.file() << '\n';
      out0 << "Line: "   << x.line() << '\n';
      out0 << "Column: " << x.column() << '\n';
      exit(true);
    }
  catch (const IllegalLinebreak x)
    {
      // A line break in an illegal position:
      out0 << "Illegal Line break:\n";
      out0 << x.what()   << '\n';
      out0 << "File: "   << x.file() << '\n';
      out0 << "Line: "   << x.line() << '\n';
      exit(true);
    }
  catch (const UnknownMethod x)
    {
      // Method unknown:
      // [**This should give a hint on how to obtain a list of allowed 
      // methods.]
      out0 << "Unknown Method:\n";
      out0 << x.what()   << '\n';
      out0 << "File: "   << x.file() << '\n';
      out0 << "Line: "   << x.line() << '\n';
      out3 << "Column: " << x.column() << '\n';
      exit(true);
    }
  catch (const UnknownWsv x)
    {
      // Workspace variable unknown:
      // [**This should give a hint on how to obtain a list of allowed 
      // Wsvs.]
      out0 << "Unknown workspace variable:\n";
      out0 << x.what()   << '\n';
      out0 << "File: "   << x.file() << '\n';
      out0 << "Line: "   << x.line() << '\n';
      out3 << "Column: " << x.column() << '\n';
      exit(true);
    }
  catch (const WrongWsvGroup x)
    {
      // Workspace variable unknown:
      // [**This should give a hint on how to obtain a list of Wsvs in 
      // this group.
      out0 << "Workspace variable belongs to the wrong group:\n";
      out0 << x.what()   << '\n';
      out0 << "File: "   << x.file() << '\n';
      out0 << "Line: "   << x.line() << '\n';
      out3 << "Column: " << x.column() << '\n';
      exit(true);
    }
  catch (const UnexpectedKeyword x)
    {
      // Keyword unknown:
      // [**This should give a hint on how to obtain a list of allowed 
      // keywords.]
      out0 << "Unknown keyword:\n";
      out0 << x.what()   << '\n';
      out0 << "File: "   << x.file() << '\n';
      out0 << "Line: "   << x.line() << '\n';
      out3 << "Column: " << x.column() << '\n';
      exit(true);
    }
  catch (const ParseError x)
    {
      // General Parse Error (parent of all the above):
      out0 << "Parse error:\n";
      out0 << x.what()   << '\n';
      out0 << "File: "   << x.file() << '\n';
      out0 << "Line: "   << x.line() << '\n';
      out0 << "Column: " << x.column() << '\n';
      exit(true);
    }
  catch (const runtime_error x)
    {
      cout << "Runtime error: ";
      cout << x.what() << '\n';
    }
  catch (const logic_error x)
    {
      cout << "Logic error: ";
      cout << x.what() << '\n';
    }

}
