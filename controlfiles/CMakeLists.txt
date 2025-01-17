
add_custom_command(
  OUTPUT clean-cfiles-cmd
  COMMAND rm -f "Test*.xml" "Test*.rep" "Test*.bin"
  )
add_custom_target (clean-cfiles DEPENDS clean-cfiles-cmd)

add_custom_command(
  OUTPUT mkdir-arts-results-cmd
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/results
  )
add_custom_target (mkdir-arts-results DEPENDS mkdir-arts-results-cmd)



set (ARTS_CONTROLFILE_DIR share/arts/controlfiles)

install (DIRECTORY artscomponents
                   testdata
                   general
                   instruments
         DESTINATION ${ARTS_CONTROLFILE_DIR}
         PATTERN ".svn" EXCLUDE)

file(GLOB_RECURSE ARTS_CONTROLFILES
  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/*.arts)

#######################
### ARTS Components ###
arts_test_run_ctlfile(fast artscomponents/helpers/TestForloop.arts)
arts_test_run_ctlfile(fast artscomponents/helpers/TestAgendaCopy.arts)
arts_test_run_ctlfile(fast artscomponents/helpers/TestHSE.arts)

arts_test_run_ctlfile(fast artscomponents/agendas/TestAgendaExecute.arts)
arts_test_run_ctlfile(fast artscomponents/agendas/TestArrayOfAgenda.arts)

arts_test_run_ctlfile(fast artscomponents/absorption/TestAbs.arts)
arts_test_run_ctlfile(fast
                      artscomponents/absorption/TestAbsDoppler.arts)
arts_test_run_ctlfile(slow
                      artscomponents/absorption/TestAbsParticle.arts)

arts_test_run_ctlfile(fast artscomponents/ppath/TestPpath1D.arts)
arts_test_run_ctlfile(fast artscomponents/ppath/TestPpath2D.arts)
arts_test_run_ctlfile(fast artscomponents/ppath/TestPpath3D.arts)

arts_test_run_ctlfile(fast artscomponents/pencilbeam/TestPencilBeam.arts)

arts_test_run_ctlfile(fast artscomponents/clearsky/TestClearSky.arts)
arts_test_run_ctlfile(slow artscomponents/clearsky/TestClearSky2.arts)
arts_test_run_ctlfile(fast artscomponents/clearsky/TestClearSky_StarGasScattering.arts)

arts_test_run_ctlfile(fast artscomponents/stokesrot/TestStokesRotation.arts)
arts_test_run_ctlfile(fast artscomponents/stokesrot/TestSensorPol.arts)

arts_test_run_ctlfile(fast artscomponents/regrid/TestRegrid.arts)
arts_test_run_ctlfile(fast artscomponents/regrid/TestRegridFields.arts)

arts_test_run_ctlfile(fast artscomponents/groundbased/TestFgrid.arts)
arts_test_run_ctlfile(fast artscomponents/groundbased/TestGbased.arts)
arts_test_run_ctlfile(fast artscomponents/groundbased/TestWinds.arts)

arts_test_run_ctlfile(fast artscomponents/transmission/TestTransmission.arts)
arts_test_run_ctlfile(fast artscomponents/transmission/TestTransmissionWithScat.arts)

arts_test_run_ctlfile(fast artscomponents/surfacetypes/TestSurfaceTypes.arts)

arts_test_run_ctlfile(fast artscomponents/surface/TestTessem.arts)

arts_test_run_ctlfile(fast artscomponents/faraday/TestFaradayRotation.arts)

arts_test_run_ctlfile(fast artscomponents/nlte/TestNLTE.arts)

arts_test_run_ctlfile(fast artscomponents/psd/TestFromParticleBulkProps.arts)
arts_test_run_ctlfile(fast artscomponents/psd/TestPsds.arts)

arts_test_run_ctlfile(fast artscomponents/doit/TestDOIT.arts)
arts_test_run_ctlfile(slow artscomponents/doit/TestDOITaccelerated.arts)
arts_test_run_ctlfile(fast artscomponents/doit/TestDOITprecalcInit.arts)
arts_test_ctlfile_depends(fast.artscomponents.doit.TestDOITprecalcInit
                          fast.artscomponents.doit.TestDOIT)
arts_test_run_ctlfile(fast artscomponents/doit/TestDOITsensorInsideCloudbox.arts)

arts_test_run_ctlfile(fast artscomponents/montecarlo/TestMonteCarloDataPrepare.arts)
arts_test_run_ctlfile(slow artscomponents/montecarlo/TestMonteCarloGeneral.arts)
arts_test_ctlfile_depends(slow.artscomponents.montecarlo.TestMonteCarloGeneral
                          fast.artscomponents.montecarlo.TestMonteCarloDataPrepare)
arts_test_run_ctlfile(slow artscomponents/montecarlo/TestMonteCarloGeneralGaussian.arts)
arts_test_ctlfile_depends(slow.artscomponents.montecarlo.TestMonteCarloGeneralGaussian
                          fast.artscomponents.montecarlo.TestMonteCarloDataPrepare)
arts_test_run_ctlfile(fast artscomponents/montecarlo/TestRteCalcMC.arts)
arts_test_ctlfile_depends(fast.artscomponents.montecarlo.TestRteCalcMC
                          fast.artscomponents.montecarlo.TestMonteCarloDataPrepare)

arts_test_run_ctlfile(fast artscomponents/wfuns/TestTjacStokes1.arts)
arts_test_run_ctlfile(slow artscomponents/wfuns/TestTjacStokes4_transmission.arts)
arts_test_run_ctlfile(fast artscomponents/wfuns/TestSurfaceBlackbody.arts)
arts_test_run_ctlfile(fast artscomponents/wfuns/TestSurfaceFlatScalarReflectivity.arts)

arts_test_run_ctlfile(fast artscomponents/radar/TestIyActive.arts)
arts_test_run_ctlfile(fast artscomponents/radar/TestIyActive_wfuns.arts)
arts_test_run_ctlfile(fast artscomponents/radar/TestOnionPeeling.arts)

arts_test_run_ctlfile(fast artscomponents/ycalcappend/TestYCalcAppend.arts)

if (NOT ENABLE_ARTS_LGPL)
arts_test_run_ctlfile(fast artscomponents/heatingrates/TestHeatingRates.arts)
endif()

arts_test_run_ctlfile(fast artscomponents/dobatch/TestDOBatch.arts)

arts_test_run_ctlfile(slow artscomponents/heatingrates/TestSpectralRadianceField.arts)

arts_test_run_ctlfile(fast artscomponents/xsec-fit/TestXsecFit.arts)

if (ENABLE_RT4)
  arts_test_run_ctlfile(fast artscomponents/scatsolvercomp/TestScatSolvers_fast.arts)
  arts_test_run_ctlfile(slow artscomponents/scatsolvercomp/TestScatSolvers.arts)
endif ()

if (ENABLE_TMATRIX)
  arts_test_run_ctlfile(fast artscomponents/tmatrix/TestTMatrix.arts)
endif ()

if (ENABLE_FASTEM)
  arts_test_run_ctlfile(fast artscomponents/surface/TestFastem.arts)
  arts_test_run_ctlfile(fast artscomponents/antenna/TestAntenna2D.arts)
  arts_test_run_ctlfile(fast artscomponents/iba/TestIBA.arts)
  arts_test_run_ctlfile(fast artscomponents/surface/TestSurfaceSinglePol.arts)
  arts_test_run_ctlfile(fast artscomponents/wfuns/TestTjacStokes1to4.arts)
  arts_test_run_ctlfile(slow artscomponents/wfuns/TestTjacStokes4.arts)
endif ()

if (ENABLE_NETCDF)
  arts_test_run_ctlfile(slow artscomponents/moltau/TestMolTau.arts)
endif ()

if (OEM_SUPPORT)
  arts_test_run_ctlfile(fast artscomponents/oem/TestOEM.arts)
endif ()

###################
### Instruments ###

arts_test_run_ctlfile(fast instruments/ici/TestICI_fast.arts)

arts_test_run_ctlfile(slow instruments/metmm/TestMetMM.arts)

if (ARTS_XML_DATA_DIR)

  arts_test_run_ctlfile(slow instruments/hirs/TestHIRS_fast.arts)

  arts_test_run_ctlfile(slow instruments/avhrr/TestAVHRR_fast.arts)

  arts_test_run_ctlfile(slow instruments/mviri/TestMVIRI.arts)

  arts_test_run_ctlfile(slow instruments/seviri/TestSEVIRI.arts)

  arts_test_run_ctlfile(slow artscomponents/clearsky/TestBatch.arts)
  arts_test_run_ctlfile(slow artscomponents/absorption/TestIsoRatios.arts)

  ###############################
  ### testing XML Data itself ###
  arts_test_run_ctlfile(xmldata artscomponents/arts-xml-data/TestPlanetIsoRatios.arts)

  arts_test_run_ctlfile(xmldata artscomponents/cia/TestCIA.arts)

  arts_test_run_ctlfile(xmldata artscomponents/cia/TestRTwithCIA.arts)

  arts_test_run_ctlfile(xmldata artscomponents/doit/TestDOITpressureoptimization.arts)

  ###########################################
  ### cases USING data from arts-xml-data ###
  arts_test_run_ctlfile(fast artscomponents/helpers/TestRegridding.arts)
  arts_test_run_ctlfile(fast artscomponents/helpers/TestGridsExtract.arts)

  arts_test_run_ctlfile(fast artscomponents/ppath/TestRefractPlanets.arts)
  
  arts_test_run_ctlfile(fast artscomponents/cia/TestCIADerivs.arts)

  arts_test_run_ctlfile(slow artscomponents/nlte/testRotationalConvergence.arts)
endif (ARTS_XML_DATA_DIR)

####################
### Syntax tests ###
arts_test_run_ctlfile(fast artscomponents/syntax/TestWSMCalls.arts)
