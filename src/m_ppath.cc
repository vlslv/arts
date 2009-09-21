/* Copyright (C) 2002-2008 Patrick Eriksson <Patrick.Eriksson@rss.chalmers.se>
                            
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
  \file   m_ppath.cc
  \author Patrick Eriksson <Patrick.Eriksson@rss.chalmers.se>
  \date   2002-05-08 

  \brief  Workspace functions releated to propagation paths variables.

  The file includes special functions to set the sensor position and LOS,
  and functions for calculation of propagation paths.

  These functions are listed in the doxygen documentation as entries of the
  file auto_md.h.
*/



/*===========================================================================
  === External declarations
  ===========================================================================*/

#include <cmath>
#include "arts.h"
#include "auto_md.h"
#include "check_input.h"
#include "math_funcs.h"
#include "messages.h"
#include "ppath.h"
#include "special_interp.h"
#include "xml_io.h"
#include "refraction.h"
#include "m_general.h"

extern const Numeric RAD2DEG;
extern const Numeric DEG2RAD;
extern const Numeric EARTH_GRAV_CONST;

/*===========================================================================
  === The functions (in alphabetical order)
  ===========================================================================*/

/* Workspace method: Doxygen documentation will be auto-generated */
void rte_losSet(
        // WS Output:
              Vector&    rte_los,
        // WS Input:
        const Index&     atmosphere_dim,
        // Control Parameters:
        const Numeric&   za,
        const Numeric&   aa )
{
  // Check input
  chk_if_in_range( "atmosphere_dim", atmosphere_dim, 1, 3 );

  if( atmosphere_dim == 1 )
    { rte_los.resize(1); }
  else
    {
      rte_los.resize(2);
      rte_los[1] = aa;
    }
  rte_los[0] = za;
}


/* Workspace method: Doxygen documentation will be auto-generated */
void rte_posAddGeoidWGS84(
        // WS Output:
              Vector&    rte_pos,
        // WS Input:
        const Index&     atmosphere_dim,
        const Numeric&   latitude_1d,
        const Numeric&   meridian_angle_1d )
{
  // Check input
  chk_if_in_range( "atmosphere_dim", atmosphere_dim, 1, 3 );
  chk_vector_length( "rte_pos", rte_pos, atmosphere_dim );

  // Use *sensor_posAddGeoidWGS84* to perform the calculations.
  Matrix m(1,rte_pos.nelem());
  m(0,joker) = rte_pos;
  sensor_posAddGeoidWGS84( m, atmosphere_dim, latitude_1d, meridian_angle_1d);
  rte_pos[0] = m(0,0);
}


/* Workspace method: Doxygen documentation will be auto-generated */
void rte_posAddRgeoid(
        // WS Output:
              Vector&    rte_pos,
        // WS Input:
        const Index&     atmosphere_dim,
        const Vector&    lat_grid,
        const Vector&    lon_grid,
        const Matrix&    r_geoid )
{
  // Check input
  chk_if_in_range( "atmosphere_dim", atmosphere_dim, 1, 3 );
  chk_vector_length( "rte_pos", rte_pos, atmosphere_dim );

  // Use *sensor_posAddRgeoid* to perform the calculations.
  Matrix m(1,rte_pos.nelem());
  m(0,joker) = rte_pos;
  sensor_posAddRgeoid( m, atmosphere_dim, lat_grid, lon_grid, r_geoid );
  rte_pos[0] = m(0,0);
}

/* Workspace method: Doxygen documentation will be auto-generated */
void rte_posSet(
        // WS Output:
              Vector&    rte_pos,
        // WS Input:
        const Index&     atmosphere_dim,
        // Control Parameters:
        const Numeric&   r_or_z,
        const Numeric&   lat,
        const Numeric&   lon )
{
  // Check input
  chk_if_in_range( "atmosphere_dim", atmosphere_dim, 1, 3 );

  rte_pos.resize(atmosphere_dim);
  rte_pos[0] = r_or_z;
  if( atmosphere_dim >= 2 )
    { rte_pos[1] = lat; }
  if( atmosphere_dim == 3 )
    { rte_pos[2] = lon; }
}


/* Workspace method: Doxygen documentation will be auto-generated */
void ppathCalc(
              Workspace&      ws,
        // WS Output:
              Ppath&          ppath,
        // WS Input:
        const Agenda&         ppath_step_agenda,
        const Index&          atmosphere_dim,
        const Vector&         p_grid,
        const Vector&         lat_grid,
        const Vector&         lon_grid,
        const Tensor3&        z_field,
        const Matrix&         r_geoid,
        const Matrix&         z_surface,
        const Index&          cloudbox_on, 
        const ArrayOfIndex&   cloudbox_limits,
        const Vector&         rte_pos,
        const Vector&         rte_los )
{
  ppath_calc( ws, ppath, ppath_step_agenda, atmosphere_dim, 
              p_grid, lat_grid, lon_grid, z_field, r_geoid, z_surface, 
              cloudbox_on, cloudbox_limits, rte_pos, rte_los, 1 );
}


/* Workspace method: Doxygen documentation will be auto-generated */
void ppath_stepGeometric(
        // WS Output:
              Ppath&     ppath_step,
        // WS Input:
        const Index&     atmosphere_dim,
        const Vector&    p_grid,
        const Vector&    lat_grid,
        const Vector&    lon_grid,
        const Tensor3&   z_field,
        const Matrix&    r_geoid,
        const Matrix&    z_surface,
        const Numeric&   ppath_lmax )
{
  // Input checks here would be rather costly as this function is called
  // many times. So we perform asserts in the sub-functions, but no checks 
  // here.

  if( atmosphere_dim == 1 )
    { ppath_step_geom_1d( ppath_step, p_grid, z_field(joker,0,0), 
                          r_geoid(0,0), z_surface(0,0), ppath_lmax ); }

  else if( atmosphere_dim == 2 )
    { ppath_step_geom_2d( ppath_step, p_grid, lat_grid,
                          z_field(joker,joker,0), r_geoid(joker,0), 
                          z_surface(joker,0), ppath_lmax ); }


  else if( atmosphere_dim == 3 )
    { ppath_step_geom_3d( ppath_step, p_grid, lat_grid, lon_grid,
                          z_field, r_geoid, z_surface, ppath_lmax ); }

  else
    { throw runtime_error( "The atmospheric dimensionality must be 1-3." ); }
}


/* Workspace method: Doxygen documentation will be auto-generated */
void ppath_stepRefractionEuler(
              Workspace&  ws,
        // WS Output:
              Ppath&      ppath_step,
              Numeric&    rte_pressure,
              Numeric&    rte_temperature,
              Vector&     rte_vmr_list,
              Numeric&    refr_index,
        // WS Input:
        const Agenda&     refr_index_agenda,
        const Index&      atmosphere_dim,
        const Vector&     p_grid,
        const Vector&     lat_grid,
        const Vector&     lon_grid,
        const Tensor3&    z_field,
        const Tensor3&    t_field,
        const Tensor4&    vmr_field,
        const Matrix&     r_geoid,
        const Matrix&     z_surface,
        const Numeric&    ppath_lmax,
        const Numeric&    ppath_lraytrace )
{
  // Input checks here would be rather costly as this function is called
  // many times. So we do only asserts. The keywords are checked here,
  // other input in the sub-functions to make them as simple as possible.

  assert( ppath_lraytrace > 0 );

  if( atmosphere_dim == 1 )
    { ppath_step_refr_1d( ws, ppath_step, rte_pressure, rte_temperature, 
                       rte_vmr_list, refr_index, refr_index_agenda,
                       p_grid, z_field(joker,0,0), t_field(joker,0,0), 
                       vmr_field(joker,joker,0,0), r_geoid(0,0), z_surface(0,0),
                       "linear_euler", ppath_lraytrace, ppath_lmax ); }

  else if( atmosphere_dim == 2 )
    { ppath_step_refr_2d( ws, ppath_step, rte_pressure, rte_temperature, 
                       rte_vmr_list, refr_index, refr_index_agenda,
                       p_grid, lat_grid, z_field(joker,joker,0),
                       t_field(joker,joker,0), vmr_field(joker, joker,joker,0),
                       r_geoid(joker,0), z_surface(joker,0), 
                       "linear_euler", ppath_lraytrace, ppath_lmax ); }

  else if( atmosphere_dim == 3 )
    { ppath_step_refr_3d( ws, ppath_step, rte_pressure, rte_temperature, 
                       rte_vmr_list, refr_index, refr_index_agenda,
                       p_grid, lat_grid, lon_grid, z_field, 
                       t_field, vmr_field, r_geoid, z_surface, 
                       "linear_euler", ppath_lraytrace, ppath_lmax ); }

  else
    { throw runtime_error( "The atmospheric dimensionality must be 1-3." ); }
}


/* Workspace method: Doxygen documentation will be auto-generated */
void sensor_posAddGeoidWGS84(
        // WS Output:
              Matrix&    sensor_pos,
        // WS Input:
        const Index&     atmosphere_dim,
        const Numeric&   latitude_1d,
        const Numeric&   meridian_angle_1d )
{
  // Check input
  chk_if_in_range( "atmosphere_dim", atmosphere_dim, 1, 3 );
  chk_matrix_ncols( "sensor_pos", sensor_pos, atmosphere_dim );

  // Number of positions
  const Index npos = sensor_pos.nrows();
  if( npos == 0 )
    throw runtime_error("The number of positions is 0, must be at least 1.");

  // The function *r_geoidWGS84 is used to get the geoid radius, but some
  // tricks are needed as this WSM sets *r_geoid* for all crossings of the
  // latitude and longitude grids.
  // For 2D and 3D, the function is always called with the atmospheric 
  // dimensionality set to 2, and the latitudes of concern are put into 
  // the latitude grid. An extra dummy value is needed if there is only one
  // position in *sensor_pos*.

  if( atmosphere_dim == 1 )
    {
      Vector lats(0);

      // The size of the r-matrix is set inside the function.
      Matrix r;
      r_geoidWGS84( r, 1, lats, Vector(0), latitude_1d, meridian_angle_1d );
      
      // Add the geoid radius to the geometric altitudes
      sensor_pos(joker,0) += r(0,0);
    }

  else
    {
      for( Index i=0; i<npos; i++ )
        {
          Vector lats(2);
          Index  pos_used;

          if( sensor_pos(i,1) < 90 )
            {
              lats[0]  = sensor_pos(i,1);
              lats[1]  = 90;
              pos_used = 0;
            }
          else
            {
              lats[0]  = -90;
              lats[1]  = sensor_pos(i,1);
              pos_used = 1;
            }

          // The size of the r-matrix is set inside the function.
          Matrix r;
          r_geoidWGS84( r, 2, lats, Vector(0), -999, -999 );
          
          sensor_pos(i,0) += r(pos_used,0);
        }
    }
}


/* Workspace method: Doxygen documentation will be auto-generated */
void sensor_posAddRgeoid(
        // WS Output:
              Matrix&    sensor_pos,
        // WS Input:
        const Index&     atmosphere_dim,
        const Vector&    lat_grid,
        const Vector&    lon_grid,
        const Matrix&    r_geoid )
{
  // Check input
  chk_if_in_range( "atmosphere_dim", atmosphere_dim, 1, 3 );
  chk_matrix_ncols( "sensor_pos", sensor_pos, atmosphere_dim );
  chk_atm_surface( "r_geoid", r_geoid, atmosphere_dim, lat_grid, lon_grid );

  // Number of positions
  const Index npos = sensor_pos.nrows();
  //
  if( npos == 0 )
    throw runtime_error("The number of positions is 0, must be at least 1.");

  if( atmosphere_dim == 1 )
    { sensor_pos(joker,0) += r_geoid(0,0); }

  else
    {
      // Check that positions in sensor_pos are inside the lat and lon grids
      if( min(sensor_pos(joker,1)) < lat_grid[0]  || 
                                    max(sensor_pos(joker,1)) > last(lat_grid) )
        throw runtime_error(
             "You have given a position with a latitude outside *lat_grid*." );
      if( atmosphere_dim == 3 )
        {
          if( min(sensor_pos(joker,2)) < lon_grid[0]  || 
                                    max(sensor_pos(joker,2)) > last(lon_grid) )
            throw runtime_error(
            "You have given a position with a longitude outside *lon_grid*." );
        }

      if( atmosphere_dim == 2 )
        {
          ArrayOfGridPos gp(npos);
          Matrix itw(npos,2);
          gridpos( gp, lat_grid, sensor_pos(joker,1) );
          interpweights( itw, gp );
          Vector v_rgeoid(npos);
          interp( v_rgeoid, itw, r_geoid(joker,0), gp );
          for( Index i=0; i<npos; i++ )
            { sensor_pos(i,0) += v_rgeoid[i]; } 
        }
      else
        {
          ArrayOfGridPos gp_lat(npos), gp_lon(npos);
          Matrix itw(npos,4);
          gridpos( gp_lat, lat_grid, sensor_pos(joker,1) );
          gridpos( gp_lon, lon_grid, sensor_pos(joker,2) );
          interpweights( itw, gp_lat, gp_lon );
          Vector v_rgeoid(npos);
          interp( v_rgeoid, itw, r_geoid, gp_lat, gp_lon );
          for( Index i=0; i<npos; i++ )
            { sensor_pos(i,0) += v_rgeoid[i]; } 
        }
    }
}


/* Workspace method: Doxygen documentation will be auto-generated */
void VectorZtanToZaRefr1D(
                        Workspace&          ws,
                        // WS Generic Output:
                        Vector&             za_vector,
                        // WS Input:
                        const Agenda&       refr_index_agenda,
                        const Matrix&       sensor_pos,
                        const Vector&       p_grid,
                        const Tensor3&      t_field,
                        const Tensor3&      z_field,
                        const Tensor4&      vmr_field,
                        const Matrix&       r_geoid,
                        const Index&        atmosphere_dim,
                        // WS Generic Input:
                        const Vector&       ztan_vector)
{
  if( atmosphere_dim != 1 ) {
    throw runtime_error( "The function can only be used for 1D atmospheres." );
  }

  if( ztan_vector.nelem() != sensor_pos.nrows() ) {
    ostringstream os;
    os << "The number of altitudes in true tangent altitude vector must\n"
       << "match the number of positions in *sensor_pos*.";
    throw runtime_error( os.str() );
  }

  //Set za_vector's size equal to ztan_vector
  za_vector.resize( ztan_vector.nelem() );

  // Define refraction variables
  Numeric   refr_index, rte_pressure, rte_temperature;
  Vector    rte_vmr_list;

  //Calculate refractive index for the tangential altitudes
  for( Index i=0; i<ztan_vector.nelem(); i++ ) {
    get_refr_index_1d( ws, refr_index, rte_pressure, rte_temperature, 
                       rte_vmr_list, refr_index_agenda, p_grid, r_geoid(0,0), 
                       z_field(joker,0,0), t_field(joker,0,0), 
                       vmr_field(joker,joker,0,0), ztan_vector[i]+r_geoid(0,0) );

    //Calculate zenith angle
    za_vector[i] = 180-asin(refr_index*(ztan_vector[i]+r_geoid(0,0)) /
                            sensor_pos(i,0))*RAD2DEG;
  }
}



/* Workspace method: Doxygen documentation will be auto-generated */
void VectorZtanToZa1D(
                        // WS Generic Output:
                        Vector&             za_vector,
                        // WS Input:
                        const Matrix&       sensor_pos,
                        const Matrix&       r_geoid,
                        const Index&        atmosphere_dim,
                        // WS Generic Input:
                        const Vector&       ztan_vector)
{
  if( atmosphere_dim != 1 ) {
    throw runtime_error( "The function can only be used for 1D atmospheres." );
  }

  const Index   npos = sensor_pos.nrows();

  if( ztan_vector.nelem() != npos )
    {
      ostringstream os;
      os << "The number of altitudes in the geometric tangent altitude vector must\n"
         << "match the number of positions in *sensor_pos*.";
      throw runtime_error( os.str() );
    }

  za_vector.resize(npos);

  for( Index i=0; i<npos; i++ )
    { za_vector[i] = geompath_za_at_r( r_geoid(0,0) + ztan_vector[i], 100,
                                                           sensor_pos(i,0) ); }
}

