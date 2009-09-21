/* Copyright (C) 2003-2008 Cory Davis <cory@met.ed.ac.uk>
                            
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
  === File description 
  ===========================================================================*/

/*!
  \file   m_montecarlo.cc
  \author Cory Davis <cory@met.ed.ac.uk>
  \date   2003-06-19 

  \brief  Workspace functions for the solution of cloud-box radiative transfer 
by Monte Carlo methods.  All of these functions refer to 3D calculations

  These functions are listed in the doxygen documentation as entries of the
  file auto_md.h.
*/
/*===========================================================================
  === External declarations
  ===========================================================================*/

#include "messages.h"
#include "arts.h"
#include "ppath.h"
#include "matpackI.h"
#include "special_interp.h"
#include "check_input.h"
#include <stdexcept>
#include <cmath>
#include "rte.h"
#include "lin_alg.h"
#include "auto_md.h"
#include "logic.h"
#include "physics_funcs.h"
#include "xml_io.h"
#include "montecarlo.h"
#include "rng.h"
#include <ctime>
#include <fstream>
#include "mc_interp.h"
#include "math_funcs.h"

extern const Numeric DEG2RAD;
extern const Numeric RAD2DEG;
extern const Numeric PI;
extern const Numeric BOLTZMAN_CONST;
extern const Numeric SPEED_OF_LIGHT;


/*===========================================================================
  === The functions (in alphabetical order)
  ===========================================================================*/


/* Workspace method: Doxygen documentation will be auto-generated */
void mc_IWP_cloud_opt_pathCalc(
                         Workspace& ws,
                         //output
                         Numeric& mc_IWP,
                         Numeric& mc_cloud_opt_path,
                         Numeric& mc_IWP_error,
                         Numeric& mc_cloud_opt_path_error,
                         Index&                mc_iteration_count,
                         //input
                         const MCAntenna&      mc_antenna,
                         const Matrix&         sensor_pos,
                         const Matrix&         sensor_los,
                         const Agenda&         ppath_step_agenda,
                         const Vector&         p_grid,
                         const Vector&         lat_grid, 
                         const Vector&         lon_grid, 
                         const Matrix&         r_geoid, 
                         const Matrix&         z_surface,
                         const Tensor3&        z_field, 
                         const Tensor3&        t_field, 
                         const Tensor4&        vmr_field, 
                         const ArrayOfIndex&   cloudbox_limits, 
                         const Tensor4&        pnd_field,
                         const ArrayOfSingleScatteringData& scat_data_mono,
                         const Vector& particle_masses,
                         const Index&          mc_seed,
                         //Keyword params
                         const Index&          max_iter
                         )
{
  //if antenna is pencil beam jsut need a single path integral, otherwise
  //for now do monte carlo integration
  if (mc_antenna.get_type()==ATYPE_PENCIL_BEAM)
    {
      iwp_cloud_opt_pathCalc(ws, mc_IWP,mc_cloud_opt_path,sensor_pos(0,joker),
                             sensor_los(0,joker),
                             ppath_step_agenda,p_grid,lat_grid,lon_grid,
                             r_geoid,z_surface,z_field,t_field,vmr_field,cloudbox_limits,
                             pnd_field,scat_data_mono,particle_masses);
    }
  else
    {
      Numeric iwp,cloud_opt_path;
      Numeric iwp_squared=0;
      Numeric tau_squared=0;
      mc_iteration_count=0;
      mc_IWP=0;
      mc_cloud_opt_path=0;
      Vector local_rte_los(2);
      Rng rng;
      rng.seed(mc_seed);
      //Begin Main Loop
      while (mc_iteration_count<max_iter)
        {
          mc_antenna.draw_los(local_rte_los,rng,sensor_los(0,joker));
          mc_iteration_count+=1;
          iwp_cloud_opt_pathCalc(ws, iwp,cloud_opt_path,sensor_pos(0,joker),
                                 local_rte_los,
                                 ppath_step_agenda,p_grid,lat_grid,lon_grid,
                                 r_geoid,z_surface,z_field,t_field,vmr_field,
                                 cloudbox_limits,
                                 pnd_field,scat_data_mono,particle_masses);
          mc_IWP+=iwp;
          iwp_squared+=iwp*iwp;
          mc_cloud_opt_path+=cloud_opt_path;
          tau_squared+=cloud_opt_path*cloud_opt_path;
        }
      mc_IWP/=(Numeric)mc_iteration_count;
      mc_cloud_opt_path/=(Numeric)mc_iteration_count;
      mc_IWP_error=sqrt((iwp_squared/(Numeric)mc_iteration_count-mc_IWP*mc_IWP)
                        /(Numeric)mc_iteration_count);
      mc_cloud_opt_path_error=sqrt((tau_squared/(Numeric)mc_iteration_count-
                                    mc_cloud_opt_path*mc_cloud_opt_path)
                                   /(Numeric)mc_iteration_count);
      
    }

}


/* Workspace method: Doxygen documentation will be auto-generated */
void mc_antennaSetGaussian(
                           MCAntenna& mc_antenna,
                           //keyword arguments
                           const Numeric& za_sigma,
                           const Numeric& aa_sigma
                           )
{
  mc_antenna.set_gaussian(za_sigma,aa_sigma);
}


/* Workspace method: Doxygen documentation will be auto-generated */
void mc_antennaSetGaussianByFWHM(
                           MCAntenna& mc_antenna,
                           //keyword arguments
                           const Numeric& za_fwhm,
                           const Numeric& aa_fwhm
                           )
{
  mc_antenna.set_gaussian_fwhm(za_fwhm,aa_fwhm);
}


/* Workspace method: Doxygen documentation will be auto-generated */
void mc_antennaSetPencilBeam(
                            MCAntenna& mc_antenna
                            )
{
  mc_antenna.set_pencil_beam();
}


/* Workspace method: Doxygen documentation will be auto-generated */
void MCGeneral(Workspace&            ws,
               Vector&               y,
               Index&                mc_iteration_count,
               Vector&               mc_error,
               Tensor3&              mc_points,
               const MCAntenna&      mc_antenna,
               const Vector&         f_grid,
               const Index&          f_index,
               const Matrix&         sensor_pos,
               const Matrix&         sensor_los,
               const Index&          stokes_dim,
               const Agenda&         iy_space_agenda,
               const Agenda&         surface_prop_agenda,
               const Agenda&         opt_prop_gas_agenda,
               const Agenda&         abs_scalar_gas_agenda, 
               const Vector&         p_grid,
               const Vector&         lat_grid, 
               const Vector&         lon_grid, 
               const Tensor3&        z_field, 
               const Matrix&         r_geoid, 
               const Matrix&         z_surface,
               const Tensor3&        t_field, 
               const Tensor4&        vmr_field, 
               const ArrayOfIndex&   cloudbox_limits, 
               const Tensor4&        pnd_field,
               const ArrayOfSingleScatteringData& scat_data_mono,
               const Index&          mc_seed,
               const String&         y_unit,
               const Numeric&        std_err,
               const Index&          max_time,
               const Index&          max_iter,
               const Index&          z_field_is_1D
               )
{
  //Check keyword input
  if (max_time<0 && max_iter<0 && std_err<0){
    throw runtime_error( "At least one of std_err, max_time, and max_iter must be positive" );
  }

  if( f_index < 0 )
    {
      throw runtime_error( 
                   "The option of f_index < 0 is not handled by this method." );
    }
  if( f_index >= f_grid.nelem() )
    {
      throw runtime_error( 
                   "*f_index* is outside the range of *f_grid*." );
    }

  Ppath ppath_step;
  Rng rng;                      //Random Number generator
  time_t start_time=time(NULL);
  Index N_pt=pnd_field.nbooks();//Number of particle types
  Vector pnd_vec(N_pt); //Vector of particle number densities used at each point
  Vector Z11maxvector;//Vector holding the maximum phase function for each 
  bool anyptype30=is_anyptype30(scat_data_mono);
  if (anyptype30)
    {
      findZ11max(Z11maxvector,scat_data_mono);
    }
  rng.seed(mc_seed);
  bool keepgoing,inside_cloud; // flag indicating whether to stop tracing a photons path
  Numeric g,temperature,albedo,g_los_csc_theta;
  Matrix A(stokes_dim,stokes_dim),Q(stokes_dim,stokes_dim),evol_op(stokes_dim,stokes_dim),

    ext_mat_mono(stokes_dim,stokes_dim),q(stokes_dim,stokes_dim),newQ(stokes_dim,stokes_dim),
    Z(stokes_dim,stokes_dim);
  q=0.0;newQ=0.0;
  mc_iteration_count=0;
  Vector vector1(stokes_dim),abs_vec_mono(stokes_dim),I_i(stokes_dim),Isum(stokes_dim),
    Isquaredsum(stokes_dim);
  y.resize(stokes_dim);
  y=0;
  Index termination_flag=0;
  mc_error.resize(stokes_dim);
  //local versions of workspace
  Matrix local_iy(1,stokes_dim),local_surface_emission(1,stokes_dim),local_surface_los;
  Tensor4 local_surface_rmatrix;
  Vector local_rte_pos(2);
  Vector local_rte_los(2);
  Vector new_rte_los(2);
    Index np;
  mc_points.resize(p_grid.nelem(),lat_grid.nelem(),lon_grid.nelem());
  mc_points=0;
  Isum=0.0;Isquaredsum=0.0;
  Numeric std_err_i;
  bool convert_to_rjbt=false;
  if ( y_unit == "RJBT" )
    { 
      std_err_i=f_grid[f_index]*f_grid[f_index]*2*BOLTZMAN_CONST/
                                          SPEED_OF_LIGHT/SPEED_OF_LIGHT*std_err;
      convert_to_rjbt=true;
    }
  else if ( y_unit == "1" )
    {
      std_err_i=std_err;
    }
  else
    {
      ostringstream os;
      os << "Invalid value for *y_unit*:" << y_unit <<".\n" 
         << "This method allows only the options \"RJBT\" and \"1\".";
      throw runtime_error( os.str() );
    }
      

  //Begin Main Loop
  while (true)
    {
      mc_iteration_count+=1;
      keepgoing=true; //flag indicating whether to continue tracing a photon path
      //Sample a FOV direction
      mc_antenna.draw_los(local_rte_los,rng,sensor_los(0,joker));
      id_mat(Q);
      local_rte_pos=sensor_pos(0,joker);
      I_i=0.0;

      
      while (keepgoing)
        {
          mcPathTraceGeneral( ws,
                      evol_op, abs_vec_mono, temperature, ext_mat_mono, 
                      rng, local_rte_pos, local_rte_los, pnd_vec, g,ppath_step,
                      termination_flag, inside_cloud, opt_prop_gas_agenda,
                      abs_scalar_gas_agenda, stokes_dim, f_index, p_grid, 
                      lat_grid, lon_grid, z_field, r_geoid, z_surface,
                      t_field, vmr_field, cloudbox_limits, pnd_field,
                      scat_data_mono, z_field_is_1D );
           
          np=ppath_step.np;
          mc_points(ppath_step.gp_p[np-1].idx,ppath_step.gp_lat[np-1].idx,
                    ppath_step.gp_lon[np-1].idx)+=1;
          if (termination_flag==1)
            {
              iy_space_agendaExecute(ws, local_iy,local_rte_pos,local_rte_los,
                                     iy_space_agenda);
              mult(vector1,evol_op,local_iy(f_index,joker));
              mult(I_i,Q,vector1);
              I_i/=g;
              keepgoing=false; //stop here. New photon.
            }
          else if (termination_flag==2)
            {
              //decide whether we have reflection or emission
              surface_prop_agendaExecute( ws, local_surface_emission, 
                local_surface_los, local_surface_rmatrix, 
                local_rte_pos, local_rte_los, ppath_step.gp_p[np-1],
                ppath_step.gp_lat[np-1],ppath_step.gp_lon[np-1],
                surface_prop_agenda );

              if( local_surface_los.nrows() > 1 )
                {
                  throw runtime_error( 
                              "The method handles only specular reflections." );
                }

              //deal with blackbody case
              if (local_surface_los.nrows()==0)
                {
                  mult(vector1,evol_op,local_surface_emission(f_index,joker));
                  mult(I_i,Q,vector1);
                  I_i/=g;
                  keepgoing=false;
                }
              else
                //decide between reflection and emission
                {
                  Numeric R11=local_surface_rmatrix(0,f_index,0,0);
                  if (rng.draw()>R11)
                    {
                      //then we have emission
                      //Matrix oneminusR(stokes_dim,stokes_dim);
                      //id_mat(oneminusR);
                      //oneminusR-=local_surface_rmatrix(0,0,joker,joker);
                      //oneminusR/=1-R11;
                      //mult(vector1,oneminusR,local_surface_emission(0,joker));
                      mult(vector1,evol_op,local_surface_emission(f_index,joker));
                      mult(I_i,Q,vector1);
                      I_i/=g*(1-R11);
                      keepgoing=false;
                    }
                  else
                    {
                      //we have reflection
                      local_rte_los=local_surface_los(0,joker);
                      
                      mult(q,evol_op,local_surface_rmatrix(0,f_index,joker,joker));
                      mult(newQ,Q,q);
                      Q=newQ;
                      Q/=g*R11;
                    }
                }
            }
          else if (inside_cloud)
            {
              //we have another scattering/emission point 
              //Estimate single scattering albedo
              albedo=1-abs_vec_mono[0]/ext_mat_mono(0,0);
              //cout<<"albedo = "<<albedo<<" ext_mat_mono(0,0) = "<<ext_mat_mono(0,0)<<" abs_vec_mono[0] = "<<abs_vec_mono[0]<<"\n";
              //determine whether photon is emitted or scattered
              if (rng.draw()>albedo)
                {
                  //Calculate emission
                  Numeric planck_value = planck( f_grid[f_index], temperature );
                  Vector emission=abs_vec_mono;
                  emission*=planck_value;
                  Vector emissioncontri(stokes_dim);
                  mult(emissioncontri,evol_op,emission);
                  emissioncontri/=(g*(1-albedo));//yuck!
                  mult(I_i,Q,emissioncontri);
                  keepgoing=false;
                  //cout << "emission contri" <<  I_i[0] << "\n";
                }
              else
                {
                  //we have a scattering event
                  //Sample new line of sight.
                  
                  Sample_los (new_rte_los,g_los_csc_theta,Z,rng,local_rte_los,
                              scat_data_mono,stokes_dim,
                              pnd_vec,anyptype30,Z11maxvector,ext_mat_mono(0,0)-abs_vec_mono[0],temperature);
                                           
                  Z/=g*g_los_csc_theta*albedo;
                  
                  mult(q,evol_op,Z);
                  mult(newQ,Q,q);
                  Q=newQ;
                  //scattering_order+=1;
                  local_rte_los=new_rte_los;
                  //if (silent==0){cout <<"mc_iteration_count = "<<mc_iteration_count << 
                  //                 ", scattering_order = " <<scattering_order <<"\n";}
                }
            }
          else
            {
              //Must be clear sky emission point
              //Calculate emission
              Numeric planck_value = planck( f_grid[f_index], temperature );
              Vector emission=abs_vec_mono;
              emission*=planck_value;
              Vector emissioncontri(stokes_dim);
              mult(emissioncontri,evol_op,emission);
              emissioncontri/=g;
              mult(I_i,Q,emissioncontri);
              keepgoing=false;
              //cout << "emission contri" <<  I_i[0] << "\n";
            }
        }
      Isum += I_i;
      for(Index j=0; j<stokes_dim; j++)
        {
          assert(!isnan(I_i[j]));
          Isquaredsum[j] += I_i[j]*I_i[j];
        }
      y=Isum;
      y/=(Numeric)mc_iteration_count;
      for(Index j=0; j<stokes_dim; j++) 
        {
          mc_error[j]=sqrt((Isquaredsum[j]/(Numeric)mc_iteration_count-y[j]*y[j])/(Numeric)mc_iteration_count);
        }
      if (std_err>0 && mc_iteration_count>=100 && mc_error[0]<std_err_i){break;}
      if (max_time>0 && (Index)(time(NULL)-start_time)>=max_time){break;}
      if (max_iter>0 && mc_iteration_count>=max_iter){break;}
    }
  if ( convert_to_rjbt )
    {
      for(Index j=0; j<stokes_dim; j++) 
        {
          y[j]=invrayjean(y[j],f_grid[f_index]);
          mc_error[j]=invrayjean(mc_error[j],f_grid[f_index]);
        }
    }
}


/* Workspace method: Doxygen documentation will be auto-generated */
void MCIPA(Workspace&            ws,
           Vector&               y,
           Index&                mc_iteration_count,
           Vector&               mc_error,
           Tensor3&              mc_points,
           const MCAntenna&      mc_antenna,
           const Vector&         f_grid,
           const Index&          f_index,
           const Matrix&         sensor_pos,
           const Matrix&         sensor_los,
           const Index&          stokes_dim,
           const Agenda&         iy_space_agenda,
           const Agenda&         surface_prop_agenda,
           const Agenda&         opt_prop_gas_agenda,
           const Agenda&         abs_scalar_gas_agenda, 
           const Agenda&         ppath_step_agenda,
           const Vector&         p_grid,
           const Vector&         lat_grid, 
           const Vector&         lon_grid, 
           const Tensor3&        z_field, 
           const Matrix&         r_geoid, 
           const Matrix&         z_surface,
           const Tensor3&        t_field, 
           const Tensor4&        vmr_field, 
           const ArrayOfIndex&   cloudbox_limits, 
           const Tensor4&        pnd_field,
           const ArrayOfSingleScatteringData& scat_data_mono,
           const Index&          mc_seed,
           const String&         y_unit,
           //Keyword params
           const Numeric&        std_err,
           const Index&          max_time,
           const Index&          max_iter,
           const Index&          z_field_is_1D
           )
{
  //Check keyword input
  if (max_time<0 && max_iter<0 && std_err<0){
    throw runtime_error( "At least one of std_err, max_time, and max_iter must be positive" );
  }
  Ppath ppath_step;
  Rng rng;                      //Random Nuimber generator
  time_t start_time=time(NULL);
  Index N_pt=pnd_field.nbooks();//Number of particle types
  Vector pnd_vec(N_pt); //Vector of particle number densities used at each point
  Vector Z11maxvector;//Vector holding the maximum phase function for each 
  bool anyptype30=is_anyptype30(scat_data_mono);
  if (anyptype30)
    {
      findZ11max(Z11maxvector,scat_data_mono);
    }
  rng.seed(mc_seed);
  bool keepgoing,inside_cloud; // flag indicating whether to stop tracing a photons path
  Numeric g,temperature,albedo,g_los_csc_theta;
  Matrix A(stokes_dim,stokes_dim),Q(stokes_dim,stokes_dim),evol_op(stokes_dim,stokes_dim),
    ext_mat_mono(stokes_dim,stokes_dim),q(stokes_dim,stokes_dim),newQ(stokes_dim,stokes_dim),
    Z(stokes_dim,stokes_dim);
  q=0.0;newQ=0.0;
  mc_iteration_count=0;
  Vector vector1(stokes_dim),abs_vec_mono(stokes_dim),I_i(stokes_dim),Isum(stokes_dim),
    Isquaredsum(stokes_dim);
  y.resize(stokes_dim);
  y=0;
  Index termination_flag=0;
  mc_error.resize(stokes_dim);
  //local versions of workspace
  Matrix local_iy(1,stokes_dim),local_surface_emission(1,stokes_dim),local_surface_los;
  Tensor4 local_surface_rmatrix;
  Vector local_rte_pos(2);
  Vector local_rte_los(2);
  Vector new_rte_los(2);
    Index np;
  mc_points.resize(p_grid.nelem(),lat_grid.nelem(),lon_grid.nelem());
  mc_points=0;
  Isum=0.0;Isquaredsum=0.0;
  Numeric std_err_i;
  bool convert_to_rjbt=false;
  if ( y_unit == "RJBT" )
    { 
      std_err_i=f_grid[0]*f_grid[0]*2*BOLTZMAN_CONST/SPEED_OF_LIGHT/SPEED_OF_LIGHT*
        std_err;
      convert_to_rjbt=true;
    }
  else if ( y_unit == "1" )
    {
      std_err_i=std_err;
    }
  else
    {
      ostringstream os;
      os << "Invalid value for *y_unit*:" << y_unit <<".\n" 
         << "This method allows only the options \"RJBT\" and \"1\".";
      throw runtime_error( os.str() );
    }
      
  //Begin Main Loop
  while (true)
    {
      mc_iteration_count+=1;
      keepgoing=true; //flag indicating whether to continue tracing a photon path
      //Sample a FOV direction
      mc_antenna.draw_los(local_rte_los,rng,sensor_los(0,joker));
      id_mat(Q);
      local_rte_pos=sensor_pos(0,joker);
      I_i=0.0;

      //Obtain a reference propagation path to use for obtaining optical properties
      //for the IPA method.
      Ppath ppath;
      ppath_calc( ws, ppath, ppath_step_agenda, 3, 
                  p_grid, lat_grid, lon_grid, z_field, r_geoid, z_surface, 
                  0, cloudbox_limits, local_rte_pos, local_rte_los, 1 );
      
      while (keepgoing)
        {
          //modified path tracing routine for independent pixel approximation
          mcPathTraceIPA( ws,
                     evol_op, abs_vec_mono, temperature, ext_mat_mono, rng,
                     local_rte_pos, local_rte_los, pnd_vec, g,
                     termination_flag, inside_cloud, 
                     opt_prop_gas_agenda,abs_scalar_gas_agenda, 
                     stokes_dim, f_index, p_grid, 
                     lat_grid, lon_grid, z_field, r_geoid, z_surface,
                     t_field, vmr_field, cloudbox_limits, pnd_field,
                     scat_data_mono, z_field_is_1D, ppath );
            
          np=ppath.np;
          mc_points(ppath.gp_p[np-1].idx,ppath.gp_lat[np-1].idx,
                    ppath.gp_lon[np-1].idx)+=1;
          if (termination_flag==1)
            {
              iy_space_agendaExecute(ws, local_iy,local_rte_pos,local_rte_los,
                                     iy_space_agenda);
              mult(vector1,evol_op,local_iy(0,joker));
              mult(I_i,Q,vector1);
              I_i/=g;
              keepgoing=false; //stop here. New photon.
            }
          else if (termination_flag==2)
            {
              //Choose appropriate lat and lon grid points for IPA 
              //if ppath (the reference ppath) hits the srface just take the last point.
              GridPos latgp;
              GridPos longp;
              if (ppath.background=="surface")
                {
                  latgp=ppath.gp_lat[ppath.np-1];
                  longp=ppath.gp_lon[ppath.np-1];
                }
              else
                {
                  //Use the lat and lon of the geometric tangent point
                  gridpos(latgp,lat_grid,ppath.geom_tan_pos[1]);
                  gridpos(longp,lon_grid,ppath.geom_tan_pos[2]);
                }
              //decide whether we have reflection or emission
              surface_prop_agendaExecute(ws,
                    local_surface_emission, local_surface_los, 
                    local_surface_rmatrix, local_rte_pos, local_rte_los,
                    ppath.gp_p[np-1],latgp,longp, surface_prop_agenda);
              //deal with blackbody case
              if (local_surface_los.nrows()==0)
                {
                  mult(vector1,evol_op,local_surface_emission(0,joker));
                  mult(I_i,Q,vector1);
                  I_i/=g;
                  keepgoing=false;
                }
              else
                //decide between reflection and emission
                {
                  Numeric R11=local_surface_rmatrix(0,0,0,0);
                  if (rng.draw()>R11)
                    {
                      //then we have emission
                      //Matrix oneminusR(stokes_dim,stokes_dim);
                      //id_mat(oneminusR);
                      //oneminusR-=local_surface_rmatrix(0,0,joker,joker);
                      //oneminusR/=1-R11;
                      //mult(vector1,oneminusR,local_surface_emission(0,joker));
                      mult(vector1,evol_op,local_surface_emission(0,joker));
                      mult(I_i,Q,vector1);
                      I_i/=g*(1-R11);
                      keepgoing=false;
                    }
                  else
                    {
                      //we have reflection
                      local_rte_los=local_surface_los(0,joker);
                      
                      mult(q,evol_op,local_surface_rmatrix(0,0,joker,joker));
                      mult(newQ,Q,q);
                      Q=newQ;
                      Q/=g*R11;
                    }
                }
            }
          else if (inside_cloud)
            {
              //we have another scattering/emission point 
              //Estimate single scattering albedo
              albedo=1-abs_vec_mono[0]/ext_mat_mono(0,0);
              //cout<<"albedo = "<<albedo<<" ext_mat_mono(0,0) = "<<ext_mat_mono(0,0)<<" abs_vec_mono[0] = "<<abs_vec_mono[0]<<"\n";
              //determine whether photon is emitted or scattered
              if (rng.draw()>albedo)
                {
                  //Calculate emission
                  Numeric planck_value = planck( f_grid[0], temperature );
                  Vector emission=abs_vec_mono;
                  emission*=planck_value;
                  Vector emissioncontri(stokes_dim);
                  mult(emissioncontri,evol_op,emission);
                  emissioncontri/=(g*(1-albedo));//yuck!
                  mult(I_i,Q,emissioncontri);
                  keepgoing=false;
                  //cout << "emission contri" <<  I_i[0] << "\n";
                }
              else
                {
                  //we have a scattering event
                  //Sample new line of sight.
                  
                  Sample_los (new_rte_los,g_los_csc_theta,Z,rng,local_rte_los,
                              scat_data_mono,stokes_dim,
                              pnd_vec,anyptype30,Z11maxvector,ext_mat_mono(0,0)-abs_vec_mono[0],temperature);
                                           
                  Z/=g*g_los_csc_theta*albedo;
                  
                  mult(q,evol_op,Z);
                  mult(newQ,Q,q);
                  Q=newQ;
                  //scattering_order+=1;
                  local_rte_los=new_rte_los;
                  //if (silent==0){cout <<"mc_iteration_count = "<<mc_iteration_count << 
                  //                 ", scattering_order = " <<scattering_order <<"\n";}
                }
            }
          else
            {
              //Must be clear sky emission point
              //Calculate emission
              Numeric planck_value = planck( f_grid[0], temperature );
              Vector emission=abs_vec_mono;
              emission*=planck_value;
              Vector emissioncontri(stokes_dim);
              mult(emissioncontri,evol_op,emission);
              emissioncontri/=g;
              mult(I_i,Q,emissioncontri);
              keepgoing=false;
              //cout << "emission contri" <<  I_i[0] << "\n";
            }
        }
      Isum += I_i;
      for(Index j=0; j<stokes_dim; j++)
        {
          assert(!isnan(I_i[j]));
          Isquaredsum[j] += I_i[j]*I_i[j];
        }
      y=Isum;
      y/=(Numeric)mc_iteration_count;
      for(Index j=0; j<stokes_dim; j++) 
        {
          mc_error[j]=sqrt((Isquaredsum[j]/(Numeric)mc_iteration_count-y[j]*y[j])/(Numeric)mc_iteration_count);
        }
      if (std_err>0 && mc_iteration_count>=100 && mc_error[0]<std_err_i){break;}
      if (max_time>0 && (Index)(time(NULL)-start_time)>=max_time){break;}
      if (max_iter>0 && mc_iteration_count>=max_iter){break;}
    }
  if ( convert_to_rjbt )
    {
      for(Index j=0; j<stokes_dim; j++) 
        {
          y[j]=invrayjean(y[j],f_grid[0]);
          mc_error[j]=invrayjean(mc_error[j],f_grid[0]);
        }
    }
}


/* Workspace method: Doxygen documentation will be auto-generated */
void MCSetSeedFromTime(
                       Index&    mc_seed
                       )
{
  mc_seed=(Index)time(NULL);
}


/* Workspace method: Doxygen documentation will be auto-generated */
void rte_posShift(
                  Vector&         rte_pos,
                  Vector&         rte_los,
                  GridPos&        rte_gp_p,
                  GridPos&        rte_gp_lat,
                  GridPos&        rte_gp_lon,
                  const Ppath&    ppath,
                  const Index&    atmosphere_dim)
{
  const Index np      = ppath.np;
  
  rte_pos.resize( atmosphere_dim );
  rte_pos = ppath.pos(np-1,Range(0,atmosphere_dim));
  rte_los.resize( ppath.los.ncols() );
  rte_los = ppath.los(np-1,joker);
  gridpos_copy( rte_gp_p, ppath.gp_p[np-1] );
  if( atmosphere_dim > 1 )
    { gridpos_copy( rte_gp_lat, ppath.gp_lat[np-1] ); }
  if( atmosphere_dim > 2 )
    { gridpos_copy( rte_gp_lon, ppath.gp_lon[np-1] ); }
}  
