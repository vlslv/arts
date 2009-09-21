/* Copyright (C) 2002-2008 Oliver Lemke <olemke@core-dump.info>

   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; either version 2, or (at your option) any
   later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA. */

/*!
  \file   m_xml.h
  \author Stefan Buehler <sbuehler@ltu.se>
  \date   2002-06-18

  \brief  Workspace methods and template functions for supergeneric XML IO.

*/

#ifndef m_xml_h
#define m_xml_h

#include "exceptions.h"
#include "xml_io.h"
#include "workspace_ng.h"
#include "agenda_class.h"

/* Workspace method: Doxygen documentation will be auto-generated */
template<typename T> void
ReadXML (// WS Generic Output:
         T&            v,
         // WS Generic Output Names:
         const String& v_name,
         // WS Generic Input:
         const String& f,
         // WS Generic Input Names:
         const String& f_name _U_)
{
  String filename = f;

  // Create default filename if empty
  filename_xml (filename, v_name);

  xml_read_from_file (filename, v);
}


/* Workspace method: Doxygen documentation will be auto-generated */
void
ReadXML (Workspace&    ws _U_,
         // WS Generic Output:
         Agenda&       v,
         // WS Generic Output Names:
         const String& v_name,
         // WS Generic Input:
         const String& f,
         // WS Generic Input Names:
         const String& f_name)
{
  ReadXML (v, v_name, f, f_name);
}


/* Workspace method: Doxygen documentation will be auto-generated */
template<typename T> void
WriteXML (//WS Input:
          const String& file_format,
          // WS Generic Input:
          const T&            v,
          const String& f,
          // WS Generic Input Names:
          const String& v_name,
          const String& f_name _U_)

{
  String filename = f;
  FileType ftype;

  // Create default filename if empty
  filename_xml (filename, v_name);

  if (file_format == "ascii")
    ftype = FILE_TYPE_ASCII;
  else if (file_format == "zascii")
    ftype = FILE_TYPE_ZIPPED_ASCII;
  else if (file_format == "binary")
    ftype = FILE_TYPE_BINARY;
  else
    throw runtime_error ("file_format contains illegal string. "
                         "Valid values are:\n"
                         "  ascii:  XML output\n"
                         "  zascii: Zipped XML output\n"
                         "  binary: XML + binary output");

  xml_write_to_file (filename, v, ftype);
}


/* Workspace method: Doxygen documentation will be auto-generated */
void
WriteXML (Workspace& ws _U_,
          //WS Input:
          const String& file_format,
          // WS Generic Input:
          const Agenda& v,
          const String& f,
          // WS Generic Input Names:
          const String& v_name,
          const String& f_name)
{
  WriteXML (file_format, v, f, v_name, f_name);
}


/* Workspace method: Doxygen documentation will be auto-generated */
template<typename T> void
WriteXMLIndexed (//WS Input:
                 const String& file_format,
                 const Index&  file_index,
                 // WS Generic Input:
                 const T&      v,
                 const String& f,
                 // WS Generic Input Names:
                 const String& v_name,
                 const String& f_name _U_)
{
  String filename = f;

  // Create default filename if empty
  filename_xml_with_index( filename, file_index, v_name );

  WriteXML( file_format, v, filename, v_name, f_name );
}


/* Workspace method: Doxygen documentation will be auto-generated */
void
WriteXMLIndexed (Workspace& ws _U_,
                 //WS Input:
                 const String& file_format,
                 const Index&  file_index,
                 // WS Generic Input:
                 const Agenda& v,
                 const String& f,
                 // WS Generic Input Names:
                 const String& v_name,
                 const String& f_name)
{
  WriteXMLIndexed (file_format, file_index, v, f, v_name, f_name);
}


/* Workspace method: Doxygen documentation will be auto-generated */
void
output_file_formatSetAscii (// WS Output:
                            String& file_format)
{
  file_format = "ascii";
}


/* Workspace method: Doxygen documentation will be auto-generated */
void
output_file_formatSetZippedAscii (// WS Output:
                                  String& file_format)
{
  file_format = "zascii";
}


/* Workspace method: Doxygen documentation will be auto-generated */
void
output_file_formatSetBinary (// WS Output:
                             String& file_format)
{
  file_format = "binary";
}


#endif // m_xml_h
