/* Copyright (C) 2004 Mattias Ekström <ekstrom@rss.chalmers.se>

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



/*===========================================================================
  ===  File description
  ===========================================================================*/

/*!
  \file   m_jacobian.cc
  \author Mattias Ekström <ekstrom@rss.chalmers.se>
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


/*===========================================================================
  === The functions (in alphabetical order)
  ===========================================================================*/

//! jacobianAddGas
/*!
   See the online help (arts -d FUNCTION_NAME)
   
   \author Mattias Ekström
   \date   2004-09-30
*/
void jacobianAddGas(// WS Output:
                    ArrayOfRetrievalQuantity& jq,
                    ArrayOfArrayOfSpeciesTag& gas_species,
                    // WS Input:
                    const Sparse&             jac,
                    const Index&              atmosphere_dim,
                    // WS Generic Input:
                    const Vector&             rq_p_grid,
                    const Vector&             rq_lat_grid,
                    const Vector&             rq_lon_grid,
                    // WS Generic Input Names:
                    const String&             rq_p_grid_name,
                    const String&             rq_lat_grid_name,
                    const String&             rq_lon_grid_name,
                    // Control Parameters:
                    const String&             species,
                    const String&             method,
                    const String&             unit,
                    const Numeric&            dx)
{
  // Check that the jacobian matrix is empty. Otherwise it is either
  // not initialised or it is closed.
  if (jac.nrows()!=0 && jac.ncols()!=0)
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
  if (!check_retrieval_grids( grids, os, rq_p_grid, rq_lat_grid, rq_lon_grid,
        rq_p_grid_name, rq_lat_grid_name, rq_lon_grid_name, atmosphere_dim))
    throw runtime_error(os.str());
  }
  
  // Check that method is either "analytic" or "perturbation"
  Index method_index;
  if (method=="perturbation")
  {
    method_index = 0;
  }
  else if (method=="analytic")
  {
    //FIXME: Only "perturbation" implemented so far
    throw runtime_error(
      "Only perturbation method implemented for gas retrieval quantities");
    
    method_index = 1;
  }
  else
  {
    ostringstream os;
    os << "The method for gas species retrieval can only be \"analytic\"\n"
       << "or \"perturbation\".";
    throw runtime_error(os.str());
  }
  
  // Check that unit is either 'abs' or 'rel'
  if (unit!="abs" && unit!="rel")
  {
    ostringstream os;
    os << "The unit of perturbation for pointing offset can only be either\n"
       << "absolute (\"abs\") or relative (\"rel\")."; 
    throw runtime_error(os.str());
  }

  // Check that this species is not already included in the jacobian.
  for (Index it=0; it<jq.nelem(); it++)
  {
    if (jq[it].MainTag()=="Gas species" && jq[it].Subtag()==species)
    {
      ostringstream os;
      os << "The gas species:\n" << species << "\nis already included in "
         << "*jacobian_quantities*.";
      throw runtime_error(os.str());
    }
  }

  // Create the new retrieval quantity
  RetrievalQuantity rq = RetrievalQuantity();
  rq.MainTag("Gas species");
  rq.Subtag(species);
  rq.Unit(unit);
  rq.Method(method_index);
  rq.Perturbation(dx);
  rq.Grids(grids);

  // Add it to the *jacobian_quantities*
  jq.push_back(rq);
  
  out2 << "  Adding gas species: " << species << " to *jacobian_quantities*.\n";
  if (method_index==0) 
  { 
    out3 << "  Calculations done by perturbation, size " << dx 
         << " " << unit << ".\n"; 
  }
  else if (method_index==1) 
  {
    out3 << "  Calculations done by analytical expression.\n"; 
  }

  // Add retrieval quantity to *gas_species*
  ArrayOfSpeciesTag tags;
  array_species_tag_from_string( tags, species );
  gas_species.push_back( tags );
  // FIXME: Here we could just add rq.SpeciesIndex = gas_species.nelem()-1;
  Index si = chk_contains( "species", gas_species, tags );
  rq.SpeciesIndex(si);
  
  // Print list of added tag group to the most verbose output stream:
  out3 << "  Appended tag group:";
  out3 << "\n  " << gas_species.nelem()-1 << ":";
  for ( Index s=0; s<tags.nelem(); ++s )
  {
    out3 << " " << tags[s].Name();
  }
  out3 << '\n';
}                    


//! jacobianAddPointing
/*!
   See the online help (arts -d FUNCTION_NAME)

   \author Mattias Ekström�
   \date   2004-09-14
*/
void jacobianAddPointing(// WS Output:
                         ArrayOfRetrievalQuantity&  jq,
                         // WS Input:
                         const Sparse&              jac,
                         // Control Parameters:
                         const Numeric&             dza,
                         const String&              unit,
                         const Index&               poly_order)
{
  // Check that the jacobian matrix is empty. Otherwise it is either
  // not initialised or it is closed.
  if (jac.nrows()!=0 && jac.ncols()!=0)
  {
    ostringstream os;
    os << "The Jacobian matrix is not initialised correctly or closed.\n"
       << "New retrieval quantities can not be added at this point.";
    throw runtime_error(os.str());
  }
  
  // Check that unit is either 'abs' or 'rel'
  if (unit!="abs" && unit!="rel")
  {
    ostringstream os;
    os << "The unit of perturbation for pointing offset can only be either\n"
       << "absolute (\"abs\") or relative (\"rel\")."; 
    throw runtime_error(os.str());
  }

  // Check that poly_order is positive
  if (poly_order<0)
    throw runtime_error("The polynomial order has to be positive.");
    
  // Define subtag here to easily expand function.
  String subtag="za offset";

  // Check that this type of pointing is not already included in the
  // jacobian.
  for (Index it=0; it<jq.nelem(); it++)
  {
    if (jq[it].MainTag()=="Pointing" && jq[it].Subtag()==subtag)
    {
      ostringstream os;
      os << "A zenith angle pointing offset is already included in\n"
         << "*jacobian_quantities*.";
      throw runtime_error(os.str());
    }
  }

  // Create the new retrieval quantity
  RetrievalQuantity rq = RetrievalQuantity();
  rq.MainTag("Pointing");
  rq.Subtag(subtag);
  rq.Unit(unit);
  rq.Method(0);
  rq.Perturbation(dza);
  Vector grid(0,poly_order+1,1);
  ArrayOfVector grids(1, grid);
  rq.Grids(grids);
  rq.SpeciesIndex(-1);

  // Add it to the *jacobian_quantities*
  jq.push_back(rq);

  out2 << "  Adding zenith angle pointing offset to *jacobian_quantities*\n"
       << "  with perturbation size " << dza << "\n";
}


//! jacobianAddTemperature
/*!
   See the online help (arts -d FUNCTION_NAME)
   
   \author Mattias Ekström
   \date   2004-10-14
*/
void jacobianAddTemperature(// WS Output:
                    ArrayOfRetrievalQuantity& jq,
                    // WS Input:
                    const Sparse&             jac,
                    const Index&              atmosphere_dim,
                    // WS Generic Input:
                    const Vector&             rq_p_grid,
                    const Vector&             rq_lat_grid,
                    const Vector&             rq_lon_grid,
                    // WS Generic Input Names:
                    const String&             rq_p_grid_name,
                    const String&             rq_lat_grid_name,
                    const String&             rq_lon_grid_name,
                    // Control Parameters:
                    const String&             method,
                    const String&             unit,
                    const Numeric&            dx)
{
  // Check that the jacobian matrix is empty. Otherwise it is either
  // not initialised or it is closed.
  if (jac.nrows()!=0 && jac.ncols()!=0)
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
  if (!check_retrieval_grids( grids, os, rq_p_grid, rq_lat_grid, rq_lon_grid,
        rq_p_grid_name, rq_lat_grid_name, rq_lon_grid_name, atmosphere_dim))
    throw runtime_error(os.str());
  }
  
  // Check that method is either "analytic" or "perturbation"
  Index method_index;
  if (method=="perturbation")
  {
    method_index = 0;
  }
  else if (method=="analytic")
  {
    //FIXME: Only "perturbation" implemented so far
    throw runtime_error(
      "Only perturbation method implemented for temperature retrieval.");
    
    method_index = 1;
  }
  else
  {
    ostringstream os;
    os << "The method for gas species retrieval can only be \"analytic\"\n"
       << "or \"perturbation\".";
    throw runtime_error(os.str());
  }
  
  // Check that unit is either 'abs' or 'rel'
  if (unit!="abs" && unit!="rel")
  {
    ostringstream os;
    os << "The unit of perturbation for pointing offset can only be either\n"
       << "absolute (\"abs\") or relative (\"rel\")."; 
    throw runtime_error(os.str());
  }
  
  // Set subtag to "HSE off"
  String subtag = "HSE off";

  // Check that temperature is not already included in the jacobian.
  // We only check the main tag.
  for (Index it=0; it<jq.nelem(); it++)
  {
    if (jq[it].MainTag()=="Temperature")
    {
      ostringstream os;
      os << "Temperature is already included in *jacobian_quantities*.";
      throw runtime_error(os.str());
    }
  }

  // Create the new retrieval quantity
  RetrievalQuantity rq = RetrievalQuantity();
  rq.MainTag("Temperature");
  rq.Subtag(subtag);
  rq.Unit(unit);
  rq.Method(method_index);
  rq.Perturbation(dx);
  rq.Grids(grids);
  rq.SpeciesIndex(-1);

  // Add it to the *jacobian_quantities*
  jq.push_back(rq);
  
  out2 << "  Adding temperature to *jacobian_quantities*.\n";
  if (method_index==0) 
  { 
    out3 << "  Calculations done by perturbation, size " << dx 
         << " " << unit << ".\n"; 
  }
  else if (method_index==1) 
  {
    out3 << "  Calculations done by analytical expression.\n"; 
  }
}                    


//! jacobianCalc
/*!
   See the online help (arts -d FUNCTION_NAME)
   
   \author Mattias Ekström
   \date   2004-09-28
*/
void jacobianCalc(// WS Output:
                        Sparse&                    jacobian,
                  // WS Input:
                  const Agenda&                    jacobian_agenda,
                  const ArrayOfRetrievalQuantity&  jq)
{
  Index n_jq = jq.nelem();
  
  // Check that the jacobian has been initialised. This is covered by the
  // next test, but gives a better output for this specific case.
  if (jacobian.ncols()==0)
  {
    ostringstream os;
    os << "The Jacobian matrix has not been properly initialised.\n"
       << "The WSM jacobianClose has to be called prior to jacobianCalc.\n";
    throw runtime_error(os.str());
  }

  // Check that *jacobian_quantities* and *jacobian* are consistent
  ArrayOfIndex last_ind = jq[n_jq-1].JacobianIndices();
  if (jacobian.ncols()-1!=last_ind[1])
  {
    ostringstream os;
    os << "There are more retrieval quantities in *jacobian_quantities*\n"
       << "than in *jacobian*. After calling jacobianClose no more\n"
       << "quantities can be added.";
    throw runtime_error(os.str());
  }

  // Output message
  out2 << "  Calculating *jacobian*.\n";
  
  // Run jacobian_agenda
  jacobian_agenda.execute();
}


//! jacobianCalcGas
/*!
   See the online help (arts -d FUNCTION_NAME)

   \author Mattias Ekström�
   \date   2004-10-01
*/
void jacobianCalcGas(
     // WS Output:
           Sparse&                   jacobian,
           Tensor4&                  vmr_field,
           Vector&                   y,
           Ppath&                    ppath,
           Ppath&                    ppath_step,
           Matrix&                   iy, 
           Vector&                   rte_pos,
           GridPos&                  rte_gp_p,
           GridPos&                  rte_gp_lat,
           GridPos&                  rte_gp_lon,
           Vector&                   rte_los,
     // WS Input:
     const ArrayOfRetrievalQuantity& jq,
     const ArrayOfArrayOfSpeciesTag& gas_species,
     const Agenda&                   ppath_step_agenda,
     const Agenda&                   rte_agenda,
     const Agenda&                   iy_space_agenda,
     const Agenda&                   iy_surface_agenda,
     const Agenda&                   iy_cloudbox_agenda,
     const Index&                    atmosphere_dim,
     const Vector&                   p_grid,
     const Vector&                   lat_grid,
     const Vector&                   lon_grid,
     const Tensor3&                  z_field,
     const Matrix&                   r_geoid,
     const Matrix&                   z_surface,
     const Index&                    cloudbox_on,
     const ArrayOfIndex&             cloudbox_limits,
     const Sparse&                   sensor_response,
     const Matrix&                   sensor_pos,
     const Matrix&                   sensor_los,
     const Vector&                   f_grid,
     const Index&                    stokes_dim,
     const Index&                    antenna_dim,
     const Vector&                   mblock_za_grid,
     const Vector&                   mblock_aa_grid,
     // Control Parameters:
     const String&                   species)
{
  // Set some useful (and needed) variables. 
  Index n_jq = jq.nelem();
  RetrievalQuantity rq;
  Index it, method;
  
  // Find the retrieval quantity related to this method, i.e. Gas species -
  // species. This works since the combined MainTag and Subtag is individual.
  bool check_rq = false;
  for (Index n=0; n<n_jq; n++)
  {
    if (jq[n].MainTag()=="Gas species" && jq[n].Subtag()==species)
    {
      check_rq = true;
      rq = jq[n];
    }
  }
  if (!check_rq)
  {
    ostringstream os;
    os << "There is no gas species retrieval quantities defined for:\n"
       << species;
    throw runtime_error(os.str());
  }
  
  // Store the start JacobianIndices and the Grids for this quantity
  ArrayOfIndex ji = rq.JacobianIndices();
  it = ji[0];
  ArrayOfVector jg = rq.Grids();
  if (rq.Unit()=="rel")
    method = 0;
  else
    method = 1;
  
  /* For each atmospheric dimension option
     1. Check that the retrieval grids are contained within the atmospheric
        grids.
     2. Create perturbation grids by adding points outside the atmospheric
        grids to the retrieval grids.
     3. Finally calculate a ArrayOfGridPos for each grid, these will be
        used to interpolate a perturbation into the atmospheric grids.
     Vector sizes initiated to 1 for correct sizing of perturbation field.*/
  Vector p_pert(1),lat_pert(1),lon_pert(1);
  ArrayOfGridPos p_gp,lat_gp,lon_gp;
  Index j_p = jg[0].nelem();
  Index j_lat = 1;
  Index j_lon = 1;
  if (!get_perturbation_grid( p_pert,p_gp,p_grid,jg[0]))
  {
    ostringstream os;
    os << "The atmospheric pressure grid *p_grid* is not defined "
       << "for all gridpoints \nin the retrieval pressure grid for "
       << "quantity " << rq;
    throw runtime_error(os.str());
  }
  
  if (atmosphere_dim>=2) 
  {
    j_lat = jg[1].nelem();
    if (!get_perturbation_grid( lat_pert, lat_gp, lat_grid, jg[1]))
    {
      ostringstream os;
      os << "The atmospheric latitude grid *lat_grid* is not defined "
         << "for all gridpoints \nin the retrieval latitude grid for "
         << "quantity " << rq;
      throw runtime_error(os.str());
    }
    
    if (atmosphere_dim==3) 
    {
      j_lon = jg[2].nelem();
      if (!get_perturbation_grid( lon_pert, lon_gp, lon_grid, jg[2]))
      {
        ostringstream os;
        os << "The atmospheric latitude grid *lat_grid* is not defined "
           << "for all gridpoints \nin the retrieval latitude grid for "
           << "quantity " << rq;
        throw runtime_error(os.str());
      }
    }
  }
  
  // Give verbose output
  out2 << "  Calculating retrieval quantity " << rq << "\n";
  
  // Find VMR field for these species. 
  ArrayOfSpeciesTag tags;
  array_species_tag_from_string( tags, species );
  Index si = chk_contains( "species", gas_species, tags );
  //FIXME: Why does the species index change???
  //Index si = rq.SpeciesIndex();

  // Calculate the reference spectrum, y. FIXME: Is this unnecessary if *y*
  out2 << "  Calculating the reference spectra.\n";  
  RteCalc( y, ppath, ppath_step, iy, rte_pos, rte_gp_p, rte_gp_lat, rte_gp_lon,
           rte_los, ppath_step_agenda, rte_agenda, iy_space_agenda,
           iy_surface_agenda, iy_cloudbox_agenda, atmosphere_dim, p_grid,
           lat_grid, lon_grid, z_field, r_geoid, z_surface, cloudbox_on, 
           cloudbox_limits, sensor_response, sensor_pos, sensor_los, f_grid,
           stokes_dim, antenna_dim, mblock_za_grid, mblock_aa_grid);

  // Declare variables for reference and difference spectrum
  Vector y_ref = y;
  
  // Variables for vmr field perturbation
  Tensor4 vmr_ref = vmr_field;
  //Tensor3 pert_field(p_pert.nelem(),lat_pert.nelem(),lon_pert.nelem());
  
  // Loop through the retrieval grid and calculate perturbation effect
  for (Index p_it=0; p_it<j_p; p_it++)
  {
    for (Index lat_it=0; lat_it<j_lat; lat_it++)
    {
      for (Index lon_it=0; lon_it<j_lon; lon_it++)
      {
        // FIXME: Only relative perturbation implemented
        // That is; pert_field is initiated to 1.0
        
        // Here we calculate the ranges of the perturbation. We want the
        // perturbation to continue outside the atmospheric grids for the
        // edge values.
        Range p_range = Range(0,0);
        Range lat_range = Range(0,0);
        Range lon_range = Range(0,0);
        get_perturbation_range( p_range, p_it, j_p);
        if (atmosphere_dim>=2)
        {
          get_perturbation_range( lat_range, lat_it, j_lat);
          if (atmosphere_dim==3)
          {
            get_perturbation_range( lon_range, lon_it, j_lon);
          }
        }
                              
        // Calculate the perturbed field according to atmosphere_dim
        switch (atmosphere_dim)
        {
          case 1:
          {
            // Here we perturb a vector
            perturbation_field_1d( vmr_field(si,joker,lat_it,lon_it), 
              p_gp, p_pert, p_range, rq.Perturbation(), method);
            break;
          }
          case 2:
          {
            // Here we perturb a matrix
            perturbation_field_2d( vmr_field(si,joker,joker,lon_it),
              p_gp, lat_gp, p_pert, lat_pert, p_range, lat_range, 
              rq.Perturbation(), method);
            break;
          }    
          case 3:
          {  
            // Here we need to perturb a tensor3
            perturbation_field_3d( vmr_field(si,joker,joker,joker), 
              p_gp, lat_gp, lon_gp, p_pert, lat_pert, lon_pert, p_range, 
              lat_range, lon_range, rq.Perturbation(), method);
            break;
          }
        }
          
        // Calculate the perturbed spectrum  
        out2 << "  Calculating perturbed spectra no. " << it+1 << " of "
             << j_p*j_lat*j_lon+ji[0] << "\n";
        RteCalc( y, ppath, ppath_step, iy, rte_pos, rte_gp_p, rte_gp_lat,
                 rte_gp_lon, rte_los, ppath_step_agenda, rte_agenda,
                 iy_space_agenda, iy_surface_agenda, iy_cloudbox_agenda, 
                 atmosphere_dim, p_grid, lat_grid, lon_grid, z_field, 
                 r_geoid, z_surface, cloudbox_on, cloudbox_limits, 
                 sensor_response, sensor_pos, sensor_los, f_grid, 
                 stokes_dim, antenna_dim, mblock_za_grid, mblock_aa_grid);
    
        // Restore the vmr_field
        vmr_field = vmr_ref;               
         
        // Add dy/dx as column in jacobian
        // FIXME: Save cpu time by implementing a sparse::insert_column()
        for (Index y_it=0; y_it<y.nelem(); y_it++)
        {
          jacobian.rw(y_it,it) = (y[y_it]-y_ref[y_it])/rq.Perturbation();
        }
        it++;
      }
    }
  }
  
  // Restore y before returning
  y = y_ref;

}

                     
//! jacobianCalcPointing
/*!
   See the online help (arts -d FUNCTION_NAME)

   \author Mattias Ekström�
   \date   2004-09-27
*/
void jacobianCalcPointing(
     // WS Output:
           Sparse&                   jacobian,
           Vector&                   y,
           Ppath&                    ppath,
           Ppath&                    ppath_step,
           Matrix&                   iy,
           Vector&                   rte_pos,
           GridPos&                  rte_gp_p,
           GridPos&                  rte_gp_lat,
           GridPos&                  rte_gp_lon,
           Vector&                   rte_los,
     // WS Input:
     const ArrayOfRetrievalQuantity& jq,
     const Vector&                   sensor_time,
     const Agenda&                   ppath_step_agenda,
     const Agenda&                   rte_agenda,
     const Agenda&                   iy_space_agenda,
     const Agenda&                   iy_surface_agenda,
     const Agenda&                   iy_cloudbox_agenda,
     const Index&                    atmosphere_dim,
     const Vector&                   p_grid,
     const Vector&                   lat_grid,
     const Vector&                   lon_grid,
     const Tensor3&                  z_field,
     const Matrix&                   r_geoid,
     const Matrix&                   z_surface,
     const Index&                    cloudbox_on,
     const ArrayOfIndex&             cloudbox_limits,
     const Sparse&                   sensor_response,
     const Matrix&                   sensor_pos,
     const Matrix&                   sensor_los,
     const Vector&                   f_grid,
     const Index&                    stokes_dim,
     const Index&                    antenna_dim,
     const Vector&                   mblock_za_grid,
     const Vector&                   mblock_aa_grid)
{
  // Set some useful (and needed) variables. 
//  Index n_los = sensor_los.nrows();
  Matrix sensor_los_pert = sensor_los;
  RetrievalQuantity rq;
  Index it;
  
  // Check that sensor_time is consistent with sensor_pos
  if (sensor_time.nelem()!=sensor_pos.nrows())
  {
    ostringstream os;
    os << "The WSV *sensor_time* must be defined for every "
       << "measurement block.\n";
    throw runtime_error(os.str());
  }

  // Find the retrieval quantity related to this method, i.e. Pointing
  // za offset. This works since the combined MainTag and Subtag is individual.
  bool check_rq = false;
  for (Index n=0; n<jq.nelem(); n++)
  {
    if (jq[n].MainTag()=="Pointing" && jq[n].Subtag()=="za offset")
    {
      check_rq = true;
      rq = jq[n];
    }
  }
  if (!check_rq)
  {
    throw runtime_error(
      "There is no pointing offset retrieval quantities defined.\n");
  }

  // Assert that the chosen unit is implemented
  assert( rq.Unit()=="abs" || rq.Unit()=="rel" );
  
  // FIXME: Should the size of *jacobian* be checked here?
  
  // Calculate the weight vector. We set sensor_time[0] to correspond to
  // -1 and sensor_time[end] to 1.
  chk_if_increasing("sensor_time", sensor_time);
  Vector weight(sensor_time.nelem());
  Numeric t0 = sensor_time[0];
  Numeric t1 = sensor_time[sensor_time.nelem()-1];
  for (Index tt=0; tt<weight.nelem(); tt++)
  {
    weight[tt] = 2*(sensor_time[tt]-t0)/(t1-t0)-1;
  }

  // Give verbose output
  out2 << "  Calculating retrieval quantity " << rq << "\n";
  
  // Calculate the reference spectrum, y
  RteCalc( y, ppath, ppath_step, iy, rte_pos, rte_gp_p, rte_gp_lat, rte_gp_lon,
           rte_los, ppath_step_agenda, rte_agenda, iy_space_agenda,
           iy_surface_agenda, iy_cloudbox_agenda, atmosphere_dim, p_grid,
           lat_grid, lon_grid, z_field, r_geoid, z_surface, cloudbox_on, 
           cloudbox_limits, sensor_response, sensor_pos, sensor_los, f_grid,
           stokes_dim, antenna_dim, mblock_za_grid, mblock_aa_grid);
  
  // Declare variables for reference and difference spectrum
  Vector y_ref = y;
  Vector dydx(y.nelem());
  
  // Calculate the jacobian for the zeroth order polynomial
  ArrayOfIndex ji = rq.JacobianIndices();
  
  // Add the pointing offset. It should be given as a relative change.
  // FIXME 2: this could be adjusted to account for azimuth offset
  if (rq.Unit()=="abs")
  {
    sensor_los_pert(joker,0) += rq.Perturbation();
  } 
  else if (rq.Unit()=="rel")
  {
    sensor_los_pert(joker,0) *= 1+rq.Perturbation();
  }
     
  // Calculate the perturbed spectrum  
  RteCalc( y, ppath, ppath_step, iy, rte_pos, rte_gp_p, rte_gp_lat,
           rte_gp_lon, rte_los, ppath_step_agenda, rte_agenda,
           iy_space_agenda, iy_surface_agenda, iy_cloudbox_agenda, 
           atmosphere_dim, p_grid, lat_grid, lon_grid, z_field, r_geoid, 
           z_surface, cloudbox_on, cloudbox_limits, sensor_response, 
           sensor_pos, sensor_los_pert, f_grid, stokes_dim, antenna_dim, 
           mblock_za_grid, mblock_aa_grid);
    
  // Calculate difference in spectrum and divide by perturbation,
  // here we want the whole dy/dx vector so that we can use it later
  dydx = y;
  dydx -= y_ref;
  dydx /= rq.Perturbation();
    
  // Add the weighted dy/dx as column in jacobian
  // FIXME: Save cpu time by implementing a sparse::insert_column()
  Index ny = y.nelem()/sensor_pos.nrows();
  for (it=ji[0]; it<=ji[1]; it++)
  {
    out2 << "  Calculating perturbed spectra no. " << it+1 << " of "
         << jacobian.ncols() << "\n";
    Index y_it = 0;
    for (Index ns=0; ns<sensor_pos.nrows(); ns++)
    {
      for (Index dummy=0; dummy<ny; dummy++)
      {
        jacobian.rw(y_it,it) = dydx[y_it]*pow(weight[ns],(Numeric) it-ji[0]);
        y_it++;
      }
    }
  }
     
  // Restore y before returning
  y = y_ref;
}


//! jacobianCalcTemperature
/*!
   See the online help (arts -d FUNCTION_NAME)

   \author Mattias Ekström�
   \date   2004-10-15
*/
void jacobianCalcTemperature(
     // WS Output:
           Sparse&                   jacobian,
           Tensor3&                  t_field,
           Vector&                   y,
           Ppath&                    ppath,
           Ppath&                    ppath_step,
           Matrix&                   iy, 
           Vector&                   rte_pos,
           GridPos&                  rte_gp_p,
           GridPos&                  rte_gp_lat,
           GridPos&                  rte_gp_lon,
           Vector&                   rte_los,
     // WS Input:
     const ArrayOfRetrievalQuantity& jq,
     const Agenda&                   ppath_step_agenda,
     const Agenda&                   rte_agenda,
     const Agenda&                   iy_space_agenda,
     const Agenda&                   iy_surface_agenda,
     const Agenda&                   iy_cloudbox_agenda,
     const Index&                    atmosphere_dim,
     const Vector&                   p_grid,
     const Vector&                   lat_grid,
     const Vector&                   lon_grid,
     const Tensor3&                  z_field,
     const Matrix&                   r_geoid,
     const Matrix&                   z_surface,
     const Index&                    cloudbox_on,
     const ArrayOfIndex&             cloudbox_limits,
     const Sparse&                   sensor_response,
     const Matrix&                   sensor_pos,
     const Matrix&                   sensor_los,
     const Vector&                   f_grid,
     const Index&                    stokes_dim,
     const Index&                    antenna_dim,
     const Vector&                   mblock_za_grid,
     const Vector&                   mblock_aa_grid)
{
  // Set some useful (and needed) variables. 
  RetrievalQuantity rq;
  Index it, method;
  
  // Find the retrieval quantity related to this method, i.e. Temperature.
  // For temperature only the main tag is checked.
  bool check_rq = false;
  for (Index n=0; n<jq.nelem(); n++)
  {
    if (jq[n].MainTag()=="Temperature")
    {
      check_rq = true;
      rq = jq[n];
    }
  }
  if (!check_rq)
  {
    ostringstream os;
    os << "There is no temperature retrieval quantities defined.\n";
    throw runtime_error(os.str());
  }
  
  // Store the start JacobianIndices and the Grids for this quantity
  ArrayOfIndex ji = rq.JacobianIndices();
  it = ji[0];
  ArrayOfVector jg = rq.Grids();
  if (rq.Unit()=="rel")
    method = 0;
  else
    method = 1;
      
  /* For each atmospheric dimension option
     1. Check that the retrieval grids are contained within the atmospheric
        grids.
     2. Create perturbation grids by adding points outside the atmospheric
        grids to the retrieval grids.
     3. Finally calculate a ArrayOfGridPos for each grid, these will be
        used to interpolate a perturbation into the atmospheric grids.
     Vector sizes initiated to 1 for correct sizing of perturbation field.*/
  Vector p_pert(1),lat_pert(1),lon_pert(1);
  ArrayOfGridPos p_gp,lat_gp,lon_gp;
  Index j_p = jg[0].nelem();
  Index j_lat = 1;
  Index j_lon = 1;
  if (!get_perturbation_grid( p_pert,p_gp,p_grid,jg[0]))
  {
    ostringstream os;
    os << "The atmospheric pressure grid *p_grid* is not defined "
       << "for all gridpoints \nin the retrieval pressure grid for "
       << "quantity " << rq;
    throw runtime_error(os.str());
  }
  
  if (atmosphere_dim>=2) 
  {
    j_lat = jg[1].nelem();
    if (!get_perturbation_grid( lat_pert, lat_gp, lat_grid, jg[1]))
    {
      ostringstream os;
      os << "The atmospheric latitude grid *lat_grid* is not defined "
         << "for all gridpoints \nin the retrieval latitude grid for "
         << "quantity " << rq;
      throw runtime_error(os.str());
    }
    
    if (atmosphere_dim==3) 
    {
      j_lon = jg[2].nelem();
      if (!get_perturbation_grid( lon_pert, lon_gp, lon_grid, jg[2]))
      {
        ostringstream os;
        os << "The atmospheric latitude grid *lat_grid* is not defined "
           << "for all gridpoints \nin the retrieval latitude grid for "
           << "quantity " << rq;
        throw runtime_error(os.str());
      }
    }
  }
  
  // Give verbose output
  out2 << "  Calculating retrieval quantity " << rq << "\n";
  
  // Calculate the reference spectrum, y. FIXME: Is this unnecessary if *y*
  out2 << "  Calculating the reference spectra.\n";  
  RteCalc( y, ppath, ppath_step, iy, rte_pos, rte_gp_p, rte_gp_lat, rte_gp_lon,
           rte_los, ppath_step_agenda, rte_agenda, iy_space_agenda,
           iy_surface_agenda, iy_cloudbox_agenda, atmosphere_dim, p_grid,
           lat_grid, lon_grid, z_field, r_geoid, z_surface, cloudbox_on, 
           cloudbox_limits, sensor_response, sensor_pos, sensor_los, f_grid,
           stokes_dim, antenna_dim, mblock_za_grid, mblock_aa_grid);

  // Declare variables for reference and difference spectrum
  Vector y_ref = y;
  
  // Variables for temperature field perturbation
  Tensor3 t_ref = t_field;
  
  // Loop through the retrieval grid and calculate perturbation effect
  for (Index p_it=0; p_it<j_p; p_it++)
  {
    for (Index lat_it=0; lat_it<j_lat; lat_it++)
    {
      for (Index lon_it=0; lon_it<j_lon; lon_it++)
      {
        // FIXME: Only relative perturbation implemented
        // That is; pert_field is initiated to 1.0
        
        // Here we calculate the ranges of the perturbation. We want the
        // perturbation to continue outside the atmospheric grids for the
        // edge values.
        Range p_range = Range(0,0);
        Range lat_range = Range(0,0);
        Range lon_range = Range(0,0);
        get_perturbation_range( p_range, p_it, j_p);
        if (atmosphere_dim>=2)
        {
          get_perturbation_range( lat_range, lat_it, j_lat);
          if (atmosphere_dim==3)
          {
            get_perturbation_range( lon_range, lon_it, j_lon);
          }
        }
                              
        // Calculate the perturbed field according to atmosphere_dim
        switch (atmosphere_dim)
        {
          case 1:
          {
            // Here we perturb a vector
            perturbation_field_1d( t_field(joker,lat_it,lon_it), 
              p_gp, p_pert, p_range, rq.Perturbation(), method);
            break;
          }
          case 2:
          {
            // Here we perturb a matrix
            perturbation_field_2d( t_field(joker,joker,lon_it), 
              p_gp, lat_gp, p_pert, lat_pert, p_range, lat_range, 
              rq.Perturbation(), method);
            break;
          }    
          case 3:
          {  
            // Here we need to perturb a tensor3
            perturbation_field_3d( t_field(joker,joker,joker), 
              p_gp, lat_gp, lon_gp, p_pert, lat_pert, lon_pert, p_range, 
              lat_range, lon_range, rq.Perturbation(), method);
            break;
          }
        }
          
        // Calculate the perturbed spectrum  
        out2 << "  Calculating perturbed spectra no. " << it+1 << " of "
             << j_p*j_lat*j_lon+ji[0] << "\n";
        RteCalc( y, ppath, ppath_step, iy, rte_pos, rte_gp_p, rte_gp_lat,
                 rte_gp_lon, rte_los, ppath_step_agenda, rte_agenda,
                 iy_space_agenda, iy_surface_agenda, iy_cloudbox_agenda, 
                 atmosphere_dim, p_grid, lat_grid, lon_grid, z_field, 
                 r_geoid, z_surface, cloudbox_on, cloudbox_limits, 
                 sensor_response, sensor_pos, sensor_los, f_grid, 
                 stokes_dim, antenna_dim, mblock_za_grid, mblock_aa_grid);
    
        // Restore the vmr_field
        t_field = t_ref;               
         
        // Add dy/dx as column in jacobian
        // FIXME: Save cpu time by implementing a sparse::insert_column()
        for (Index y_it=0; y_it<y.nelem(); y_it++)
        {
          jacobian.rw(y_it,it) = (y[y_it]-y_ref[y_it])/rq.Perturbation();
        }
        it++;
      }
    }
  }
  
  // Restore y before returning
  y = y_ref;

}

                     
//! jacobianClose
/*!
   See the online help (arts -d FUNCTION_NAME)

   \author Mattias Ekström�
   \date   2004-09-19
*/
void jacobianClose(// WS Output:
                   Sparse&                    jacobian,
                   ArrayOfRetrievalQuantity&  jacobian_quantities,
                   // WS Input:
                   const Matrix&              sensor_pos,
                   const Sparse&              sensor_response)
{
  // Check that *jacobian* has been initialised
  if (jacobian.nrows()!=0 && jacobian.ncols()!=0)
    throw runtime_error("The Jacobian matrix has not been initialised.");

  // Make sure that the array is not empty
  if (jacobian_quantities.nelem()==0)
    throw runtime_error(
      "No retrieval quantities has been added to *jacobian_quantities*");

  // Check that sensor_pol and sensor_response has been initialised
  if (sensor_pos.nrows()==0)
  {
    ostringstream os;
    os << "The number of rows in *sensor_pos* is zero, i.e. no measurement\n"
       << "blocks has been defined. This has to be done before calling\n"
       << "jacobianClose.";
    throw runtime_error(os.str());
  }
  if (sensor_response.nrows()==0)
  {
    ostringstream os;
    os << "The sensor has either to be defined or turned off before calling\n"
       << "jacobianClose.";
    throw runtime_error(os.str());
  }

  // Loop over retrieval quantities, set JacobianIndices
  Index nrows = sensor_pos.nrows()*sensor_response.nrows();
  Index ncols = 0;
  for (Index it=0; it<jacobian_quantities.nelem(); it++)
  {
    // Store start jacobian index
    ArrayOfIndex indices(2);
    indices[0] = ncols;

    // Count total number of field points, i.e. product of grid lengths
    Index cols = 1;
    ArrayOfVector grids = jacobian_quantities[it].Grids();
    for (Index jt=0; jt<grids.nelem(); jt++)
      cols *= grids[jt].nelem();
    ncols += cols;

    // Store stop index
    indices[1] = ncols-1;
    jacobian_quantities[it].JacobianIndices(indices);
  }
  
  // Resize *jacobian*
  jacobian.resize( nrows, ncols);
}


//! jacobianInit
/*!
   See the online help (arts -d FUNCTION_NAME)

   \author Mattias Ekström
   \date   2004-09-14
*/
void jacobianInit(// WS Output:
                  Sparse&                    jacobian,
                  ArrayOfRetrievalQuantity&  jacobian_quantities)
{
  // FIXME: Check if array and sparse has already been initialised?
  // If so, what to do? Error/warning

  // Resize arrays and sparse to zero.
  jacobian_quantities.resize(0);
  jacobian.resize(0,0);

  out2 <<
    "  Initialising *jacobian* and *jacobian_quantities*.\n";
}

