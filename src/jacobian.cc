/* Copyright (C) 2004 Mattias Ekström  <ekstrom@rss.chalmers.se>

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
  \file   jacobian.cc
  \author Mattias Ekström <ekstrom@rss.chalmers.se>
  \date   2004-09-14

  \brief  Routines for setting up the jacobian.
*/

#include "arts.h"
#include "jacobian.h"

ostream& operator << (ostream& os, const RetrievalQuantity& ot)
{
  return os << ot.MainTag() << " " << ot.Subtag();
}

/*===========================================================================
  === The functions in alphabetical order
  ===========================================================================*/

//! Check that the retrieval grids are defined for each atmosphere dim
/*!
   This function checks that the retrieval grids needed for the atmosphere
   dimension are defined. If not returns false and an output to print to
   the user. If they are defined they are stored in an array and true is
   returned.
   
   \param grids         The array of retrieval grids.
   \param os            The output string stream.
   \param p_grid        The pressure retrieval grid.
   \param lat_grid      The latitude retrieval grid.
   \param lon_grid      The longitude retrieval grid.
   \param p_grid_name   The control file name used for the pressure grid.
   \param lat_grid_name The control file name for the latitude grid.
   \param lon_grid_name The control file name for the longitude grid.
   \param dim           The atmosphere dimension
   \return              Boolean for check.
   
   \author Mattias Ekström
   \date   2004-10-14
*/ 
bool check_retrieval_grids(       ArrayOfVector& grids,
                                  ostringstream& os,
                            const Vector&        p_grid,
                            const Vector&        lat_grid,
                            const Vector&        lon_grid,
                            const String&        p_grid_name,
                            const String&        lat_grid_name,
                            const String&        lon_grid_name,
                            const Index&         dim)
{
  if (p_grid.nelem()==0)
  {
    os << "The grid vector *" << p_grid_name << "* is empty,"
       << " at least one pressure level\n"
       << "should be specified.";
    return false;
  }
  else
  {
    // Pressure grid ok, add it to grids
    grids[0]=p_grid;  
  }
  if (dim>=2)
  {
    // If 2D and 3D atmosphere, check latitude grid
    if (lat_grid.nelem()==0)
    {
      os << "The grid vector *" << lat_grid_name << "* is empty,"
         << " at least one latitude\n"
         << "should be specified for a 2D/3D atmosphere.";
      return false;
    }
    else
    {
      // Latitude grid ok, add it to grids
      grids[1]=lat_grid;
    }
    if (dim==3)
    {
      // For 3D atmosphere check longitude grid
      if (lon_grid.nelem()==0)
      {
        os << "The grid vector *" << lon_grid_name << "* is empty,"
           << " at least one longitude\n"
           << "should be specified for a 3D atmosphere.";
        return false;
      }
      else
      {
        // Longitude grid ok, add it to grids      
        grids[2]=lon_grid;
      }
    }
  }
  return true;
}             


//! Make perturbation grid and calculate array of GridPos
/*!
   This function constructs a perturbation grid which consists of the
   given retrieval grid with an extra endpoint added at each end.
   These endpoints lies outside the atmospheric grid. This enables the
   interpolation of an perturbation on the perturbation grid to be
   interpolated to the atmospheric grid. For this reason also the 
   ArrayOfGridPos is calculated. 
   
   The function returns false if the atmospheric grid does not cover the
   retrieval grid. If everything is fine, it returns true.
   
   \param pert      The perturbation grid.
   \param gp        Array of GridPos for interpolation.
   \param atm_grid  Atmospheric grid.
   \param jac_grid  Retrieval grid.
   \return          Boolean for check failure.
   
   \author Mattias Ekström
   \date   2004-10-14
*/   
bool get_perturbation_grid(       Vector&         pert,
                                  ArrayOfGridPos& gp,
                            const Vector&         atm_grid,
                            const Vector&         jac_grid)
{
  Index nj = jac_grid.nelem();
  Index na = atm_grid.nelem();
  Numeric ext = 1.0;
  
  // Check if the atmospheric grid is decreasing or increasing, and then
  // check that the atmospheric grid covers the jacobian grid
  if (is_decreasing(atm_grid))
  {
    ext *= -1;
    if (jac_grid[0]>atm_grid[0] || jac_grid[nj-1]<atm_grid[na-1])
      return false;
  }
  else
  {    
    if (jac_grid[0]<atm_grid[0] || jac_grid[nj-1]>atm_grid[na-1]) 
      return false;
  }
  
  // Atmospheric grid ok, create perturbation grid
  pert.resize(nj+2);
  pert[0] = atm_grid[0]-ext;
  pert[Range(1,nj)] = jac_grid;
  pert[nj+1] = atm_grid[na-1]+ext;
  gp.resize(na);
  gridpos( gp, pert, atm_grid);

  return true;
}


//! Get range for perturbation 
/*!
   This is a helper function that calculates the range in which the 
   perturbation should be added to the perturbation grid. This is needed
   to handle the edge effects. At the edges we want the perturbation to 
   continue outwards. 
   
   \param range     The range in the perturbation grid.
   \param index     The index of the perturbation in the retrieval grid.
   \param length    The length of retrieval grid
   
   \author Mattias Ekström
   \date   2004-10-14
*/   
void get_perturbation_range(       Range& range,
                             const Index& index,
                             const Index& length)
{
  if (index==0)
    range = Range(index,2);
  else if (index==length-1)
    range = Range(index+1,2);
  else 
    range = Range(index+1,1);

}


//! Calculate the 1D perturbation for a relative perturbation.
/*!
   This is a helper function that interpolated the perturbation field for
   a 1D relative perturbation onto the atmospheric field. 
   
   \param field    The interpolated perturbation field.
   \param p_gp     The GridPos for interpolation.
   \param p_pert   The perturbation grid.
   \param p_range  The range in the perturbation grid for the perturbation.
   \param size     The size of the perturbation.
   \param method   Relative perturbation==0, absolute==1
   
   \author Mattias Ekström
   \date   2004-10-14
*/   
void perturbation_field_1d(       VectorView      field,
                            const ArrayOfGridPos& p_gp,
                            const Vector&         p_pert,
                            const Range&          p_range,
                            const Numeric&        size,
                            const Index&          method)
{
  // Here we only perturb a vector
  Vector pert(field.nelem());
  Matrix itw(p_gp.nelem(),2);
  interpweights(itw,p_gp);
  // For relative pert_field should be 1.0 and for absolute 0.0
  Vector pert_field(p_pert.nelem(),1.0-(Numeric)method);
  pert_field[p_range] += size;
  interp( pert, itw, pert_field, p_gp);
  if (method==0)
  {
    field *= pert;
  }
  else
  {
    field += pert;
  }
}            


//! Calculate the 2D perturbation for a relative perturbation.
/*!
   This is a helper function that interpolated the perturbation field for
   a 2D relative perturbation onto the atmospheric field. 
   
   \param field     The interpolated perturbation field.
   \param p_gp      The GridPos for interpolation in the 1st dim.
   \param lat_gp    The GridPos for interpolation in the 2nd dim.
   \param p_pert    The perturbation grid in the 1st dim.
   \param lat_pert  The perturbation grid in the 2nd dim.
   \param p_range   The perturbation range in the 1st dim.
   \param lat_range The perturbation range in the 2nd dim.
   \param size      The size of the perturbation.
   \param method    Relative perturbation==0, absolute==1
   
   \author Mattias Ekström
   \date   2004-10-14
*/   
void perturbation_field_2d(       MatrixView      field,
                            const ArrayOfGridPos& p_gp,
                            const ArrayOfGridPos& lat_gp,
                            const Vector&         p_pert,
                            const Vector&         lat_pert,
                            const Range&          p_range,
                            const Range&          lat_range,
                            const Numeric&        size,
                            const Index&          method)
{
  // Here we perturb a matrix
  Matrix pert(field.nrows(),field.ncols());
  Tensor3 itw(p_gp.nelem(),lat_gp.nelem(),4);
  interpweights(itw,p_gp,lat_gp);
  // Init pert_field to 1.0 for relative and 0.0 for absolute
  Matrix pert_field(p_pert.nelem(),lat_pert.nelem(),1.0-(Numeric)method);
  pert_field(p_range,lat_range) += size;
  interp( pert, itw, pert_field, p_gp, lat_gp);
  if (method==0)
  {
    field *= pert;
  }
  else
  { 
    field += pert;
  }
}            


//! Calculate the 3D perturbation for a relative perturbation.
/*!
   This is a helper function that interpolated the perturbation field for
   a 3D relative perturbation onto the atmospheric field. 
   
   \param field     The interpolated perturbation field.
   \param p_gp      The GridPos for interpolation in the 1st dim.
   \param lat_gp    The GridPos for interpolation in the 2nd dim.
   \param lon_gp    The GridPos for interpolation in the 3rd dim.
   \param p_pert    The perturbation grid in the 1st dim.
   \param lat_pert  The perturbation grid in the 2nd dim.
   \param lon_pert  The perturbation grid in the 3rd dim.
   \param p_range   The perturbation range in the 1st dim.
   \param lat_range The perturbation range in the 2nd dim.
   \param lon_range The perturbation range in the 3rd dim.
   \param size      The size of the perturbation.
   \param method    Set to 0 for relative, and 1 for absolute.
   
   \author Mattias Ekström
   \date   2004-10-14
*/   
void perturbation_field_3d(       Tensor3View     field,
                            const ArrayOfGridPos& p_gp,
                            const ArrayOfGridPos& lat_gp,
                            const ArrayOfGridPos& lon_gp,
                            const Vector&         p_pert,
                            const Vector&         lat_pert,
                            const Vector&         lon_pert,
                            const Range&          p_range,
                            const Range&          lat_range,
                            const Range&          lon_range,
                            const Numeric&        size,
                            const Index&          method)
{
  // Here we need to perturb a tensor3
  Tensor3 pert(field.npages(),field.nrows(),field.ncols());
  Tensor4 itw(p_gp.nelem(),lat_gp.nelem(),lon_gp.nelem(),8);
  interpweights(itw,p_gp,lat_gp,lon_gp);
  // Init pert_field to 1.0 for relative and 0.0 for absolute
  Tensor3 pert_field(p_pert.nelem(),lat_pert.nelem(), lon_pert.nelem(),
    1.0-(Numeric)method);
  pert_field(p_range,lat_range,lon_range) += size;
  interp( pert, itw, pert_field, p_gp, lat_gp, lon_gp);
  if (method==0)
  {
    field *= pert;
  }
  else
  {
    field += pert;
  }
}            
