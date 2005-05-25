/* Copyright (C) 2000, 2001 Stefan Buehler <sbuehler@uni-bremen.de>

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
  \file   agenda_class.h
  \author Stefan Buehler <sbuehler@uni-bremen.de>
  \date   Thu Mar 14 08:49:33 2002
  
  \brief  Declarations for agendas.
*/

#ifndef agenda_class_h
#define agenda_class_h

#include "token.h"

// ... and MRecord
class MRecord;

//! The Agenda class.
/*! An agenda is a list of workspace methods (including keyword data)
  to be executed. There are workspace variables of class agenda that
  can contain a list of methods to execute for a particular purpose,
  for example to compute the lineshape in an absorption
  calculation. 
*/
class Agenda {
public:
  void push_back(MRecord n);
  void execute(bool silent=false) const;
  void resize(Index n);
  Index nelem() const;
  Agenda& operator=(const Agenda& x);
  bool is_input(Index var) const;
  bool is_output(Index var) const;
  void set_name(const String& nname);
  String name() const;
  void print( ostream& os,
              const String& indent ) const;
private:
  String         mname; /*!< Agenda name. */
  Array<MRecord> mml;   /*!< The actual list of methods to execute. */
};

// Documentation with implementation.
ostream& operator<<(ostream& os, const Agenda& a);


/** Method runtime data. In contrast to MdRecord, an object of this
    class contains the runtime information for one method: The method
    id and the keyword parameter values. This is all that the engine
    needs to execute the stack of methods.

    An MRecord includes a member magenda, which can contain an entire
    agenda, i.e., a list of other MRecords. 

    @author Stefan Buehler */
class MRecord {
public:
  MRecord(){ /* Nothing to do here. */ }
  MRecord(const Index id,
          const Array<TokVal>& values,
          const ArrayOfIndex& output,
          const ArrayOfIndex& input,
          const Agenda&       tasks)
    : mid(id),
      mvalues( values ),
      moutput( output ),
      minput(  input  ),
      mtasks( tasks )
  { 
    // Initialization of arrays from other array now works correctly.
  }
  Index                Id()       const { return mid;     }
  const Array<TokVal>& Values()   const { return mvalues; }
  const ArrayOfIndex&  Output()   const { return moutput; }
  const ArrayOfIndex&  Input()    const { return minput;  }
  const Agenda&        Tasks()    const { return mtasks;  }

  // Assignment operator:
  MRecord& operator=(const MRecord& x);

  // Output operator:
  void                 print( ostream& os,
                              const String& indent ) const;

private:
  /** Method id. */
  Index mid;
  /** List of parameter values (see methods.h for definition of
      TokVal). */
  Array<TokVal> mvalues;
  /** Output workspace variables (for generic methods). */
  ArrayOfIndex moutput;
  /** Input workspace variables (for generic methods). */
  ArrayOfIndex minput;
  /** An agenda, which can be given in the controlfile instead of
      keywords. */
  Agenda mtasks;
};

// Documentation is with implementation.
ostream& operator<<(ostream& os, const MRecord& a);

/**************************************************************
 * only for testing
 * these functions will be autogenerated.
 **************************************************************/

void
doit_conv_test_agendaExecute(// WS Input & Output
                             Index &doit_conv_flag,
                             Index &doit_iteration_counter,
                             // WS Input
                             const Tensor6 &doit_i_field,
                             const Tensor6 &doit_i_field_old,
                             const Agenda &input_agenda,
                             bool silent);

void
doit_scat_field_agendaExecute(// WS Input & Output
                              Tensor6 &doit_scat_field,
                              // WS Input
                              const Tensor6 &doit_i_field,
                              const Agenda &input_agenda,
                              bool silent);

void
doit_rte_agendaExecute(// WS Output
                       Tensor6 &doit_i_field,
                       const Agenda &input_agenda,
                       bool silent);

void
pha_mat_spt_agendaExecute(// WS Input & Output
                          Tensor5 &pha_mat_spt,
                          // WS Input
                          Index &scat_za_index,
                          Index &scat_lat_index,
                          Index &scat_lon_index,
                          Index &scat_p_index,
                          Index &scat_aa_index,
                          Numeric &rte_temperature,
                          const Agenda &input_agenda,
                          bool silent);

#endif
