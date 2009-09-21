/* Copyright (C) 2004-2008 Mattias Ekstrom <ekstrom@rss.chalmers.se>

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
   USA. 
*/



/*===========================================================================
  ===  File description
  ===========================================================================*/

/*!
  \file   m_jacobian.cc
  \author Mattias Ekstrom <ekstrom@rss.chalmers.se>
  \date   2004-09-14

  \brief  Workspace functions related to the jacobian.

  These functions are listed in the doxygen documentation as entries of the
  file auto_md.h.
*/


/*===========================================================================
  === External declarations
  ===========================================================================*/

#include <cmath>
#include <string>
#include "arts.h"
#include "auto_md.h"
#include "check_input.h"
#include "math_funcs.h"
#include "messages.h"
#include "jacobian.h"
#include "rte.h"
#include "absorption.h"
#include "physics_funcs.h"

extern const String ABSSPECIES_MAINTAG;
extern const String FREQUENCY_MAINTAG;
extern const String POINTING_MAINTAG;
extern const String POLYFIT_MAINTAG;
extern const String TEMPERATURE_MAINTAG;


/*===========================================================================
  === The methods, with general methods first followed by the Add/Calc method
  === pairs for each retrieval quantity.
  ===========================================================================*/


//----------------------------------------------------------------------------
// General methods:
//----------------------------------------------------------------------------


/* Workspace method: Doxygen documentation will be auto-generated */
void jacobianCalc(      
        Workspace&                 ws,
        Matrix&                    jacobian,
  const Agenda&                    jacobian_agenda,
  const ArrayOfArrayOfIndex&       jacobian_indices )
{
  // Check that the jacobian has been initialised. This is covered by the
  // next test, but gives a better output for this specific case.
  if( jacobian.ncols() == 0 )
    {
      ostringstream os;
      os << "The Jacobian matrix has not been properly initialised.\n"
         << "The WSM jacobianClose has to be called prior to jacobianCalc.\n";
      throw runtime_error(os.str());
    }

  // Check that *jacobian_indices* and *jacobian* are consistent
  ArrayOfIndex last_ind = jacobian_indices[jacobian_indices.nelem()-1];
  if( jacobian.ncols()-1!=last_ind[1] )
    {
      ostringstream os;
      os << "There are more retrieval quantities in *jacobian_quantities*\n"
         << "than in *jacobian*. After calling jacobianClose no more\n"
         << "quantities can be added.";
      throw runtime_error(os.str());
    }

  // Output message
  out2 << "  Calculating *jacobian*.\n";

  // Run jacobian_agenda (can be empty)
  if( jacobian_agenda.nelem() > 0 )
    jacobian_agendaExecute( ws, jacobian, jacobian_agenda );
}



/* Workspace method: Doxygen documentation will be auto-generated */
void jacobianClose(
        Matrix&                    jacobian,
        ArrayOfArrayOfIndex&       jacobian_indices,
  const ArrayOfRetrievalQuantity&  jacobian_quantities,
  const Matrix&                    sensor_pos,
  const Sparse&                    sensor_response )
{
  // Check that *jacobian* has been initialised
  if( jacobian.nrows() != 0  &&  jacobian.ncols() != 0 )
    {
      ostringstream os;
      os << "The Jacobian matrix has not been initialised, or calculations\n"
         << "have alread been performed.";
      throw runtime_error(os.str());
    }

  // Make sure that the array is not empty
  if( jacobian_quantities.nelem() == 0 )
    throw runtime_error(
          "No retrieval quantities has been added to *jacobian_quantities*." );

  // Check that sensor_pol and sensor_response has been initialised
  if( sensor_pos.nrows() == 0 )
    {
      ostringstream os;
      os << "The number of rows in *sensor_pos* is zero, i.e. no measurement\n"
         << "blocks has been defined. This has to be done before calling\n"
         << "jacobianClose.";
      throw runtime_error(os.str());
    }
  if( sensor_response.nrows() == 0 )
    {
      ostringstream os;
      os << "The sensor has either to be defined or turned off before calling\n"
         << "jacobianClose.";
      throw runtime_error(os.str());
    }

  // Loop over retrieval quantities, set JacobianIndices
  Index nrows = sensor_pos.nrows() * sensor_response.nrows();
  Index ncols = 0;
  //
  for( Index it=0; it<jacobian_quantities.nelem(); it++ )
    {
      // Store start jacobian index
      ArrayOfIndex indices(2);
      indices[0] = ncols;

      // Count total number of field points, i.e. product of grid lengths
      Index cols = 1;
      ArrayOfVector grids = jacobian_quantities[it].Grids();
      for( Index jt=0; jt<grids.nelem(); jt++ )
        { cols *= grids[jt].nelem(); }

      // Store stop index
      indices[1] = ncols + cols - 1;
      jacobian_indices.push_back( indices );

      ncols += cols;
    }
  
  // Resize *jacobian*
  jacobian.resize( nrows, ncols );
  jacobian = 0;
}



/* Workspace method: Doxygen documentation will be auto-generated */
void jacobianInit(
        Matrix&                    jacobian,
        ArrayOfRetrievalQuantity&  jacobian_quantities,
        ArrayOfArrayOfIndex&       jacobian_indices,
        Agenda&                    jacobian_agenda )
{
  jacobian.resize(0,0);
  jacobian_quantities.resize(0);
  jacobian_indices.resize(0);
  jacobian_agenda = Agenda();
  jacobian_agenda.set_name( "jacobian_agenda" );
}



/* Workspace method: Doxygen documentation will be auto-generated */
void jacobianOff(
        Matrix&                    jacobian,
        ArrayOfRetrievalQuantity&  jacobian_quantities,
        ArrayOfArrayOfIndex&       jacobian_indices,
        String&                    jacobian_unit )
{
  Agenda dummy;

  jacobianInit( jacobian, jacobian_quantities, jacobian_indices, dummy );
  
  jacobian_unit = "-";
}



/* Workspace method: Doxygen documentation will be auto-generated */
void jacobianUnit(
        Matrix&   jacobian,
  const String&   jacobian_unit,
  const String&   y_unit,
  const Vector&   y_f )
{
  String j_unit = jacobian_unit;
  //
  if ( jacobian_unit == "-" )
    { j_unit = y_unit; }
  
  try
    {
      ybatchUnit( jacobian, j_unit, y_f );
    }
  catch( runtime_error ) 
    {
      ostringstream os;
      os << "Unknown option: jacobian_unit = \"" << j_unit << "\"\n" 
         << "Recognised choices are: \"-\", \"1\", \"RJBT\" and \"PlanckBT\"";
      throw runtime_error( os.str() );      
    }
}





//----------------------------------------------------------------------------
// Absorption species:
//----------------------------------------------------------------------------

/* Workspace method: Doxygen documentation will be auto-generated */
void jacobianAddAbsSpecies(
  Workspace&                  ws,
  ArrayOfRetrievalQuantity&   jq,
  Agenda&                     jacobian_agenda,
  const Matrix&               J,
  const Index&                atmosphere_dim,
  const Vector&               p_grid,
  const Vector&               lat_grid,
  const Vector&               lon_grid,
  const Vector&               rq_p_grid,
  const Vector&               rq_lat_grid,
  const Vector&               rq_lon_grid,
  const String&               species,
  const String&               method,
  const String&               mode,
  const Numeric&              dx )
{
  // Check that the jacobian matrix is empty. Otherwise it is either
  // not initialised or it is closed.
  if( J.nrows()!=0 && J.ncols()!=0 )
    {
      ostringstream os;
      os << "The Jacobian matrix is not initialised correctly or closed.\n"
         << "New retrieval quantities can not be added at this point.";
      throw runtime_error(os.str());
    }
  
  // Check retrieval grids, here we just check the length of the grids
  // vs. the atmosphere dimension
  ArrayOfVector grids(atmosphere_dim);
  {
    ostringstream os;
    if( !check_retrieval_grids( grids, os, p_grid, lat_grid, lon_grid,
                                rq_p_grid, rq_lat_grid, rq_lon_grid,
                                "retrieval pressure grid", 
                                "retrieval latitude grid", 
                                "retrievallongitude_grid", 
                                atmosphere_dim ) )
    throw runtime_error(os.str());
  }
  
  // Check that method is either "analytic" or "perturbation"
  bool analytical;
  if( method == "perturbation" )
    { analytical = 0; }
  else if( method == "analytical" )
    { analytical = 1; }
  else
    {
      ostringstream os;
      os << "The method for absorption species retrieval can only be "
         << "\"analytical\"\n or \"perturbation\".";
      throw runtime_error(os.str());
    }
  
  // Check that mode is either "vmr", "nd" or "rel"
  if( mode != "vmr"  &&  mode != "nd"  &&  mode != "rel" )
    {
      throw runtime_error(
                "The retrieval mode can only be \"vmr\", \"nd\" or \"rel\"." );
    }

  // Check that this species is not already included in the jacobian.
  for( Index it=0; it<jq.nelem(); it++ )
    {
      if( jq[it].MainTag() == ABSSPECIES_MAINTAG  && jq[it].Subtag() == species )
        {
          ostringstream os;
          os << "The gas species:\n" << species << "\nis already included in "
             << "*jacobian_quantities*.";
          throw runtime_error(os.str());
        }
    }

  // Create the new retrieval quantity
  RetrievalQuantity rq = RetrievalQuantity();
  rq.MainTag( ABSSPECIES_MAINTAG );
  rq.Subtag( species );
  rq.Mode( mode );
  rq.Analytical( analytical );
  rq.Perturbation( dx );
  rq.Grids( grids );

  // Add it to the *jacobian_quantities*
  jq.push_back( rq );
  
  // Add gas species method to the jacobian agenda, if perturbation
  if( analytical )
    {
      out3 << "  Calculations done by semi-analytical expressions.\n"; 
    }
  else
    {
      out2 << "  Adding absorption species: " << species 
           << " to *jacobian_quantities*\n" << "  and *jacobian_agenda*\n";
      out3 << "  Calculations done by perturbation, size " << dx 
           << " " << mode << ".\n"; 

      jacobian_agenda.append( ws, "jacobianCalcAbsSpecies", species );
    }
}                    



/* Workspace method: Doxygen documentation will be auto-generated */
void jacobianCalcAbsSpecies(
        Workspace&                 ws,
        Matrix&                    J,
  const Agenda&                    jacobian_y_agenda,
  const ArrayOfRetrievalQuantity&  jq,
  const ArrayOfArrayOfIndex&       jacobian_indices,
  const Index&                     atmosphere_dim,
  const Vector&                    p_grid,
  const Vector&                    lat_grid,
  const Vector&                    lon_grid,
  const ArrayOfArrayOfSpeciesTag&  abs_species,
  const Vector&                    f_grid,
  const Tensor4&                   vmr_field,
  const Tensor3&                   t_field,
  const Matrix&                    sensor_los,
  const Vector&                    y,
  const String&                    species )
{
  // Set some useful variables. 
  RetrievalQuantity rq;
  ArrayOfIndex      ji;
  Index             it, pertmode;

  // Find the retrieval quantity related to this method, i.e. Abs. species -
  // species. This works since the combined MainTag and Subtag is individual.
  bool found = false;
  for( Index n=0; n<jq.nelem() && !found; n++ )
    {
      if( jq[n].MainTag() == ABSSPECIES_MAINTAG  &&  jq[n].Subtag() == species )
        {
          found = true;
          rq    = jq[n];
          ji    = jacobian_indices[n];
        }
    }
  if( !found )
    {
      ostringstream os;
      os << "There is no gas species retrieval quantities defined for:\n"
         << species;
      throw runtime_error(os.str());
    }

  if( rq.Analytical() )
    {
      ostringstream os;
      os << "This WSM handles only perturbation calculations.\n"
         << "Are you using the method manually? This method should normally be\n"
         << "used through *jacobianCalc*.";
      throw runtime_error(os.str());
    }
  
  // Store the start JacobianIndices and the Grids for this quantity
  it = ji[0];
  ArrayOfVector jg = rq.Grids();

  // Check if a relative pertubation is used or not, this information is needed
  // by the methods 'perturbation_field_?d'.
  // Note: both 'vmr' and 'nd' are absolute perturbations
  if( rq.Mode()=="rel" )
    pertmode = 0;
  else 
    pertmode = 1;

  // For each atmospheric dimension option calculate a ArrayOfGridPos, these
  // are the base functions for interpolating the perturbations into the
  // atmospheric grids.
  ArrayOfGridPos p_gp, lat_gp, lon_gp;
  Index j_p   = jg[0].nelem();
  Index j_lat = 1;
  Index j_lon = 1;
  //
  get_perturbation_gridpos( p_gp, p_grid, jg[0], true );
  //
  if( atmosphere_dim >= 2 ) 
    {
      j_lat = jg[1].nelem();
      get_perturbation_gridpos( lat_gp, lat_grid, jg[1], false );
      if( atmosphere_dim == 3 ) 
        {
          j_lon = jg[2].nelem();
          get_perturbation_gridpos( lon_gp, lon_grid, jg[2], false );
        }
    }

  // Give verbose output
  out2 << "  Calculating retrieval quantity " << rq << "\n";
  
  // Find VMR field for these species. 
  ArrayOfSpeciesTag tags;
  array_species_tag_from_string( tags, species );
  Index si = chk_contains( "species", abs_species, tags );

  // Variables for vmr field perturbation unit conversion
  Tensor3 nd_field( t_field.npages(), t_field.nrows(), t_field.ncols(), 1.0 );
  if( rq.Mode()=="nd" )
    calc_nd_field( nd_field, p_grid, t_field );
  
  // Vector for perturbed measurement vector
  Vector yp;

  // Loop through the retrieval grid and calculate perturbation effect
  for( Index lon_it=0; lon_it<j_lon; lon_it++ )
    {
      for( Index lat_it=0; lat_it<j_lat; lat_it++ )
        {
          for (Index p_it=0; p_it<j_p; p_it++)
            {
              // Here we calculate the ranges of the perturbation. We want the
              // perturbation to continue outside the atmospheric grids for the
              // edge values.
              Range p_range   = Range(0,0);
              Range lat_range = Range(0,0);
              Range lon_range = Range(0,0);

              get_perturbation_range( p_range, p_it, j_p);

              if( atmosphere_dim>=2 )
                {
                  get_perturbation_range( lat_range, lat_it, j_lat );
                  if( atmosphere_dim == 3 )
                    {
                      get_perturbation_range( lon_range, lon_it, j_lon);
                    }
                }

              // Create VMR field to perturb
              Tensor4 vmr_p = vmr_field;
                              
              // If perturbation given in ND convert the vmr-field to ND before
              // the perturbation is added          
              if( rq.Mode() == "nd" )
                vmr_p(si,joker,joker,joker) *= nd_field;
        
              // Calculate the perturbed field according to atmosphere_dim, 
              // the number of perturbations is the length of the retrieval 
              // grid +2 (for the end points)
              switch (atmosphere_dim)
                {
                case 1:
                  {
                    // Here we perturb a vector
                    perturbation_field_1d( vmr_p(si,joker,lat_it,lon_it), 
                                           p_gp, jg[0].nelem()+2, p_range, 
                                           rq.Perturbation(), pertmode );
                    break;
                  }
                case 2:
                  {
                    // Here we perturb a matrix
                    perturbation_field_2d( vmr_p(si,joker,joker,lon_it),
                                           p_gp, lat_gp, jg[0].nelem()+2, 
                                           jg[1].nelem()+2, p_range, lat_range, 
                                           rq.Perturbation(), pertmode );
                    break;
                  }    
                case 3:
                  {  
                    // Here we need to perturb a tensor3
                    perturbation_field_3d( vmr_p(si,joker,joker,joker), 
                                           p_gp, lat_gp, lon_gp, jg[0].nelem()+2,
                                           jg[1].nelem()+2, jg[2].nelem()+2, 
                                           p_range, lat_range, lon_range, 
                                           rq.Perturbation(), pertmode );
                    break;
                  }
                }

              // If perturbation given in ND convert back to VMR          
              if (rq.Mode()=="nd")
                vmr_p(si,joker,joker,joker) /= nd_field;
        
              // Calculate the perturbed spectrum  
              out2 << "  Calculating perturbed spectra no. " << it+1 << " of "
                   << ji[1]+1 << "\n";

              // Run jacobian_y_agenda
              jacobian_y_agendaExecute( ws, yp, f_grid, vmr_p, t_field, 
                                        sensor_los, jacobian_y_agenda );

              // Add dy/dx as column in jacobian
              for( Index y_it=0; y_it<y.nelem(); y_it++ )
                { J(y_it,it) = ( yp[y_it] - y[y_it] ) / rq.Perturbation(); }

              // Result from next loop shall go into next column of J
              it++;
            }
        }
    }
}




//----------------------------------------------------------------------------
// Frequency:
//----------------------------------------------------------------------------

/* Workspace method: Doxygen documentation will be auto-generated */
void jacobianAddFreqShiftAndStretch(
  Workspace&                 ws,
  ArrayOfRetrievalQuantity&  jq,
  Agenda&                    jacobian_agenda,
  const Matrix&              J,
  const Numeric&             df,
  const Index&               do_stretch )
{
  // Check that the jacobian matrix is empty. Otherwise it is either
  // not initialised or it is closed.
  if( J.nrows()!=0 && J.ncols()!=0 )
    {
      ostringstream os;
      os << "The Jacobian matrix is not initialised correctly or closed.\n"
         << "New retrieval quantities can not be added at this point.";
      throw runtime_error(os.str());
    }

  // Define subtag here to easily expand function.
  String subtag = "shift+stretch";
  String mode = "abs" ;

  // Check that this type of polyfit is not already included in the
  // jacobian.
  for( Index it=0; it<jq.nelem(); it++ )
    {
      if( jq[it].MainTag() == FREQUENCY_MAINTAG )
        {
          throw runtime_error( 
                "Frequency fit is already included in *jacobian_quantities*." );
        }
    }

  // Check that this type of pointing is not already included in the
  // jacobian.
  for( Index it=0; it<jq.nelem(); it++ )
    {
      if (jq[it].MainTag()== FREQUENCY_MAINTAG  &&  jq[it].Subtag() == subtag )
        {
          ostringstream os;
          os << "This type of frequency fit is already included in\n"
             << "*jacobian_quantities*.";
          throw runtime_error(os.str());
        }
    }

  // Check that do_stretch is 0 or 1
  if( do_stretch!=0 && do_stretch!=1 )
    throw runtime_error( "The argument *do_stretch* must be 0 or 1." );


  // Create the new retrieval quantity
  RetrievalQuantity rq = RetrievalQuantity();
  rq.MainTag( FREQUENCY_MAINTAG );
  rq.Subtag( subtag );
  rq.Mode( mode );
  rq.Analytical( 0 );
  rq.Perturbation( df );
  // To store the value or the polynomial order, create a vector with length
  // poly_order+1, in case of gitter set the size of the grid vector to be the
  // number of measurement blocks, all elements set to -1.
  Vector grid(0,1+do_stretch,1);
  ArrayOfVector grids(1,grid);
  rq.Grids(grids);

  // Add it to the *jacobian_quantities*
  jq.push_back( rq );

  // Add pointing method to the jacobian agenda
  jacobian_agenda.append( ws, "jacobianCalcFreqShiftAndStretch", "" );
}


/* Workspace method: Doxygen documentation will be auto-generated */
void jacobianCalcFreqShiftAndStretch(
        Workspace&                 ws,
        Matrix&                    J,
  const Agenda&                    jacobian_y_agenda,
  const ArrayOfRetrievalQuantity&  jq,
  const ArrayOfArrayOfIndex&       jacobian_indices,
  const Vector&                    f_grid,
  const Tensor4&                   vmr_field,
  const Tensor3&                   t_field,
  const Matrix&                    sensor_los,
  const Vector&                    sensor_response_f,
  const Vector&                    y )
{
  // Set some useful (and needed) variables.  
  RetrievalQuantity rq;
  ArrayOfIndex ji;

  // Find the retrieval quantity related to this method, i.e. Pointing
  // za offset. This works since the combined MainTag and Subtag is individual.
  bool found = false;
  for( Index n=0; n<jq.nelem() && !found; n++ )
    {
      if( jq[n].MainTag() == FREQUENCY_MAINTAG && 
          jq[n].Subtag() == "shift+stretch" )
        {
          found = true;
          rq = jq[n];
          ji = jacobian_indices[n];
        }
    }
  if( !found )
  {
    throw runtime_error(
      "There is no frequency retrieval quantity defined.\n");
  }

  // Check that sensor_reponse_f is consistent with y
  //
  if( sensor_response_f.nelem() != y.nelem() )
    {
      throw runtime_error( 
                 "Mismatch in length between *sensor_response_f* and *y*." );
    }

  // This method only handles absolute perturbations
  assert( rq.Mode()=="abs" );


  // Determine derivative by calculating spectra with perturbed f_grid
  //
  const Index ly = y.nelem(); 
  Vector dydx(ly);
  {
    Vector yp;
    Vector f_grid_pert  = f_grid;
    f_grid_pert[joker] += rq.Perturbation();
    // 
    jacobian_y_agendaExecute( ws, yp, f_grid_pert, vmr_field, t_field, 
                            sensor_los, jacobian_y_agenda );

    // Calculate difference in spectrum and divide by perturbation,
    // here we want the whole dy/dx vector so that we can use it later
    for( Index i=0; i<ly; i++ )
      {
        dydx[i] = ( yp[i] - y[i] ) / rq.Perturbation();
      }
  }

  //--- Create jacobians ---

  Index lg = rq.Grids()[0].nelem();
  Index it = ji[0];
  Vector w;

  for( Index c=0; c<lg; c++ )
    {
      assert( Numeric(c) == rq.Grids()[0][c] );
      //
      polynomial_basis_func( w, sensor_response_f, c );
      //
      for( Index iy=0; iy<ly; iy++ )
        {
          J(iy,it) = w[iy] * dydx[iy];
        }
      it++;
    }
}



//----------------------------------------------------------------------------
// Pointing:
//----------------------------------------------------------------------------

/* Workspace method: Doxygen documentation will be auto-generated */
void jacobianAddPointing(
  Workspace&                 ws,
  ArrayOfRetrievalQuantity&  jq,
  Agenda&                    jacobian_agenda,
  const Matrix&              J,
  const Matrix&              sensor_pos,
  const Vector&              sensor_time,
  const Numeric&             dza,
  const Index&               poly_order )
{
  // Check that the jacobian matrix is empty. Otherwise it is either
  // not initialised or it is closed.
  if( J.nrows()!=0 && J.ncols()!=0 )
    {
      ostringstream os;
      os << "The Jacobian matrix is not initialised correctly or closed.\n"
         << "New retrieval quantities can not be added at this point.";
      throw runtime_error(os.str());
    }

  // Check that poly_order is -1 or positive
  if( poly_order < -1 )
    throw runtime_error(
                  "The polynomial order has to be positive or -1 for gitter." );
 
  // Define subtag here to easily expand function.
  String subtag = "za offset";
  String mode = "abs" ;

  // Check that this type of polyfit is not already included in the
  // jacobian.
  for( Index it=0; it<jq.nelem(); it++ )
    {
      if( jq[it].MainTag() == POINTING_MAINTAG )
        {
          ostringstream os;
          os << "Polynomial baseline fit is already included in\n"
             << "*jacobian_quantities*.";
          throw runtime_error(os.str());
        }
    }

  // Check that this type of pointing is not already included in the
  // jacobian.
  for( Index it=0; it<jq.nelem(); it++ )
    {
      if (jq[it].MainTag()== POINTING_MAINTAG  &&  jq[it].Subtag() == subtag )
        {
          ostringstream os;
          os << "A zenith angle pointing offset is already included in\n"
             << "*jacobian_quantities*.";
          throw runtime_error(os.str());
        }
    }

  // Check that sensor_time is consistent with sensor_pos
  if( sensor_time.nelem() != sensor_pos.nrows() )
    {
      ostringstream os;
      os << "The WSV *sensor_time* must be defined for every "
         << "measurement block.\n";
      throw runtime_error(os.str());
    }

  // Do not allow that *poly_order* is not too large compared to *sensor_time*
  if( poly_order > sensor_time.nelem()-1 )
    { throw runtime_error( 
             "The polynomial order can not be >= length of *sensor_time*." ); }

  // Create the new retrieval quantity
  RetrievalQuantity rq = RetrievalQuantity();
  rq.MainTag( POINTING_MAINTAG );
  rq.Subtag( subtag );
  rq.Mode( mode );
  rq.Analytical( 0 );
  rq.Perturbation( dza );
  // To store the value or the polynomial order, create a vector with length
  // poly_order+1, in case of gitter set the size of the grid vector to be the
  // number of measurement blocks, all elements set to -1.
  Vector grid(0,poly_order+1,1);
  if( poly_order == -1 )
    {
      grid.resize(sensor_pos.nrows());
      grid = -1.0;
    }
  ArrayOfVector grids(1,grid);
  rq.Grids(grids);

  // Add it to the *jacobian_quantities*
  jq.push_back( rq );

  // Add pointing method to the jacobian agenda
  jacobian_agenda.append( ws, "jacobianCalcPointing", "" );
}



/* Workspace method: Doxygen documentation will be auto-generated */
void jacobianCalcPointing(
        Workspace&                 ws,
        Matrix&                    J,
  const Agenda&                    jacobian_y_agenda,
  const ArrayOfRetrievalQuantity&  jq,
  const ArrayOfArrayOfIndex&       jacobian_indices,
  const Vector&                    f_grid,
  const Tensor4&                   vmr_field,
  const Tensor3&                   t_field,
  const Matrix&                    sensor_los,
  const Vector&                    sensor_time,
  const Vector&                    y )
{
  // Set some useful (and needed) variables.  
  RetrievalQuantity rq;
  ArrayOfIndex ji;

  // Find the retrieval quantity related to this method, i.e. Pointing
  // za offset. This works since the combined MainTag and Subtag is individual.
  bool found = false;
  for( Index n=0; n<jq.nelem() && !found; n++ )
    {
      if( jq[n].MainTag() == POINTING_MAINTAG && jq[n].Subtag() == "za offset" )
        {
          found = true;
          rq = jq[n];
          ji = jacobian_indices[n];
        }
    }
  if( !found )
  {
    throw runtime_error(
      "There is no pointing offset retrieval quantities defined.\n");
  }

  // Check that sensor_time is consistent with sensor_pos
  //
  chk_if_increasing( "sensor_time", sensor_time );
  //
  if( sensor_time.nelem() != sensor_los.nrows() )
    {
      ostringstream os;
      os << "The WSV *sensor_time* must be defined for every "
         << "measurement block.\n";
      throw runtime_error(os.str());
    }

  // This method only handles absolute perturbations
  assert( rq.Mode()=="abs" );


  // Determine derivative by calculating spectra with *perturbed sensor_los* 
  //
  const Index ly = y.nelem(); 
  Vector dydx(ly);
  {
    Vector yp;
    Matrix sensor_los_pert = sensor_los;
    sensor_los_pert(joker,0) += rq.Perturbation();
    // 
    jacobian_y_agendaExecute( ws, yp, f_grid, vmr_field, t_field, 
                            sensor_los_pert, jacobian_y_agenda );

    // Calculate difference in spectrum and divide by perturbation,
    // here we want the whole dy/dx vector so that we can use it later
    for( Index i=0; i<ly; i++ )
      {
        dydx[i] = ( yp[i] - y[i] ) / rq.Perturbation();
      }
  }


  //--- Create jacobians ---

  Index nb = sensor_los.nrows();
  Index lb = y.nelem() / nb;
  Index lg = rq.Grids()[0].nelem();
  Index it = ji[0];
  
  // Check that integer division worked
  assert( nb*lb == y.nelem() );

  // Handle gitter seperately
  if( rq.Grids()[0][0] == -1 )
    {
      assert( lg == nb );
      //
      Index iy = 0;
      //
      for( Index b=0; b<nb; b++ )
        {
          assert( rq.Grids()[0][b] == -1 );
          for( Index dummy=0; dummy<lb; dummy++ )
            {
              J(iy,it) = dydx[iy];     // Not all elements are filled here,
              iy++;                    // but J is zeroed in jacobianClose
            }
          it++;
        }
    }

  // Polynomial representation
  else
    {
      Vector w;
      for( Index c=0; c<lg; c++ )
        {
          assert( Numeric(c) == rq.Grids()[0][c] );
          //
          polynomial_basis_func( w, sensor_time, c );
          //
          Index iy = 0;
          //
          for( Index b=0; b<nb; b++ )
            {
              for( Index dummy=0; dummy<lb; dummy++ )
                {
                  J(iy,it) = w[b] * dydx[iy];
                  iy++;
                }
            }
          it++;
        }
    }
}





//----------------------------------------------------------------------------
// Polynomial baseline fits:
//----------------------------------------------------------------------------

/* Workspace method: Doxygen documentation will be auto-generated */
void jacobianAddPolyfit(
  Workspace&                 ws,
  ArrayOfRetrievalQuantity&  jq,
  Agenda&                    jacobian_agenda,
  const Matrix&              J,
  const ArrayOfIndex&        sensor_response_pol_grid,
  const Vector&              sensor_response_f_grid,
  const Vector&              sensor_response_za_grid,
  const Matrix&              sensor_pos,
  const Index&               poly_order,
  const Index&               no_pol_variation,
  const Index&               no_za_variation,
  const Index&               no_mblock_variation )
{
  // Check that the jacobian matrix is empty. Otherwise it is either
  // not initialised or it is closed.
  if( J.nrows()!=0 && J.ncols()!=0 )
    {
      ostringstream os;
      os << "The Jacobian matrix is not initialised correctly or closed.\n"
         << "New retrieval quantities can not be added at this point.";
      throw runtime_error(os.str());
    }

  // Check that poly_order is >= 0
  if( poly_order < 0 )
    throw runtime_error(
      "The polynomial order has to be >= 0.");

  // Compare poly_order with number of frequency points
  if( poly_order > sensor_response_f_grid.nelem() - 1 )
    {
      ostringstream os;
      os << "The polynomial order can not exceed the length of\n"
         << "*sensor_response_f_grid* -1.";
      throw runtime_error(os.str());
    }
 
  // Check that this type of polyfit is not already included in the
  // jacobian.
  for( Index it=0; it<jq.nelem(); it++ )
    {
      if( jq[it].MainTag() == POLYFIT_MAINTAG )
        {
          ostringstream os;
          os << "Polynomial baseline fit is already included in\n"
             << "*jacobian_quantities*.";
          throw runtime_error(os.str());
        }
    }

  // "Grids"
  //
  // Grid dimensions correspond here to 
  //   1: polynomial order
  //   2: polarisation
  //   3: viewing direction
  //   4: measurement block
  //
  ArrayOfVector grids(4);
  //
  if( no_pol_variation )
    grids[1] = Vector(1,1);
  else
    grids[1] = Vector(0,sensor_response_pol_grid.nelem(),1);
  if( no_za_variation )
    grids[2] = Vector(1,1);
  else
    grids[2] = Vector(0,sensor_response_za_grid.nelem(),1); 
  if( no_mblock_variation )
    grids[3] = Vector(1,1);
  else
    grids[3] = Vector(0,sensor_pos.nrows(),1);

  // Create the new retrieval quantity
  RetrievalQuantity rq = RetrievalQuantity();
  rq.MainTag( POLYFIT_MAINTAG );
  rq.Mode( "" );
  rq.Analytical( 1 );
  rq.Perturbation( 0 );

  // Each polynomial coeff. is treated as a retrieval quantity
  //
  for( Index i=0; i<=poly_order; i++ )
    {
      ostringstream sstr;
      sstr << "Coefficient " << i;
      rq.Subtag( sstr.str() ); 

      // Grid is a scalar, use polynomial coeff.
      grids[0] = Vector(1,(Numeric)i);
      rq.Grids( grids );

      // Add it to the *jacobian_quantities*
      jq.push_back( rq );

      // Add pointing method to the jacobian agenda
      jacobian_agenda.append( ws, "jacobianCalcPolyfit", i );
    }
}



/* Workspace method: Doxygen documentation will be auto-generated */
void jacobianCalcPolyfit(
        Matrix&                   J,
  const ArrayOfRetrievalQuantity& jq,
  const ArrayOfArrayOfIndex&      jacobian_indices,
  const ArrayOfIndex&             sensor_response_pol_grid,
  const Vector&                   sensor_response_f_grid,
  const Vector&                   sensor_response_za_grid,
  const Matrix&                   sensor_pos,
  const Index&                    poly_coeff )
{  
  // Find the retrieval quantity related to this method
  RetrievalQuantity rq;
  ArrayOfIndex ji;
  bool found = false;
  Index iq;
  ostringstream sstr;
  sstr << "Coefficient " << poly_coeff;
  for( iq=0; iq<jq.nelem() && !found; iq++ )
    {
      if( jq[iq].MainTag() == POLYFIT_MAINTAG  && 
          jq[iq].Subtag() == sstr.str() )
        {
          found = true;
          break;
        }
    }
  if( !found )
  {
    throw runtime_error(
      "There is no polynomial baseline fit retrieval quantities defined.\n");
  }

  // Get sizes and check
  //
  //ArrayOfVector grids = jq[iq].Grids();
  //
  const Index nf     = sensor_response_f_grid.nelem();
  const Index npol   = sensor_response_pol_grid.nelem();
  const Index nza    = sensor_response_za_grid.nelem();
  const Index nblock = sensor_pos.nrows();
  //
  if( nf*npol*nza*nblock != J.nrows() )
    {
      throw runtime_error( 
         "Sensor grids have changed since *jacobianAddPolyfit* was called.\n");
    }

  // Make a vector with values to distribute over *jacobian*
  //
  Vector w; 
  //
  polynomial_basis_func( w, sensor_response_f_grid, poly_coeff );
  
  // Fill J
  //
  ArrayOfVector jg = jq[iq].Grids();
  //
  const Index n1 = jg[1].nelem();
  const Index n2 = jg[2].nelem();
  const Index n3 = jg[3].nelem();
  //
  for( Index b=0; b<nblock; b++ )  
    {
      const Index row0 = b*nf*npol*nza;
            Index col0 = jacobian_indices[iq][0];
      if( n3 > 1 )
        { col0 += b*n2*n1; } 

      for( Index v=0; v<jg[2].nelem(); v++ )
        {
          const Index row1 = row0 + v*nf*npol;
                Index col1 = col0;
          if( n2 > 1 )
            { col1 += v*n1; } 

          for( Index p=0; p<nza; p++ )      // Not all elements are filled here
            {                               // but J is zeroed in jacobianClose
              const Index row2 = row1 + p;
                    Index col2 = col1;
              if( n1 > 1 )
                { col2 += p; } 
              for( Index f=0; f<nf; f++ )
                {
                  J(row2+f*npol,col2) = w[f];  
                }             
            }
        }
    }
}





//----------------------------------------------------------------------------
// Temperatures (atmospheric):
//----------------------------------------------------------------------------

/* Workspace method: Doxygen documentation will be auto-generated */
void jacobianAddTemperature(
  Workspace&                  ws,
  ArrayOfRetrievalQuantity&   jq,
  Agenda&                     jacobian_agenda,
  const Matrix&               J,
  const Index&                atmosphere_dim,
  const Vector&               p_grid,
  const Vector&               lat_grid,
  const Vector&               lon_grid,
  const Vector&               rq_p_grid,
  const Vector&               rq_lat_grid,
  const Vector&               rq_lon_grid,
  const String&               hse,
  const String&               method,
  const Numeric&              dx )
{
  // Check that the jacobian matrix is empty. Otherwise it is either
  // not initialised or it is closed.
  if( J.nrows()!=0 && J.ncols()!=0 )
    {
      ostringstream os;
      os << "The Jacobian matrix is not initialised correctly or closed.\n"
         << "New retrieval quantities can not be added at this point.";
      throw runtime_error(os.str());
    }
  
  // Check retrieval grids, here we just check the length of the grids
  // vs. the atmosphere dimension
  ArrayOfVector grids(atmosphere_dim);
  {
    ostringstream os;
    if( !check_retrieval_grids( grids, os, p_grid, lat_grid, lon_grid,
                                rq_p_grid, rq_lat_grid, rq_lon_grid,
                                "retrieval pressure grid", 
                                "retrieval latitude grid", 
                                "retrievallongitude_grid", 
                                atmosphere_dim ) )
    throw runtime_error(os.str());
  }
  
  // Check that method is either "analytic" or "perturbation"
  bool analytical;
  if( method == "perturbation" )
    { analytical = 0; }
  else if( method == "analytical" )
    { analytical = 1; }
  else
    {
      ostringstream os;
      os << "The method for atmospheric temperature retrieval can only be "
         << "\"analytical\"\n or \"perturbation\".";
      throw runtime_error(os.str());
    }

  // Set subtag 
  String subtag;
  if( hse == "on" )
    {
      subtag = "HSE on";
      ostringstream os;
      os << "The keyword for hydrostatic equilibrium can so far only be set to\n"
         << "\"off\"\n";
      throw runtime_error(os.str());
    }
  else if( hse == "off" )
    {
      subtag = "HSE off";
    }
  else
    {
      ostringstream os;
      os << "The keyword for hydrostatic equilibrium can only be set to\n"
         << "\"on\" or \"off\"\n";
      throw runtime_error(os.str());
    }

  // HSE can not be combined with analytical 
  if( method == "analytical"  &&  hse == "on" )
    {
      ostringstream os;
      os << "Hydrostatic equilibrium can only be included for perturbation\n"
         << "calculations.";
      throw runtime_error(os.str());
    }

  // Check that temperature is not already included in the jacobian.
  // We only check the main tag.
  for (Index it=0; it<jq.nelem(); it++)
    {
      if( jq[it].MainTag() == "Temperature" )
        {
          ostringstream os;
          os << "Temperature is already included in *jacobian_quantities*.";
          throw runtime_error(os.str());
        }
    }

  // Create the new retrieval quantity
  RetrievalQuantity rq = RetrievalQuantity();
  rq.MainTag( TEMPERATURE_MAINTAG );
  rq.Subtag( subtag );
  rq.Mode( "abs" );
  rq.Analytical( analytical );
  rq.Perturbation( dx );
  rq.Grids( grids );

  // Add it to the *jacobian_quantities*
  jq.push_back( rq );

  if( analytical ) 
    {
      out3 << "  Calculations done by semi-analytical expression.\n"; 
    }
  else
    { 
      out3 << "  Calculations done by perturbations, of size " << dx << ".\n"; 

      jacobian_agenda.append( ws, "jacobianCalcTemperature", "" );
    }
}                    



/* Workspace method: Doxygen documentation will be auto-generated */
void jacobianCalcTemperature(
        Workspace&                 ws,
        Matrix&                    J,
  const Agenda&                    jacobian_y_agenda,
  const ArrayOfRetrievalQuantity&  jq,
  const ArrayOfArrayOfIndex&       jacobian_indices,
  const Index&                     atmosphere_dim,
  const Vector&                    p_grid,
  const Vector&                    lat_grid,
  const Vector&                    lon_grid,
  const Vector&                    f_grid,
  const Tensor4&                   vmr_field,
  const Tensor3&                   t_field,
  const Matrix&                    sensor_los,
  const Vector&                    y )
{
  // Set some useful (and needed) variables. 
  RetrievalQuantity rq;
  ArrayOfIndex      ji;
  Index             it;

  // Find the retrieval quantity related to this method, i.e. Temperature.
  // For temperature only the main tag is checked.
  bool found = false;
  for( Index n=0; n<jq.nelem() && !found; n++ )
    {
      if( jq[n].MainTag() == TEMPERATURE_MAINTAG )
        {
          found = true;
          rq = jq[n];
          ji = jacobian_indices[n];
        }
    }
  if( !found )
    {
      ostringstream os;
      os << "There is no temperature retrieval quantities defined.\n";
      throw runtime_error(os.str());
    }

  // FIXME: Only HSE off is implemented
  assert( rq.Subtag()=="HSE off" );

  // Store the start JacobianIndices and the Grids for this quantity
  it = ji[0];
  ArrayOfVector jg = rq.Grids();

  // "Perturbation mode". 1 means absolute perturbations
  const Index pertmode = 1;
   
  // For each atmospheric dimension option calculate a ArrayOfGridPos, 
  // these will be used to interpolate a perturbation into the atmospheric 
  // grids.
  ArrayOfGridPos p_gp, lat_gp, lon_gp;
  Index j_p   = jg[0].nelem();
  Index j_lat = 1;
  Index j_lon = 1;
  //
  get_perturbation_gridpos( p_gp, p_grid, jg[0], true );
  //
  if( atmosphere_dim >= 2 ) 
    {
      j_lat = jg[1].nelem();
      get_perturbation_gridpos( lat_gp, lat_grid, jg[1], false );
      if( atmosphere_dim == 3 ) 
        {
          j_lon = jg[2].nelem();
          get_perturbation_gridpos( lon_gp, lon_grid, jg[2], false );
        }
    }

  // Give verbose output
  out2 << "  Calculating retrieval quantity " << rq << "\n";

  // Vector for perturbed measurement vector
  Vector yp;

  // Loop through the retrieval grid and calculate perturbation effect
  for( Index lon_it=0; lon_it<j_lon; lon_it++ )
    {
      for( Index lat_it=0; lat_it<j_lat; lat_it++ )
        {
          for( Index p_it=0; p_it<j_p; p_it++ )
            {
              // Perturbed temperature field
              Tensor3 t_p = t_field;

              // Here we calculate the ranges of the perturbation. We want the
              // perturbation to continue outside the atmospheric grids for the
              // edge values.
              Range p_range   = Range(0,0);
              Range lat_range = Range(0,0);
              Range lon_range = Range(0,0);
              get_perturbation_range( p_range, p_it, j_p );
              if( atmosphere_dim >= 2 )
                {
                  get_perturbation_range( lat_range, lat_it, j_lat );
                  if( atmosphere_dim == 3 )
                    {
                      get_perturbation_range( lon_range, lon_it, j_lon );
                    }
                }
                           
              // Calculate the perturbed field according to atmosphere_dim, 
              // the number of perturbations is the length of the retrieval 
              // grid +2 (for the end points)
              switch (atmosphere_dim)
                {
                case 1:
                  {
                    // Here we perturb a vector
                    perturbation_field_1d( t_p(joker,lat_it,lon_it), 
                                           p_gp, jg[0].nelem()+2, p_range, 
                                           rq.Perturbation(), pertmode );
                    break;
                  }
                case 2:
                  {
                    // Here we perturb a matrix
                    perturbation_field_2d( t_p(joker,joker,lon_it), 
                                           p_gp, lat_gp, jg[0].nelem()+2, 
                                           jg[1].nelem()+2, p_range, lat_range, 
                                           rq.Perturbation(), pertmode );
                    break;
                  }    
                case 3:
                  {  
                    // Here we need to perturb a tensor3
                    perturbation_field_3d( t_p(joker,joker,joker), 
                                           p_gp, lat_gp, lon_gp, jg[0].nelem()+2,
                                           jg[1].nelem()+2, jg[2].nelem()+2, 
                                           p_range, lat_range, lon_range, 
                                           rq.Perturbation(), pertmode );
                    break;
                  }
                }
       
              // Calculate the perturbed spectrum  
              out2 << "  Calculating perturbed spectra no. " << it+1 << " of "
                   << ji[1]+1 << "\n";

              // Run jacobian_y_agenda
              jacobian_y_agendaExecute( ws, yp, f_grid, vmr_field, t_p, 
                                        sensor_los, jacobian_y_agenda );
 
              // Add dy/dx as column in jacobian
              for( Index y_it=0; y_it<y.nelem(); y_it++ )
                { J(y_it,it) = ( yp[y_it] - y[y_it] ) / rq.Perturbation(); }

              // Result from next loop shall go into next column of J
              it++;
            }
        }
    }
}











//----------------------------------------------------------------------------
// Old:
//----------------------------------------------------------------------------


// /* Workspace method: Doxygen documentation will be auto-generated */
// void jacobianAddParticle(// WS Output:
//                          ArrayOfRetrievalQuantity& jq,
//                          Agenda&                   jacobian_agenda,
//                          // WS Input:
//                          const Matrix&             jac,
//                          const Index&              atmosphere_dim,
//                          const Vector&             p_grid,
//                          const Vector&             lat_grid,
//                          const Vector&             lon_grid,
//                          const Tensor4&            pnd_field,
//                          const Tensor5&            pnd_perturb,
//                          const ArrayOfIndex&       cloudbox_limits,
//                          // WS Generic Input:
//                          const Vector&             rq_p_grid,
//                          const Vector&             rq_lat_grid,
//                          const Vector&             rq_lon_grid)
// {
//   throw runtime_error("Particle jacobians not yet handled correctly.");

//   // Check that the jacobian matrix is empty. Otherwise it is either
//   // not initialised or it is closed.
//   if( jac.nrows()!=0 && jac.ncols()!=0 )
//     {
//       ostringstream os;
//       os << "The Jacobian matrix is not initialised correctly or closed.\n"
//          << "New retrieval quantities can not be added at this point.";
//       throw runtime_error(os.str());
//     }
  
//   // Check that pnd_perturb is consistent with pnd_field
//   if( pnd_perturb.nbooks()!=pnd_field.nbooks() ||
//       pnd_perturb.npages()!=pnd_field.npages() ||
//       pnd_perturb.nrows()!=pnd_field.nrows() ||
//       pnd_perturb.ncols()!=pnd_field.ncols() )
//     {
//       ostringstream os;
//       os << "The perturbation field *pnd_field_perturb* is not consistent with"
//          << "*pnd_field*,\none or several dimensions do not match.";
//       throw runtime_error(os.str());
//     }
  
//   // Check that particles are not already included in the jacobian.
//   for( Index it=0; it<jq.nelem(); it++ )
//     {
//       if( jq[it].MainTag()=="Particles" )
//         {
//           ostringstream os;
//           os << "The particles number densities are already included in "
//              << "*jacobian_quantities*.";
//           throw runtime_error(os.str());
//         }
//     }
  
//   // Particle Jacobian only defined for 1D and 3D atmosphere. check the 
//   // retrieval grids, here we just check the length of the grids vs. the 
//   // atmosphere dimension
//   if (atmosphere_dim==2) 
//   {
//     ostringstream os;
//     os << "Atmosphere dimension not equal to 1 or 3. " 
//        << "Jacobians for particle number\n"
//        << "density only available for 1D and 3D atmosphere.";
//     throw runtime_error(os.str());
//   }

//   ArrayOfVector grids(atmosphere_dim);
//   // The retrieval grids should only consists of gridpoints within
//   // the cloudbox. Setup local atmospheric fields inside the cloudbox
//   {
//     Vector p_cbox = p_grid;
//     Vector lat_cbox = lat_grid;
//     Vector lon_cbox = lon_grid;
//     switch (atmosphere_dim)
//       {
//       case 3:
//         {
//           lon_cbox = lon_grid[Range(cloudbox_limits[4], 
//                                     cloudbox_limits[5]-cloudbox_limits[4]+1)];
//         }
//       case 2:
//         {
//           lat_cbox = lat_grid[Range(cloudbox_limits[2], 
//                                     cloudbox_limits[3]-cloudbox_limits[2]+1)];
//         }    
//       case 1:
//         {  
//           p_cbox = p_grid[Range(cloudbox_limits[0], 
//                                 cloudbox_limits[1]-cloudbox_limits[0]+1)];
//         }
//       }
//     ostringstream os;
//     if( !check_retrieval_grids( grids, os, p_cbox, lat_cbox, lon_cbox,
//                                 rq_p_grid, rq_lat_grid, rq_lon_grid, 
//         // FIXMEOLE: These strings have to replaced later with the proper
//         //           names from the WSM documentation in methods.cc
//           "rq_p_grid", "rq_lat_grid", "rq_lon_grid", atmosphere_dim ))
//       throw runtime_error(os.str());
//   }

//   // Common part for all particle variables
//   RetrievalQuantity rq = RetrievalQuantity();
//   rq.MainTag("Particles");
//   rq.Grids(grids);
//   rq.Analytical(0);
//   rq.Perturbation(-999.999);
//   rq.Mode("Fields *mode* and *perturbation* are not defined");

//   // Set info for each particle variable
//   for( Index ipt=0; ipt<pnd_perturb.nshelves(); ipt++ )
//     {
//       out2 << "  Adding particle variable " << ipt +1 
//            << " to *jacobian_quantities / agenda*.\n";

//       ostringstream os;
//       os << "Variable " << ipt+1;
//       rq.Subtag(os.str());
      
//       jq.push_back(rq);
//     }

//   // Add gas species method to the jacobian agenda
//   String methodname = "jacobianCalcParticle";
//   String kwv = "";
//   jacobian_agenda.append (methodname, kwv);
// }                    












// /* Workspace method: Doxygen documentation will be auto-generated */
// void jacobianCalcParticle(
//            Workspace&                  ws,
//      // WS Output:
//            Matrix&                     jacobian,
//      // WS Input:
//      const Vector&                     y,
//      const ArrayOfRetrievalQuantity&   jq,
//      const ArrayOfArrayOfIndex&        jacobian_indices,
//      const Tensor5&                    pnd_field_perturb,
//      const Agenda&                     jacobian_particle_update_agenda,
//      const Agenda&                     ppath_step_agenda,
//      const Agenda&                     rte_agenda,
//      const Agenda&                     iy_space_agenda,
//      const Agenda&                     surface_prop_agenda,
//      const Agenda&                     iy_cloudbox_agenda,
//      const Index&                      atmosphere_dim,
//      const Vector&                     p_grid,
//      const Vector&                     lat_grid,
//      const Vector&                     lon_grid,
//      const Tensor3&                    z_field,
//      const Tensor3&                    t_field,
//      const Tensor4&                    vmr_field,
//      const Matrix&                     r_geoid,
//      const Matrix&                     z_surface,
//      const Index&                      cloudbox_on,
//      const ArrayOfIndex&               cloudbox_limits,
//      const Tensor4&                    pnd_field,
//      const Sparse&                     sensor_response,
//      const Matrix&                     sensor_pos,
//      const Matrix&                     sensor_los,
//      const Vector&                     f_grid,
//      const Index&                      stokes_dim,
//      const Index&                      antenna_dim,
//      const Vector&                     mblock_za_grid,
//      const Vector&                     mblock_aa_grid )
// {
//   // Set some useful (and needed) variables. 
//   Index n_jq = jq.nelem();
//   RetrievalQuantity rq;
//   ArrayOfIndex ji;
  
//   // Setup local atmospheric fields inside the cloudbox
//   Vector p_cbox = p_grid;
//   Vector lat_cbox = lat_grid;
//   Vector lon_cbox = lon_grid;
//   switch (atmosphere_dim)
//     {
//     case 3:
//       {
//         lon_cbox = lon_grid[Range(cloudbox_limits[4], 
//                                   cloudbox_limits[5]-cloudbox_limits[4]+1)];
//       }
//     case 2:
//       {
//         lat_cbox = lat_grid[Range(cloudbox_limits[2], 
//                                   cloudbox_limits[3]-cloudbox_limits[2]+1)];
//       }    
//     case 1:
//       {  
//         p_cbox = p_grid[Range(cloudbox_limits[0], 
//                               cloudbox_limits[1]-cloudbox_limits[0]+1)];
//       }
//     }


//   // Variables to handle and store perturbations
//   Vector yp;
//   Tensor4  pnd_p, base_pert = pnd_field;


//   // Loop particle variables (indexed by *ipt*, where *ipt* is zero based)
//   //
//   Index ipt      = -1;
//   bool not_ready = true;
  
//   while( not_ready )
//     {
//       // Step *ipt*
//       ipt++;

//       // Define sub-tag string
//       ostringstream os;
//       os << "Variable " << ipt+1;

//       // Find the retrieval quantity related to this particle type
//       //
//       bool  found = false;
//       //
//       for( Index n=0; n<n_jq; n++ )
//         {
//           if( jq[n].MainTag()=="Particles" && jq[n].Subtag()== os.str() )
//             {
//               found = true;
//               rq = jq[n];
//               ji = jacobian_indices[n];
//               n  = n_jq;                   // To jump out of for-loop
//             }
//         }

//       // At least one particle type must be found
//       assert( !( ipt==0  &&  !found ) );

//       // Ready or something to do?
//       if( !found )
//         { 
//           not_ready = false;
//         }
//       else
//         {
//           // Counters for report string
//           Index   it  = 0;
//           Index   nit = ji[1] -ji[0] + 1;
          
//           // Counter for column in *jacobian*
//           Index   icol = ji[0];

//           // Retrieval grid positions
//           ArrayOfVector jg = rq.Grids();
//           ArrayOfGridPos p_gp, lat_gp, lon_gp;
//           Index j_p = jg[0].nelem();
//           Index j_lat = 1;
//           Index j_lon = 1;
//           get_perturbation_gridpos( p_gp, p_cbox, jg[0], true );
//           if (atmosphere_dim==3) 
//             {
//               j_lat = jg[1].nelem();
//               get_perturbation_gridpos( lat_gp, lat_cbox, jg[1], false );
              
//               j_lon = jg[2].nelem();
//               get_perturbation_gridpos( lon_gp, lon_cbox, jg[2], false );
//             }

//           // Give verbose output
//           out1 << "  Calculating retrieval quantity:" << rq << "\n";
  

//           // Loop through the retrieval grid and calculate perturbation effect
//           for (Index lon_it=0; lon_it<j_lon; lon_it++)
//             {
//               for (Index lat_it=0; lat_it<j_lat; lat_it++)
//                 {
//                   for (Index p_it=0; p_it<j_p; p_it++)
//                     {
//                       // Update the perturbation field
//                       pnd_p = 
//                            pnd_field_perturb( ipt, joker, joker, joker, joker);

//                       it++;
//                       out1 << "  Calculating perturbed spectra no. " << it
//                            << " of " << nit << "\n";

//                       // Here we calculate the ranges of the perturbation. 
//                       // We want the perturbation to continue outside the 
//                       // atmospheric grids for the edge values.
//                       Range p_range   = Range(0,0);
//                       Range lat_range = Range(0,0);
//                       Range lon_range = Range(0,0);
//                       get_perturbation_range( p_range, p_it, j_p );
//                       if (atmosphere_dim==3)
//                         {
//                           get_perturbation_range( lat_range, lat_it, j_lat);
//                           get_perturbation_range( lon_range, lon_it, j_lon);
//                         }
                          
//                       // Make empty copy of pnd_pert for base functions
//                       base_pert *= 0;
            
//                       // Calculate the perturbed field according to atm_dim, 
//                       switch (atmosphere_dim)
//                         {
//                         case 1:
//                           {
//                             for( Index typ_it=0; typ_it<pnd_field.nbooks(); 
//                                                                      typ_it++ )
//                               {
//                                 perturbation_field_1d( 
//                                       base_pert(typ_it,joker,lat_it,lon_it),
//                                       p_gp, jg[0].nelem()+2, p_range, 1.0, 1 );
//                               }
//                             break;
//                           }
//                         case 3:
//                           {  
//                             for( Index typ_it=0; typ_it<pnd_field.nrows(); 
//                                                                      typ_it++ )
//                               {
//                                 perturbation_field_3d( 
//                                       base_pert(typ_it,joker,joker,joker),
//                                       p_gp, lat_gp, lon_gp, jg[0].nelem()+2, 
//                                       jg[1].nelem()+2, jg[2].nelem()+2, 
//                                       p_range, lat_range, lon_range, 1.0, 1);
//                               }
//                             break;
//                           }
//                         }
          
//                       // Now add the weighted perturbation field to the 
//                       // reference field and recalculate the scattered field
//                       pnd_p *= base_pert;
//                       pnd_p += pnd_field;
//                       jacobian_particle_update_agendaExecute( ws, pnd_p, 
//                                       jacobian_particle_update_agenda );
            
//                       // Calculate the perturbed spectrum  
//                       yCalc( ws, yp, ppath_step_agenda, rte_agenda, 
//                              iy_space_agenda, surface_prop_agenda, 
//                              iy_cloudbox_agenda, atmosphere_dim,
//                              p_grid, lat_grid, lon_grid, z_field, t_field, 
//                              vmr_field, r_geoid, z_surface, cloudbox_on, 
//                              cloudbox_limits, sensor_response, sensor_pos, 
//                              sensor_los, f_grid, stokes_dim, antenna_dim, 
//                              mblock_za_grid, mblock_aa_grid);
    
//                       // Add dy as column in jacobian. Note that we just return
//                       // the difference between the two spectra.
//                       for( Index y_it=0; y_it<yp.nelem(); y_it++ )
//                         {
//                           jacobian(y_it,icol) = yp[y_it]-y[y_it];
//                         }

//                       // Step *icol*
//                       icol++;
//                     }
//                 }
//             }
//         }
//     }
// }


                     










