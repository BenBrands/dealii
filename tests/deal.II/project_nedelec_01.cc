//----------------------------  project_nedelec_01.cc  ---------------------------
//    $Id: project_nedelec_01.cc 12732 2006-03-28 23:15:45Z wolf $
//    Version: $Name$ 
//
//    Copyright (C) 2006 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  project_nedelec_01.cc  ---------------------------


// check that VectorTools::project works for Nedelec elements correctly on
// a uniformly refined mesh for functions of degree q

char logname[] = "project_nedelec_01/output";


#include "project_common.cc"


template <int dim>
void test ()
{
  if (dim > 1)
				     // only p=1 implemented at present
    for (unsigned int p=1; p<2; ++p)
      test_no_hanging_nodes (FE_Nedelec<dim>(p), p, 1);
}
