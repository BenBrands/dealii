//----------------------------  grid_in.cc  ---------------------------
//    $Id$
//    Version: $Name$ 
//
//    Copyright (C) 2002, 2003, 2004, 2005 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  grid_in.cc  ---------------------------


#include "../tests.h"
#include <dofs/dof_handler.h>
#include <grid/tria.h>
#include <grid/tria_boundary.h>
#include <grid/tria_boundary_lib.h>
#include <grid/tria_accessor.h>
#include <grid/tria_iterator.h>
#include <grid/grid_out.h>
#include <grid/grid_in.h>
#include <grid/grid_generator.h>
#include <base/logstream.h>

#include <fstream>
#include <string>

std::ofstream logfile("grid_in.output");


template <int dim>
void test1 ()
{  
  Triangulation<dim> tria;
  GridIn<dim> gi;
  gi.attach_triangulation (tria);
  std::ifstream in ("grid_in_2d.inp");
  gi.read_ucd (in);
  
  GridOut grid_out;
  grid_out.write_ucd (tria, logfile);
}


template <int dim>
void test2 ()
{
                                   // read a much larger grid (30k
                                   // cells). with the old grid
                                   // reordering scheme, this took >90
                                   // minutes (exact timing not
                                   // available, program was killed
                                   // before), with the new one it
                                   // takes less than 8 seconds
  Triangulation<dim> tria;
  GridIn<dim> gi;
  gi.attach_triangulation (tria);
  std::ifstream in ("grid_in_2d.xda");
  gi.read_xda (in);

  int hash = 0;
  int index = 0;
  for (typename Triangulation<dim>::active_cell_iterator c=tria.begin_active();
       c!=tria.end(); ++c, ++index)
    for (unsigned int i=0; i<GeometryInfo<dim>::vertices_per_cell; ++i)
      hash += (index * i * c->vertex_index(i)) % (tria.n_active_cells()+1);
  deallog << hash << std::endl;
}


template <int dim>
void test3 ()
{
  Triangulation<dim> tria;
  GridIn<dim> gi;
  gi.attach_triangulation (tria);
  gi.read ("grid_in_2d.nc");

  GridOut grid_out;
  std::ofstream gnufile("grid_in_2d.gnuplot");  
  grid_out.write_gnuplot (tria, gnufile);
}


template<int dim>
void check_file (const std::string name,
		 typename GridIn<dim>::Format format)
{
  Triangulation<dim> tria;
  GridIn<dim> gi;
  gi.attach_triangulation (tria);
  gi.read(name, format);
  deallog << name
	  << '\t' << tria.n_vertices()
	  << '\t' << tria.n_cells()
	  << std::endl;
}

void filename_resolution()
{
  check_file<2> (std::string("grid_in_2d"), GridIn<2>::ucd);
  check_file<2> (std::string("grid_in_2d"), GridIn<2>::xda);
}


int main ()
{
  logfile.precision (2);
  deallog.attach(logfile);
  deallog.depth_console(0);
  deallog.threshold_double(1.e-10);

  test1<2> ();
  test2<2> ();
				   // test3 needs NetCDF
//    test3<2> ();

  filename_resolution();
}

